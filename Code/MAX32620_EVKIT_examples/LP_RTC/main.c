/**
 * @file    
 * @brief   Shows an example of how to sleep from LP3 (run mode) and wake-up
 *          from both LP1 and LP0 by using the RTC compare wake-up triggers
 *
 * @details Operation 
 *          1. On power up, device is in LP3 (LED2 and LED3 on)
 *          2. Push SW2(P55) push button and the device will go to LP1 sleep 
 *             mode (only LED1 on)
 *          3. After 7 seconds the device will wake up (only LED2 on)
 *          4. Push SW3(P60) push button and the device will go to LP1 sleep 
 *             mode (only LED1 on)
 *          5. The device will wake up every second and toggle LED1 then go back
 *             to sleep
 *          6. After 7 seconds the device will stay awake (only LED2 on)
 *          4. Push SW1(P54) push button and the device will go to LP0 sleep 
 *             mode (only LED0 on)
 *          5. After 5 seconds the device will wake up and restarts (three 
 *             blinks on LED3 then goes to step 1)
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
 **************************************************************************** */

/* **** Includes **** */
#include <stdio.h>
#include <stdint.h>
#include "mxc_config.h"
#include "board.h"
#include "lp.h"
#include "rtc.h"

/* **** Globals **** */
#define LP0_WakeTime    5 //seconds
#define LP1_WakeTime    7 //seconds

/* **** Functions **** */

/* ************************************************************************** */
void pause(void)
{
    unsigned int i;

    for (i = 0; i < 5000000; i++) {
        __NOP();
    }
}

/* ************************************************************************** */
void Wakeup_LP0()
{
    printf("Woke-up from LP0 - program reset\n");

    int numBlinks = 3;

    while(numBlinks) {
        LED_On(3);
        pause();
        LED_Off(3);
        pause();

        numBlinks--;
    }

}

/* ************************************************************************** */
void Wakeup_LP1()
{
    printf("Woke-up from LP1\n");

    //turn on LED2 and the rest off
    LED_Off(0);
    LED_Off(1);
    LED_On(2);
    LED_Off(3);
}

/* ************************************************************************** */
void RTC_Setup()
{
    rtc_cfg_t RTCconfig;

    RTCconfig.compareCount[0] = 5;//5 second timer
    RTCconfig.compareCount[1] = 7;//7 second timer
    RTCconfig.prescaler = RTC_PRESCALE_DIV_2_12; //1Hz clock
    RTCconfig.prescalerMask = RTC_PRESCALE_DIV_2_12;//used for prescaler compare
    RTCconfig.snoozeCount = 0;
    RTCconfig.snoozeMode = RTC_SNOOZE_DISABLE;

    RTC_Init(&RTCconfig);

    RTC_Start();
}

/* *************************************************************************** */
int main(void)
{
    //check if starting at main because of LP0 wake-up
    if(LP_IsLP0WakeUp())
        Wakeup_LP0();
    else {
        printf("\n**********RTC Power Demo***********\n");
        printf("Press a button to enter a sleep state:\n");
        printf("SW1 = Enter LP0 with RTC COMP0 wake-up\n");
        printf("SW2 = Enter LP1 with RTC COMP1 wake-up\n");
        printf("SW3 = Enter LP1 with RTC prescaler compare and COMP1 wake-up\n");
    }

    //initialize LEDs
    //set LED2 and LED3 on and the rest off
    LED_Off(0);
    LED_Off(1);
    LED_On(2);
    LED_On(3);

    //configure RTC and start
    RTC_Setup();

    while(1) {
        // Enter LP1 when SW2 is pushed
        if(PB_Get(SW2))
        {
            //set LED1 on and the rest off
            LED_Off(0);
            LED_On(1);
            LED_Off(2);
            LED_Off(3);

            //Clear existing wake-up config
            LP_ClearWakeUpConfig();

            //Clear any event flags
            LP_ClearWakeUpFlags();

            //configure wake-up on RTC compare 1
            LP_ConfigRTCWakeUp(0, 1, 0, 0);

            printf("Enter LP1\n");
            while (Console_PrepForSleep() != E_NO_ERROR);

            //set RTC compare 1 value
            uint32_t cmp = RTC_GetCount() + LP1_WakeTime;
            RTC_SetCompare(1,cmp);
            RTC_ClearFlags(MXC_F_RTC_FLAGS_COMP1);

            //global disable interrupt
            __disable_irq();

            LP_EnterLP1();

            //global enable interrupt
            __enable_irq();

            Wakeup_LP1();
        }

        // Enter LP0 when SW1 is pushed
        if(PB_Get(SW1))
        {
            //turn on LED0 and the rest are off
            LED_On(0);
            LED_Off(1);
            LED_Off(2);
            LED_Off(3);

            //Clear existing wake-up config
            LP_ClearWakeUpConfig();

            //Clear any event flags
            LP_ClearWakeUpFlags();

            //configure wakeup on RTC compare 0
            LP_ConfigRTCWakeUp(1, 0, 0, 0);

            printf("Enter LP0\n");
            while (Console_PrepForSleep() != E_NO_ERROR);

            //set RTC compare 0
            uint32_t cmp = RTC_GetCount() + LP0_WakeTime;
            RTC_SetCompare(0,cmp);
            RTC_ClearFlags(MXC_F_RTC_FLAGS_COMP0);

            //interrupts are disabled in LP_EnterLP0
            LP_EnterLP0();

            //firmware will reset on wake-up
        }

        // Enter LP1 with prescaler wake-up when SW3 is pushed
        if(PB_Get(SW3))
        {
            //set LED1 on and the rest off
            LED_Off(0);
            LED_On(1);
            LED_Off(2);
            LED_Off(3);

            //Clear existing wake-up config
            LP_ClearWakeUpConfig();

            //Clear any event flags
            LP_ClearWakeUpFlags();

            //configure wake-up on RTC prescaler and COMP1
            LP_ConfigRTCWakeUp(0, 1, 1, 0);

            printf("Enter LP1 with prescaler wake-up\n");
            while (Console_PrepForSleep() != E_NO_ERROR);

            //set RTC compare 1 value
            uint32_t cmp = RTC_GetCount() + LP1_WakeTime;
            RTC_SetCompare(1,cmp);
            RTC_ClearFlags(MXC_F_RTC_FLAGS_COMP1 | MXC_F_RTC_FLAGS_PRESCALE_COMP);

            //global disable interrupt
            __disable_irq();

            LP_EnterLP1();

            //global enable interrupt
            __enable_irq();

            //if wake-up triggered by prescaler and not COMP1, then go back to sleep
            while((LP_GetWakeUpFlags() & MXC_F_PWRSEQ_FLAGS_RTC_CMPR1) == 0) {
                LED_Toggle(1); //toggle LED1

                printf("Prescaler wake-up, count = %d\n",RTC_GetCount());
                while (Console_PrepForSleep() != E_NO_ERROR);

                //clear wakeup and RTC flags
                LP_ClearWakeUpFlags();
                RTC_ClearFlags(MXC_F_RTC_FLAGS_PRESCALE_COMP);

                //global disable interrupt
                __disable_irq();

                LP_EnterLP1();

                //global enable interrupt
                __enable_irq();
            }

            Wakeup_LP1();
        }
    }
}
