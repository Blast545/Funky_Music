/*
 * Program to get ADC info and turn on lights based on the rhythms
 * @author: Blast_545
 * @email: j.j.perez13@hotmail.comm
 * 
*/

/* **** Includes **** */
#include <stdio.h>
#include <stdint.h>
#include "mxc_config.h"
#include "max32620.h"
#include "lp.h"
#include "pmu.h"
#include "rtc.h"
#include "nvic_table.h"
#include "adc.h"

// Define needed for the CMSIS core
//#define ARM_MATH_CM4
#include "arm_math.h"
#include "arm_const_structs.h"

#include <Wire.h>

/* **** Definitions **** */
#define ADC_INT_REG     MXC_BASE_ADC + MXC_R_ADC_OFFS_INTR
#define ADC_CTRL_REG    MXC_BASE_ADC + MXC_R_ADC_OFFS_CTRL
#define ADC_DATA_REG    MXC_BASE_ADC + MXC_R_ADC_OFFS_DATA
#define VSYS_REG 0x1B

// Number of samples to take before triggering a pmu interrupt
#define AMOUNT_SAMPLES 256

// Definitions used to control the different leds on the board:
// Sorted in the code, as they appear sorted left to right resistors
#define BLUE_LED_DOWN         P5_3
#define BLUE_LED_UP           P3_3

#define GREEN_LED_DOWN        P3_2
#define GREEN_LED_UP          P5_0

#define WHITE_LED_DOWN        P5_1
#define WHITE_LED_UP          P5_2

#define ORANGE_LED_DOWN       P3_0
#define ORANGE_LED_UP         P3_1

#define RED_LED_DOWN          P3_5
#define RED_LED_UP            P3_4

#define BUILTIN_RED           P2_4
#define BUILTIN_GREEN         P2_5
#define BUILTIN_BLUE          P2_6
#define BOOT_BUTTON           P2_7

// Times constant used to compare using the millis function
#define SECOND 1000
#define MINUTE 60*SECOND

// Modes used in different parts of the program
#define IDLE_MODE 0
#define FUNKY_MUSIC_MODE 1
#define CALIBRATION_MODE 2
#define POWER_OFF_MODE 3 // Getting in this mode, turns off the board
#define ARMONICS_TEST_MODE 4 // Test mode

// Times required to hold the boot button in order to change mode
/*
#define TIME_FUNKY 2*SECOND
#define TIME_CALIBRATION 4*SECOND
#define TIME_POWER_OFF 8*SECOND
#define TIME_ARMONICS_TEST 10*SECOND
*/
#define TIME_FUNKY 2*SECOND
#define TIME_CALIBRATION 3*SECOND
#define TIME_POWER_OFF 4*SECOND
#define TIME_ARMONICS_TEST 5*SECOND

// Times used for a idle led ping
#define TIME_LED_ON_IDLE 1*SECOND
#define TIME_LED_OFF_IDLE 4*SECOND
#define TIME_IDLE_SEQUENCE_TOTAL TIME_LED_ON_IDLE+TIME_LED_OFF_IDLE

/* Use two different modes, one with 10 singular lights
 One with 5 "coupled" lights
 Comment the following line in order to use the "single use" lights mode
 */
#define COUPLED_MODE 1
/* Use a "in house mode" to rescan the signal if the first threshold did not give signals*/
#define IN_HOUSE_MODE 1
/* A define used to enable/disable the Serial comm messages
 Change to #undef if serial is not required */
#define DEBUG_MODE 1

/* Enable/disable serial port communication*/
#ifdef DEBUG_MODE
#define DEBUG_CMD(cmd) cmd
#else
#define DEBUG_CMD(cmd)
#endif

/* **** Globals **** */
unsigned char current_mode = 0;
unsigned long last_time_led_idle = millis();

