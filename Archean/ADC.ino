/*       .                       
        \   /     
         .-.       ..
    -―  (   ) ― -('   ') 
         `-’    (       ).     --
        /   \   .(     )     (     )
                   --       (       ) 
   (_)                       (     )
             (__)               --

*/

#define TUNE_ADC_PIN A0
#define FINE_ADC_PIN A1
#define ATTACK_ADC_PIN A9
#define DECAY_ADC_PIN  A7
#define SUSTAIN_ADC_PIN A8
#define RELEASE_ADC_PIN  A6
#define ELEMENT_ADC_PIN  A5
#define LFO_ADC_PIN  A4

#define NUMBER_OF_CHANNELS 8
#define TUNE_CHANNEL 0
#define FINE_CHANNEL 1
#define ATTACK_CHANNEL  2
#define DECAY_CHANNEL 3
#define SUSTAIN_CHANNEL 4
#define RELEASE_CHANNEL 5
#define ELEMENT_CHANNEL 6
#define LFO_CHANNEL 7

ADC *adc = new ADC();

const uint32_t InitialAverageValue = 2;
const uint32_t BufferSize = 10;

DMAMEM static volatile uint16_t __attribute__((aligned(32))) dma_adc_buff1[BufferSize];
DMAMEM static volatile uint16_t __attribute__((aligned(32))) dma_adc_buff2[BufferSize];
AnalogBufferDMA abdma(dma_adc_buff1, BufferSize, dma_adc_buff2, BufferSize);

uint16_t ADCvalue;
uint16_t TuneADCvalue;
uint16_t ElementADCvalue;
uint16_t ADC_Pitch;
uint16_t Fine;
uint16_t Attack, Decay, Sust, Release;
uint16_t LFO, oldLFO; 
uint16_t Element, oldElement;
uint16_t ElementLinarFmCalculation;
short ADC_Parameter_Switch = 0;
short ADCchannelFAST = 0;

// ADC Filter
uint16_t Glitch[NUMBER_OF_CHANNELS][4];
short GlitchCounter[NUMBER_OF_CHANNELS] = { 0, 0, 0, 0, 0, 0, 0, 0 };
short Sleep[NUMBER_OF_CHANNELS] = { 0, 0, 0, 0, 0, 0, 0, 0 };
uint16_t adcOld[NUMBER_OF_CHANNELS];

// Low Pass Filter (LPF) to remove high-frequency noise from ADC  
// Based on an implementation from ArduinoTutorials by Curiores
template <int order> // order is 1 or 2
class LowPass
{
  private:
    float a[order];
    float b[order+1];
    float omega0;
    float dt;
    bool adapt;
    float tn1 = 0;
    float x[order+1]; // Raw values
    float y[order+1]; // Filtered values

  public:  
    LowPass(float f0, float fs, bool adaptive){
      // f0: cutoff frequency (Hz)
      // fs: sample frequency (Hz)
      // adaptive: boolean flag, if set to 1, the code will automatically set
      // the sample frequency based on the time history.
      
      omega0 = 6.28318530718*f0;
      dt = 1.0/fs;
      adapt = adaptive;
      tn1 = -dt;
      for(int k = 0; k < order+1; k++){
        x[k] = 0;
        y[k] = 0;        
      }
      setCoef();
    }

    void setCoef(){
      if(adapt){
        float t = micros()/1.0e6;
        dt = t - tn1;
        tn1 = t;
      }
      
      float alpha = omega0*dt;
      if(order==1){
        a[0] = -(alpha - 2.0)/(alpha+2.0);
        b[0] = alpha/(alpha+2.0);
        b[1] = alpha/(alpha+2.0);        
      }
      if(order==2){
        float alphaSq = alpha*alpha;
        float beta[] = {1, sqrt(2), 1};
        float D = alphaSq*beta[0] + 2*alpha*beta[1] + 4*beta[2];
        b[0] = alphaSq/D;
        b[1] = 2*b[0];
        b[2] = b[0];
        a[0] = -(2*alphaSq*beta[0] - 8*beta[2])/D;
        a[1] = -(beta[0]*alphaSq - 2*beta[1]*alpha + 4*beta[2])/D;      
      }
    }

