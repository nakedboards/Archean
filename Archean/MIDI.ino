MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);
uint16_t MIDItype, MIDInote, MIDIvelocity;
uint16_t MidiUsbCounter = 0;
uint16_t MidiSerialCounter = 0;
volatile byte MIDInoteRecieved = false;
uint16_t RecievedMIDINote;
uint16_t Channel;
uint16_t oldFineCC;
uint16_t oldAttackCC;
uint16_t oldDecayCC;
uint16_t oldSustCC;
uint16_t oldReleaseCC;
uint16_t oldDistanceCC;

void MIDIinit(void){
  MIDI.begin(MIDI_CHANNEL_OMNI);
}

void serialMIDIread(void){
  if (MIDI.read()){
    byte type = MIDI.getType();
    switch (type){
      case midi::NoteOn:
        MIDInoteRecieved = true;
        RecievedMIDINote = MIDI.getData1();
        MIDIvelocity = MIDI.getData2();
        Channel = MIDI.getChannel();
        if(RecievedMIDINote >= 12 && RecievedMIDINote <= 96 && Channel == 1){
          if(MIDIvelocity > 0){
            MidiSerialCounter++;
            Gate = true;
            analogWrite(LED_PIN, 255);
          }else{
            MidiSerialCounter--;
            if(MidiSerialCounter == 0){
              Gate = false;
              analogWrite(LED_PIN, 0);
            }
          }
          break;
        }
      case midi::NoteOff:
        RecievedMIDINote = MIDI.getData1();
        Channel = MIDI.getChannel();
        if(RecievedMIDINote >= 12 && RecievedMIDINote <= 96 && Channel == 1){
          MidiSerialCounter--;
          if(MidiSerialCounter == 0){
            Gate = false;
            analogWrite(LED_PIN, 0);
          }
          break;
        }
      default:
        break;
    }
  }
}

void OnNoteOn(byte channel, byte note, byte velocity){
  if(note >= 12 && note <= 96 && channel == 1){
    RecievedMIDINote = note;
    MidiUsbCounter++;
    MIDInoteRecieved = true;
    Gate = true;
    analogWrite(LED_PIN, 255);
  }
}

void OnNoteOff(byte channel, byte note, byte velocity){
  if(note >= 12 && note <= 96 && channel == 1){
    MidiUsbCounter--;
    if(MidiUsbCounter == 0){
      Gate = false;
      analogWrite(LED_PIN, 0);
    }
  }
}

void USBMIDISendMessages(void){
  // Note on
  for(int i = 0; i < 22; i++){
    if(KeyboardMIDIstatus[i] == 1 && KeyboardMIDInotes[i] != -1){
      usbMIDI.sendNoteOn(KeyboardMIDInotes[i], 127, 1);
      usbMIDI.send_now();
      KeyboardMIDIstatus[i] = -1;
    }
  }
  // Note off
  for(int i = 0; i < 22; i++){
    if(KeyboardMIDIstatus[i] == 0 && KeyboardMIDInotes[i] != -1){
        usbMIDI.sendNoteOff(KeyboardMIDInotes[i], 0, 1);
        usbMIDI.send_now();
        KeyboardMIDInotes[i] = -1;
    }
  }

  // Distance CC
  uint16_t DistanceCC = map(SmoothedDistance, 50, 350, 127, 0);
  if(oldDistanceCC != DistanceCC){
    usbMIDI.sendControlChange(22, DistanceCC, 1);
    oldDistanceCC = DistanceCC;
  }

  // Fine CC
  uint16_t FineCC = Fine >> 1;
  if(oldFineCC != FineCC){
    usbMIDI.sendControlChange(23, FineCC, 1);
    oldFineCC = FineCC;
  }

  // Attack CC
  uint16_t AttackCC = Attack >> 1;
  if(oldAttackCC != AttackCC){
    usbMIDI.sendControlChange(24, AttackCC, 1);
    oldAttackCC = AttackCC;
  }

  // Decay CC
  uint16_t DecayCC = Decay >> 1;
  if(oldDecayCC != DecayCC){
    usbMIDI.sendControlChange(25, DecayCC, 1);
    oldDecayCC = DecayCC;
  }

  // Sustain CC
  uint16_t SustCC = Sust >> 1;
  if(oldSustCC != SustCC){
    usbMIDI.sendControlChange(26, SustCC, 1);
    oldSustCC = SustCC;
  }

  // Release CC
  uint16_t ReleaseCC = Release >> 1;
  if(oldReleaseCC != ReleaseCC){
    usbMIDI.sendControlChange(27, ReleaseCC, 1);
    oldReleaseCC = ReleaseCC;
  }
}
