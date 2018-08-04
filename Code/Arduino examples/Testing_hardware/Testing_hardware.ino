
// Communicate via I2C to change the SYS REG
// Default voltage (measured) is 4.475V
// Change it to 

#include <Wire.h>

byte color = 0;
#define PMIC_LED0     0x40
#define PMIC_LED_ON      0x40
#define VSYS_REG 0x1B

byte values[2] = {PMIC_LED0, 0};
byte aux_vsys[2] = {VSYS_REG, 0x1f};
bool led_state = true;

void setup() {

  // I2C library start
  Wire2.begin();

  // Delay half a second
  delay(500);
  
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, led_state); 

  // Change SYS voltage to maximum (4.8V)  
  Wire2.beginTransmission(0x48);
  Wire2.write(aux_vsys, 2);
  Wire2.endTransmission();

  //Init all the pins of the board as outputs (except A0)
  pinMode(P3_2, OUTPUT);
  pinMode(P3_3, OUTPUT);
  pinMode(P5_3, OUTPUT);
  pinMode(P5_0, OUTPUT);
  pinMode(P5_1, OUTPUT);
  pinMode(P5_2, OUTPUT);
  pinMode(P3_0, OUTPUT);
  pinMode(P3_1, OUTPUT);
  pinMode(P3_4, OUTPUT);
  pinMode(P3_5, OUTPUT);
  
  //digitalWrite(LED_BUILTIN, led_state);


    
}

void loop() {

  digitalWrite(P3_2, !led_state);
  digitalWrite(P3_3, led_state);
  digitalWrite(P5_3, !led_state);

  digitalWrite(P5_0, led_state);
  digitalWrite(P5_1, !led_state);
  digitalWrite(P5_2, led_state);
  digitalWrite(P3_0, !led_state);
  digitalWrite(P3_1, led_state);
  digitalWrite(P3_4, !led_state);
  digitalWrite(P3_5, led_state);
  
  digitalWrite(LED_BUILTIN, led_state);
  
  
  led_state = !led_state;
  delay(2000);                         // wait for a second

}

