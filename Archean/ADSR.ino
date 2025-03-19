/*
          /\          /\     
         /  \        /\/\    
    /\  /    \  /\  /    \   
   /  \/      \/  \/      \  
  /                    /\  \  
        /\      /\    /  \   
       /  \    /  \  /    \  

*/

#define MAX_TIME_ADSR 1000 
#define MIN_TIME_ADSR 1
#define STEP 16

uint16_t AdsrStepCounter = 0;
int Envelope = 4095;
int oldEnvelope = -1;
boolean NoteIsActive = false;
boolean DecayState = false;
boolean ReleaseState = false;
boolean ReleaseStateIsDone = true;

void GateInterrupt(void){
  Gate = digitalRead(GATE_PIN);
  if (Gate == true){
    Gate = false;
    analogWrite(LED_PIN, 0);
  }else{
    Gate = true;
    analogWrite(LED_PIN, 255);
  }
}

void ADSRinterrupt(void){
  if(oldEnvelope != Envelope){
    if(SPIfree == true && PIT_CVAL0 > 15){
      SPIfree = false;
      DAC(Envelope, DAC_ADSR_AND_DISTANCE_CS, false);
      SPIfree = true;
      oldEnvelope = Envelope;
    }
  }
  ADSRInterruptState = HIGH;
}

void ADSR(void){  
  // Note on
  if(Gate == true){
    if(NoteIsActive == false){ 
      NoteIsActive= true;
      AdsrStepCounter = 0;
    }
    // ATTACK
    if(DecayState == false){
      AdsrStepCounter++;
      if(AdsrStepCounter >= map(Attack, 0, 255, MIN_TIME_ADSR, MAX_TIME_ADSR)){
        Envelope -= STEP; 
        AdsrStepCounter = 0;
      }
    }
    // DECAY
    if(Envelope < 95 && DecayState == false){
      DecayState = true;
      AdsrStepCounter = 0;
    }
    if(DecayState == true && Envelope <= map(Sust, 0, 255, 4095, 0)){ 
      AdsrStepCounter++;
      if(AdsrStepCounter >= map(Decay, 0, 255, MIN_TIME_ADSR, MAX_TIME_ADSR)){
        Envelope += STEP;
        if(Envelope > 4095) Envelope = 4095;
        AdsrStepCounter = 0;
      }
    }
  }
  
  // Note Off
  if(Gate != true){ 
    // RELEASE
    if(NoteIsActive == true){
      NoteIsActive = false;
      DecayState = false;
      ReleaseState = true;
      AdsrStepCounter = 0;
    }
    if(ReleaseState == true && Envelope <= 4094){
      AdsrStepCounter++;
      if(AdsrStepCounter >= map(Release, 0, 255, MIN_TIME_ADSR, MAX_TIME_ADSR)){
       Envelope += STEP;
       AdsrStepCounter = 0;
      }
    }
    if(ReleaseState == true && Envelope >= 4094){
      ReleaseState = false;
      Envelope = 4095;
    }
  }
}
