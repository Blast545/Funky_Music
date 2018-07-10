/**
 * @file
 * @brief   USB HID keyboard demo with lower power on suspend
 * @details Demonstrates how to configure the USB device controller as a HID keyboard class device. If USB is suspended
 *          LP1 is entered and can wakeup on on VBUS. This demo requires a board modification of connecting
 *          D+ to a GPIO. See R69 and R70 on the schematic.
 *
 *          1. LED0 will illuminate once enumeration and configuration is complete.
 *          2. Open a text editor on the PC host and place cursor in edit box.
 *          3. Pressing pushbutton SW1 will cause a message to be typed in on a virtual keyboard one character at a time.
 *          4. To suspend USB put PC host in sleep mode. Observe lower current on VDD12.
 *          5. Wake up host PC and observe current on VDD12 is higher and the program resumes.
 *
 */

/* *****************************************************************************
 * Copyright (C) 2016 Maxim Integrated Products, Inc., All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
 * OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name of Maxim Integrated
 * Products, Inc. shall not be used except as stated in the Maxim Integrated
 * Products, Inc. Branding Policy.
 *
 * The mere transfer of this software does not imply any licenses
 * of trade secrets, proprietary technology, copyrights, patents,
 * trademarks, maskwork rights, or any other form of intellectual
 * property whatsoever. Maxim Integrated Products, Inc. retains all
 * ownership rights.
 *
 * $Date: 2018-03-05 18:40:00 -0600 (Mon, 05 Mar 2018) $
 * $Revision: 33772 $
 *
 **************************************************************************** */

#include <stdio.h>
#include <stddef.h>
#include "mxc_config.h"
#include "mxc_sys.h"
#include "pwrman_regs.h"
#include "board.h"
#include "lp.h"
#include "led.h"
#include "pb.h"
#include "usb.h"
#include "usb_event.h"
#include "enumerate.h"
#include "hid_kbd.h"
#include "descriptors.h"

/* **** Definitions **** */
#define EVENT_ENUM_COMP     MAXUSB_NUM_EVENTS
#define EVENT_REMOTE_WAKE   (EVENT_ENUM_COMP + 1)

/* **** Global Data **** */
volatile uint8_t connected;
volatile uint8_t configured;
volatile uint8_t suspended;
volatile uint8_t vbus_wakeup;
volatile unsigned int event_flags;
int remote_wake_en;

/* **** Local Scope Variable **** */
/* Workaround: This pin is connected to D+ on the board and will wake us up from LP1 */
static const gpio_cfg_t dp_wake = { PORT_3, PIN_7, GPIO_FUNC_GPIO, MXC_V_GPIO_OUT_MODE_SLOW_HIGH_Z };

/* **** Local Function Prototypes **** */
static int setconfig_callback(usb_setup_pkt *sud, void *cbdata);
static int setfeature_callback(usb_setup_pkt *sud, void *cbdata);
static int clrfeature_callback(usb_setup_pkt *sud, void *cbdata);
static int event_callback(maxusb_event_t evt, void *data);
static void usb_app_sleep(void);
static void usb_app_wakeup(void);
static void button_callback(void *pb);

