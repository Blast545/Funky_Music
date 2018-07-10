/**
 * @file	
 * @brief   Demonstrates using the nanoring watchdog timer (WDT2) to wake-up from LP1 and/or reset the program
 *
 * @details When the program starts LED3 is turned on (LP3 mode) and LED0 start blinking continuously.
 *          Open a terminal program to see interrupt messages.
 *
 *          SW1: Push SW1 to enter LP1 and wake-up based on watchdog timer's wake-up period (2 seconds).
 *               While in LP1 LED0 stops blinking and LED1 turns on. After about 2 seconds the watchdog
 *               wake-up period expires and wakes up to LP3 (LED3 on, LED0 continuous blinking).
 *
 *          SW2: Push SW2 to enter LP1 and reset program based on watchdog timer's reset period (4 seconds).
 *               While in LP1 LED0 stops blinking and LED1 turns on.
 *               After about 4 seconds the watchdog reset period expires and resets the program. (LED3 blinks three times).
 *
 *          SW3: Push SW3 to trigger a long delay and see the program restart by blinking LED3 three times.
 *               During the delay LED0 will stop blinking. This delay is long enough for the reset period to
 *               expire and trigger a reset (4 seconds).
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

/* **** Includes ****   */
#include <stdio.h>
#include <stdint.h>
#include "mxc_config.h"
#include "board.h"
#include "wdt2.h"
#include "lp.h"
#include "mxc_sys.h"

/* **** Definitions **** */

/* **** Globals **** */

/* **** Functions **** */

/* ************************************************************************** */
void delay(uint32_t ms)
{
    int i;

    for(i = 0; i < ms; i++)
        SYS_SysTick_Delay(94600);
}

/* ************************************************************************** */
void WDT2_Setup()
{
    int enableInSleep = 1;
    //initialize watchdog clock for run and sleep mode
    WDT2_Init(enableInSleep, MXC_V_WDT2_UNLOCK_KEY); //WDT2 uses 8KHz NanoRing clock

    //setup wake-up and reset times
    WDT2_EnableWakeUp(WDT2_PERIOD_2_14_CLKS, MXC_V_WDT2_UNLOCK_KEY); // 2^14/8KHz = 2.048s
    WDT2_EnableReset(WDT2_PERIOD_2_15_CLKS, MXC_V_WDT2_UNLOCK_KEY);  // 2^15/8KHz = 4.096s

    //start watchdog timer
    WDT2_Start(MXC_V_WDT2_UNLOCK_KEY);
}

/* ************************************************************************** */
void WakupFromLP1()
{
    //Clear existing wake-up config
    LP_ClearWakeUpConfig();

    //Clear any event flags
    LP_ClearWakeUpFlags();

    //Let console finish before going to sleep
    printf("Enter LP1.\n");
    while (Console_PrepForSleep() != E_NO_ERROR);

    //turn on LED1
    LED_On(1);
    LED_Off(3);

    //reset watchdog before sleep
    WDT2_Reset();

    //global disable interrupt
    __disable_irq();

    //Enter sleep
    LP_EnterLP1();

    //global enable interrupt
    __enable_irq();

    //Wake from sleep and turn on LED3
    LED_Off(1);
    LED_On(3);

    //Clear all wake-up flags
    printf("Woke-up from LP1\n");
    LP_ClearWakeUpFlags();
}

/* ************************************************************************** */
int WDTResetFlag()
{
    int reset = 0;
    uint32_t flags = WDT2_GetFlags();

    //check for watchdog reset
    if(flags & MXC_F_WDT2_FLAGS_RESET_OUT) {
        printf("Program reset\n");
        reset = 1;

        WDT2_ClearFlags(MXC_F_WDT2_FLAGS_RESET_OUT);

        //blink LED3 three times
        int numBlinks = 3;

        while(numBlinks) {
            LED_On(3);
            delay(100);
            LED_Off(3);
            delay(100);

            numBlinks--;
        }
    }

    return reset;
}

/******************************************************************************/
int main(void)
{
    //check if starting at main because of watchdog reset
    if(WDTResetFlag() == 0) {
        printf("\n**********Lower Power with Watchdog Demo***********\n");
        printf("Press a button to start action:\n");
        printf("SW1 = Enter LP1 and wake-up based on watchdog timer\n");
        printf("SW2 = Enter LP1 and reset based on watchdog timer\n");
        printf("SW3 = In run mode (LP3), delay long enough to trigger watchdog reset timeout\n");
    }

    //set LED3 on
    LED_On(3);

    //Initialize and setup watchdog wake-up and reset times
    WDT2_Setup();

    while(1) {
        //Push SW1 to enter LP1 and wake-up based on watchdog timer
        if(PB_Get(SW1)) {
            WakupFromLP1();
        }

        //Push SW2 to enter LP1 and reset based on watchdog timer
        if(PB_Get(SW2)) {
            //disable wake-up time for watchdog
            WDT2_DisableWakeUp(MXC_V_WDT2_UNLOCK_KEY);
            WakupFromLP1();
        }

        //Push SW3 to delay long enough to reset program based on watchdog timer
        if(PB_Get(SW3)) {
            printf("delaying...\n");
            delay(4100);
        }

        //blink LED0
        delay(100);
        LED_On(0);
        delay(100);
        LED_Off(0);

        //Reset watchdog
        WDT2_Reset();
    }
}
