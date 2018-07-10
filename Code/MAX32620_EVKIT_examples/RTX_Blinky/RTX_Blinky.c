/**
 * @file
 * @brief      RTX_Blinky RTOS example application.
 * @details    RTX_Blinky example application showing a simple getting
 *             started application for the RTX RTOS with CMSIS.
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
 * $Date: 2017-02-28 17:00:42 -0600 (Tue, 28 Feb 2017) $
 * $Revision: 26761 $
 *
 *************************************************************************** */

/* **** Includes **** */
#include "cmsis_os.h"                   /* ARM::CMSIS:RTOS:Keil RTX */
#include <stdio.h>
#include <stdint.h>
#include "mxc_config.h"
#include "mxc_sys.h"
#include "nvic_table.h"
#include "board.h"
#include "nhd12832.h"

/* RTX Blinky Thread Function Declaration */
void blinkLED(void const *argument);

/* **** Definitions **** */
/** 
 * System clock specifier.
 * @arg 0 for external 32kHz crystal
 * @arg 1 for system clock
 */
#define USE_SYSTEM_CLK 1

/* SysTick will be tied to 96MHz internal oscillator */
#define SYSTICK_DIV  96000 /* make about 1ms timeout (96MHz / 96KHz) = 1ms) */

#define SYSTICK_PERIOD_SYS_CLK 96000 // 1ms with 96MHz system clock
#define SYSTICK_PERIOD_EXT_CLK 32 // ~1ms with 32kHz used for system clock

/**
 * Push button number to use for the user start, 0 for SW1 will be used as the user input push button to start the LED blinking. 
 */
#define PUSH_BUTTON     0

/**
 * Number of milliseconds that the LED is turned on/off. 
 */
#define LED_DISPLAY_TIME 512 

/**
 * RTX thead declaration
 */
osThreadId tid_blinkLED;

/**
 * RTX thread definition/creation
 */
osThreadDef (blinkLED, osPriorityNormal, 1, 0);

#ifndef TOP_MAIN

/**
 * @brief Waits for the user to push SW1 and begins blinking LED0.
 * @param argument  Pointer to any parameter that was used when creating this 
 *                  thread using osThreadCreate().
 *
 */
void blinkLED(void const *argument) {
    uint32_t max_num = num_leds-1;
    uint32_t num = 0;
    int32_t dir = 1;
    // Print to the OLED
    NHD12832_Init();
    NHD12832_ShowString((uint8_t*)"Push PB0 to Start!", 0, 4);
    printf("Push PB0 to Start!");
    /* wait for the user to push PB0 */
    while (!PB_Get(PUSH_BUTTON));
    for (;;) {
          
        /* Turn on the next LED */
        LED_On(num);                                           
        /* delay half a second */
        osDelay(500);
        /* Turn off the LED */
        LED_Off(num);                                          
        /* make sure it stays off for a half a second minimum*/
        osDelay(500);                   
        /* leave LED off if the user is holding down the push button 0 */                       
        //while (PB_Get(PUSH_BUTTON));                 

        /* move to the next LED on the evaluation kit board */
        num += dir;                      

        /* if going up on the LEDs and we have reached the last one       *
         * change the direction to down, else set it to up if we are at 0 */                      
        if (dir == 1 && num == max_num) {
            dir = -1;                                            
        }
        else if (num == 0) {
            dir =  1;                                            
        }
    }
}

/*----------------------------------------------------------------------------
 * main: initialize and start the system
 *----------------------------------------------------------------------------*/
int main (void) {
    uint32_t sysTicks;
    printf("\n************ RTX_Blinky ****************\n");
    
    if(USE_SYSTEM_CLK)
        sysTicks = SYSTICK_PERIOD_SYS_CLK;
    else
        sysTicks = SYSTICK_PERIOD_EXT_CLK;
    
    uint32_t error = SYS_SysTick_Config(sysTicks, USE_SYSTEM_CLK);

    printf("SysTick Clock = %d Hz\n", SYS_SysTick_GetFreq());
    printf("SysTick Period = %d ticks\n", sysTicks);
    /* create RTX threads */
    tid_blinkLED = osThreadCreate (osThread(blinkLED), NULL);

    /* Start the RTX Kernel and the blink thread */
    osKernelStart ();

    /* Terminate the RTX Kernel */
    osThreadTerminate (osThreadGetId ());
}
#endif /* TOP_MAIN */