// Variables used to perform the DSP processing
float32_t process_buffer[AMOUNT_SAMPLES];
float32_t fft_result[AMOUNT_SAMPLES];
float32_t fft_result_mag[AMOUNT_SAMPLES/2];
uint16_t fftSize = AMOUNT_SAMPLES;
static arm_rfft_fast_instance_f32  fft_instance;

// Frequency bands RMS 
float32_t bands[10] = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f,  
                      0.0f, 0.0f, 0.0f, 0.0f, 0.0f};
                      
/* Values that hold the average of the external signal
   Used to calibrate the "sound" of the surroundings */
float32_t env_bias[10] =  {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 
                      0.0f, 0.0f, 0.0f, 0.0f, 0.0f};                    

/* Constant values used to output music with the bands
 * if current value - avg > threshold, turn on led
*/
const float32_t threshold_values[10] = { 10, 10, 
                                        10, 10, 
                                        10, 10, 
                                        10, 10, 
                                        10, 10};  
                                        
/* Constant values used to help the algorithm when the sound is low*/                                        
const float32_t threshold_halves[10] = { 3, 3, 
                                         3, 3, 
                                         3, 3, 
                                         3, 3, 
                                         3, 3};

/* Array with the leds ports used 
   Sorted in array form to be able of iterating over them in order to avoid repeating code
*/                                        
const int music_leds_array[10] = {BLUE_LED_DOWN, BLUE_LED_UP,
                                 GREEN_LED_DOWN, GREEN_LED_UP,
                                 WHITE_LED_DOWN, WHITE_LED_UP,
                                 ORANGE_LED_DOWN, ORANGE_LED_UP,
                                 RED_LED_DOWN, RED_LED_UP};
                                                                 
// Array of sampled data
uint32_t adc_acquired_data[AMOUNT_SAMPLES];

// ADC data, taken from an interrupt routine 
int16_t adc_buffer[AMOUNT_SAMPLES];
volatile unsigned int adc_done = 0;
volatile uint16_t counts = 0;

// Constant used to change V_SYS voltage to 4.8V
const byte aux_vsys[2] = {VSYS_REG, 0x1f};

/* Number of 32 bits words:
// MOVE: 3 = OP + W_Address + R_Address
// WAIT: 4 = OP + Mask1 + Mask2 + Wait_Count
// WRITE: 4 = OP + W_Address + W_Value + W_Mask 
// JUMP: 2 = ¨OP + NEXT_ADDRESS
// LOOP: 2 = OP + NEXT_ADDRESS
// POLL: 5 = OP + POLL_ADDRESS + Data_Expected + Data_Mask + Poll_intervals 
// BRANCH: 5 = OP + POLL_ADDRESS + Data_Expected + Data_Mask + BRANCH_NEXT_ADDRESS
// TRANSFER: 4 = OP + W_ADDRESS + R_Address + Int_Mask
*/

