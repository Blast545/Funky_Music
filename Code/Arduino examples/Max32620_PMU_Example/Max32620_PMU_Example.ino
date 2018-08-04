/* **** Includes **** */
#include <stdio.h>
#include <stdint.h>
#include "mxc_config.h"
#include "max32620.h"
#include "lp.h"
#include "pmu.h"
#include "rtc.h"
#include "nvic_table.h"

/* **** Definitions **** */
#define LP2_WAKEUP_TIME   5

#define GPIO3_VALUE_REG   MXC_BASE_GPIO+MXC_R_GPIO_OFFS_OUT_VAL_P3
#define RTC_INTFL_REG     MXC_BASE_RTCTMR + MXC_R_RTCTMR_OFFS_FLAGS
#define RTC_INTEN_REG     MXC_BASE_RTCTMR + MXC_R_RTCTMR_OFFS_INTEN

/* **** Globals **** */
static  uint32_t pmu_program[] = {
    PMU_WAIT(PMU_NO_INTERRUPT, PMU_NO_STOP, PMU_WAIT_SEL_0, 0, PMU_WAIT_IRQ_MASK2_SEL0_RTC_PRESCALE, 0), //wait for prescaler compare interrupt
    PMU_WRITE(PMU_NO_INTERRUPT, PMU_NO_STOP, PMU_WRITE_MASKED_WRITE_VALUE, RTC_INTFL_REG, MXC_F_RTC_FLAGS_PRESCALE_COMP, 0xffffffff),//clear flag

    PMU_WRITE(PMU_NO_INTERRUPT, PMU_NO_STOP, PMU_WRITE_MASKED_WRITE_VALUE, GPIO3_VALUE_REG, 0x0, 0b1111), //set  LED0 on (0)

    PMU_WAIT(PMU_NO_INTERRUPT, PMU_NO_STOP, PMU_WAIT_SEL_0, 0, PMU_WAIT_IRQ_MASK2_SEL0_RTC_PRESCALE, 0), //wait for prescale compare interrupt
    PMU_WRITE(PMU_NO_INTERRUPT, PMU_NO_STOP, PMU_WRITE_MASKED_WRITE_VALUE, RTC_INTFL_REG, MXC_F_RTC_FLAGS_PRESCALE_COMP, 0xffffffff),//clear flag

    PMU_WRITE(PMU_NO_INTERRUPT, PMU_NO_STOP, PMU_WRITE_MASKED_WRITE_VALUE, GPIO3_VALUE_REG, 0b1111, 0b1111), //set green LED0 off (1)

    PMU_JUMP(PMU_NO_INTERRUPT, PMU_NO_STOP, (uint32_t)(pmu_program)), //loop back to first PMU WAIT instruction
};

/* **** Functions **** */

/* ****************************************************************************/
void pause(void){
    unsigned int i;

    for (i = 0; i < 5000000; i++) {
        __NOP();
    }
}

/* ****************************************************************************/
void RTC_handler_Compare1(){
    //Disable and clear RTC compare1 interrupt
    RTC_DisableINT(MXC_F_RTC_INTEN_COMP1);
    RTC_ClearFlags(MXC_F_RTC_FLAGS_COMP1);
}

/* ****************************************************************************/
void RTC_Setup(){
    rtc_cfg_t RTCconfig;

    RTCconfig.compareCount[0] = 5;//5 second timer
    RTCconfig.compareCount[1] = 7;//7 second timer
    RTCconfig.prescaler = RTC_PRESCALE_DIV_2_12; //1Hz clock
    RTCconfig.prescalerMask = RTC_PRESCALE_DIV_2_11;//0.5s prescaler comp
    RTCconfig.snoozeCount = LP2_WAKEUP_TIME;
    RTCconfig.snoozeMode = RTC_SNOOZE_MODE_B;

    RTC_Init(&RTCconfig);

    //enable Prescaler compare interrupt
    RTC_EnableINT(MXC_F_RTC_INTEN_PRESCALE_COMP);

    //setup interrupts
    NVIC_SetVector(RTC1_IRQn, RTC_handler_Compare1);

    RTC_Start();
}

void setup() {
  pinMode(P3_0, OUTPUT);
  pinMode(P3_1, OUTPUT);
  pinMode(P3_2, OUTPUT);
  pinMode(P3_3, OUTPUT);
  //pinMode(P5_3, OUTPUT);
  

}

void loop() {
    //printf("\n**********LP2 PMU Demo***********\n");

    //configure RTC and start
    RTC_Setup();

    //start PMU - blinks LED0 every 1s (toggles every 0.5s)
    PMU_Start(0, pmu_program, NULL);

    while(1) {
        //set the snooze and enable compare 1
        RTC_Snooze();  //comp1 = time + 5
        RTC_EnableINT(MXC_F_RTC_INTEN_COMP1);

        //printf("Enter LP2\n");
        LP_EnterLP2();

        //printf("Woke up from LP2\n");

        //blink LED1, LED2 and LED3 in sequence
        //LED_On(1);
        pause();

        //LED_Off(1);
        //LED_On(2);
        pause();

        //LED_Off(2);
        //LED_On(3);
        pause();

        //LED_Off(3);
    }

}

