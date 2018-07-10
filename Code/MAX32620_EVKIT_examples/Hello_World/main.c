/**
 * @file    
 * @brief   Hello World!
 * @details This example uses the UART to print to a terminal and flashes an LED.
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
#include "led.h"
#include "nhd12832.h"
#include "tmr_utils.h"

/* **** Definitions **** */

/* **** Globals **** */

/* **** Functions **** */

/* ************************************************************************** */
int main(void)
{
    printf("Hello World!\n");

    // Print to the OLED
    NHD12832_Init();
    NHD12832_ShowString((uint8_t*)"Hello World!", 0, 4);

    int count = 0;
    while(1) {
        LED_On(0);
        TMR_Delay(MXC_TMR0, MSEC(500));
        LED_Off(0);
        TMR_Delay(MXC_TMR0, MSEC(500));
        printf("count = %d\n", count++);
    }
}
