/*
   16n Faderbank Configuration Sysex Processing
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



void processIncomingSysex(byte* sysexData, unsigned size) {
  D(Serial.println("Ooh, sysex"));
  D(printHexArray(sysexData, size));
  D(Serial.println());

  if (size < 3) {
    D(Serial.println("That's an empty sysex, bored now"));
    return;
  }

  // if(!(sysexData[1] == 0x00 && sysexData[2] == 0x58 && sysexData[3] == 0x49)) {
  if (!(sysexData[1] == 0x7d && sysexData[2] == 0x00 && sysexData[3] == 0x00)) {
    D(Serial.println("That's not a sysex message for us"));
    return;
  }

  switch (sysexData[4]) {
    case 0x1f:
      // 1F = "1nFo" - please send me your current config
      D(Serial.println("Got an 1nFo request"));
      if (pageChanged || millis() - lastSysex > pageChangedTimeout) {
        sendCurrentState();
        pageChanged = false;
        lastSysex = millis();
      }
      break;
    case 0x0e:
      // 0E - c0nfig Edit - here is a new config
      D(Serial.println("Incoming c0nfig Edit"));
      updateAllSettingsAndStoreInEEPROM(sysexData, size);
      break;
    case 0x1a:
      // 1A - 1nitiAlize - blank EEPROM and reset to factory settings.
      D(Serial.println("Incoming 1nitiAlize request"));
      initializeFactorySettingsFast();
      break;
  }
}

void updateAllSettingsAndStoreInEEPROM(byte* newConfig, unsigned size) {
  // store the settings from sysex in flash
  // also update all our settings.
  D(Serial.print("Received a new config with size "));
  D(Serial.println(size));
  // D(printHexArray(newConfig,size));

  // Changed to add bufferSize
  updateSettingsBlockAndStoreInEEPROM(newConfig, size, 9, bufferSize, 0);
}

void updateSettingsBlockAndStoreInEEPROM(byte* configFromSysex, unsigned sysexSize, int configStartIndex, int configDataLength, int EEPROMStartIndex) {
  int b_off = page * bufferSize;
  EEPROMStartIndex = EEPROMStartIndex + b_off;
  D(Serial.print("Storing data of size "));
  D(Serial.print(configDataLength));
  D(Serial.print(" at location "));
  D(Serial.print(EEPROMStartIndex));
  D(Serial.print(" from data of length "));
  D(Serial.print(sysexSize));
  D(Serial.print(" beginning at byte "));
  D(Serial.println(configStartIndex));
  D(printHexArray(configFromSysex, sysexSize));

  // walk the config, ignoring the top, tail, and firmware version
  byte dataToWrite[configDataLength];

  for (int i = 0; i < (configDataLength); i++) {
    int configIndex = i + configStartIndex;
    dataToWrite[i] = configFromSysex[configIndex];
  }

  // write new Data
  EEPROM.writeArray(EEPROMStartIndex, dataToWrite, configDataLength);


  // now load that.
  loadSettingsFromEEPROM();
}

void sendCurrentState() {
  //   0F - "c0nFig" - outputs its config:

  byte sysexData[8 + bufferSize];


  sysexData[0] = 0x7d; // manufacturer
  sysexData[1] = 0x00;
  sysexData[2] = 0x00;

  sysexData[3] = 0x0F; // ConFig;

  sysexData[4] = DEVICE_ID; // Device 01, ie, dev board
  sysexData[5] = MAJOR_VERSION; // major version
  sysexData[6] = MINOR_VERSION; // minor version
  sysexData[7] = POINT_VERSION; // point version

  byte buffer[bufferSize];
  int b_off = page * bufferSize;

  readEEPROMArray(b_off, buffer, bufferSize);

  int offset = 8;
  for (int i = 0; i < bufferSize; i++) {
    byte data = buffer[i];
    if (data == 0xff) {
      data = 0x7f;
    }
    sysexData[i + offset] = data;
  }
  byte howMuchToSend = 120;

  D(Serial.println("Sending this data"));
  D(printBytesAsIntsArray(sysexData, howMuchToSend));
  myMidi.sendSysEx(howMuchToSend, sysexData);
  //  myMidi.sendSysEx(bufferSize+8, sysexData);
  forceMidiWrite = true;
}