    float filt(float xn){
      // Provide me with the current raw value: x
      // I will give you the current filtered value: y
      if(adapt){
        setCoef(); // Update coefficients if necessary      
      }
      y[0] = 0;
      x[0] = xn;
      // Compute the filtered values
      for(int k = 0; k < order; k++){
        y[0] += a[k]*y[k+1] + b[k]*x[k];
      }
      y[0] += b[order]*x[order];

      // Save the historical values
      for(int k = order; k > 0; k--){
        y[k] = y[k-1];
        x[k] = x[k-1];
      }
  
      // Return the filtered value    
      return y[0];
    }
};

// Filter instance
LowPass<2> lp(80,20000,0);
LowPass<2> lp2(80,20000,0);

void InterruptADC_DMA(void){
  if(abdma.interrupted()){
    ChangeADCchannel();
  }
}

// Timer alternates ADC channels to read parameters  
// Switching between Tune and Element input at 20 kHz each
void FASTadc(void){
  if(ADCchannelFAST == 0){
    while(!(ADC1_HS & ADC_HS_COCO0));
    TuneADCvalue = ADC1_R0;
    ADC1_HC0 = 5; 
    ADCchannelFAST = 1;
    ADCPitchInterruptState = HIGH;
  }else{
    while(!(ADC1_HS & ADC_HS_COCO0));
    ElementADCvalue = ADC1_R0;
    ADC1_HC0 = 7;
    ADCchannelFAST = 0;
    ADCElementInterruptState = HIGH;
  }
}

void FilterTuneADC(void){
  float yn = lp.filt(TuneADCvalue);
  ADC_Pitch = yn;
}

void FilterElementADC(void){
  float yn = lp2.filt(ElementADCvalue);
  Element = yn;
}

void ADCsetup(void){
   pinMode(TUNE_ADC_PIN, INPUT); 
   pinMode(FINE_ADC_PIN, INPUT); 
   pinMode(ATTACK_ADC_PIN, INPUT);
   pinMode(DECAY_ADC_PIN, INPUT);
   pinMode(SUSTAIN_ADC_PIN, INPUT); 
   pinMode(RELEASE_ADC_PIN, INPUT); 
   pinMode(ELEMENT_ADC_PIN, INPUT);
   pinMode(LFO_ADC_PIN, INPUT);
    
   adc->adc0->setAveraging(1); // set number of averages
   adc->adc0->setResolution(10); // set bits of resolution  
   adc->adc0->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED); // fastest conversion
   adc->adc0->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED); // fastest sampling

   adc->adc1->setAveraging(4); // set number of averages
   adc->adc1->setResolution(8); // set bits of resolution  
   adc->adc1->setConversionSpeed(ADC_CONVERSION_SPEED::VERY_HIGH_SPEED); // fastest conversion
   adc->adc1->setSamplingSpeed(ADC_SAMPLING_SPEED::VERY_HIGH_SPEED); // fastest sampling
   abdma.init(adc, ADC_1);
   abdma.userData(InitialAverageValue); // save away initial starting average
   adc->adc1->startContinuous(RELEASE_ADC_PIN); 
}

