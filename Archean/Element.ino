/*
    ___(((                     )))
   ((                          _))
  (_                       ____))
    ((                _____))
      (_________)----'
                 _/  /
                /  _/
              _/  /
             / __/
           _/ /
          /__/
         //
        /

  Element parameter configuration.  

  The "Element" parameter in the synthesizer is a programmable module  
  that can serve different functions depending on its settings.  

  Currently, the module supports the following functions:  
  1. Digital noise (random generator)  
  2. ADSR repeater  
  3. Linear modulation using the CV input  

  The code can be modified or extended to support additional functionalities  
  as needed. 
*/

volatile byte LinarFM = false;
boolean TimerChanger = false; // Determines whether the timer should be updated
short NoiseDelay = 0; // Controls the speed of random generation
signed int LinarFMvalue;

void ElementInterrupt(void){
  ElementInterruptState = HIGH;
}

// Main function of the element block  
// Defines how this parameter is used in the synthesizer  
// Handles different modes: Digital Noise, ADSR repeater, and linear oscillator modulation  
// Can be extended with additional functions 
void ElementUpdate(void){ 
  // 0 - Random noise
  // 1 - ADSR repeater
  // 2 - Linar FM
  switch(SelectElementFunction){
    case 0:
      ElementTimer.setPeriod(map(Element, 0, 1023, 10000, 3000));
      NoiseDelay++;
      if(NoiseDelay > map(Element, 0, 1023, 50, 1)){
       DigitalRandomNoise();
       NoiseDelay = 0;
      }
      LinarFM = false;
      TimerChanger = false;
      break;
    case 1:
      if(TimerChanger == false){
        TimerChanger = true;
        ElementTimer.setPeriod(10000);
      }
      RepeatADSR();
      LinarFM = false;
      break;
    case 2:
      LinarFM = true;
      break; 
    default: 
      ElementTimer.setPeriod(map(Element, 0, 1023, 10000, 3000));
      NoiseDelay++;
      if(NoiseDelay > map(Element, 0, 1023, 50, 1)){
       DigitalRandomNoise();
       NoiseDelay = 0;
      }
      LinarFM = false;
      TimerChanger = false;
      break;
  }
}

// Updates the element's function based on the CV input and potentiometer position  
void ElementCVUpdate(void){
  if(oldElement != Element){
    oldElement = Element;
    if(GlitchFiter(ELEMENT_CHANNEL, Element)){
      // Adjusts the timer speed for the Digital Noise Generator  
      // CV voltage and potentiometer position control the noise update rate  
      if(SelectElementFunction == 0) ElementTimer.setPeriod(map(Element, 0, 1023, 80000, 200));
      // Controls the linear modulation of the oscillator  
      if(SelectElementFunction == 2) LinarFMvalue = map(Element, 0, 1023, -500, 500);
    }
  }
}

void DigitalRandomNoise(void){
  if(SPIfree == true && PIT_CVAL0 > 40){
      SPIfree = false;
      DAC(random(0,4095), DAC_LFO_AND_ELEMENT_CS, false);
      SPIfree = true;
   }
}

void RepeatADSR(void){
  if(SPIfree == true && PIT_CVAL0 > 40){
      SPIfree = false;
      DAC(Envelope, DAC_LFO_AND_ELEMENT_CS, false);
      SPIfree = true;
   }
}
  
