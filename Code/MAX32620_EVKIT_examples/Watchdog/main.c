/**
 * @file    
 * @brief   Demonstrates a watchdog timer in run mode
 *
 * @details 
 *          1. When the program starts LED3 blinks three times and stops.
 *          2. LED0 starts blinking continuously.
 *          3. Open a terminal program to see interrupt messages (115200 Baud, 8N1).
 *
 *          SW1: Push SW1 to trigger a watchdog reset. This will reset the watchdog before
 *               the wait period has expired and trigger an interrupt.
 *
 *          SW2: Push SW2 to trigger a delay and see LED0 stop blinking momentarily.
 *               This delay is long enough for the timeout period to expire and trigger a watchdog interrupt.
 *
 *          SW3: Push SW3 to trigger a longer delay and see the program restart by blinking LED3 three times.
 *               This delay is long enough for the reset period to expire and trigger a watchdog reset.
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
 * $Date: 2017-05-02 13:57:41 -0500 (Tue, 02 May 2017) $
 * $Revision: 27735 $
 *
 **************************************************************************** */

/* **** Includes **** */
#include <stdio.h>
#include <stdint.h>
#include "mxc_config.h"
#include "nvic_table.h"
#include "board.h"
#include "mxc_sys.h"
#include "wdt.h"

/* **** Definitions **** */

/* **** Global Variables **** */

// use push buttons defined in board.h
extern const gpio_cfg_t pb_pin[];

/* **** Functions **** */

/* ************************************************************************** */
void delay(uint32_t ms)
{
    int i;

    for(i = 0; i < ms; i++)
        SYS_SysTick_Delay(94600);
}

/* ************************************************************************** */
void watchdog_timeout_handler()
{
    //get and clear flag
    int flags = WDT_GetFlags(MXC_WDT0);
    WDT_ClearFlags(MXC_WDT0, MXC_F_WDT_FLAGS_TIMEOUT);

    printf("TIMEOUT! ");
    printf("FLAGS = 0x%08x\n", flags);
}

/* ************************************************************************** */
void watchdog_wait_handler()
{
    //get and clear flag
    int flags = WDT_GetFlags(MXC_WDT0);
    WDT_ClearFlags(MXC_WDT0, MXC_F_WDT_FLAGS_PRE_WIN);

    printf("WDT reset too soon! ");
    printf("FLAGS = 0x%08x\n", flags);

}

/* ************************************************************************** */
void WDT_Setup()
{
    //setup interrupts
    NVIC_SetVector(WDT0_IRQn, watchdog_timeout_handler);
    NVIC_SetVector(WDT0_P_IRQn, watchdog_wait_handler);

    //select clock source and scale
    sys_cfg_wdt_t wdtClk;
    wdtClk.clk = CLKMAN_WDT_SELECT_SCALED_SYS_CLK_CTRL; //select system clock as wdt clk source
    wdtClk.clk_scale = CLKMAN_SCALE_DIV_1;              //WDT clock = system clock = 96MHz
    WDT_Init(MXC_WDT0, &wdtClk, MXC_V_WDT_UNLOCK_KEY);

    //enable and set wait, timeout and reset periods
    WDT_EnableInt(MXC_WDT0, WDT_PERIOD_2_26_CLKS, MXC_V_WDT_UNLOCK_KEY);   //timeout = 2^26/96MHz = 700ms
    WDT_EnableWait(MXC_WDT0, WDT_PERIOD_2_24_CLKS, MXC_V_WDT_UNLOCK_KEY);  // wait time = 2^24/96MHz = 175ms
    WDT_EnableReset(MXC_WDT0, WDT_PERIOD_2_27_CLKS, MXC_V_WDT_UNLOCK_KEY); // reset time = 2^27/96MHz = 1.4s

    //start watchdog timer
    WDT_Start(MXC_WDT0, MXC_V_WDT_UNLOCK_KEY);
}

/* ************************************************************************** */
int main(void)
{
    printf("\n************** Watchdog Timer Demo ****************\n");
    printf("Press a button to create watchdog interrupt or reset:\n");
    printf("SW1 = wait interrupt\n");
    printf("SW2 = timeout interrupt\n");
    printf("SW3 = reset program\n");

    LED_Off(0);

    //blink LED3 three times at startup
    int numBlinks = 3;

    while(numBlinks) {
        LED_On(3);
        delay(100);
        LED_Off(3);
        delay(100);

        numBlinks--;
    }

    //setup watchdog
    WDT_Setup();

    while(1) {
        //Push SW1 to reset watchdog - used to trigger wait interrupt
        if(GPIO_InGet(&pb_pin[SW1]) == 0) {
            printf("Reset watchdog early\n");
            WDT_Reset(MXC_WDT0);
        }

        //Push SW2 to start delay - used to trigger timeout interrupt
        if(GPIO_InGet(&pb_pin[SW2]) == 0) {
            printf("delaying...\n");
            delay(800);
        }

        //Push SW3 to start longer delay - used to trigger reset
        if(GPIO_InGet(&pb_pin[SW3]) == 0) {
            printf("delaying...\n");
            delay(1500);
        }

        //blink LED0
        delay(100);
        LED_On(0);
        delay(100);
        LED_Off(0);

        //Reset watchdog
        WDT_Reset(MXC_WDT0);
    }
}
