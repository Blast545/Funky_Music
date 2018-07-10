/**
 * @file    
 * @brief   ADC demo application
 * @details Continuously monitors the ADC channels
 */

/* ******************************************************************************
 * Copyright (C) 2017 Maxim Integrated Products, Inc., All Rights Reserved.
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
#include "led.h"
#include "adc.h"
#include "nhd12832.h"
#include "tmr_utils.h"

/* **** Definitions **** */

/* Change to #undef USE_INTERRUPTS for polling mode */
#define USE_INTERRUPTS 1

/* **** Globals **** */
#ifdef USE_INTERRUPTS
volatile unsigned int adc_done = 0;
#endif

/* **** Functions **** */

#ifdef USE_INTERRUPTS
void AFE_IRQHandler(void)
{
    ADC_ClearFlags(MXC_F_ADC_INTR_ADC_DONE_IF);
    
    /* Signal bottom half that data is ready */
    adc_done = 1;
    
    return;
}
#endif /* USE_INTERRUPTS */

int main(void)
{
    uint16_t adc_val[4];
    unsigned int overflow[4];
    uint8_t fmtstr[40];
    
    NHD12832_Init();
#ifdef USE_INTERRUPTS
    NHD12832_ShowString((uint8_t*)"MAX32620 ADC Demo (I)", 0, 4);
#else /* synchronous/polled mode */
    NHD12832_ShowString((uint8_t*)"MAX32620 ADC Demo (P)", 0, 4);
#endif /* USE_INTERRUPTS */

    /* Initialize ADC */
    ADC_Init();

    /* Set up LIMIT0 to monitor high and low trip points */
    ADC_SetLimit(ADC_LIMIT_0, ADC_CH_1, 1, 0x25, 1, 0x300);
    
#ifdef USE_INTERRUPTS
    NVIC_EnableIRQ(AFE_IRQn);
#endif /* USE_INTERRUPTS */
    
    while(1) {
	/* Flash LED when starting ADC cycle */
	LED_On(0);
	TMR_Delay(MXC_TMR0, MSEC(10));
	LED_Off(0);

	/* Convert channel 0 */
#ifdef USE_INTERRUPTS
	/* set global to indicate we are starting an adc operation. */
	adc_done = 0;
	ADC_StartConvert(ADC_CH_0, 0, 1);
	while (!adc_done);
#else
	ADC_StartConvert(ADC_CH_0, 0, 1);
#endif /* USE_INTERRUPTS */
	overflow[0] = (ADC_GetData(&adc_val[0]) == E_OVERFLOW ? 1 : 0);

	/* Convert channel 1 */
#ifdef USE_INTERRUPTS
	adc_done = 0;
	ADC_StartConvert(ADC_CH_1, 0, 1);
	while (!adc_done);
#else
	ADC_StartConvert(ADC_CH_1, 0, 1);
#endif /* USE_INTERRUPTS */
	overflow[1] = (ADC_GetData(&adc_val[1]) == E_OVERFLOW ? 1 : 0);

	/* Convert channel 2 */
#ifdef USE_INTERRUPTS
	adc_done = 0;
	ADC_StartConvert(ADC_CH_2, 0, 1);
	while (!adc_done);
#else
	ADC_StartConvert(ADC_CH_2, 0, 1);
#endif /* USE_INTERRUPTS */
	overflow[2] = (ADC_GetData(&adc_val[2]) == E_OVERFLOW ? 1 : 0);

	/* Convert channel 3 */
#ifdef USE_INTERRUPTS
	adc_done = 0;
	ADC_StartConvert(ADC_CH_3, 0, 1);
	while (!adc_done);
#else
	ADC_StartConvert(ADC_CH_3, 0, 1);
#endif /* USE_INTERRUPTS */
	overflow[3] = (ADC_GetData(&adc_val[3]) == E_OVERFLOW ? 1 : 0);

	/* Display results on OLED display, display asterisk if overflow */
	snprintf((char *)fmtstr, 40, "0: 0x%04x%s 2: 0x%04x%s",
		 adc_val[0], overflow[0] ? "*" : " ",
		 adc_val[2], overflow[2] ? "*" : " ");
	NHD12832_ShowString(fmtstr, 1, 4);
	snprintf((char *)fmtstr, 40, "1: 0x%04x%s 3: 0x%04x%s",
		 adc_val[1], overflow[1] ? "*" : " ",
		 adc_val[3], overflow[3] ? "*" : " ");
	NHD12832_ShowString(fmtstr, 2, 4);

	/* Determine if programmable limits on AIN1 were exceeded */
	if (ADC_GetFlags() & (MXC_F_ADC_INTR_ADC_LO_LIMIT_IF | MXC_F_ADC_INTR_ADC_HI_LIMIT_IF)) {
	  snprintf((char *)fmtstr, 40, " %s Limit on AIN1 ",
		   (ADC_GetFlags() & MXC_F_ADC_INTR_ADC_LO_LIMIT_IF) ? "Low" : "High");
	  ADC_ClearFlags(MXC_F_ADC_INTR_ADC_LO_LIMIT_IF | MXC_F_ADC_INTR_ADC_HI_LIMIT_IF);
	} else {
	  snprintf((char *)fmtstr, 40, "                   ");
	}
	NHD12832_ShowString(fmtstr, 3, 4);

	/* Delay for 1/4 second before next reading */
	TMR_Delay(MXC_TMR0, MSEC(250));
    }
}
