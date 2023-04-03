/*
   16n Faderbank EEPROM-based configuration
   (c) 2020 by Tom Armitage
   (c) 2023 by Tom Whitwell
   MIT License
*/

// activates printing of debug messages
// #define DEBUG 0

// wrap code to be executed only under DEBUG conditions in D()
#ifdef DEBUG
#define D(x) x
#else
#define D(x)
#endif


const int defaultUSBCCs[] = {34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49};
const int defaultTRSCCs[] = {34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49};

const int defaultUSBChannels[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
const int defaultTRSChannels[] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};


const int defaultButtonUSBMode[] = {1, 1, 1, 1};
const int defaultButtonTRSMode[] = {1, 1, 1, 1};

const int defaultButtonUSBParamA[] = {36, 48, 60, 72};
const int defaultButtonUSBParamB[] = {64, 64, 64, 64};

const int defaultButtonTRSParamA[] = {36, 48, 60, 72};
const int defaultButtonTRSParamB[] = {64, 64, 64, 64};

const int defaultButtonUSBChannels[] = {1, 1, 1, 1};
const int defaultButtonTRSChannels[] = {1, 1, 1, 1};

const int bufferSize = 112;
const int bankCount = 8;

void checkDefaultSettings() {
  // if byte1 of EEPROM is FF for whatever reason, let's assume the machine needs initializing
  int firstByte = EEPROM.read(0x00);

  if (firstByte > 0x01) {
    D(Serial.println("First Byte is > 0x01, probably needs initialising"));
    initializeFactorySettingsFast();
  } else {
    D(Serial.print("First Byte is set to: "));
    D(printHex(firstByte));
    D(Serial.println());
    byte buffer[bufferSize];
    readEEPROMArray(0, buffer, bufferSize);
    D(Serial.println("Config found:"));
    D(printHexArray(buffer, bufferSize));
  }
}

void initializeFactorySettingsFast() {
  BANK_STORE.write(0, 0); // Start in Page 0

  byte bytesToWrite[bufferSize];

  // set default config flags (LED ON, LED DATA, ROTATE, etc)
  bytesToWrite[0] = 1; // Fader Blink
  bytesToWrite[1] = 0; // Accelerometer Blink
  bytesToWrite[2] = 0; // ROTATE
  bytesToWrite[3] = 0; // Ignore
  bytesToWrite[4] = 15; // fadermin LSB
  bytesToWrite[5] = 0; // fadermin MSB
  bytesToWrite[6] = 112; // fadermax LSB
  bytesToWrite[7] = 31; // fadermax MSB
  bytesToWrite[8] = 1; // Soft midi thru
  bytesToWrite[9] = 0; // Midi Mode A
  bytesToWrite[10] = 0; // DX7 Mode



  for (int i = 12; i < 16; i ++) {
    bytesToWrite[i] =  0; // blank remaining config slots.
  }

  // set default USB channels
  for (int i = 0; i < channelCount; i++) {
    int baseAddress = 16;
    int writeAddress = baseAddress + i;
    bytesToWrite[writeAddress] = defaultUSBChannels[i];
  }

  // set default TRS channels
  for (int i = 0; i < channelCount; i++) {
    int baseAddress = 32;
    int writeAddress = baseAddress + i;
    bytesToWrite[writeAddress] = defaultTRSChannels[i];
  }

  // set default USB  ccs
  for (int i = 0; i < channelCount; i++) {
    int baseAddress = 48;
    int writeAddress = baseAddress + i;
    bytesToWrite[writeAddress] = defaultUSBCCs[i];
  }

  // set default TRS  ccs
  for (int i = 0; i < channelCount; i++) {
    int baseAddress = 64;
    int writeAddress = baseAddress + i;
    bytesToWrite[writeAddress] = defaultTRSCCs[i];
  }


  // set default Button usb channels
  for (int i = 0; i < buttonCount; i++) {
    int baseAddress = 80;
    int writeAddress = baseAddress + i;
    bytesToWrite[writeAddress] = defaultButtonUSBChannels[i];
  }

  // set default Button TRS channels
  for (int i = 0; i < buttonCount; i++) {
    int baseAddress = 84;
    int writeAddress = baseAddress + i;
    bytesToWrite[writeAddress] = defaultButtonTRSChannels[i];
  }

  // set default Button usb modes
  for (int i = 0; i < buttonCount; i++) {
    int baseAddress = 88;
    int writeAddress = baseAddress + i;
    bytesToWrite[writeAddress] =  defaultButtonUSBMode[i];
  }

  // set default Button TRS modes
  for (int i = 0; i < buttonCount; i++) {
    int baseAddress = 92;
    int writeAddress = baseAddress + i;
    bytesToWrite[writeAddress] = defaultButtonTRSMode[i];
  }

  // set default Button USB Param A
  for (int i = 0; i < buttonCount; i++) {
    int baseAddress = 96;
    int writeAddress = baseAddress + i;
    bytesToWrite[writeAddress] = defaultButtonUSBParamA[i];
  }

  // set default Button TRS Param A
  for (int i = 0; i < buttonCount; i++) {
    int baseAddress = 100;
    int writeAddress = baseAddress + i;
    bytesToWrite[writeAddress] = defaultButtonTRSParamA[i];
  }

  // set default Button USB Param B
  for (int i = 0; i < buttonCount; i++) {
    int baseAddress = 104;
    int writeAddress = baseAddress + i;
    bytesToWrite[writeAddress] = defaultButtonUSBParamB[i];
  }

  // set default Button TRS Param B
  for (int i = 0; i < buttonCount; i++) {
    ledRandom();
    int baseAddress = 108;
    int writeAddress = baseAddress + i;
    bytesToWrite[writeAddress] = defaultButtonTRSParamB[i];
  }



  for (int bankToWrite = 0; bankToWrite < bankCount; bankToWrite++) {
    int b_off = bankToWrite * bufferSize;
    EEPROM.writeArray(b_off, bytesToWrite, bufferSize);
    EEPROM.write(11 + b_off, bankToWrite); // Current Bank Number
    ledRandom();
  }


  // serial dump that config.
  byte buffer[bufferSize];
  readEEPROMArray(0, buffer, bufferSize);
  D(Serial.println("Config Instantiated."));
  D(Serial.print(bankCount));
  D(Serial.println(" copies of:"));
  D(printHexArray(buffer, bufferSize));
}

