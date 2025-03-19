/*
       _(  )_( )_
     (_   _    _)
    / /(_) (__)
   / / / / / /
  / / / / / /

*/
 
boolean ButtonState = false;
boolean CreateFlag = false;
volatile boolean MenuState = false;
boolean Skipped = true;
boolean SkipFlag = false;
boolean MenuSaveEEPROM = false;
short OldScale = -1;
short OldElement = -1;
unsigned long ButtonPressTime = 0;
boolean LedLastState = false;
volatile short MenuKeyPressed = 0;
volatile byte MenuUpdated = LOW;

PeriodicTimer ButtonTimer(TMR3);

void LedMenu(void){
  if(LedLastState == false){
    analogWrite(LED_PIN, 255);
    LedLastState = true;
  }else{
    analogWrite(LED_PIN, 0);
    LedLastState = false;
  }
}

void CheckButtonState(void){
  ButtonState = !digitalRead(BUTTON_PIN); // HIGH is Pressed!
}

void HandleButtonActions(){

  // Create a new landsacape after the button is released
  if(ButtonState == LOW && MenuState == false && CreateFlag == true && millis() - ButtonPressTime > 40){
    CreateNewLandscape();
    CreateFlag = false;
   } 

  // The menu settings are adjusted via the keyboard  
  // Pressing the button skips parameter selection and keeps the current values
  if(ButtonState == LOW && SkipFlag == false && MenuState == true){
    SkipFlag = true;
    Skipped = false;
  }

  // Button is pressed!
  if(ButtonState == HIGH){

     // Create landscape after the button is released is menu is off
     CreateFlag = true;

    // Start debounce filter
    if(ButtonPressTime ==  0) ButtonPressTime = millis();

    // Skip changes if the button is pressed
    if(MenuState == true && MenuKeyPressed < 2 && Skipped == false && millis() - ButtonPressTime > 40){
      Skipped = true;
      SkipFlag = false;
      MenuKeyPressed++;
      
      // Skip Element settings
      if(MenuKeyPressed == 1){
        MenuUpdated = HIGH;
        ButtonTimer.setPeriod(25000);
      }
      // Skip Scale settings
      if(MenuKeyPressed > 1){
        MenuKeyPressed = 0;
        ButtonTimer.stop();
        MenuState = false; 
        MenuUpdated = LOW;
        analogWrite(LED_PIN, 0);
      }
   } 

   // Open menu by holding the button
    if(millis() - ButtonPressTime >  1000 && !MenuState){
      MenuState = true;
      analogWrite(LED_PIN, 0);
      ButtonTimer.begin(LedMenu, 50000);
    }

  // When the button is not pressed
  }else{
    ButtonPressTime = 0;
    CreateFlag = false;
  }

  if(MenuState == true){
    Menu();
  }

  // Save to EEPROM if settings were changed
  SaveToEEPOMMenuSettings();
}

// Change settings using the keyboard 
void Menu(void){
  // Change Element settings using the keyboard
  if(MenuKeyPressed == 1 && MenuUpdated == LOW){
    MenuUpdated = HIGH;
    SelectElementFunction = KeybordKey;
    ButtonTimer.setPeriod(25000);
  }
  // Change Scale settings using the keyboard
  if(MenuKeyPressed > 1){
    MenuUpdated = LOW;
    MenuState = false; // Exit menu
    ScaleNumber = KeybordKey;
    ButtonTimer.stop();
    MenuKeyPressed = 0;
    analogWrite(LED_PIN, 0);
  }
}

void SaveToEEPOMMenuSettings(void){
  // Update OldElement and OldScale values
  if(MenuState == true && MenuSaveEEPROM == false){
    MenuSaveEEPROM = true;
    OldElement = SelectElementFunction;
    OldScale = ScaleNumber;
  }
  if(MenuState == false && MenuSaveEEPROM == true){
    MenuSaveEEPROM = false;
    SkipFlag = false;
    Skipped = true;
    if(OldScale != ScaleNumber || OldElement != SelectElementFunction){
      noInterrupts();
      EEPROMwriteSettings(ScaleNumber+1, SelectElementFunction+1);
      interrupts();
    }
  }
}
