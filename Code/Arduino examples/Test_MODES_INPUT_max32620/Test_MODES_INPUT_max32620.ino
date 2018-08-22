/*
 * Program used to test browsing different modes, using the Boot pin
*/

//unsigned long currentTime = millis();

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // Configure BOOT button as input
  pinMode(P2_7, INPUT_PULLUP);  

  // prints title with ending line break
  Serial.println("Initial message");
  Serial.println("Start the program in custom mode");
}

void loop() {

  unsigned long currentTime = millis();

  // Check if the button is pressed 
  if( !digitalRead(P2_7) ){    
    int currentMode = 0;

    // Stay on this loop if the button is pressed
    while( !digitalRead(P2_7) ){
      
      // Output different modes
      if( (currentMode==0) && (millis() - currentTime > 2000  )  ){
        currentMode = 1;
        Serial.println( "Activated Mode1" );        
      }

      // Output different modes
      if( (currentMode==1) && (millis() - currentTime > 4000  )  ){
        currentMode = 2;
        Serial.println( "Activated Mode2" );        
      }

      // Output different modes
      if( (currentMode==2) && (millis() - currentTime > 6000  )  ){
        currentMode = 3;
        Serial.println( "Activated Mode3" );        
      }
    }
    Serial.print("Time pressed: ");
    Serial.println(millis() - currentTime);
  } 
  //delay(700);
}