void loadSettingsFromEEPROM() {
  // load current page
  page = BANK_STORE.read(0);
  D(Serial.print("Active page loaded = "));
  D(Serial.println(page));
  allLeds(0);
  ledAnimate(page);
  digitalWrite(leds[page], HIGH);

  int b_off = page * bufferSize;


  // load usb channels
  for (int i = 0; i < channelCount; i++) {
    int baseAddress = 16;
    int readAddress = baseAddress + i + b_off;
    usbChannels[i] = EEPROM.read(readAddress);
  }

  D(Serial.println("USB Channels loaded:"));
  D(printIntArray(usbChannels, channelCount));

  // load TRS channels
  for (int i = 0; i < channelCount; i++) {
    int baseAddress = 32;
    int readAddress = baseAddress + i + b_off;
    trsChannels[i] = EEPROM.read(readAddress);
  }

  D(Serial.println("TRS Channels loaded:"));
  D(printIntArray(trsChannels, channelCount));

  // load USB ccs
  for (int i = 0; i < channelCount; i++) {
    int baseAddress = 48;
    int readAddress = baseAddress + i + b_off;
    usbCCs[i] = EEPROM.read(readAddress);
  }

  D(Serial.println("USB CCs loaded:"));
  D(printIntArray(usbCCs, channelCount));


  // load TRS ccs
  for (int i = 0; i < channelCount; i++) {
    int baseAddress = 64;
    int readAddress = baseAddress + i + b_off;
    trsCCs[i] = EEPROM.read(readAddress);
  }

  D(Serial.println("TRS CCs loaded:"));
  D(printIntArray(trsCCs, channelCount));

  // ** Buttons **
  // load Button USB Channels
  for (int i = 0; i < buttonCount; i++) {
    int baseAddress = 80;
    int readAddress = baseAddress + i + b_off;
    buttonUSBChannels[i] = EEPROM.read(readAddress);
  }

  D(Serial.println("Button USB Channels loaded:"));
  D(printIntArray(buttonUSBChannels, buttonCount));

  // load Button TRS Channels
  for (int i = 0; i < buttonCount; i++) {
    int baseAddress = 84;
    int readAddress = baseAddress + i + b_off;
    buttonTRSChannels[i] = EEPROM.read(readAddress);
  }

  D(Serial.println("Button TRS Channels loaded:"));
  D(printIntArray(buttonTRSChannels, buttonCount));

  // load Button USB Modes
  for (int i = 0; i < buttonCount; i++) {
    int baseAddress = 88;
    int readAddress = baseAddress + i + b_off;
    buttonUSBMode[i] = EEPROM.read(readAddress);
  }

  D(Serial.println("Button USB Modes loaded:"));
  D(printIntArray(buttonUSBMode, buttonCount));

  // load Button TRS Modes
  for (int i = 0; i < buttonCount; i++) {
    int baseAddress = 92;
    int readAddress = baseAddress + i + b_off;
    buttonTRSMode[i] = EEPROM.read(readAddress);
  }

  D(Serial.println("Button TRS Modes loaded:"));
  D(printIntArray(buttonTRSMode, buttonCount));

  // load Button USB Param A
  for (int i = 0; i < buttonCount; i++) {
    int baseAddress = 96;
    int readAddress = baseAddress + i + b_off;
    buttonUSBParamA[i] = EEPROM.read(readAddress);
  }

  D(Serial.println("Button USB ParamA loaded:"));
  D(printIntArray(buttonUSBParamA, buttonCount));

  // load Button TRS Param A
  for (int i = 0; i < buttonCount; i++) {
    int baseAddress = 100;
    int readAddress = baseAddress + i + b_off;
    buttonTRSParamA[i] = EEPROM.read(readAddress);
  }

  D(Serial.println("Button TRS ParamA loaded:"));
  D(printIntArray(buttonTRSParamA, buttonCount));

  // load Button USB Param B
  for (int i = 0; i < buttonCount; i++) {
    int baseAddress = 104;
    int readAddress = baseAddress + i + b_off;
    buttonUSBParamB[i] = EEPROM.read(readAddress);
  }

  D(Serial.println("Button USB ParamB loaded:"));
  D(printIntArray(buttonUSBParamB, buttonCount));

  // load Button TRS Param B
  for (int i = 0; i < buttonCount; i++) {
    int baseAddress = 108;
    int readAddress = baseAddress + i + b_off;
    buttonTRSParamB[i] = EEPROM.read(readAddress);
  }

  D(Serial.println("Button TRS ParamB loaded:"));
  D(printIntArray(buttonTRSParamB, buttonCount));

  // load other config
  faderBlink = EEPROM.read(0 + b_off);
  accelBlink = EEPROM.read(1 + b_off);
  flip = EEPROM.read(2 + b_off);
  myMidi._midiThru = EEPROM.read(8 + b_off);
  midiMode = EEPROM.read(9 + b_off);
  myMidi.changeHardMidiMode(midiMode);
  dxMode = EEPROM.read(10 + b_off);

  // Set the current page to the page recieved from the Editor
  // page = EEPROM.read(11 + b_off);
  // Ensure we don't confuse page switching system
  //  tempPage = page;
  //  changingPage = false;



  int faderminLSB = EEPROM.read(4); // NB: Only read from Bank 0 = no b_off
  int faderminMSB = EEPROM.read(5); // NB: Only read from Bank 0 = no b_off

  D(Serial.print ("Setting fadermin to "));
  D(Serial.println((faderminMSB << 7) + faderminLSB));
  faderMin = (faderminMSB << 7) + faderminLSB;

  int fadermaxLSB = EEPROM.read(6); // NB: Only read from Bank 0 = no b_off
  int fadermaxMSB = EEPROM.read(7); // NB: Only read from Bank 0 = no b_off

  D(Serial.print ("Setting fadermax to "));
  D(Serial.println((fadermaxMSB << 7) + fadermaxLSB));
  faderMax = (fadermaxMSB << 7) + fadermaxLSB;
}