// ADC filter to prevent false oscillations  
// If the ADC value fluctuates by ±1 and returns to the previous value,  
// it is ignored until a change of at least 2 occurs
boolean GlitchFiter(int Channel, int Value){
  if((Sleep[ATTACK_CHANNEL] == 1) && (ADCvalue != Glitch[ATTACK_CHANNEL][0]) && (ADCvalue != Glitch[ATTACK_CHANNEL][1])) Sleep[ATTACK_CHANNEL] = 0;

  if((ADCvalue != adcOld[ATTACK_CHANNEL]) && Sleep[ATTACK_CHANNEL] != 1){
    GlitchCounter[Channel]++;
    if (GlitchCounter[Channel] > 3) GlitchCounter[Channel] = 0;
    Glitch[Channel][GlitchCounter[Channel]] = Value;
    if ((Glitch[Channel][0] == Glitch[Channel][2]) && (Glitch[Channel][1] == Glitch[Channel][3])) Sleep[Channel] = 1;
    adcOld[Channel] = Value;
    return true;
  }else{
    return false;
  }
}

void ChangeADCchannel(void){
  if(ADC_Parameter_Switch == 0){
    ADCvalue =  ProcessAnalogData(&abdma, 1);
    if(GlitchFiter(SUSTAIN_CHANNEL, ADCvalue)){
      Sust = ADCvalue;
    }
    adc->adc1->startContinuous(FINE_ADC_PIN);
  }
  if(ADC_Parameter_Switch == 1){
    ADCvalue =  ProcessAnalogData(&abdma, 1);
    if(GlitchFiter(FINE_CHANNEL, ADCvalue)){
      Fine = ADCvalue;
    }
    adc->adc1->startContinuous(RELEASE_ADC_PIN);
  }
  if(ADC_Parameter_Switch == 2){
    ADCvalue =  ProcessAnalogData(&abdma, 1);
    if(GlitchFiter(RELEASE_CHANNEL, ADCvalue)){
      Release = ADCvalue;
    }
    adc->adc1->startContinuous(ATTACK_ADC_PIN);
  }
  if(ADC_Parameter_Switch == 3){
    ADCvalue =  ProcessAnalogData(&abdma, 1);
    if(GlitchFiter(ATTACK_CHANNEL, ADCvalue)){
      Attack = ADCvalue;
    }
    adc->adc1->startContinuous(DECAY_ADC_PIN);
  }
  if(ADC_Parameter_Switch == 4){
    ADCvalue =  ProcessAnalogData(&abdma, 1);
    if(GlitchFiter(DECAY_CHANNEL, ADCvalue)){
      Decay = ADCvalue;
    }
    adc->adc1->startContinuous(LFO_ADC_PIN);
  }
  if(ADC_Parameter_Switch == 5){
    ADCvalue = ProcessAnalogData(&abdma, 1);
    if(GlitchFiter(LFO_CHANNEL, ADCvalue)){
      LFO = ADCvalue;
    }
    adc->adc1->startContinuous(SUSTAIN_ADC_PIN);
  }
  
  ADC_Parameter_Switch++;
  if(ADC_Parameter_Switch > 5){
    ADC_Parameter_Switch = 0; 
  }
}

int ProcessAnalogData(AnalogBufferDMA *pabdma, int8_t adc_num) {
  uint32_t sum_values = 0;
  uint16_t min_val = 0xffff;
  uint16_t max_val = 0;
  uint32_t average_value = pabdma->userData();
  volatile uint16_t *pbuffer = pabdma->bufferLastISRFilled();
  volatile uint16_t *end_pbuffer = pbuffer + pabdma->bufferCountLastISRFilled();
  float sum_delta_sq = 0.0;
  
  if ((uint32_t)pbuffer >= 0x20200000u)  arm_dcache_delete((void*)pbuffer, sizeof(dma_adc_buff1));
  while (pbuffer < end_pbuffer) {
    if (*pbuffer < min_val) min_val = *pbuffer;
    if (*pbuffer > max_val) max_val = *pbuffer;
    sum_values += *pbuffer;
    int delta_from_center = (int) * pbuffer - average_value;
    sum_delta_sq += delta_from_center * delta_from_center;
    pbuffer++;
  } 

  average_value = sum_values / BufferSize;
  pabdma->clearInterrupt();
  pabdma->userData(average_value);

  return average_value;
} 
