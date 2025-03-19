#include <VL53L0X.h>
#define Address 0x29

VL53L0X Sensor;

uint16_t DistanceValue = 0;
uint16_t DistanceValueBuffer[5];
uint16_t DistanceBufferCounter = 0;
uint16_t SmoothedDistance = 4095;
uint16_t oldDistance = 0;

// Initializing the distance sensor
void DistanceSensorInit(void){
  Sensor.setTimeout(500);
  Sensor.init();
  Sensor.setMeasurementTimingBudget(20000);
  Sensor.startContinuous();
}

// Reads data from the distance sensor via the I2C port.
// This function checks if the I2C port is available. If the port is free, 
// it reads the data from the distance sensor.
void ReadDistanceSensor(void){
  if(I2Cfree == true){
    I2Cfree = false;
    Wire1.beginTransmission(Address);
    Wire1.write(0x14 + 10); 
    Wire1.endTransmission();
    Wire1.requestFrom(0x29, 2);
    DistanceValue  = (uint16_t)Wire1.read() << 8;
    DistanceValue |= Wire1.read();
    I2Cfree = true;
  }
}

// Smooths sensor data by averaging the last 5 readings.
// Stores the last 5 measurements in a buffer, calculates their average, and returns the result.
// This helps to reduce noise and improve accuracy.
void SmoothSensorData(void){
  DistanceValueBuffer[DistanceBufferCounter] = DistanceValue;
  DistanceBufferCounter++;
  if(DistanceBufferCounter > 4) DistanceBufferCounter = 0;
  SmoothedDistance = (DistanceValueBuffer[0] + DistanceValueBuffer[1] + DistanceValueBuffer[2] + DistanceValueBuffer[3] + DistanceValueBuffer[4]) / 5;
  if(SmoothedDistance > 350) SmoothedDistance = 350;
  if(SmoothedDistance < 50) SmoothedDistance = 50;
}

// Updates the distance sensor data and processes it.
// This function performs the following steps:
// 1. Reads raw sensor data.
// 2. Smooths the data by averaging recent values.
// 3. Sends the processed data to the DAC if necessary.
// 4. Updates the LED status based on data changes
void DistanceUpdate(void){
  ReadDistanceSensor();
  SmoothSensorData();
  if(oldDistance != SmoothedDistance){
    uint16_t DACmapped = map(SmoothedDistance, 50, 350, 0, 4095);
    if(SPIfree == true && PIT_CVAL0 > 40){
      SPIfree = false;
      DAC(DACmapped, DAC_ADSR_AND_DISTANCE_CS, true);
      SPIfree = true;
      oldDistance = SmoothedDistance;
    }
    if(!Gate) analogWrite(LED_PIN, map(SmoothedDistance, 50, 350, 255, 0));
   } 
}
