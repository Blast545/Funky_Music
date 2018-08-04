
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
    
}

void loop() {
  Wire2.beginTransmission(0x48);
  //Wire2.write(PMIC_LED0);
  //Wire2.write(PMIC_LED_ON  + color );
  values[1] = PMIC_LED_ON + color;
  Wire2.write(values, 2);
  Wire2.endTransmission();

  color++;
  // Overflow condition, color can be controlled by 5 bits
  if(color==16) {
    color = 0;
    led_state = !led_state;
    digitalWrite(LED_BUILTIN, led_state); 

    Wire2.beginTransmission(0x48);
    Wire2.write(aux_vsys, 2);
    Wire2.endTransmission();
  }
  delay(150);                         // wait for a second

}

