/**
 * @file
 * @brief       Generic USB HID example that can receive/send 64 bytes of data
 * @details     Open a COM terminal window for debug messaging.
 *              1. Wait for the message "Enumeration complete. Ready to receive or Press SW1 to send 64 bytes..."
 *              2. Open the Host GUI, USB HID Terminal.
 *              3. If not already connected to device, enter the Product ID: 0x0480 and Vendor ID: 0x0B6A and click Connect.
 *              4. Once connected, enter the hex data to send from host to MAX32620 and click Send.
 *              5. See the data received by MAX32620 in the COM terminal.
 *              6. On the HID terminal click Wait for Data and then push SW1 on the EVK within 7 seconds to send data from MAX32620 to Host.
 *
 * @note Download and install the example Host GUI, "USB HID Terminal" from [Maxim Integrated](http://www.maximintegrated.com/evkitsoftware)
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

/* **** Include Files **** */
#include <stdio.h>
#include <stddef.h>
#include <string.h>
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
#include "hid_raw.h"
#include "descriptors.h"

/* **** Definitions **** */
#define EVENT_ENUM_COMP     MAXUSB_NUM_EVENTS
#define EVENT_REMOTE_WAKE   (EVENT_ENUM_COMP + 1)

/* **** Global Data **** */
volatile int configured;
volatile int suspended;
volatile unsigned int event_flags;
int remote_wake_en;

/* **** Local Variables **** */
/* This EP assignment must match the Configuration Descriptor */
static const hid_cfg_t hid_cfg = {
  1,                  /* EP IN */
  MXC_USB_MAX_PACKET, /* OUT max packet size */
  2,                  /* EP OUT */
  MXC_USB_MAX_PACKET, /* IN max packet size */
};

static volatile int usb_read_complete;

/* **** Local Function Prototypes **** */
static int setconfig_callback(usb_setup_pkt *sud, void *cbdata);
static int setfeature_callback(usb_setup_pkt *sud, void *cbdata);
static int clrfeature_callback(usb_setup_pkt *sud, void *cbdata);
static int event_callback(maxusb_event_t evt, void *data);
void usb_app_task(void);
static void usb_app_sleep(void);
static void usb_app_wakeup(void);
static void button_callback(void *pb);
static int data_available(void);

/* ************************************************************************** */
int main(void)
{
    printf("\n\n***** MAX32620 USB HID Raw Example *****\n");
    printf("Waiting for VBUS...\n");

    /* Initialize state */
    configured = 0;
    suspended = 0;
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
    if (hidraw_init(&config_descriptor.hid_descriptor, report_descriptor) != 0) {
        printf("hidraw_init() failed\n");
        while (1);
    }

    /* Register callbacks */
    usb_event_enable(MAXUSB_EVENT_NOVBUS, event_callback, NULL);
    usb_event_enable(MAXUSB_EVENT_VBUS, event_callback, NULL);
    hidraw_register_callback(data_available);

    /* Register callback for keyboard events */
    if (PB_RegisterCallback(0, button_callback) != E_NO_ERROR) {
        printf("PB_RegisterCallback() failed\n");
        while (1);
    }

    /* Start with USB in low power mode */
    usb_app_sleep();
    NVIC_EnableIRQ(USB_IRQn);

    /* Wait for events */
    while (1) {

        usb_app_task();

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
                printf("Enumeration complete. Ready to receive or Press SW1 to send %d bytes...\n", MXC_USB_MAX_PACKET);
            } else if (MXC_GETBIT(&event_flags, EVENT_REMOTE_WAKE)) {
                MXC_CLRBIT(&event_flags, EVENT_REMOTE_WAKE);
                printf("Remote Wakeup\n");
            }
        } else {
            LP_EnterLP2();
        }
    }
}

/* ************************************************************************** */
void usb_app_task(void)
{
  int bytes, i;
  uint8_t data_from_host[128];

  while ((bytes = hidraw_canread()) > 0) {

    if (bytes > sizeof(data_from_host)) {
      bytes = sizeof(data_from_host);
    }

    if (hidraw_read(data_from_host, bytes) != bytes) {
      printf("hidraw_read() failed\n");
      break;
    }

    printf("Received %u bytes from host:\n", bytes);
    for (i = 0; i < bytes; i++) {
      printf(" %02x", data_from_host[i]);
    }
    printf("\n");
  }
}

/* ************************************************************************** */
static int setconfig_callback(usb_setup_pkt *sud, void *cbdata)
{
    /* Confirm the configuration value */
    if (sud->wValue == config_descriptor.config_descriptor.bConfigurationValue) {
        configured = 1;
        MXC_SETBIT(&event_flags, EVENT_ENUM_COMP);
        return hidraw_configure(&hid_cfg);
    } else if (sud->wValue == 0) {
        configured = 0;
        return hidraw_deconfigure();
    }

    return -1;
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
static void usb_app_sleep(void)
{
    usb_sleep();
    MXC_PWRMAN->pwr_rst_ctrl &= ~MXC_F_PWRMAN_PWR_RST_CTRL_USB_POWERED;
    if (MXC_USB->dev_cn & MXC_F_USB_DEV_CN_CONNECT) {
        usb_event_clear(MAXUSB_EVENT_DPACT);
        usb_event_enable(MAXUSB_EVENT_DPACT, event_callback, NULL);
    } else {
        usb_event_disable(MAXUSB_EVENT_DPACT);
    }
    suspended = 1;
}

/* ************************************************************************** */
static void usb_app_wakeup(void)
{
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
            configured = 0;
            enum_clearconfig();
            hidraw_deconfigure();
            usb_app_sleep();
            break;
        case MAXUSB_EVENT_VBUS:
            usb_event_clear(MAXUSB_EVENT_BRST);
            usb_event_enable(MAXUSB_EVENT_BRST, event_callback, NULL);
            usb_event_clear(MAXUSB_EVENT_SUSP);
            usb_event_enable(MAXUSB_EVENT_SUSP, event_callback, NULL);
            usb_connect();
            usb_app_sleep();
            break;
        case MAXUSB_EVENT_BRST:
            usb_app_wakeup();
            enum_clearconfig();
            hidraw_deconfigure();
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
static int data_available(void)
{
  usb_read_complete = 1;
  return 0;
}

/* ************************************************************************** */
static void button_callback(void *pb)
{
    static uint8_t data[MXC_USB_MAX_PACKET];
    memset(data, 0x0, MXC_USB_MAX_PACKET);
    int count = 0;
    int button_pressed = 0;
    int i;

    //determine if interrupt triggered by bounce or a true button press
    while (PB_Get(0) && !button_pressed)
    {
        count++;

        if(count > 1000)
            button_pressed = 1;
    }

    if(button_pressed) {
        for(i = 0; i < MXC_USB_MAX_PACKET; i++)
            data[i] = i+1;

        if (configured) {
            LED_Toggle(1);
            if (suspended && remote_wake_en) {
                /* The bus is suspended. Wake up the host */
                usb_app_wakeup();
                usb_remote_wakeup();
                suspended = 0;
                MXC_SETBIT(&event_flags, EVENT_REMOTE_WAKE);
            } else {
                printf("Sending data to host...\n");
                hidraw_write(data, sizeof(data));
            }
        }
    }
}

/* ************************************************************************** */
void USB_IRQHandler(void)
{
    usb_event_handler();
}
