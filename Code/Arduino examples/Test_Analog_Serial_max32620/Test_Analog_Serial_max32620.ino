/*
 * Example setup to read an analog value from the analog pin A0
 * Using the divider by 5.
 * Data will come from a sound sensor, powered from the board sys pin.
 * As the divider is used, PIN_A0 is not used as a variable, and is required
 * 
*/

// Using the PIN_A0 to read the analog value
//int analogPin = PIN_A0;
int val = 0;           // variable to store the value read
float voltage_level = 0;
bool out = false;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // prints title with ending line break
  delay(600);
  Serial.println("First test message");

  // Configure pin A0 to output the value divided by 5
  changeA0_A4();

  pinMode(LED_BUILTIN, OUTPUT);
}


void loop() {
  // Analog value can vary between 0-4.475V
  // After divider: 0-0.895
  out = ! out;
  digitalWrite(LED_BUILTIN, out);
  val=analogRead(PIN_A0);   //connect mic sensor to Analog 0
  //Serial.print("Sound value: ");
  voltage_level = (val * 5.5)/1023.0;
  //voltage_level = val * 5;
  Serial.println(voltage_level, 4);//print the sound value to serial 

  delay(50);
  //Serial.println("Bye");
  //delay(1500);
}

