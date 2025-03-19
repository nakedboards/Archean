#define MAX_TIME_LFO 300
#define MIN_TIME_LFO 2

// Counter that continuously increments and is compared with the ADC value from the LFO control  
// Determines the speed of the LFO based on the potentiometer and CV input  
uint16_t LFOCounter = 0; 
boolean LFOWaveformSelector = true;

// Pointer to the current position in the LFO waveform data array  
// Each time the LFO updates, it selects the next value from the array  
short LFOPointer = 0;

// The current LFO value to be sent to the DAC
uint16_t LFOData;

void LfoInterrupt(void){
  LFOCounter++;
  if(LFOCounter >= map(LFO, 0, 255, MAX_TIME_LFO, MIN_TIME_LFO)){
    LFOPointer += map(LFO, 0, 255, 1, 8);
    if(LFOPointer > 1023) LFOPointer = 0;
    // Determines which waveform is used for the LFO output  
    if (LFOWaveformSelector == true){
      LFOData = SINE[LFOPointer];
    }else{
      LFOData = PULSE[LFOPointer];
    }
    LFOCounter = 0;
  }

  if(SPIfree == true && PIT_CVAL0 > 35){
    SPIfree = false;
    DAC(LFOData, DAC_LFO_AND_ELEMENT_CS, true); 
    SPIfree = true;  
  } 
}

void LFOSliderInterrupt(void){
  LFOWaveformSelector = digitalRead(LFO_SLIDER_PIN);
}
