/**
 * @file    
 * @brief   Configures and starts the RTC with LED alarms
 * @details RTC set to 1s ticks with alarms at 3sec and 5sec.
 *
 *          LED0 - Turns on when alarm 0 value is reached (3secs)
 *          LED1 - Turns on when alarm 1 value is reached (5secs)
 *          LED3 - Toggles every 0.5s (LED turns on every 1sec)
 *
 *          SW1 - Push to reset the RTC count to 0 and reset alarm interrupts
 *          SW2 - Push to set snooze for alarm 1 - alarm 1 = current timer + 10sec
 */

/* ****************************************************************************
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
 *************************************************************************** */

/* **** Includes **** */
#include <stdio.h>
#include <stdint.h>
#include "board.h"
#include "mxc_config.h"
#include "gpio.h"
#include "rtc.h"
#include "nvic_table.h"

/* **** Definitions **** */
#define LED_Alarm0		  0
#define LED_Alarm1		  1
#define LED_tick		  2

#define ALARM0_SEC        3
#define ALARM1_SEC        5
#define SNOOZE_SEC        7

/* **** Globals **** */


/* **** Functions **** */

void pb_handler_ResetRTC(void *pb_num)
{
    //turn off alarm LEDs
    LED_Off(LED_Alarm0);
    LED_Off(LED_Alarm1);

    //clear all alarm flags and enable them
    RTC_ClearFlags(MXC_F_RTC_FLAGS_COMP0 | MXC_F_RTC_FLAGS_COMP1);
    RTC_EnableINT(MXC_F_RTC_INTEN_COMP0 | MXC_F_RTC_INTEN_COMP1);

    //reset alarm 1 time to 5secs
    RTC_SetCompare(1,5);

    //reset RTC timer to 0
    RTC_SetCount(0);
    printf("RTC Reset\n");
}

void pb_handler_SetSnooze(void *pb_num)
{
    //turn off alarm 1 LED
    LED_Off(LED_Alarm1);

    //set snooze
    RTC_Snooze();

    //enable alarm 1 interrupt
    RTC_EnableINT(MXC_F_RTC_INTEN_COMP1);

    printf("RTC Snooze\n");
}

void pb_handler_GetCount(void *pb_num)
{
    int count = RTC_GetCount();
    printf("RTC count = %d,\n", count);
}

void RTC0_handler_Compare0()
{
    printf("RTC COMP0 Interrupt\n");

    //turn alarm0 LED on
    LED_On(LED_Alarm0);

    //disable interrupt
    RTC_DisableINT(MXC_F_RTC_INTEN_COMP0);

    //clear flag
    RTC_ClearFlags(MXC_F_RTC_FLAGS_COMP0);
}

void RTC1_handler_Compare1()
{
    printf("RTC COMP1 Interrupt\n");

    //turn alarm1 LED on
    LED_On(LED_Alarm1);

    //disable interrupt
    RTC_DisableINT(MXC_F_RTC_INTEN_COMP1);

    //clear flag
    RTC_ClearFlags(MXC_F_RTC_FLAGS_COMP1);
}

void RTC2_handler_PrescalerCMP()
{
    //toggle tick LED
    LED_Toggle(LED_tick);

    //clear flag
    RTC_ClearFlags(MXC_F_RTC_FLAGS_PRESCALE_COMP);
}

void RTC_Setup()
{
    rtc_cfg_t RTCconfig;

    //set RTC configuration
    RTCconfig.compareCount[0] = ALARM0_SEC; //alarm0 time in seconds
    RTCconfig.compareCount[1] = ALARM1_SEC; //alarm1 time in seconds
    RTCconfig.prescaler = RTC_PRESCALE_DIV_2_12; //1Hz clock
    RTCconfig.prescalerMask = RTC_PRESCALE_DIV_2_11;//0.5s prescaler compare
    RTCconfig.snoozeCount = SNOOZE_SEC;//snooze time in seconds
    RTCconfig.snoozeMode = RTC_SNOOZE_MODE_B;
    RTC_Init(&RTCconfig);

    //setup interrupt callbacks and enable
    NVIC_SetVector(RTC0_IRQn, RTC0_handler_Compare0);
    NVIC_SetVector(RTC1_IRQn, RTC1_handler_Compare1);
    NVIC_SetVector(RTC2_IRQn, RTC2_handler_PrescalerCMP);

    RTC_EnableINT(MXC_F_RTC_INTEN_COMP0 | MXC_F_RTC_INTEN_COMP1 | MXC_F_RTC_INTEN_PRESCALE_COMP);

    //start RTC
    RTC_Start();
}

void GPIO_Setup()
{
    //turn off all LEDs
    LED_Off(LED_Alarm0);
    LED_Off(LED_Alarm1);
    LED_Off(LED_tick);

    //push buttons to reset RTC
    PB_RegisterCallback(SW1,pb_handler_ResetRTC);

    //push button to set snooze
    PB_RegisterCallback(SW2,pb_handler_SetSnooze);

    //push button to get RTC count
    PB_RegisterCallback(SW3,pb_handler_GetCount);
}

/* ************************************************************************** */
int main(void)
{
    printf("\n********************* RTC Demo ************************\n");
    printf("LED0 = Turns on when alarm 0 value is reached (%d secs)\n", ALARM0_SEC);
    printf("LED1 = Turns on when alarm 1 value is reached (%d secs)\n", ALARM1_SEC);
    printf("LED3 = Toggles every 0.5s (LED turns on every 1sec)\n");
    printf("SW1  = Push to reset the RTC count to 0 and reset alarm interrupts\n");
    printf("SW2  = Push to set snooze for alarm 1,  alarm 1 = current time + %d sec\n", SNOOZE_SEC);

    //set LEDs off and interrupt callbacks for pushbuttons
    GPIO_Setup();

    //configure and start RTC
    RTC_Setup();

    while(1) {}
}
