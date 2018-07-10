/**
 * @file    
 * @brief   Display logo on OLED display and blinks LEDs based on Systick timer
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
 * $Date: 2015-12-04 08:35:27 -0600 (Fri, 04 Dec 2015) $
 * $Revision: 20285 $
 *
 **************************************************************************** */
 
/* **** Includes **** */
#include <stdio.h>
#include <stdint.h>
#include "mxc_config.h"
#include "mxc_sys.h"
#include "nvic_table.h"
#include "nhd12832.h"
#include "board.h"
#include "oled_logo.h"
#include "tmr_utils.h"

/* **** Definitions **** */
#define USE_SYSTEM_CLK 0
#define SYSTICK_PERIOD_SYS_CLK 4800000 /**< 50ms with 96MHz system clock */
#define SYSTICK_PERIOD_EXT_CLK 3277 /**< 100ms with 32768Hz external rtc xtal */
#define BLINK_FAST 1
#define BLINK_MEDIUM 2
#define BLINK_SLOW 4

/* **** Globals **** */

/* **** Functions **** */

void SysTick_Setup(uint32_t divider)
{
	uint32_t sysTicks;

    sysTicks = SYSTICK_PERIOD_EXT_CLK * divider;

    uint32_t error = SYS_SysTick_Config(sysTicks, USE_SYSTEM_CLK);

    printf("SysTick Clock = %d Hz\n", SYS_SysTick_GetFreq());
    printf("SysTick Period = %d ticks\n", sysTicks);

    if(error != E_NO_ERROR) {
        printf("ERROR: Ticks is not valid");
    }
}

void pb_handler_Fast(void *pb)
{
    printf("\nBlink Rate:  Fast\n");
    SysTick_Setup(BLINK_FAST);
}

void pb_handler_Medium(void *pb)
{
	printf("\nBlink Rate:  Medium\n");
	SysTick_Setup(BLINK_MEDIUM);
}

void pb_handler_Slow(void *pb)
{
    printf("\nBlink Rate:  Slow\n");
	SysTick_Setup(BLINK_SLOW);
}

void SysTick_Handler(void)
{
    //Toggle LED0 every systick period
    LED_Toggle(0);
    LED_Toggle(1);
    LED_Toggle(2);
    LED_Toggle(3);
}

void GPIO_Setup()
{
    //turn off all LEDs
    LED_Off(0);
    LED_Off(1);
    LED_Off(2);
    LED_Off(3);

    //push buttons to enable fast blinky
    PB_RegisterCallback(SW1,pb_handler_Fast);

    //push button to enable medium blinky
    PB_RegisterCallback(SW2,pb_handler_Medium);

    //push button to enable slow blinky
    PB_RegisterCallback(SW3,pb_handler_Slow);
}

/* ************************************************************************** */
int main(void)
{
    NHD12832_Init();
    NHD12832_ShowString((uint8_t*)"MAX32620 EV Kit Demo", 0, 4);

    printf("\n\nMAX32620 EV Kit Demo\n\n");

    /* Display the splash screen for  3 seconds*/
	TMR_Delay(MXC_TMR0, MSEC(3000));

    NHD12832_LoadImage(&maxim_integrated_logo);
    NHD12832_PrintScreen();

    GPIO_Setup();

    printf("Press buttons to change LED blinking rate:\n");
    printf("SW1 = Fast\n");
    printf("SW2 = Medium\n");
    printf("SW3 = Slow\n");

	printf("\nBlink Rate:  Medium\n");
	SysTick_Setup(BLINK_MEDIUM);

    LED_On(0);
    LED_On(1);
    LED_On(2);
    LED_On(3);

    while(1) {}
}