uint32_t pmu_program[] = {    
  // Trigger ADC conversion:
  PMU_WRITE(PMU_NO_INTERRUPT, PMU_NO_STOP, PMU_WRITE_MASKED_WRITE_VALUE, ADC_CTRL_REG, MXC_F_ADC_CTRL_CPU_ADC_START, MXC_F_ADC_CTRL_CPU_ADC_START), 
  // Wait for ADC Done interrupt
  PMU_WAIT(PMU_NO_INTERRUPT, PMU_NO_STOP, PMU_WAIT_SEL_0, PMU_WAIT_IRQ_MASK1_SEL0_ADC_DONE, 0, 0), 
    // Clear interrupt ADC_DONE flag, to re enable module 
  PMU_WRITE(PMU_NO_INTERRUPT, PMU_NO_STOP, PMU_WRITE_MASKED_WRITE_VALUE, ADC_INT_REG, MXC_F_ADC_INTR_ADC_DONE_IF, MXC_F_ADC_INTR_ADC_DONE_IF),
  // Move ADC data to memory
  PMU_MOVE(PMU_NO_INTERRUPT, PMU_NO_STOP, PMU_MOVE_READ_32_BIT, PMU_MOVE_READ_NO_INC, PMU_MOVE_WRITE_32_BIT, PMU_MOVE_WRITE_NO_INC, PMU_MOVE_NO_CONT, 4, (uint32_t)&(adc_acquired_data[0]), ADC_DATA_REG),
  // Increase the ADC data pointer by 4 (4 bytes increase), directly in the sequence WRITE instruction
  // 4 bytes for write1, 4 bytes for wait1, 4bytes for write2, 2bytes for value offset from move1
  PMU_WRITE(PMU_NO_INTERRUPT, PMU_NO_STOP, PMU_WRITE_PLUS_1, (uint32_t)&(pmu_program[13]), 0, 0),
  PMU_WRITE(PMU_NO_INTERRUPT, PMU_NO_STOP, PMU_WRITE_PLUS_1, (uint32_t)&(pmu_program[13]), 0, 0),
  PMU_WRITE(PMU_NO_INTERRUPT, PMU_NO_STOP, PMU_WRITE_PLUS_1, (uint32_t)&(pmu_program[13]), 0, 0),
  PMU_WRITE(PMU_NO_INTERRUPT, PMU_NO_STOP, PMU_WRITE_PLUS_1, (uint32_t)&(pmu_program[13]), 0, 0),
  // Loop instruction, loop for the number of "SAMPLES" required to perform the fourier transform
  // Use counter 0, it has to be loaded with the number of samples before starting the program
  PMU_LOOP(PMU_INTERRUPT, PMU_NO_STOP, 0, (uint32_t)&(pmu_program[0])),  
  // If the number of samples has been taken, then restart the index pointer
  PMU_WRITE(PMU_NO_INTERRUPT, PMU_NO_STOP, PMU_WRITE_MASKED_WRITE_VALUE, (uint32_t)&(pmu_program[13]), (uint32_t)&(adc_acquired_data[0]), 0xffffffff),
  // Repeat the loop forever
  PMU_JUMP(PMU_NO_INTERRUPT, PMU_NO_STOP, (uint32_t)(pmu_program)),
};

// Get here once that the pmu triggers an interrupt
void PMU_IRQ_Handler(void) {  
  // PMU_Handler function calls the function callbacks triggered
  // And clears the needed flags
  PMU_Handler();    
}

/* ADC interrupt function
 Sets a boolean to start with the program core */
void Process_ADC_Data(int err){  
  counts++;
  adc_done=1;
}

