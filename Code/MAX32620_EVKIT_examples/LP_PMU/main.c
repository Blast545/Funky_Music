/**
 * @file
 * @brief      This example show how to run the PMU while the CPU is in a low
 *             power state. The PMU blinks LED0 every second based on the RTC's
 *             prescaler compare interrupt. Every 5 seconds the CPU wakes up
 *             from LP2 and blinks LED1, LED2, and LED3 in sequence.
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
#include "board.h"
#include "lp.h"
#include "pmu.h"
#include "rtc.h"
#include "nvic_table.h"

/* **** Definitions **** */
#define LP2_WAKEUP_TIME 	5

#define GPIO3_VALUE_REG		MXC_BASE_GPIO+MXC_R_GPIO_OFFS_OUT_VAL_P3
#define RTC_INTFL_REG  		MXC_BASE_RTCTMR + MXC_R_RTCTMR_OFFS_FLAGS
#define RTC_INTEN_REG  		MXC_BASE_RTCTMR + MXC_R_RTCTMR_OFFS_INTEN

/* **** Globals **** */
static  uint32_t pmu_program[] = {
    PMU_WAIT(PMU_NO_INTERRUPT, PMU_NO_STOP, PMU_WAIT_SEL_0, 0, PMU_WAIT_IRQ_MASK2_SEL0_RTC_PRESCALE, 0), //wait for prescaler compare interrupt
    PMU_WRITE(PMU_NO_INTERRUPT, PMU_NO_STOP, PMU_WRITE_MASKED_WRITE_VALUE, RTC_INTFL_REG, MXC_F_RTC_FLAGS_PRESCALE_COMP, 0xffffffff),//clear flag

    PMU_WRITE(PMU_NO_INTERRUPT, PMU_NO_STOP, PMU_WRITE_MASKED_WRITE_VALUE, GPIO3_VALUE_REG, 0x0, 0x1), //set  LED0 on (0)

    PMU_WAIT(PMU_NO_INTERRUPT, PMU_NO_STOP, PMU_WAIT_SEL_0, 0, PMU_WAIT_IRQ_MASK2_SEL0_RTC_PRESCALE, 0), //wait for prescale compare interrupt
    PMU_WRITE(PMU_NO_INTERRUPT, PMU_NO_STOP, PMU_WRITE_MASKED_WRITE_VALUE, RTC_INTFL_REG, MXC_F_RTC_FLAGS_PRESCALE_COMP, 0xffffffff),//clear flag

    PMU_WRITE(PMU_NO_INTERRUPT, PMU_NO_STOP, PMU_WRITE_MASKED_WRITE_VALUE, GPIO3_VALUE_REG, 0x1, 0x1), //set green LED0 off (1)

    PMU_JUMP(PMU_NO_INTERRUPT, PMU_NO_STOP, (uint32_t)(pmu_program)), //loop back to first PMU WAIT instruction
};

/* **** Functions **** */

/* ****************************************************************************/
void pause(void)
{
    unsigned int i;

    for (i = 0; i < 5000000; i++) {
        __NOP();
    }
}

/* ****************************************************************************/
void RTC_handler_Compare1()
{
    //Disable and clear RTC compare1 interrupt
    RTC_DisableINT(MXC_F_RTC_INTEN_COMP1);
    RTC_ClearFlags(MXC_F_RTC_FLAGS_COMP1);
}

/* ****************************************************************************/
void RTC_Setup()
{
    rtc_cfg_t RTCconfig;

    RTCconfig.compareCount[0] = 5;//5 second timer
    RTCconfig.compareCount[1] = 7;//7 second timer
    RTCconfig.prescaler = RTC_PRESCALE_DIV_2_12; //1Hz clock
    RTCconfig.prescalerMask = RTC_PRESCALE_DIV_2_11;//0.5s prescaler comp
    RTCconfig.snoozeCount = LP2_WAKEUP_TIME;
    RTCconfig.snoozeMode = RTC_SNOOZE_MODE_B;

    RTC_Init(&RTCconfig);

    //enable Prescaler compare interrupt
    RTC_EnableINT(MXC_F_RTC_INTEN_PRESCALE_COMP);

    //setup interrupts
    NVIC_SetVector(RTC1_IRQn, RTC_handler_Compare1);

    RTC_Start();
}

/* ****************************************************************************/
int main(void)
{
    printf("\n**********LP2 PMU Demo***********\n");

    //configure RTC and start
    RTC_Setup();

    //start PMU - blinks LED0 every 1s (toggles every 0.5s)
    PMU_Start(0, pmu_program, NULL);

    while(1) {
        //set the snooze and enable compare 1
        RTC_Snooze();	//comp1 = time + 5
        RTC_EnableINT(MXC_F_RTC_INTEN_COMP1);

        printf("Enter LP2\n");
        LP_EnterLP2();

        printf("Woke up from LP2\n");

        //blink LED1, LED2 and LED3 in sequence
        LED_On(1);
        pause();

        LED_Off(1);
        LED_On(2);
        pause();

        LED_Off(2);
        LED_On(3);
        pause();

        LED_Off(3);
    }
}
