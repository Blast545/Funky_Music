#include <stdio.h>
#include <stdint.h>
#include "mxc_config.h"
#include "max32620.h"
#include "lp.h"
#include "pmu.h"
#include "rtc.h"
#include "nvic_table.h"
#include "adc.h"

volatile unsigned int adc_done = 0;

// Interrupt handler for the ADC interrupt
void AFE_IRQHandler(void){
    //ADC_ClearFlags(MXC_F_ADC_INTR_ADC_DONE_IF);
    ADC_ClearFlags(ADC_IF_MASK);   
    adc_done = 1;
}


void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  /* Initialize ADC */
  ADC_Init();
  Serial.println("Test message~: ");

    // Clear the ready adc interrupt flag
  ADC_ClearFlags(MXC_F_ADC_INTR_ADC_REF_READY_IF);
  ADC_DisableINT(MXC_F_ADC_INTR_ADC_DONE_IE);
  delay(500);

  NVIC_EnableIRQ(AFE_IRQn);
  delay(1000);
  
  Serial.println("Test: ");

  // Start a measurement, this is supposed to trigger the interrupts
  ADC_StartConvert(ADC_CH_0_DIV_5, 0, 1);

}

void loop() {
  // put your main code here, to run repeatedly:
  if(adc_done){
    Serial.println("here");
    delay(800);
    adc_done = 0;
    ADC_StartConvert(ADC_CH_0_DIV_5, 0, 1);
  }
}