/* ****************************************************************************/
void setup() {
  // Configure the Serial port communication, only used if debug is enabled
  DEBUG_CMD(Serial.begin(115200);)
  
  // Keep the device ON if connected to power using a battery
  pinMode(P2_2, OUTPUT);
  digitalWrite(P2_2, HIGH);
  
  // Init I2C module, and configure VSYS voltage to 4.8V
  Wire2.begin();
  // Change SYS voltage to maximum (4.8V)  
  Wire2.beginTransmission(0x48);
  Wire2.write(aux_vsys, 2);
  Wire2.endTransmission();
  
  // Configure all the outputs
  pinMode(BLUE_LED_DOWN, OUTPUT);
  pinMode(BLUE_LED_UP, OUTPUT);
  pinMode(GREEN_LED_DOWN, OUTPUT);
  pinMode(GREEN_LED_UP, OUTPUT);
  pinMode(WHITE_LED_DOWN, OUTPUT);
  pinMode(WHITE_LED_UP, OUTPUT);
  pinMode(ORANGE_LED_DOWN, OUTPUT);
  pinMode(ORANGE_LED_UP, OUTPUT);
  pinMode(RED_LED_DOWN, OUTPUT);
  pinMode(RED_LED_UP, OUTPUT);

  digitalWrite(BLUE_LED_DOWN, LOW);
  digitalWrite(BLUE_LED_UP, LOW);
  digitalWrite(GREEN_LED_DOWN, LOW);
  digitalWrite(GREEN_LED_UP, LOW);
  digitalWrite(WHITE_LED_DOWN, LOW);
  digitalWrite(WHITE_LED_UP, LOW);
  digitalWrite(ORANGE_LED_DOWN, LOW);
  digitalWrite(ORANGE_LED_UP, LOW);
  digitalWrite(RED_LED_DOWN, LOW);
  digitalWrite(RED_LED_UP, LOW);  
  
  //pinMode(LED_BUILTIN, OUTPUT); //builtin = P
  pinMode(BUILTIN_RED, OUTPUT);
  pinMode(BUILTIN_GREEN, OUTPUT);
  pinMode(BUILTIN_BLUE, OUTPUT);
  pinMode(BOOT_BUTTON, INPUT_PULLUP); // Used to select mode
  
  /* Initialize ADC */
  ADC_Init();
  // Use a dummy conversion to configure reading channel 0 / 5
  ADC_StartConvert(ADC_CH_0_DIV_5, 0, 1);
        
  // Enable PMU interrupts
  NVIC_SetVector(PMU_IRQn, PMU_IRQ_Handler);   
  delay(100);
  // Set flag to 0
  adc_done = 0;
  
  // Initialize the samples array to zero:
  memset(adc_buffer, 0, 4*AMOUNT_SAMPLES);
    // Load PMU0 Counter0 to acquire the number of samples
  PMU_SetCounter(0, 0, AMOUNT_SAMPLES-1);
  
  // Initialize the fft core
  arm_status status;
  status = arm_rfft_fast_init_f32(	&fft_instance, fftSize);  
  DEBUG_CMD(Serial.print("Status: "); Serial.println(status);)

  // Start the PMU free run adc acquisition
  PMU_Start(0, pmu_program, Process_ADC_Data); 
  
}

void loop() {
  // Based on the current mode, proceed with a diferent task    
  if(current_mode == FUNKY_MUSIC_MODE) mainProcessloop();
  else if(current_mode == IDLE_MODE) idleModeOperation();
  else if(current_mode == CALIBRATION_MODE) calibrateVariables();
  else if(current_mode == POWER_OFF_MODE) powerOff();
  else if(current_mode == ARMONICS_TEST_MODE) armonicsTest();
  
  // Check if boot button is pressed, is pressed, proceed selecting mode
  if( !digitalRead(BOOT_BUTTON) ) selectOperationMode();
}

// Function used to select between different modes
void selectOperationMode(void){
  unsigned long currentTime = millis();  
  current_mode = 0;
  last_time_led_idle = millis();
  DEBUG_CMD(Serial.println( "Activated Idle Mode" );)
  digitalWrite(BUILTIN_GREEN, LOW);
  
  // Stay on this loop if the button is pressed
  while( !digitalRead(BOOT_BUTTON) ){
    // Output different modes
    if( (current_mode==0) && (millis() - currentTime > TIME_FUNKY  )  ){
      current_mode = 1;
      digitalWrite(BUILTIN_GREEN, HIGH);
      DEBUG_CMD(Serial.println( "Activated Funky mode" );)
      digitalWrite(BUILTIN_RED, LOW);   
      digitalWrite(BUILTIN_BLUE, LOW);       
    }

    // Output different modes
    if( (current_mode==1) && (millis() - currentTime > TIME_CALIBRATION  )  ){
      current_mode = 2;
      digitalWrite(BUILTIN_RED, HIGH);
      DEBUG_CMD(Serial.println( "Activated calibration mode" );)             
    }
    // Output different modes
    if( (current_mode==2) && (millis() - currentTime > TIME_POWER_OFF  )  ){
      current_mode = 3;
      digitalWrite(BUILTIN_BLUE, HIGH);
      DEBUG_CMD(Serial.println( "Activated Power off sequence" );)     
      digitalWrite(BUILTIN_RED, LOW);
    }
    
    /* Special mode, used for testing */
    if( (current_mode==3) && (millis() - currentTime > TIME_ARMONICS_TEST  )  ){
      current_mode = 4;
      digitalWrite(BUILTIN_GREEN, LOW);
      DEBUG_CMD(Serial.println( "Activated armonics test" );)     
    }
    
    
  }
  // After selecting mode, delay system execution 100ms to avoid debounce
  delay(100);  
  // Turn off the three leds
  digitalWrite(BUILTIN_RED, HIGH);
  digitalWrite(BUILTIN_GREEN, HIGH);
  digitalWrite(BUILTIN_BLUE, HIGH);  
}