/* ************************************************************************** */
int main(void)
{
    printf("\n\n***** MAX32620 USB HID Keyboard LP1 Example *****\n");
    printf("Waiting for VBUS...\n");

    /* Initialize state */
    connected = 0;
    configured = 0;
    suspended = 0;
    vbus_wakeup = 0;
    event_flags = 0;
    remote_wake_en = 0;

    /* Enable the USB clock and power */
    SYS_USB_Enable(1);

    /* Initialize the usb module */
    if (usb_init(NULL) != 0) {
        printf("usb_init() failed\n");
        while (1);
    }

    /* Initialize the enumeration module */
    if (enum_init() != 0) {
        printf("enum_init() failed\n");
        while (1);
    }

    /* Register enumeration data */
    enum_register_descriptor(ENUM_DESC_DEVICE, (uint8_t*)&device_descriptor, 0);
    enum_register_descriptor(ENUM_DESC_CONFIG, (uint8_t*)&config_descriptor, 0);
    enum_register_descriptor(ENUM_DESC_STRING, lang_id_desc, 0);
    enum_register_descriptor(ENUM_DESC_STRING, mfg_id_desc, 1);
    enum_register_descriptor(ENUM_DESC_STRING, prod_id_desc, 2);
    enum_register_descriptor(ENUM_DESC_STRING, serial_id_desc, 3);

    /* Handle configuration */
    enum_register_callback(ENUM_SETCONFIG, setconfig_callback, NULL);

    /* Handle feature set/clear */
    enum_register_callback(ENUM_SETFEATURE, setfeature_callback, NULL);
    enum_register_callback(ENUM_CLRFEATURE, clrfeature_callback, NULL);

    /* Initialize the class driver */
    if (hidkbd_init(&config_descriptor.hid_descriptor, report_descriptor) != 0) {
        printf("hidkbd_init() failed\n");
        while (1);
    }

    /* Register callbacks */
    usb_event_enable(MAXUSB_EVENT_NOVBUS, event_callback, NULL);
    usb_event_enable(MAXUSB_EVENT_VBUS, event_callback, NULL);

    /* Register callback for keyboard events */
    if (PB_RegisterCallback(0, button_callback) != E_NO_ERROR) {
        printf("PB_RegisterCallback() failed\n");
        while (1);
    }

    /* Prepare for GPIO wakeup workaround */
    if (GPIO_Config(&dp_wake) != E_NO_ERROR) {
        printf("GPIO_Config() failed\n");
        while (1);
    }
    NVIC_EnableIRQ(MXC_GPIO_GET_IRQ(dp_wake.port));
    GPIO_IntDisable(&dp_wake);
    GPIO_IntConfig(&dp_wake, GPIO_INT_FALLING_EDGE);

    /* Workaround: Set power registers for D+ GPIO wakeup */
    MXC_PWRSEQ->reg0 |= (MXC_F_PWRSEQ_REG0_PWR_VDD12_SWEN_SLP | MXC_F_PWRSEQ_REG0_PWR_TVDD12_SWEN_SLP | MXC_F_PWRSEQ_REG0_PWR_VDD18_SWEN_SLP);
    MXC_PWRSEQ->reg1 &= ~MXC_F_PWRSEQ_REG1_PWR_LP1_CORE_RSTN_EN;
    MXC_PWRSEQ->msk_flags &= ~MXC_F_PWRSEQ_MSK_FLAGS_PWR_VDDB_RST_BAD;

    LP_ClearWakeUpFlags();

    /* Configure USB wakeup */
    if (LP_ConfigUSBWakeUp(1, 1) != E_NO_ERROR) {
        printf("LP_ConfigUSBWakeUp() failed\n");
        while (1);
    }

    /* Configure GPIO pushbutton wakeup */
    if (LP_ConfigGPIOWakeUpDetect(&pb_pin[0], 0, LP_WEAK_PULL_UP) != E_NO_ERROR) {
        printf("LP_ConfigGPIOWakeUpDetect() failed\n");
        while (1);
    }

    /* Start with USB in low power mode */
    usb_app_sleep();
    NVIC_EnableIRQ(USB_IRQn);

    /* Wait for events */
    while (1) {

        if (suspended || !configured) {
            LED_Off(0);
        } else {
            LED_On(0);
        }

        if (event_flags) {
            /* Display events */
            if (MXC_GETBIT(&event_flags, MAXUSB_EVENT_NOVBUS)) {
                MXC_CLRBIT(&event_flags, MAXUSB_EVENT_NOVBUS);
                printf("VBUS Disconnect\n");
            } else if (MXC_GETBIT(&event_flags, MAXUSB_EVENT_VBUS)) {
                MXC_CLRBIT(&event_flags, MAXUSB_EVENT_VBUS);
                printf("VBUS Connect\n");
            } else if (MXC_GETBIT(&event_flags, MAXUSB_EVENT_BRST)) {
                MXC_CLRBIT(&event_flags, MAXUSB_EVENT_BRST);
                printf("Bus Reset\n");
            } else if (MXC_GETBIT(&event_flags, MAXUSB_EVENT_SUSP)) {
                MXC_CLRBIT(&event_flags, MAXUSB_EVENT_SUSP);
                printf("Suspended\n");
            } else if (MXC_GETBIT(&event_flags, MAXUSB_EVENT_DPACT)) {
                MXC_CLRBIT(&event_flags, MAXUSB_EVENT_DPACT);
                printf("Resume\n");
            } else if (MXC_GETBIT(&event_flags, EVENT_ENUM_COMP)) {
                MXC_CLRBIT(&event_flags, EVENT_ENUM_COMP);
                printf("Enumeration complete. Press SW1 to send character.\n");
            } else if (MXC_GETBIT(&event_flags, EVENT_REMOTE_WAKE)) {
                MXC_CLRBIT(&event_flags, EVENT_REMOTE_WAKE);
                printf("Remote Wakeup\n");
            }
        } else {
            if (suspended && !vbus_wakeup) {

                while (Console_PrepForSleep() != E_NO_ERROR);
                LED_Off(0);

                /* Prepare for GPIO wakeup */
                LP_ClearWakeUpFlags();

                __disable_irq();
                LP_EnterLP1();

                /* The VBUS interrupts are delayed from the wakeup. Don't go back into LP1 until it is serviced */
                if (LP_GetWakeUpFlags() & (MXC_F_PWRSEQ_MSK_FLAGS_PWR_USB_PLUG_WAKEUP | MXC_F_PWRSEQ_MSK_FLAGS_PWR_USB_REMOVE_WAKEUP)) {
                    vbus_wakeup = 1;
                }
                __enable_irq();
            } else {
                LP_EnterLP2();
            }
        }
    }
}

