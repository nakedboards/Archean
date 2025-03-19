/*
  In addition to the standard triangular waveform, the oscillator  
  can generate random landscapes—waveforms that resemble mountains or terrains.  
  
  The oscillator has three modes:  
  1. Triangle Wave - A standard triangular waveform.  
  2. Static Landscape - A randomly generated landscape waveform that remains unchanged.  
  3. Morphing Landscapes - Two random landscapes are generated,  
     and one gradually morphs into the other.  
     Once the transition is complete, a new landscape is generated,  
     and the cycle repeats, creating a continuously evolving waveform. 
*/
   
#define SIZE 256 // Size of the landscape array

uint16_t Landscape[SIZE]; // Main landscape height array
uint16_t LandscapeA[SIZE];
uint16_t LandscapeB[SIZE];

int MinValue;
int MaxValue;

int ReboundZoneMin = 30;   // Minimum height before rebound effect
int ReboundZoneMax = 190;  // Maximum height before rebound effect
int ReboundZoneStrength = 5;  // Strength of the rebound effect

int StartingHeight = 50;  // Initial terrain height

int Roughness = 5; // Controls terrain steepness (higher values create steeper slopes)

int CurrentHeight = StartingHeight;  // Current height of the terrain
int HeightIncrement = 0;  // Change in height for each step

extern short OSCsliderSelect;

// Generates three random landscapes for all oscillator modes at startup
void CreateLandsacapes(void){
  CreateLandscape(Landscape, SIZE);
  CreateLandscape(LandscapeA, SIZE);
  CreateLandscape(LandscapeB, SIZE);
}

// Wrapper function for generating a new landscape.  
// Calls the parameterized function with predefined settings.  
// Used in the menu and other general contexts.
void CreateNewLandscape(void){
  CreateLandscape(Landscape, 256);
}

// Generates a new random landscape 
void CreateLandscape(uint16_t *Landscape, int size){
  for (int i = 1; i < size; i++) {
    // Adjust height based on rebound zones
    if (CurrentHeight <= ReboundZoneMin) {
        HeightIncrement += random(0, ReboundZoneStrength);
    }
    if (CurrentHeight >= ReboundZoneMax) {
        HeightIncrement -= random(0, ReboundZoneStrength);
    }

    // Apply random variation with respect to roughness
    HeightIncrement += random(-1 - round(HeightIncrement / Roughness), 2 - round(HeightIncrement / Roughness));
    CurrentHeight += HeightIncrement;

    // Store the generated height in the array
    Landscape[i] = CurrentHeight;
  }

  // Find the minimum and maximum height values
  MinValue = 300;
  MaxValue = 0;
  for (int i = 1; i < size; i++) {
    if (Landscape[i] > MaxValue) MaxValue = Landscape[i];
    if (Landscape[i] < MinValue) MinValue = Landscape[i];
  }

  // Normalize the height values to the 0-4095 range
  for (int i = 1; i < size; i++) {
    Landscape[i] = map(Landscape[i], MinValue, MaxValue, 0, 4095);
  }
}

// Compares two arrays to check if they are identical.  
// If they match, a new landscape is generated for morphing
int CompareArrays(uint16_t *a, uint16_t *b, uint16_t n) {
  for (int i = 0; i < n; i++) {
    if (a[i] != b[i]) {
      return 0;
    }
  }
  return 1;
} 

// Gradually transforms one random landscape into another, step by step 
void Transform(uint16_t *a, uint16_t *b, uint16_t n) {
  for (int i = 0; i < n; i++) {
    if (a[i] < b[i]) {
      a[i] = a[i] + 1;
    }
    if (a[i] > b[i]) {
      a[i] = a[i] - 1;
    }
  }
}

// Metamorphism Update
void MetamorphismUpdate(void){
  if(oldLFO != LFO){
    oldLFO = LFO;
    MetamorphismTimer.setPeriod(map(LFO, 0, 255, 1000, 70));
  } 
}

// Handles landscape morphing by transforming one landscape into another.  
// Calls the Transform function and compares arrays to generate a new landscape if needed
void LandscapeMorph(void){
  if(OSCsliderSelect == 0){
    Transform(LandscapeA, LandscapeB, SIZE);
    if(CompareArrays(LandscapeA, LandscapeB, SIZE)){
      CreateLandscape(LandscapeB, SIZE);
    }
  }
}

void LandscapeMorphInterrupt(void){
  LandscapeMorphInterruptState = HIGH;
}