/* Main system task, proceeds by 
 * 1. Move the ADC data from input buffer, to aux buffer 
 * 2. fft of the adc data
 * 3. Gets magnitude of the fft obtained
 * 4. Gets RMS value of the elements, based on different indexes (bands)
 * 5. For each, the RMS value is converted to a logarithmic scale  
 * 6. Subtract this value from the average of the environment (calibration needed)
 * 7. Based on thresholds assigned, turn on or off the assigned leds to each band
  Steps 1-5 are completed with the updateSoundsBands (to be used when calibrating)
 */
void mainProcessloop(void){
  // Once a set of data has been acquired, process it
  if(adc_done){
    
    updateSoundBands();
    
    // Keep a count of the leds that were turned on
    int leds_on = 0;
    
    #ifdef COUPLED_MODE
    // If coupled mode, turn lights in pairs
    for(int i=0; i<5; i++){
      float32_t aux_difference = bands[i]-env_bias[i];
      // Serial.print(aux_difference, 2); Serial.print(" ");
      // Check if the current value requires a change in the output
      if(aux_difference > threshold_values[i]){
        // Turn on
        digitalWrite(music_leds_array[2*i], HIGH);
        digitalWrite(music_leds_array[2*i+1], HIGH);
        leds_on++;
      }
      else{
        // Turn off
        digitalWrite(music_leds_array[2*i], LOW);
        digitalWrite(music_leds_array[2*i+1], LOW);
      }
    }
    
    #else
    // If coupled mode, turn lights individually
    for(int i=0; i<10; i++){
      float32_t aux_difference = bands[i]-env_bias[i];
      // Serial.print(aux_difference, 2); Serial.print(" ");
      // Check if the current value requires a change in the output
      if(aux_difference > threshold_values[i]){
        // Turn on
        digitalWrite(music_leds_array[i], HIGH);
        leds_on++;
      }
      else{
        // Turn off
        digitalWrite(music_leds_array[i], LOW);
      }
    }
    #endif  
    
    /* If only a led (or none) is on, test again with a lower threshold 
       Only for the "in house" mode
    */
    #ifdef IN_HOUSE_MODE    
    #ifdef COUPLED_MODE
    if(leds_on<1){ 
      for(int i=0; i<5; i++){
        float32_t aux_difference = bands[i]-env_bias[i];
        if(aux_difference > threshold_halves[i]){
          // Turn on
          digitalWrite(music_leds_array[2*i], HIGH);
          digitalWrite(music_leds_array[2*i+1], HIGH);
        }
        else{
          // Turn off
          digitalWrite(music_leds_array[2*i], LOW);
          digitalWrite(music_leds_array[2*i+1], LOW);
        }
      }
    }    
    #else
    if(leds_on<1){ 
      for(int i=0; i<10; i++){
        float32_t aux_difference = bands[i]-env_bias[i];
        if(aux_difference > threshold_halves[i]){
          // Turn on
          digitalWrite(music_leds_array[i], HIGH);
        }
        else{
          // Turn off
          digitalWrite(music_leds_array[i], LOW);
        }
      }
    }      
    #endif
    #endif
    
    /*
    Serial.print(bands[0]-env_bias[0], 2); Serial.print(" ");    
    Serial.print(bands[1]-env_bias[1], 2); Serial.print(" ");    
    Serial.print(bands[2]-env_bias[2], 2); Serial.print(" ");
    Serial.print(bands[3]-env_bias[3], 2); Serial.print(" ");
    Serial.print(bands[4]-env_bias[4], 2); Serial.print(" ");
    Serial.print(bands[5]-env_bias[5], 2); Serial.print(" ");
    Serial.print(bands[6]-env_bias[6], 2); Serial.print(" ");
    Serial.print(bands[7]-env_bias[7], 2); Serial.println(" ");
    */    
    // Allow the system to process the next set of data
    adc_done = 0;
  }  
}