/* ************************************************************************** */
static int setconfig_callback(usb_setup_pkt *sud, void *cbdata)
{
    /* Confirm the configuration value */
    if (sud->wValue == config_descriptor.config_descriptor.bConfigurationValue) {
        configured = 1;
        MXC_SETBIT(&event_flags, EVENT_ENUM_COMP);
        return hidkbd_configure(config_descriptor.endpoint_descriptor.bEndpointAddress & USB_EP_NUM_MASK);
    } else if (sud->wValue == 0) {
        configured = 0;
        return hidkbd_deconfigure();
    }

    return -1;
}

/* ************************************************************************** */
static void usb_app_sleep(void)
{
    usb_sleep();
    MXC_PWRMAN->pwr_rst_ctrl &= ~MXC_F_PWRMAN_PWR_RST_CTRL_USB_POWERED;
    if (connected) {
        usb_event_clear(MAXUSB_EVENT_DPACT);
        usb_event_enable(MAXUSB_EVENT_DPACT, event_callback, NULL);
        LP_ConfigGPIOWakeUpDetect(&dp_wake, 0, LP_NO_PULL);
        GPIO_IntClr(&dp_wake);
        GPIO_IntEnable(&dp_wake);
    } else {
        GPIO_IntDisable(&dp_wake);
        LP_ClearGPIOWakeUpDetect(&dp_wake);
        usb_event_disable(MAXUSB_EVENT_DPACT);
    }
    suspended = 1;
}

/* ************************************************************************** */
static void usb_app_wakeup(void)
{
    GPIO_IntDisable(&dp_wake);
    LP_ClearGPIOWakeUpDetect(&dp_wake);
    usb_event_disable(MAXUSB_EVENT_DPACT);
    MXC_PWRMAN->pwr_rst_ctrl |= MXC_F_PWRMAN_PWR_RST_CTRL_USB_POWERED;
    usb_wakeup();
    suspended = 0;
}

/* ************************************************************************** */
static int event_callback(maxusb_event_t evt, void *data)
{
    /* Set event flag */
    MXC_SETBIT(&event_flags, evt);

    switch (evt) {
        case MAXUSB_EVENT_NOVBUS:
            usb_event_disable(MAXUSB_EVENT_BRST);
            usb_event_disable(MAXUSB_EVENT_SUSP);
            usb_event_disable(MAXUSB_EVENT_DPACT);
            usb_disconnect();
            connected = 0;
            configured = 0;
            vbus_wakeup = 0;
            enum_clearconfig();
            hidkbd_deconfigure();
            usb_app_sleep();
            break;
        case MAXUSB_EVENT_VBUS:
            usb_event_clear(MAXUSB_EVENT_BRST);
            usb_event_enable(MAXUSB_EVENT_BRST, event_callback, NULL);
            usb_event_clear(MAXUSB_EVENT_SUSP);
            usb_event_enable(MAXUSB_EVENT_SUSP, event_callback, NULL);
            usb_connect();
            connected = 1;
            vbus_wakeup = 0;
            usb_app_sleep();
            break;
        case MAXUSB_EVENT_BRST:
            usb_app_wakeup();
            enum_clearconfig();
            hidkbd_deconfigure();
            configured = 0;
            suspended = 0;
            break;
        case MAXUSB_EVENT_SUSP:
            usb_app_sleep();
            break;
        case MAXUSB_EVENT_DPACT:
            usb_app_wakeup();
            break;
        default:
            break;
    }

    return 0;
}

/* ************************************************************************** */
void button_callback(void *pb)
{
    static const uint8_t chars[] = "Maxim Integrated\n";
    static int i = 0;
    int count = 0;
    int button_pressed = 0;

    //determine if interrupt triggered by bounce or a true button press
    while (PB_Get(0) && !button_pressed)
    {
        count++;

        if(count > 1000)
            button_pressed = 1;
    }

    if(button_pressed)
    {
        LED_Toggle(1);
        if (configured) {
            if (suspended && remote_wake_en) {
                /* The bus is suspended. Wake up the host */
                usb_app_wakeup();
                usb_remote_wakeup();
                suspended = 0;
                MXC_SETBIT(&event_flags, EVENT_REMOTE_WAKE);
            } else {
                if (i >= (sizeof(chars) - 1)) {
                    i = 0;
                }
                hidkbd_keypress(chars[i++]);
            }
        }
    }
}

/* ************************************************************************** */
static int setfeature_callback(usb_setup_pkt *sud, void *cbdata)
{
    if(sud->wValue == FEAT_REMOTE_WAKE) {
        remote_wake_en = 1;
    } else {
        // Unknown callback
        return -1;
    }

    return 0;
}

/* ************************************************************************** */
static int clrfeature_callback(usb_setup_pkt *sud, void *cbdata)
{
    if(sud->wValue == FEAT_REMOTE_WAKE) {
        remote_wake_en = 0;
    } else {
        // Unknown callback
        return -1;
    }

    return 0;
}

/* ************************************************************************** */
void USB_IRQHandler(void)
{
    usb_event_handler();
}
