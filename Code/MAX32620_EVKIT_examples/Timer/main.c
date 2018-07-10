/**
 * @file    
 * @brief   Configures and starts four different timers.
 * @details PWM Timer - Outputs a PWM signal (2Hz, 30% duty cycle) on P3.0 LED0
 *          Continuous Timer - Outputs a continuous 300ms timer on P3.1 LED1 (GPIO toggles every 300ms)
 *          One Shot Timer - Starts a one shot time - P3.2 LED2 turns on when one shot time (1 sec) is complete
 *                           Press P54 SW1 to start the one shot timer again
 *          Counter Timer - Counts the number of falling edges on P0.4 up to 5 and then turns on P3.3 LED3
 *                          On the EV kit, connect P5.5 to P0.4 and use SW2 to change P0.4 input state
 * @note    For the counter timer, LED3 may appear to turn on/off sooner than 5 counts because of debounce
 *          on the switch creating extra edges/counts.
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
#include "mxc_sys.h"
#include "nvic_table.h"
#include "tmr.h"
#include "board.h"

/* **** Definitions **** */

/* **** Globals **** */

/* **** Functions **** */


/* ************************************************************************** */
int PWM_Output()
{
    int error = 0;

    uint32_t freq = 2;//Hz
    uint32_t dutyCycle = 30;//%
    int8_t inverted = FALSE;

    tmr32_cfg_pwm_t pwm_cfg;

    //Setup GPIO to timer output function
    sys_cfg_tmr_t gpio;
    gpio.port = 3;
    gpio.mask = PIN_0;
    gpio.func = GPIO_FUNC_TMR;
    gpio.pad = GPIO_PAD_OPEN_DRAIN;

    //initialize timer and GPIO
    tmr_prescale_t prescale = TMR_PRESCALE_DIV_2_12;
    error = TMR_Init(MXC_TMR0, prescale, &gpio);

    if(error != E_NO_ERROR)
        return error;

    //calculate the ticks values for frequency and duty cycle
    error = TMR32_GetPWMTicks(MXC_TMR0, dutyCycle, freq, &(pwm_cfg.dutyCount),&(pwm_cfg.periodCount));

    if(error != E_NO_ERROR)
        return error;

    //select the polarity
    if(inverted)
        pwm_cfg.polarity = TMR_PWM_INVERTED;
    else
        pwm_cfg.polarity = TMR_PWM_NONINVERTED;


    //configure and start the PWM timer
    TMR32_PWMConfig(MXC_TMR0, &pwm_cfg);
    TMR32_Start(MXC_TMR0);

    return error;
}

void ContinuousTimer_handler()
{
    TMR32_ClearFlag(MXC_TMR1);
}

int ContinuousTimer()
{
    int error = 0;
    tmr32_cfg_t cont_cfg;

    uint32_t IntervalTime = 300;//ms

    //enable timer interrupt
    NVIC_SetVector(TMR1_0_IRQn, ContinuousTimer_handler);
    TMR32_EnableINT(MXC_TMR1);

    //Setup GPIO to timer output function
    sys_cfg_tmr_t gpio;
    gpio.port = 3;
    gpio.mask = PIN_1;
    gpio.func = GPIO_FUNC_TMR;
    gpio.pad = GPIO_PAD_OPEN_DRAIN;

    //initialize timer and GPIO
    tmr_prescale_t prescale = TMR_PRESCALE_DIV_2_12;
    error = TMR_Init(MXC_TMR1, prescale, &gpio);

    if(error != E_NO_ERROR)
        return error;

    cont_cfg.mode = TMR32_MODE_CONTINUOUS;
    cont_cfg.polarity = TMR_POLARITY_INIT_LOW;	//start GPIO low

    //calculate the ticks values for given time
    error = TMR32_TimeToTicks(MXC_TMR1, IntervalTime, TMR_UNIT_MILLISEC, &(cont_cfg.compareCount));

    if(error != E_NO_ERROR)
        return error;

    //configure and start the timer
    TMR32_Config(MXC_TMR1, &cont_cfg);
    TMR32_Start(MXC_TMR1);

    return error;
}

/* ************************************************************************** */
void PB0_handler( void * pb_num)
{
    //start OneShot Timer if not already enabled
    if(TMR16_IsActive(MXC_TMR2,0) == 0) {
        //reset P32 LED
        LED_Off(2);
        TMR16_Start(MXC_TMR2,0);
        printf("oneshot start\n");
    }
}