/* Basically blinks a led every certain amount of time
 This method should be changed for a sleep mode of the max32620 */
void idleModeOperation(void){
  
  if( millis() - last_time_led_idle > TIME_LED_ON_IDLE){
    digitalWrite(P2_5, HIGH);        
  }
  
  if( millis() - last_time_led_idle > TIME_IDLE_SEQUENCE_TOTAL){
    // Turn of the leds, in case these are ON 
    turnOffLeds();
    digitalWrite(P2_5, LOW);
    last_time_led_idle = millis();    
  }
}

/* Method used to calibrate the default sound of the environment
 * Takes 50 sets of measurements, 
*/
void calibrateVariables(){
  float32_t auxBias[10];
  // Erase previously saved values
  for(int i=0; i<10; i++){
    auxBias[i] = 0.0f;
  }
  
  /* For some reason, the first two set of variables are not valid
     Discard the 5 first set of values to avoid possible errors
  */
  for(int i = 0; i<55; i++){
    // Wait until a new set of value is obtained
    while(!adc_done);
    
    // Process the new set of data
    updateSoundBands();
    
    // Save the values to the array of bias
    if(i>4){
      #ifdef COUPLED_MODE
      for(int j = 0; j<5; j++){
        auxBias[j] = auxBias[j] + bands[j]; 
      }
      #else
      for(int j = 0; j<10; j++){
        auxBias[j] = auxBias[j] + bands[j]; 
      }
      #endif
    }
    
    // Allow the system to process the next set of data
    adc_done = 0;
  }  
  
  // After 50 iterations, divide the bias values by 50
  delay(100);
  Serial.println("New average values");  
  #ifdef COUPLED_MODE
  for(int k = 0; k <5; k++){    
    env_bias[k] = auxBias[k]/50; 
    Serial.print("Band "); Serial.print(k); Serial.print(": ");
    Serial.println(env_bias[k], 2);
  }
  #else
  for(int k = 0; k <10; k++){    
    env_bias[k] = auxBias[k]/50; 
    Serial.print("Band "); Serial.print(k); Serial.print(": ");
    Serial.println(env_bias[k], 2);
  }
  #endif
  
  // Restore the system to IDLE mode
  current_mode = IDLE_MODE;
  last_time_led_idle = millis();
}

/* Method used to power off the system */
void powerOff(){
  Serial.println("Powering off...");
  digitalWrite(P2_2, LOW);
  // If using USB as power, output some text to the serial port
  // blink a red led, and continue  
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  current_mode = IDLE_MODE;
  last_time_led_idle = millis();
  Serial.println("Leaving power off...");
}

/* Mode used to print in the screen the frequency bands, 
   specifically, it is output the changes in armonics content
   measured from 300 samples. "Change" is measured as the current
   magnitude, substracted from the previous magnitude*/
