/*******************************************************************************
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
 * $Date: 2016-08-15 17:36:02 -0500 (Mon, 15 Aug 2016) $
 * $Revision: 24080 $
 *
 ******************************************************************************/

#include <stdio.h>
#include "mxc_config.h"
#include "mxc_assert.h"
#include "board.h"
#include "gpio.h"
#include "uart.h"
#include "spim.h"
#include "max14690.h"

/***** Global Variables *****/

// LEDs
// Note: EvKit board uses 3.3v supply so these must be open-drain.
const gpio_cfg_t led_pin[] = {
    { PORT_3, PIN_0, GPIO_FUNC_GPIO, GPIO_PAD_OPEN_DRAIN },
    { PORT_3, PIN_1, GPIO_FUNC_GPIO, GPIO_PAD_OPEN_DRAIN },
    { PORT_3, PIN_2, GPIO_FUNC_GPIO, GPIO_PAD_OPEN_DRAIN },
    { PORT_3, PIN_3, GPIO_FUNC_GPIO, GPIO_PAD_OPEN_DRAIN },
};
const unsigned int num_leds = (sizeof(led_pin) / sizeof(gpio_cfg_t));

// Pushbuttons
const gpio_cfg_t pb_pin[] = {
    { PORT_5, PIN_4, GPIO_FUNC_GPIO, GPIO_PAD_INPUT_PULLUP },
    { PORT_5, PIN_5, GPIO_FUNC_GPIO, GPIO_PAD_INPUT_PULLUP },
    { PORT_6, PIN_0, GPIO_FUNC_GPIO, GPIO_PAD_INPUT_PULLUP },
};
const unsigned int num_pbs = (sizeof(pb_pin) / sizeof(gpio_cfg_t));

// Console UART configuration
const uart_cfg_t console_uart_cfg = {
    .parity = UART_PARITY_DISABLE,
    .size = UART_DATA_SIZE_8_BITS,
    .extra_stop = 0,
    .cts = 0,
    .rts = 0,
    .baud = CONSOLE_BAUD,
};
const sys_cfg_uart_t console_sys_cfg = {
    .clk_scale = CLKMAN_SCALE_DIV_4,
    .io_cfg = IOMAN_UART(CONSOLE_UART, IOMAN_MAP_A, IOMAN_MAP_UNUSED, IOMAN_MAP_UNUSED, 1, 0, 0)
};
const gpio_cfg_t console_uart_rx = { PORT_0, PIN_0, GPIO_FUNC_GPIO, GPIO_PAD_INPUT };
const gpio_cfg_t console_uart_tx = { PORT_0, PIN_1, GPIO_FUNC_GPIO, GPIO_PAD_INPUT };
const gpio_cfg_t console_uart_cts = { PORT_0, PIN_2, GPIO_FUNC_GPIO, GPIO_PAD_INPUT };
const gpio_cfg_t console_uart_rts = { PORT_0, PIN_3, GPIO_FUNC_GPIO, GPIO_PAD_INPUT };


// EM9301 BTLE Radio
const gpio_cfg_t hci_rst = { PORT_4, PIN_2, GPIO_FUNC_GPIO, GPIO_PAD_NORMAL };
const gpio_cfg_t hci_irq = { PORT_5, PIN_6, GPIO_FUNC_GPIO, GPIO_PAD_INPUT_PULLUP };
const gpio_cfg_t hci_spi = { PORT_5, (PIN_0 | PIN_1 | PIN_2 | PIN_3), GPIO_FUNC_GPIO, GPIO_PAD_NORMAL };

// MAX14690 PMIC
const max14690_cfg_t max14690_cfg = {
  .ldo2mv = 3300, /* 3.3v in mV, connected to VDDB */
  .ldo2mode = MAX14690_LDO_MPC1, /* Enalbe LDO2 when +5v is present on VBUS */
  .ldo3mv = 3300,  /* 3.3v is L3OUT -- optional */
  .ldo3mode = MAX14690_LDO_ENABLED
};
const sys_cfg_i2cm_t max14690_sys_cfg = {
        .clk_scale = CLKMAN_SCALE_DIV_1,
        .io_cfg = IOMAN_I2CM0(1, 0)
};
const gpio_cfg_t max14690_int   = { PORT_4, PIN_4, GPIO_FUNC_GPIO, GPIO_PAD_INPUT_PULLUP };
const gpio_cfg_t max14690_mpc0  = { PORT_4, PIN_5, GPIO_FUNC_GPIO, GPIO_PAD_NORMAL };
const gpio_cfg_t max14690_pfn2  = { PORT_4, PIN_6, GPIO_FUNC_GPIO, GPIO_PAD_NORMAL };

// NHD12832
const sys_cfg_spim_t nhd12832_sys_cfg = {
        .clk_scale = CLKMAN_SCALE_AUTO,
        .io_cfg = IOMAN_SPIM2(IOMAN_MAP_A, 1, 1, 0, 0, 0, 0, 0, 0)
};
const gpio_cfg_t nhd12832_spi = { PORT_2, (PIN_4 | PIN_5 | PIN_7), GPIO_FUNC_GPIO, GPIO_PAD_NORMAL };
const gpio_cfg_t nhd12832_res = { PORT_4, PIN_7, GPIO_FUNC_GPIO, GPIO_PAD_NORMAL };
const gpio_cfg_t nhd12832_dc  = { PORT_5, PIN_7, GPIO_FUNC_GPIO, GPIO_PAD_NORMAL };
const spim_cfg_t nhd12832_spim_cfg = {0, SPIM_SSEL0_LOW, 1000000};

/***** File Scope Variables *****/

/******************************************************************************/
void mxc_assert(const char *expr, const char *file, int line)
{
    printf("MXC_ASSERT %s #%d: (%s)\n", file, line, expr);
    while (1);
}

/******************************************************************************/
int Board_Init(void)
{
    int err;

    if ((err = Console_Init()) != E_NO_ERROR) {
        MXC_ASSERT_FAIL();
        return err;
    }

    if ((err = LED_Init()) != E_NO_ERROR) {
        MXC_ASSERT_FAIL();
        return err;
    }

    if ((err = PB_Init()) != E_NO_ERROR) {
        MXC_ASSERT_FAIL();
        return err;
    }

    /* Configure PMIC voltages */
    if ((err = MAX14690_Init(&max14690_cfg)) != E_NO_ERROR) {
        MXC_ASSERT_FAIL();
        return err;
    }
    /* Configure PMIC LDO output on VBUS detection */
    if ((err = MAX14690_InterruptInit()) != E_NO_ERROR) {
      MXC_ASSERT_FAIL();
      return err;
    }

    return E_NO_ERROR;
}

/******************************************************************************/
int Console_Init(void)
{
    int err;

    if ((err = UART_Init(MXC_UART_GET_UART(CONSOLE_UART), &console_uart_cfg, &console_sys_cfg)) != E_NO_ERROR) {
        MXC_ASSERT_FAIL();
        return err;
    }

    return E_NO_ERROR;
}

/******************************************************************************/
int Console_PrepForSleep(void)
{
#if defined ( __GNUC__ )
    fflush(stdout);
#endif /* __GNUC__ */
    return UART_PrepForSleep(MXC_UART_GET_UART(CONSOLE_UART));
}

/******************************************************************************/
void Board_nhd12832_Init(void)
{
    /* This display is a 3.3v interface use VDDIOH*/
    SYS_IOMAN_UseVDDIOH(&nhd12832_spi);
    SYS_IOMAN_UseVDDIOH(&nhd12832_res);
    SYS_IOMAN_UseVDDIOH(&nhd12832_dc);
}
