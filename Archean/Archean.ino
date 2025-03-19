/*
                              -- (  
   ,.-.                        . `
  (___ '    /\                '  |
           /  \      /\  
          /    \    /  \  
         /      \  /    \  
        /  /\    \/      \  
       /  /  \    \  /\   \  
  ____/__/____\____\/__\___\_______  
    
  Archean Synthesizer  
  Developed by Nikolai Ershov, 2025  
  Firmware version: 1.0  
  Licensed under the MIT License.  
  
*/
 
#include "Adafruit_MPR121.h"
#include "TeensyTimerTool.h"
#include "Watchdog_t4.h"
#include "Wavetable.h"
#include "MIDIFreq.h"
#include "1V_OCT.h"
#include <SPI.h>
#include <ADC.h>
#include <MIDI.h>
#include <Wire.h>
#include <IntervalTimer.h>
#include <AnalogBufferDMA.h>

#define OSC_DAC_CS_PIN  1
#define DAC_ADSR_AND_DISTANCE_CS  10
#define DAC_LFO_AND_ELEMENT_CS  12
#define KEYBOARD_INTERRUPT_PIN  4
#define BUTTON_PIN  7
#define GATE_PIN  9
#define LED_PIN  8
#define LFO_SLIDER_PIN  3
#define OSC_SLIDER_PIN_1  5
#define OSC_SLIDER_PIN_2  6
#define PIT_CVAL_TIME 35

using namespace TeensyTimerTool;

// Global variables
volatile byte SPIfree = true;
volatile byte I2Cfree = true;
boolean Gate = false;
short ScaleNumber = 0;
short KeybordKey;
short SelectElementFunction = 0;

// Interrupt states
volatile byte ElementInterruptState = LOW;
volatile byte ADSRInterruptState = LOW;
volatile byte ADCPitchInterruptState = LOW;
volatile byte ADCElementInterruptState = LOW;
volatile byte LandscapeMorphInterruptState = LOW;

// Timers 
IntervalTimer OscillatorTimer; // Oscillator
PeriodicTimer ADSRtimer; // ADSR
PeriodicTimer LFOtimer; // LFO
PeriodicTimer ElementTimer; // Element 
PeriodicTimer DistanceSensorTimer; // Distance sensor
PeriodicTimer MetamorphismTimer; // Metamorphism
PeriodicTimer FASTadcTimer; //  ADC 44000Hz for Pitch and Element

// Watchdog
WDT_T4<WDT1> WDT;

void setup(){

  // LED blinks once at startup
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH);
  delay(10);
  digitalWrite(LED_PIN, LOW);
  
  // MIDI init
  Serial.begin(9600);
  MIDIinit();

  usbMIDI.setHandleNoteOff(OnNoteOff);
  usbMIDI.setHandleNoteOn(OnNoteOn);

  // SPI init
  SPI.begin();
  
  pinMode(OSC_DAC_CS_PIN, OUTPUT);
  digitalWrite(OSC_DAC_CS_PIN, HIGH);

  pinMode(DAC_ADSR_AND_DISTANCE_CS, OUTPUT);
  digitalWrite(DAC_ADSR_AND_DISTANCE_CS, HIGH);

  pinMode(DAC_LFO_AND_ELEMENT_CS, OUTPUT);
  digitalWrite(DAC_LFO_AND_ELEMENT_CS, HIGH);

  // External interrupts
  pinMode(KEYBOARD_INTERRUPT_PIN, INPUT_PULLUP);

  pinMode(GATE_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(GATE_PIN), GateInterrupt, CHANGE);

  pinMode(LFO_SLIDER_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(LFO_SLIDER_PIN), LFOSliderInterrupt, CHANGE);

  pinMode(OSC_SLIDER_PIN_1, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(OSC_SLIDER_PIN_1), OSC_SliderInterrupt, CHANGE);

  pinMode(OSC_SLIDER_PIN_2, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(OSC_SLIDER_PIN_2), OSC_SliderInterrupt, CHANGE);

  // Button init
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // I2C Sensors init
  Wire1.begin();
  Wire1.setClock(3400000);
  KeyboardInit();
  DistanceSensorInit();
  
  // Read settings from EEPROM
  EEPROMreadSettings();

  // Timers init
  OscillatorTimer.begin(OscillatorUpdate, 44000);
  LFOtimer.begin(LfoInterrupt, 100);
  ADSRtimer.begin(ADSRinterrupt, 40);
  ElementTimer.begin(ElementInterrupt, 10000);
  DistanceSensorTimer.begin(DistanceUpdate, 21000); 
  MetamorphismTimer.begin(LandscapeMorphInterrupt, 1000);
  FASTadcTimer.begin(FASTadc, 25); //ADC timer alternates reading Pitch and Element at 40 kHz (each ~20 kHz)
  
  OscillatorTimer.priority(0);

  // Creating landscapes for the oscillator
  CreateLandsacapes();

  // Checking the position of the sliders
  OSC_SliderInterrupt();
  LFOSliderInterrupt();

  // ADC init
  ADCsetup();

  // Watchdog init
  WDT_timings_t config;
  config.timeout = 3;
  WDT.begin(config);
}

void loop(){

  // Updating parameters from ADC
  InterruptADC_DMA();

  // Updating Tune (1V/OCT) and Element from ADC
  if(ADCPitchInterruptState == HIGH){
    FilterTuneADC();
    ADCPitchInterruptState = LOW;
  }
  if(ADCElementInterruptState == HIGH){
    FilterElementADC();
    ADCElementInterruptState = LOW;
  }

  // Set oscillator frequency
  Pitch();

  // Updating metamorphism speed via ADC LFO parameter
  MetamorphismUpdate();

  // Updating Element input via ADC
  ElementCVUpdate();

  // Reading incoming MIDI messages via USB and MIDI input
  usbMIDI.read();
  serialMIDIread();

  // Reading keyboard input
  if(digitalRead(KEYBOARD_INTERRUPT_PIN) == LOW){
    KeyboardRead();
  }

  // Sending MIDI Messages via USB
  USBMIDISendMessages();

  // Updating Element Output
  if(ElementInterruptState == HIGH){
    ElementUpdate();
    ElementInterruptState = LOW;
  }

  // ADSR
  if(ADSRInterruptState == HIGH){
    ADSR();
    ADSRInterruptState = LOW;
  }

  // Landscape metamorphism
  if(LandscapeMorphInterruptState == HIGH){
    LandscapeMorph();
    LandscapeMorphInterruptState = LOW;
  } 

  // Checking button state
  CheckButtonState();

  // Handling actions based on button state
  HandleButtonActions();

  // Feeding the Watchdog Timer to prevent system reset
  WDT.feed();
}