void armonicsTest(){
  float32_t aux_change[AMOUNT_SAMPLES/2];
  float32_t previous[AMOUNT_SAMPLES/2];
  float32_t aux_total[AMOUNT_SAMPLES/2];
  Serial.println("Starting armonics test");
  
  // Init the arrays to 0
  for(int i=0; i<(AMOUNT_SAMPLES/2); i++){
    aux_change[i] = 0.0f;
    previous[i] = 0.0f;
    aux_total[i] = 0.0f;
  }
  
  /* For some reason, the first two set of variables are not valid
     Discard the 5 first set of values to avoid possible errors
  */
  for(int i = 0; i<305; i++){
    // Wait until a new set of value is obtained
    while(!adc_done);
    
    // Process the new set of data
    updateSoundBands();
    
    // Save the current complex value to the current array
    
    
    if(i>4){      
      // Substract the current value vs the previous
      arm_sub_f32	(	previous, fft_result_mag, aux_change, AMOUNT_SAMPLES/2);	
      // Get the absolute value of this change
      arm_abs_f32	(	aux_change, aux_change, AMOUNT_SAMPLES/2);	      
      // Add it to a "total" vector
      arm_add_f32	(	aux_change, aux_total, aux_total, AMOUNT_SAMPLES/2);		
      
      /*
      for(int j = 0; j <8; j++){        
        auxBias[j] = auxBias[j] + bands[j]; 
        //Serial.print(auxBias[j], 2); Serial.print(" ");
        //Serial.print(bands[j], 2); Serial.print(" ");
      }
      */      
    }
    
    //Serial.println(" ");
    
    
    /* Save the processed complex vector as "previous"
       Number of bytes is AMOUNT_SAMPLES/2*4*/
    memcpy ( previous, fft_result_mag, AMOUNT_SAMPLES*2);
    
    // Allow the system to process the next set of data
    adc_done = 0;
  }  
  
  // Print the array of measured changes
  // Init the arrays to 0
  for(int i=0; i<(AMOUNT_SAMPLES/2); i++){
    if(i%8==0) Serial.println("");
    Serial.print(aux_total[i]); Serial.print(" ");
  }  
  
    // Restore the system to IDLE mode
  current_mode = IDLE_MODE;
  last_time_led_idle = millis();
  
}

/* Operates on the adc data to obtain magnitude of sound 
   perceived in different bands */
void updateSoundBands(void){
      // Move the data to the processing buffer
    for(int i = 0; i<AMOUNT_SAMPLES; i++){
      // 5.5/1023.0 = 0.00537634408602150537634408602151
      process_buffer[i] = (float) adc_acquired_data[i];// * 0.005376344086;   
      process_buffer[i] *= 0.005376344086; 
    }    
    
    // Init the RFFT system
    //arm_rfft_fast_f32(&arm_rfft_fast_sR_f32_len2048, process_buffer, fft_result, 0);
    arm_rfft_fast_f32(&fft_instance, process_buffer, fft_result, 0);    
    arm_cmplx_mag_f32(fft_result, fft_result_mag, fftSize);
    
    /* Calculate RMS of frequency bands 
       Play with the ranges of bands used for each led
       This was calculated by using printing functions on different stages 
       of the program, and testing, so can be changed for your needs.
       If FFT size is changed, these indexes have to be changed accordingly
    */    
    #ifdef COUPLED_MODE
    arm_rms_f32	(	&fft_result_mag[10], 14, &bands[0]);
    arm_rms_f32	(	&fft_result_mag[24], 24, &bands[1]);
    arm_rms_f32	(	&fft_result_mag[48], 46, &bands[2]);
    arm_rms_f32	(	&fft_result_mag[94], 46, &bands[3]);   
    arm_rms_f32	(	&fft_result_mag[210], 70, &bands[4]); 
    #else
    arm_rms_f32	(	&fft_result_mag[10], 7, &bands[0]);
    arm_rms_f32	(	&fft_result_mag[17], 7, &bands[1]);
    arm_rms_f32	(	&fft_result_mag[24], 12, &bands[2]);
    arm_rms_f32	(	&fft_result_mag[36], 12, &bands[3]);
    arm_rms_f32	(	&fft_result_mag[48], 23, &bands[4]);
    arm_rms_f32	(	&fft_result_mag[71], 23, &bands[5]);
    arm_rms_f32	(	&fft_result_mag[94], 23, &bands[6]);   
    arm_rms_f32	(	&fft_result_mag[117], 23, &bands[7]);
    arm_rms_f32	(	&fft_result_mag[210], 23, &bands[8]); 
    arm_rms_f32	(	&fft_result_mag[233], 47, &bands[9]);
    #endif
        
    // Apply log scale to the results
    bands[0] = 16 * log2(bands[0]);
    bands[1] = 16 * log2(bands[1]);
    bands[2] = 16 * log2(bands[2]);
    bands[3] = 16 * log2(bands[3]);
    bands[4] = 16 * log2(bands[4]);    
    #ifndef COUPLED_MODE
    bands[5] = 16 * log2(bands[5]);
    bands[6] = 16 * log2(bands[6]);
    bands[7] = 16 * log2(bands[7]);
    bands[8] = 16 * log2(bands[8]);
    bands[9] = 16 * log2(bands[9]);
    #endif
}

