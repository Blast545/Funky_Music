/**
 * @file    main.c
 * @brief   This program shows LP0/LP1 wake-up times indicated by GPIO toggles.
 * @details LP0 and LP1 both wake up to LP3 (run mode).
 *
 *          1. LED3 blinks on program start.  - LP3
 *          2. Press SW3(P60) to begin. (LED3 on, other LEDs off) - LP3
 *          3. Press SW2(P55) to enter LP1. (LED1 on, other LEDs off) - LP1
 *          4. Press SW2(P55) to wake-up from LP1. (LED3 on, other LEDs off) - LP3
 *          5. Press SW1(P54) to enter LP0. (LED0 on, other LEDs off) - LP0
 *          6. Press SW1(P54) to wake-up from LP0 and restart program. (LED3 blinking) - LP3
 *
 *          To measure wake-up times
 *          LP1 Wake-up - measure from falling edge of P5.5 to falling edge of P0.7
 *          LP0 Wake-up - measure from falling edge of P5.4 to falling edge of P0.7
 */

/* ******************************************************************************
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
 ***************************************************************************** */

/* **** Includes **** */
#include <stdint.h>
#include "board.h"
#include "lp.h"
#include "gpio.h"
#include "clkman_regs.h"
#include "icc.h"

/* **** Globals **** */
gpio_cfg_t gpio07;

//use push buttons defined in board.h
extern const gpio_cfg_t pb_pin[];

/* **** Functions **** */

/* **************************************************************************** */
void pause(void)
{
    unsigned int i;

    for (i = 0; i < 12000000; i++) {
        __NOP();
    }
}

/* **************************************************************************** */
void SetupGPIO()
{
    //setup P0.7 high - used to measure wake-up time
    gpio07.port = 0;
    gpio07.mask = PIN_7;
    gpio07.func = GPIO_FUNC_GPIO;
    gpio07.pad = GPIO_PAD_NORMAL;
    GPIO_Config(&gpio07);
    GPIO_OutSet(&gpio07);
}

int main(void)
{
    uint32_t x = 0;

    //Set P0.7 high
    SetupGPIO();

    //blink LED3 until SW3(P60) is pushed
    while (!PB_Get(SW3)) {
        x++;
        if (x & 0x10000)
            LED_On(3);
        else
            LED_Off(3);
    }

    //Turn on LED3
    LED_On(3);

    while(1) {
        // Enter LP1 when SW2 is pushed
        if(PB_Get(SW2)) {
            pause(); //wait for push button to settle

            // Clear existing wake-up config
            LP_ClearWakeUpConfig();

            // Clear any event flags
            LP_ClearWakeUpFlags();

            //Configure wake-up on SW2 (P55) push (falling edge)
            LP_ConfigGPIOWakeUpDetect(&pb_pin[1], 0, LP_WEAK_PULL_UP);

            //Set GPIO0.7 high
            GPIO_OutSet(&gpio07);

            //turn on LED1 to indicate LP1
            LED_On(1);
            LED_Off(3);

            //global disable interrupt
            __disable_irq();

            //Go to LP1
            LP_EnterLP1();

            //global enable interrupt
            __enable_irq();

            //On wake-up set LED1 = off, LED3 = On
            LED_Off(1);
            LED_On(3);

            pause(); //wait for push button to settle
        }

        // Enter LP0 when SW1 is pushed
        if(PB_Get(SW1)) {
            pause(); //wait for push button to settle

            // Clear existing wake-up config
            LP_ClearWakeUpConfig();

            // Clear any event flags
            LP_ClearWakeUpFlags();

            //Configure wake-up on SW1 (P54) push (falling edge)
            LP_ConfigGPIOWakeUpDetect(&pb_pin[0], 0, LP_WEAK_PULL_UP);

            //Set P0.7 high
            GPIO_OutSet(&gpio07);

            //turn on LED0 to indicate LP0
            LED_On(0);
            LED_Off(3);

            //Go to LP0
            //interrupts are disabled in LP_EnterLP0
            LP_EnterLP0();

            //program restarts on wake-up
        }
    }
}
