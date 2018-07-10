/**
 * @file
 * @brief Configures and starts four different pulse trains on GPIO LEDs.
 * @details
 * 			- P3.0 LED0: PT8 setup as 1Hz continuous signal that outputs 10110b
 * 			- P3.1 LED1: PT9 setup as 10Hz continuous square wave
 * 			- P3.2 LED2: PT10 setup to loop twice and then start PT11
 * 			- P3.3 LED3: PT11 setup to loop twice and then start PT10
 * 			- P5.4 SW1:  Push button setup to stop/start pulse trains
 *
 * @note Interrupts for pulse trains are enabled but the interrupt handler only clears the flags.
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
 * $Date: 2018-03-08 13:11:59 -0600 (Thu, 08 Mar 2018) $
 * $Revision: 33820 $
 *
 **************************************************************************** */

/* **** Includes **** */
#include <stdio.h>
#include <stdint.h>
#include "mxc_config.h"
#include "board.h"
#include "pt.h"

/* **** Definitions **** */
#define 	ALL_PT	0xFFFF

/* **** Globals **** */

/* **** Functions **** */

/* ************************************************************************** */
static void PB_Start_Stop_handler(void * pb_num)
{
    //Check if any pulse trains are running
    if(PT_IsActiveMulti(ALL_PT)) {
        //stop all pulse trains
        PT_StopMulti(ALL_PT);
        printf("PT Stopped\n");
    } else {
        //start PT8, PT9 an PT10
        uint32_t enablePTMask = MXC_F_PT_ENABLE_PT8 | MXC_F_PT_ENABLE_PT9 | MXC_F_PT_ENABLE_PT10;
        PT_StartMulti(enablePTMask);
        printf("PT Started\n");
    }
}

/* ************************************************************************** */
void PT_IRQHandler(void)
{
    printf("flags = 0x%08x\n", PT_GetFlags());

    PT_ClearFlags(ALL_PT);
}

/* ************************************************************************** */
void ContinuousPulseTrain()
{
    //Setup GPIO to PT output function
    //GPIO P3.0 uses PT8
    sys_cfg_pt_t gpio;
    gpio.port = 3;
    gpio.mask = PIN_0;
    gpio.func = GPIO_FUNC_PT;
    gpio.pad = GPIO_PAD_OPEN_DRAIN;

    //setup PT configuration
    pt_pt_cfg_t ptConfig;
    ptConfig.bps = 2;			//bit rate
    ptConfig.ptLength = 5;		//bits
    ptConfig.pattern = 0x16;
    ptConfig.loop = 0; 			//continuous loop
    ptConfig.loopDelay = 0;

    PT_PTConfig(MXC_PT8, &ptConfig, &gpio);

    //start PT8
    PT_Start(MXC_PT8);
}

/* ************************************************************************** */
void SquareWave()
{
    //Setup GPIO to PT output function
    //GPIO P3.1 uses PT9
    sys_cfg_pt_t gpio;
    gpio.port = 3;
    gpio.mask = PIN_1;
    gpio.func = GPIO_FUNC_PT;
    gpio.pad = GPIO_PAD_OPEN_DRAIN;

    uint32_t freq = 10;//Hz
    PT_SqrWaveConfig(MXC_PT9, freq, &gpio);

    //start PT9
    PT_Start(MXC_PT9);
}

/* ************************************************************************** */
void PulseTrainSquence()
{
    pt_pt_cfg_t ptConfig;
    sys_cfg_pt_t gpioPT10;
    sys_cfg_pt_t gpioPT11;

    //Setup GPIO to PT output function
    //GPIO P3.2 uses PT10
    gpioPT10.port = 3;
    gpioPT10.mask = PIN_2;
    gpioPT10.func = GPIO_FUNC_PT;
    gpioPT10.pad = GPIO_PAD_OPEN_DRAIN;

    //GPIO P3.3 uses PT11
    gpioPT11.port = 3;
    gpioPT11.mask = PIN_3;
    gpioPT11.func = GPIO_FUNC_PT;
    gpioPT11.pad = GPIO_PAD_OPEN_DRAIN;

    //setup PT10 configuration
    ptConfig.bps = 4;		//bit rate
    ptConfig.ptLength = 8;	//bits
    ptConfig.pattern = 0x66;
    ptConfig.loop = 2; 		//# of loops
    ptConfig.loopDelay = 0; //delay between loops
    PT_PTConfig(MXC_PT10, &ptConfig, &gpioPT10);

    //setup PT11 configuration
    ptConfig.bps = 6;		//bit rate
    ptConfig.ptLength = 8;	//bits
    ptConfig.pattern = 0x6C;
    ptConfig.loop = 2; 		//# of loops
    ptConfig.loopDelay = 0; //delay between loops in clock ticks
    PT_PTConfig(MXC_PT11, &ptConfig, &gpioPT11);

    //set PT11 to restart after PT10
    PT_SetRestart(MXC_PT11, MXC_PT10, 0);

    //set PT10 to restart after PT11
    PT_SetRestart(MXC_PT10, MXC_PT11, 0);

    //Start PT10
    PT_Start(MXC_PT10);

}

/* ************************************************************************** */
int main(void)
{
    sys_cfg_ptg_t ptg_cfg;

    printf("\n*************** Pulse Train Demo ****************\n");
    printf("LED0 = Outputs continuous pattern of 10110b at 2bps\n");
    printf("LED1 = Outputs 10Hz continuous square wave\n");
    printf("LED2 = Outputs 01100110b at 4bps, loops twice and then start LED3\n");
    printf("LED3 = Outputs 01101100b at 6bps, loops twice and then start LED2\n");
    printf("SW1  = Stop/Start all pulse trains\n");

    //Setup push button to start/stop All pulse train
    PB_RegisterCallback(0, PB_Start_Stop_handler);

    // Configure clock scaling for this peripheral
    ptg_cfg.clk_scale = CLKMAN_SCALE_DIV_256;
    // .io_cfg is unused in this context
    PT_Init(&ptg_cfg);

    NVIC_EnableIRQ(PT_IRQn);   //enable default interrupt handler
    PT_EnableINTMulti(ALL_PT); //enable interrupts for all PT

    //configure and start pulse trains
    ContinuousPulseTrain();
    SquareWave();
    PulseTrainSquence();

    while(1) {}
}