/* Function used to turn off the funky leds*/
void turnOffLeds(){
  for (int i=0; i<10; i++) digitalWrite(music_leds_array[i], LOW); 
}

// Functions used for debugging purposes, can be removed any time 
/* Function used for debug purposes, to check that the pmu program is 
/ re writing itself properly
/ Takes as input the number of double words to read from the array */
void printPmuProgram(int wordsNumber){
  Serial.print("Words: "); Serial.println(wordsNumber); 
  for (int i=0; i<wordsNumber; i++){
    Serial.println(pmu_program[i], HEX);
  }
}

// Print ADC acquired data, debug purposes 
void printAdcData(int wordsNumber){
  Serial.print("Samples: "); Serial.println(wordsNumber); 
  for (int i=0; i<wordsNumber; i++){
    Serial.print(adc_acquired_data[i], HEX);
    Serial.print(" ");
    //Serial.println(i);
  }
  Serial.println();
}

// Print float data to process, debug purposes 
void printBufferData(int wordsNumber){
  Serial.print("Buffer signal: "); Serial.println(wordsNumber); 
  for (int i=0; i<wordsNumber; i++){
    Serial.print(process_buffer[i], 3);
    Serial.print(" ");
  }
  Serial.println();
}

// Print FFT data, debug purposes 
void printFFTData(int wordsNumber){
  Serial.print("FFT: "); Serial.println(wordsNumber); 
  for (int i=0; i<wordsNumber; i++){
    Serial.print(fft_result[i], 3);
    Serial.print(" ");
  }
  Serial.println();
}

// Print FFT data, debug purposes 
void printFFTMagData(int wordsNumber){
  Serial.print("FFT_Mag: "); Serial.println(wordsNumber); 
  for (int i=0; i<wordsNumber; i++){
    Serial.print(fft_result_mag[i], 3);
    Serial.print(" ");
  }
  Serial.println();
}

// Print the process buffer, and both fft processed
void printAll(void){
  printAdcData(AMOUNT_SAMPLES);
  printBufferData(AMOUNT_SAMPLES);
  printFFTData(AMOUNT_SAMPLES);
  printFFTMagData(AMOUNT_SAMPLES/2);
}

// Print bands used
void printBands(void){  
  Serial.println("Frequency bands: ");
  Serial.println(bands[0]); 
  Serial.println(bands[1]);
  Serial.println(bands[2]);
  Serial.println(bands[3]);
  Serial.println(bands[4]);
  #ifndef COUPLED_MODE
  Serial.println(bands[5]); 
  Serial.println(bands[6]);
  Serial.println(bands[7]);
  Serial.println(bands[8]);
  Serial.println(bands[9]);
  #endif  
}