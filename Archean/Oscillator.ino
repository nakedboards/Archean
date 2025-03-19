/*
            |
            |   .
     `.  *  |     .'
       `. ._|_* .'  .
     . * .'   `.  *
  -------|     |-------
     .  *`.___.' *  .
        .'  |* `.  *
      .' *  |  . `.
          . |
            | 
 */
 
double OSC_Timer;
double oldOSC_Timer;
short oldKey;
short KeyFind = 0;
double FreqToFindMIDInote;
boolean NewTouch = false;
short OSCsliderSelect;
uint16_t OscValue;
uint16_t OldOscValue;
short WaveTablePoint = 0;
short oldWaveTablePoint = -1;
extern uint16_t MIDIdelay;
unsigned long ChangeDelay = 0;
uint16_t oldADC_Pitch = -1;
uint16_t ADC_Pitch_Filtered;


// High-priority interrupt function that updates the oscillator output.  
// - Runs at a fixed rate of 44 kHz.  
// - Disables other interrupts to prevent audio glitches.  
// - Steps through the waveform array and sends the next sample to the DAC.
void OscillatorUpdate(void){
  noInterrupts();
  WaveTablePoint++;
  if(WaveTablePoint > 255) WaveTablePoint = 0;
  if(OSCsliderSelect == 0) OscValue = LandscapeA[WaveTablePoint];
  else if(OSCsliderSelect == 1) OscValue = Triangle[WaveTablePoint];
  else if(OSCsliderSelect == 2) OscValue = Landscape[WaveTablePoint];
  if(SPIfree == true){
    SPIfree = false;
    DAC(OscValue, OSC_DAC_CS_PIN, true);
    SPIfree = true;
  }
  interrupts();
}
  
//  Determines the oscillator's pitch frequency.  
// - Uses the last received MIDI note if available.  
// - If no MIDI note is present, the pitch is set based on:  
//    - The Tune potentiometer.  
//    - The Fine adjustment potentiometer.  
//    - The 1V/oct CV input.  
//    - The pressed key on the synthesizer.
void Pitch(void){

  // Glitch filter
  if(oldADC_Pitch != ADC_Pitch){
    oldADC_Pitch = ADC_Pitch;
    if(GlitchFiter(TUNE_CHANNEL, ADC_Pitch)){
      ADC_Pitch_Filtered = ADC_Pitch;
    }
  }
  
  
   // Determines pitch source:  
   // - If no MIDI note is received, sets frequency based on:  
   //   - 1V/oct input  
   //   - Tune potentiometer  
   //   - Fine adjustment potentiometer  
   //   - Pressed keyboard key
  if(MIDInoteRecieved == false){
    if(NewTouch == true){
      KeyFind = FindTone(KeybordNote, ScaleNumber);
      NewTouch = false;
    }
    
    int OSCArrayIndex = ADC_Pitch_Filtered + int((KeyFind*12.2));
    
    // If the element function is set to linear modulation, apply additional pitch modulation.
    if(LinarFM == true) OSCArrayIndex = OSCArrayIndex + LinarFMvalue;
    
    if(OSCArrayIndex > 1023) OSCArrayIndex = 1023;
    if(OSCArrayIndex < 0) OSCArrayIndex = 0;
    OSC_Timer = V_OCT[OSCArrayIndex];
  }
  // If a MIDI note is received, sets frequency according to the MIDI pitch. 
  else{
    OSC_Timer = midiNote[RecievedMIDINote];
  }

  // Fine
  double FinePercent = map(Fine, 0, 255, 255.0, -255.0) / 100.0;
  OSC_Timer = ((OSC_Timer / 100.0) * FinePercent) + OSC_Timer;  

  // Updates the oscillator timer based on the calculated pitch frequency.
  OscillatorTimer.update(OSC_Timer);
}

// Interrupt function triggered by the slider switch.  
// Determines the oscillator mode based on the switch position.
void OSC_SliderInterrupt(void){
  if (digitalRead(OSC_SLIDER_PIN_1) == true && digitalRead(OSC_SLIDER_PIN_2) == true){
    OSCsliderSelect = 0;
  }
  if (digitalRead(OSC_SLIDER_PIN_1) != true && digitalRead(OSC_SLIDER_PIN_2) == true){
    OSCsliderSelect = 1;
  }
  if (digitalRead(OSC_SLIDER_PIN_1) == true && digitalRead(OSC_SLIDER_PIN_2) != true){
    OSCsliderSelect = 2;
  }
}
