/*
  Arduino example port
*/


#include <stdio.h>
#include <stdint.h>
#include "mxc_config.h"
//#include "led.h"
#include "tmr_utils.h"
#include "i2cm.h"

/* **** Definitions **** */
#define I2C_MASTER          MXC_I2CM2
#define I2C_SPEED           I2CM_SPEED_400KHZ
#define PMIC_SLAVE_ADDR     0x48
#define PMIC_LED0      0x40
#define PMIC_LED1     0x41
#define PMIC_LED2     0x42
#define PMIC_LED_ON     0x40
#define PMIC_LED_OFF      0x00


int i;
uint8_t bright[2];
//int count = 0;
sys_cfg_i2cm_t i2cm_sys_cfg;
ioman_cfg_t io_cfg = IOMAN_I2CM2(1, 0);

void setup() {
  // put your setup code here, to run once:
  //printf("Hello World!\n");
  i2cm_sys_cfg.clk_scale = CLKMAN_SCALE_DIV_1;
  i2cm_sys_cfg.io_cfg = io_cfg;
  I2CM_Init(I2C_MASTER, &i2cm_sys_cfg, I2C_SPEED);

  // Additional configuration
  pinMode(P2_4, OUTPUT);
  pinMode(P2_5, OUTPUT);
  pinMode(P2_6, OUTPUT);
  
    
}

void loop() {
  // put your main code here, to run repeatedly:
  //LED_Off(2);
  //LED_On(0);
  digitalWrite(P2_6, LOW);
  digitalWrite(P2_4, HIGH);  
  
  bright[0] = PMIC_LED0;
  bright[1] = PMIC_LED_ON;
  I2CM_Write(I2C_MASTER, PMIC_SLAVE_ADDR, NULL, 0, bright, 2);
  for(i=0; i<16; i++) {
    bright[0] = PMIC_LED2;
    bright[1] = PMIC_LED_ON + 15 - i;
    I2CM_Write(I2C_MASTER, PMIC_SLAVE_ADDR, NULL, 0, bright, 2);
    bright[0] = PMIC_LED0;
    bright[1] = PMIC_LED_ON + i;
    I2CM_Write(I2C_MASTER, PMIC_SLAVE_ADDR, NULL, 0, bright, 2);
    TMR_Delay(MXC_TMR0, MSEC(25));
  }
  bright[0] = PMIC_LED2;
  bright[1] = PMIC_LED_OFF;
  I2CM_Write(I2C_MASTER, PMIC_SLAVE_ADDR, NULL, 0, bright, 2);
  TMR_Delay(MXC_TMR0, MSEC(100));
  
  //LED_Off(0);
  //LED_On(1);
  digitalWrite(P2_4, LOW);
  digitalWrite(P2_5, HIGH);
  
  bright[0] = PMIC_LED1;
  bright[1] = PMIC_LED_ON;
  I2CM_Write(I2C_MASTER, PMIC_SLAVE_ADDR, NULL, 0, bright, 2);
  for(i=0; i<16; i++) {
    bright[0] = PMIC_LED0;
    bright[1] = PMIC_LED_ON + 15 - i;
    I2CM_Write(I2C_MASTER, PMIC_SLAVE_ADDR, NULL, 0, bright, 2);
    bright[0] = PMIC_LED1;
    bright[1] = PMIC_LED_ON + i;
    I2CM_Write(I2C_MASTER, PMIC_SLAVE_ADDR, NULL, 0, bright, 2);
    TMR_Delay(MXC_TMR0, MSEC(25));
  }
  bright[0] = PMIC_LED0;
  bright[1] = PMIC_LED_OFF;
  I2CM_Write(I2C_MASTER, PMIC_SLAVE_ADDR, NULL, 0, bright, 2);
  TMR_Delay(MXC_TMR0, MSEC(100));
  
  //LED_Off(1);
  //LED_On(2);
  digitalWrite(P2_5, LOW);
  digitalWrite(P2_6, HIGH);
  bright[0] = PMIC_LED2;
  bright[1] = PMIC_LED_ON;
  I2CM_Write(I2C_MASTER, PMIC_SLAVE_ADDR, NULL, 0, bright, 2);
  for(i=0; i<16; i++) {
    bright[0] = PMIC_LED1;
    bright[1] = PMIC_LED_ON + 15 - i;
      I2CM_Write(I2C_MASTER, PMIC_SLAVE_ADDR, NULL, 0, bright, 2);
    bright[0] = PMIC_LED2;
    bright[1] = PMIC_LED_ON + i;
      I2CM_Write(I2C_MASTER, PMIC_SLAVE_ADDR, NULL, 0, bright, 2);
      TMR_Delay(MXC_TMR0, MSEC(25));
  }
  bright[0] = PMIC_LED1;
  bright[1] = PMIC_LED_OFF;
  I2CM_Write(I2C_MASTER, PMIC_SLAVE_ADDR, NULL, 0, bright, 2);
  TMR_Delay(MXC_TMR0, MSEC(100));
  //printf("count = %d\n", count++);

}



