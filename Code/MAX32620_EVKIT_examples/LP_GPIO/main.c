/**
 * @file    
 * @brief   Shows an example of how to sleep from LP3 (run mode) and wake-up
 *          from LP2, LP1 and LP0 by using the GPIO wake-up triggers
 *
 * @details 1. On power up, device is in LP3 (LED3 on)
 *          2. Push SW3(P60) push button and the device will go to LP2 sleep 
 *             mode (LED2 on)
 *          3. Push SW3(P60) again to wake up from LP2 and enter LP3 (LED3 on)
 *          2. Push SW2(P55) push button and the device will go to LP1 sleep 
 *             mode (LED1 on)
 *          3. Push SW2(P55) again to wake up from LP1 and enter LP3 (LED3 on)
 *          4. Push SW1(P54) push button and the device will go to LP0 sleep 
 *             mode (LED0 on)
 *          5. Push SW1(P54) again to wake up and restart program (three blinks 
 *             on LED3 then stays on)
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
#include <stdio.h>
#include <stdint.h>
#include "mxc_config.h"
#include "board.h"
#include "lp.h"
#include "gpio.h"

/* **** Definitions **** */
#define LP0_WAKE_GPIO_PORT	5
#define LP0_WAKE_GPIO_PIN	PIN_4

#define LP1_WAKE_GPIO_PORT	5
#define LP1_WAKE_GPIO_PIN	PIN_5

#define LP2_WAKE_GPIO_PORT	6
#define LP2_WAKE_GPIO_PIN	PIN_0

/* **** Globals **** */

unsigned int SW1_pushed = 0;
unsigned int SW2_pushed = 0;

gpio_cfg_t gpioLP0, gpioLP1, gpioLP2;

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

/* ****************************************************************************/
void Wakeup_LP1()
{
    printf("Woke-up from LP1\n");

    //turn on LED3 and the rest off
    LED_Off(0);
    LED_Off(1);
    LED_Off(2);
    LED_On(3);
}

/* ****************************************************************************/
void Wakeup_LP2()
{
    printf("Woke-up from LP2\n");

    //turn on LED3 and the rest off
    LED_Off(0);
    LED_Off(1);
    LED_Off(2);
    LED_On(3);
}

/* ****************************************************************************/
void pb2_irq_handler(void *unused)
{

}

/* ****************************************************************************/
void GPIO_Setup()
{
    //set LED3 on and the rest off
    LED_Off(0);
    LED_Off(1);
    LED_Off(2);
    LED_On(3);

    //configure GPIO pin as input with pullup - use for LP0 wakeup
    gpioLP0.port = LP0_WAKE_GPIO_PORT;
    gpioLP0.mask = LP0_WAKE_GPIO_PIN;
    gpioLP0.func = GPIO_FUNC_GPIO;
    gpioLP0.pad = GPIO_PAD_INPUT_PULLUP;
    GPIO_Config(&gpioLP0);

    //configure GPIO pin as input with pullup - use for LP1 wakeup
    gpioLP1.port = LP1_WAKE_GPIO_PORT;
    gpioLP1.mask = LP1_WAKE_GPIO_PIN;
    gpioLP1.func = GPIO_FUNC_GPIO;
    gpioLP1.pad = GPIO_PAD_INPUT_PULLUP;
    GPIO_Config(&gpioLP1);

    //configure GPIO pin as input with pullup and falling edge interrupt - use for LP2 wakeup
    gpioLP2.port = LP2_WAKE_GPIO_PORT;
    gpioLP2.mask = LP2_WAKE_GPIO_PIN;
    gpioLP2.func = GPIO_FUNC_GPIO;
    gpioLP2.pad = GPIO_PAD_INPUT_PULLUP;
    GPIO_Config(&gpioLP2);

    // Register callback
    GPIO_RegisterCallback(&gpioLP2, pb2_irq_handler, NULL);

    // Configure and enable interrupt
    GPIO_IntConfig(&gpioLP2, GPIO_INT_FALLING_EDGE);
    GPIO_IntEnable(&gpioLP2);
    NVIC_EnableIRQ(MXC_GPIO_GET_IRQ(gpioLP2.port));
}

/******************************************************************************/
int main(void)
{
    //check if starting at main because of LP0 wake-up
    if(LP_IsLP0WakeUp())
        Wakeup_LP0();
    else {
        printf("\n********** GPIO Power Demo *********\n");
        printf("Press a button to enter a sleep state:\n");
        printf("SW1 = Enter LP0\n");
        printf("SW2 = Enter LP1\n");
        printf("SW3 = Enter LP2\n");
    }

    //initialize LEDs and setup push button interrupts
    GPIO_Setup();

    while(1) {
        //Enter LP2 when SW3 is pushed
        if(PB_Get(SW3))
        {
            pause(); //wait for push button to settle

            //set LED1 on and the rest off
            LED_Off(0);
            LED_Off(1);
            LED_On(2);
            LED_Off(3);

            printf("Enter LP2. Press SW3 to wake up.\n");
            LP_EnterLP2();
            Wakeup_LP2();

            pause(); //wait for push button to settle
        }

        // Enter LP1 when SW2 is pushed
        if(PB_Get(SW2))
        {
            pause(); //wait for push button to settle

            //set LED1 on and the rest off
            LED_Off(0);
            LED_On(1);
            LED_Off(2);
            LED_Off(3);

            //Clear existing wake-up config
            LP_ClearWakeUpConfig();

            //Clear any event flags
            LP_ClearWakeUpFlags();

            //configure wake-up on GPIO
            LP_ConfigGPIOWakeUpDetect(&gpioLP1, 0, LP_WEAK_PULL_UP);

            printf("Enter LP1. Press SW2 to wake-up.\n");
            while (Console_PrepForSleep()!= E_NO_ERROR);

            //global disable interrupt
            __disable_irq();

            LP_EnterLP1();

            //global enable interrupt
            __enable_irq();

            Wakeup_LP1();

            pause(); //wait for push button to settle
        }

        // Enter LP0 when SW1 is pushed
        if(PB_Get(SW1))
        {
            pause();

            //turn on LED0 and the rest are off
            LED_On(0);
            LED_Off(1);
            LED_Off(2);
            LED_Off(3);

            //Clear existing wake-up config
            LP_ClearWakeUpConfig();

            //Clear any event flags
            LP_ClearWakeUpFlags();

            //configure wake-up on GPIO
            LP_ConfigGPIOWakeUpDetect(&gpioLP0, 0, LP_WEAK_PULL_UP);

            printf("Enter LP0. Press SW1 to wake-up.\n");
            while (Console_PrepForSleep() != E_NO_ERROR);

            //interrupts are disabled in LP_EnterLP0
            LP_EnterLP0();

            //firmware will reset on wake-up
        }
    }
}
