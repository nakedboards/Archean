/* 
  EEPROM storage for synth settings.
  
  This section handles saving and retrieving essential settings 
  in non-volatile EEPROM memory. The stored settings include:
  1. The selected musical scale for the keyboard.
  2. The selected function of the "Element" parameter.
  
  By storing these values in EEPROM, the synth can retain its settings even after power loss.
*/

#include <EEPROM.h>

const int EEPROM_SIZE = 1000;
int EEPROMaddress = 1;

// Reads the last stored settings from EEPROM.
// This algorithm scans EEPROM starting from the highest address and moves backward 
// until it finds the first non-zero value. This value indicates the last saved settings. 
// The last two non-zero values correspond to:
// 1. The selected musical scale.
// 2. The selected function of the "Element" parameter.
void EEPROMreadSettings(void){
  for(int i = EEPROM_SIZE; i >= 0; i--){
    short data = EEPROM.read(i);
    if(data != 0){
      SelectElementFunction = data - 1;
      int temp = i-1;      
      ScaleNumber = EEPROM.read(temp) - 1;
      EEPROMaddress = i;
      break;
    } 
  }
}

// Writes new settings to EEPROM.   
// This function uses the last recorded address of a non-zero value,  
// which was previously found during the EEPROM reading process.  
// It writes the new values into the next two available empty addresses.   
// The stored values represent:  
// 1. The selected musical scale.  
// 2. The selected function of the "Element" parameter.
void EEPROMwriteSettings(short ScaleNumber, short SelectElementFunction){
  EEPROMaddress = EEPROMaddress + 2;
  if(EEPROMaddress > EEPROM_SIZE-1){
    EraseEEPROM();
    EEPROMaddress = 1;
  }
  EEPROM.write(EEPROMaddress, SelectElementFunction);
  int temp = EEPROMaddress-1;
  EEPROM.write(temp, ScaleNumber);
}

// Erases the entire EEPROM memory.  
// This function is called when the EEPROM storage becomes full.  
// It iterates through all memory addresses and resets them to zero,  
// clearing all stored settings.    
// This ensures that new settings can be written from the beginning  
// without conflicts with previous values.
void EraseEEPROM(void){
  for(int i = 0; i <= EEPROM_SIZE;){
      EEPROM.write(i, 0);
      i++;
    }
    EEPROMaddress = 0;  
}