/* ************************************************************************** */
void OneShot_handler()
{
    LED_On(2);
    TMR16_ClearFlag(MXC_TMR2,0);
    printf("oneshot complete\n");
}

/* ************************************************************************** */
int OneShotTimer()
{
    int error = 0;
    tmr16_cfg_t oneshot_cfg;

    uint32_t timeOut = 1000;//ms

    //Setup push button to start/stop one shot timer
    PB_RegisterCallback(0, PB0_handler);

    //setup timer interrupt to turn on P3.2 LED
    NVIC_SetVector(TMR2_0_IRQn, OneShot_handler);
    TMR16_EnableINT(MXC_TMR2,0);

    //initialize timer
    tmr_prescale_t prescale = TMR_PRESCALE_DIV_2_12;
    error = TMR_Init(MXC_TMR2, prescale, NULL);

    if(error != E_NO_ERROR)
        return error;

    oneshot_cfg.mode = TMR16_MODE_ONE_SHOT;
    error = TMR16_TimeToTicks(MXC_TMR2, timeOut, TMR_UNIT_MILLISEC, &(oneshot_cfg.compareCount));

    if(error != E_NO_ERROR)
        return error;

    //configure and start the timer
    TMR16_Config(MXC_TMR2, 0, &oneshot_cfg);
    TMR16_Start(MXC_TMR2, 0);

    printf("oneshot start\n");
    return error;
}

void PB1_handler( void * pb_num)
{
    printf("Count = %d\n", TMR32_GetCount(MXC_TMR4));
}

void Counter_handler()
{
    LED_Toggle(3);
    TMR32_ClearFlag(MXC_TMR4);
}

int CounterTimer()
{
    int error = 0;
    tmr32_cfg_t counter_cfg;

    uint32_t countLimit = 5;

    //Setup callback on button push to print out count value
    PB_RegisterCallback(1, PB1_handler);

    //enable timer interrupt
    NVIC_SetVector(TMR4_0_IRQn, Counter_handler);
    TMR32_EnableINT(MXC_TMR4);

    //Setup GPIO to timer intput function
    sys_cfg_tmr_t gpio;
    gpio.port = 0;
    gpio.mask = PIN_4;
    gpio.func = GPIO_FUNC_TMR;
    gpio.pad = GPIO_PAD_INPUT;

    //initialize timer and GPIO
    error = TMR_Init(MXC_TMR4, TMR_PRESCALE_DIV_2_0, &gpio);

    if(error != E_NO_ERROR)
        return error;

    counter_cfg.mode = TMR32_MODE_COUNTER;
    counter_cfg.polarity = TMR_POLARITY_FALLING_EDGE;
    counter_cfg.compareCount = countLimit;

    //configure and start the timer
    TMR32_Config(MXC_TMR4, &counter_cfg);
    TMR32_Start(MXC_TMR4);

    return error;
}

/* ************************************************************************** */
int main(void)
{
    printf("\n**************** Timer Demo ********************\n");
    printf("LED0 = Outputs a PWM signal (2Hz, 30%% duty cycle)\n");
    printf("LED1 = Outputs a Continuous timer (toggles every 300ms)\n");
    printf("LED2 = One Shot Timer (turns on after 1 second)\n");
    printf("LED3 = Count Timer (toggles every 5 counts - counts on P0.4 falling edge)\n");
    printf("SW1  = Start the one shot timer again\n");
    printf("SW2  = Connect P5.5 to P0.4 and use SW2 to change P0.4 input state for count timer\n");

    //Outputs a PWM signal (2Hz, 30% duty cycle) on P3.0 LED0
    PWM_Output();

    //Outputs a continuous 300ms timer on P3.1 LED1 (GPIO toggles every 300ms)
    ContinuousTimer();

    //Starts a one shot time - P3.2 LED2 turns on when one shot time (1 sec) is complete
    //Press P54 SW1 to start the one shot timer again
    OneShotTimer();

    //Counts the number of falling edges on P0.4 up to 5 and then turns on P3.3 LED3
    //On the EV kit, connect P5.5 to P0.4 and use SW2 to change P0.4 input state
    CounterTimer();

    while(1) {}
}
