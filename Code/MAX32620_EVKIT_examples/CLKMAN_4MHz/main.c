/**
 * @file
 * @brief      Example using the 4MHz oscillator.
 * @details    This example configures the system to run off the 4MHz
 *             oscillator. It then re-configures to run off of the 96MHz
 *             oscillator. If you do not re-configure the system clock, the
 *             flash loading and debugging will be noticeably slower.
 */

 /* ***************************************************************************
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
#include "mxc_sys.h"
#include "board.h"
#include "nhd12832.h"
#include "tmr_utils.h"

/* **** Definitions **** */

/* **** Globals **** */

/* **** Functions **** */

/* ************************************************************************** */
int main(void)
{
    int i = 5;

    // Initialize the OLED. 
    NHD12832_Init();

    while(i--) {

        // Set the System clock to the 96MHz oscillator
        CLKMAN_SetSystemClock(CLKMAN_SYSTEM_SOURCE_96MHZ, CLKMAN_SYSTEM_SCALE_DIV_1);
        NHD12832_ShowString((uint8_t*)"96 Mhz clock", 0, 4);
        LED_On(1);
        LED_Off(0);
        TMR_Delay(MXC_TMR0, SEC(1));

        // Set the System clock to the 4MHz oscillator
        CLKMAN_SetSystemClock(CLKMAN_SYSTEM_SOURCE_4MHZ, CLKMAN_SYSTEM_SCALE_DIV_1);
        NHD12832_ShowString((uint8_t*)"4 Mhz clock", 0, 4);
        LED_Off(1);
        LED_On(0);
        TMR_Delay(MXC_TMR0, SEC(1));
    }
    
    // Set the System clock to the 96MHz oscillator
    CLKMAN_SetSystemClock(CLKMAN_SYSTEM_SOURCE_96MHZ, CLKMAN_SYSTEM_SCALE_DIV_1);
    NHD12832_ShowString((uint8_t*)"96 Mhz clock", 0, 4);
    LED_On(1);
    LED_Off(0);   

    while(1) {}
}
