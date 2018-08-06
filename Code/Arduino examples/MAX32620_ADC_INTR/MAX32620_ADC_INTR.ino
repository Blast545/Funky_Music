#include <stdio.h>
#include <stdint.h>
#include "mxc_config.h"
#include "max32620.h"
#include "lp.h"
#include "pmu.h"
#include "rtc.h"
#include "nvic_table.h"
#include "adc.h"

// Variable to track when an adc interrupt triggers 
volatile unsigned int adc_done = 0;
// Variable to get the adc data
uint16_t adc_data = 0;
// Data in volts: 5.5 -> Max voltage -> 1023 
float AIN0_DIV5_volts = 0.0f;

// Interrupt handler for the ADC interrupt
/*
void AFE_IRQHandler(void){
    //ADC_ClearFlags(MXC_F_ADC_INTR_ADC_DONE_IF);
    ADC_ClearFlags(ADC_IF_MASK);   
    adc_done = 1;
}
*/

// ADC ISR
void ADC_interrupt_routine(){
  //ADC_ClearFlags(MXC_F_ADC_INTR_ADC_DONE_IF);
  ADC_ClearFlags(ADC_IF_MASK);   
  adc_done = 1;  
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  /* Initialize ADC */
  ADC_Init();
  
  // Clear the ready adc interrupt flag
  ADC_ClearFlags(MXC_F_ADC_INTR_ADC_REF_READY_IF);
  //ADC_DisableINT(MXC_F_ADC_INTR_ADC_DONE_IE);
  ADC_EnableINT(MXC_F_ADC_INTR_ADC_DONE_IE);

  // setup interrupts
  // Do note that the following function triggers the callback routine
  NVIC_SetVector(AFE_IRQn, ADC_interrupt_routine);  
  //NVIC_EnableIRQ(AFE_IRQn);
  
  // Start a measurement, this is supposed to trigger the interrupts
  //ADC_StartConvert(ADC_CH_0_DIV_5, 0, 1);
  delay(100);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(adc_done){
    //Serial.println("here");

    // Get the data:
    if (ADC_GetData(&adc_data) == E_OVERFLOW) {
      // Full scale result
      adc_data = 1023;
    }
    // Convert the data to volts:
    AIN0_DIV5_volts = (adc_data * 5.5)/1023.0;
    //Serial.println(adc_data);
    Serial.println(AIN0_DIV5_volts);
  
    // This delay is used to avoid filling the serial monitor
    // with data too fast
    delay(100);

    // Trigger the next measurement
    adc_done = 0;
    ADC_StartConvert(ADC_CH_0_DIV_5, 0, 1);
  }
}
