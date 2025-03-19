/* 
   Keyboard module for the synthesizer.  

   The keyboard consists of 22 keys, which are handled by two MPR121 capacitive touch sensors.  
   Each sensor is responsible for 11 keys.  

   This module includes:  
   - Initialization of both MPR121 sensors.  
   - A function for reading key presses.  
  
   The keyboard scanning function detects which keys are currently pressed  
   and processes them accordingly. 
*/
  
Adafruit_MPR121 Left_MPR121 = Adafruit_MPR121();
Adafruit_MPR121 Right_MPR121 = Adafruit_MPR121();

#define TH_TOUCH 0x0C
#define TH_RELEASE 0x02
#define TH_BASE 0xF0

extern volatile byte MIDInoteRecieved;
uint16_t KeybordNote;
short KeyboardMIDInotes[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
short KeyboardMIDIstatus[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

// Keeps track of the last pins touched so we know when buttons are 'released'
uint16_t lasttouched_left = 0;
uint16_t currtouched_left = 0;
uint16_t lasttouched_right = 0;
uint16_t currtouched_right = 0;

short TouchCounter = 0;
extern volatile short MenuKeyPressed;
extern double FreqToFindMIDInote;
extern boolean NewTouch;

// Initializes the keyboard sensors.  
// This function sets up both MPR121 touch sensors,  
// configuring them for detecting key presses.
void KeyboardInit(void){
  Left_MPR121.begin(0x5A);
  Left_MPR121.writeRegister(MPR121_ECR, 0x00);
  Left_MPR121.writeRegister(MPR121_DEBOUNCE, 0x77);
  Left_MPR121.writeRegister(MPR121_CONFIG1, 0x30);
  Left_MPR121.writeRegister(MPR121_CONFIG2, 0x20);
  Left_MPR121.writeRegister(MPR121_ECR, 0x8F);

  Right_MPR121.begin(0x5B);
  Right_MPR121.writeRegister(MPR121_ECR, 0x00);
  Right_MPR121.writeRegister(MPR121_DEBOUNCE, 0x77);
  Right_MPR121.writeRegister(MPR121_CONFIG1, 0x30);
  Right_MPR121.writeRegister(MPR121_CONFIG2, 0x20);
  Right_MPR121.writeRegister(MPR121_ECR, 0x8F); 
}

void KeyboardRead(void){
  
  MIDInoteRecieved = false; // Set oscillator frequency using keyboards

  // Read the currently pressed key
  if(I2Cfree == true){
    I2Cfree = false;
    currtouched_left = Left_MPR121.touched();
    I2Cfree = true;
  }
  if(I2Cfree == true){
    I2Cfree = false;
    currtouched_right = Right_MPR121.touched();
    I2Cfree = true;
  }

  // Interrupt triggered by MPR121 sensor, initiating key press reading.  
  // Data processing starts only after the interrupt pin returns to HIGH,  
  // ensuring the values were read correctly
  if(digitalRead(KEYBOARD_INTERRUPT_PIN) == HIGH){
    for(uint8_t i=0; i<11; i++) {
      // If a key is pressed and was not pressed before,  
      // register which specific key was pressed from the received sensor data.
      if((currtouched_left & _BV(i)) && !(lasttouched_left & _BV(i))){
        // If the menu is active, the key press is used to navigate or change settings.  
        // If the menu is not active, the key press controls the oscillator frequency.
        if(MenuState == true){
          MenuKeyPressed++;
          KeybordKey = i;
        }else{
          KeybordNote = i;
          // The pressed key will be used to send a MIDI Note On message.
          KeyboardMIDIstatus[KeybordNote] = 1;
          TouchCounter++;
          NewTouch = true;
        }
      }
      // If the key was pressed but is now released, update the state accordingly.
      if (!(currtouched_left & _BV(i)) && (lasttouched_left & _BV(i)) ) {
        if(MenuState != true){
          TouchCounter--;
          if(TouchCounter < 0) TouchCounter = 0;
          // The released key will be used to send a MIDI Note Off message.
          KeyboardMIDIstatus[i] = 0;
        }
      }
    }
    // Reset sensor state
    lasttouched_left = currtouched_left;
    currtouched_left = 0;
  }

  if(digitalRead(KEYBOARD_INTERRUPT_PIN) == HIGH){
    for (uint8_t i=0; i<11; i++) {
      // If a key is pressed and was not pressed before,  
      // register which specific key was pressed from the received sensor data.
      if ((currtouched_right & _BV(i)) && !(lasttouched_right & _BV(i))) {
        // If the menu is active, the key press is used to navigate or change settings.  
        // If the menu is not active, the key press controls the oscillator frequency
        if(MenuState == true){
          MenuKeyPressed++;
          KeybordKey = 21 - i;
        }else{
          KeybordNote = 21 - i; 
          TouchCounter++;
          NewTouch = true;
          // The pressed key will be used to send a MIDI Note On message.
          KeyboardMIDIstatus[KeybordNote] = 1;
        }
      }
      // If the key was pressed but is now released, update the state accordingly.
      if (!(currtouched_right & _BV(i)) && (lasttouched_right & _BV(i)) ) {
        if(MenuState != true){
          TouchCounter--;
          if(TouchCounter < 0) TouchCounter = 0;
          // The released key will be used to send a MIDI Note Off message.
          KeyboardMIDIstatus[21 - i] = 0;
        }
      }
    }
    // Reset sensor state
    lasttouched_right = currtouched_right;
    currtouched_right = 0;
  }

  // If any key is pressed, turn on the LED.  
  // If no keys are pressed, turn off the LED.
  if(TouchCounter == 0){
    Gate = false;
    analogWrite(LED_PIN, 0);
  }else{
    Gate = true;
    analogWrite(LED_PIN, 255);
  }
}
