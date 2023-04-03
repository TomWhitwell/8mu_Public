void printHex(uint8_t num) {
  char hexCar[2];

  sprintf(hexCar, "%02X", num);
  D(Serial.print(hexCar));
}

void printHexArray(byte* array, int size) {
  for (int i = 0; i < size; i++) {
    printHex(array[i]);
    D(Serial.print(" "));
  }
  D(Serial.println());
}

void printBytesAsIntsArray(byte* array, int size) {
  for (int i = 0; i < size; i++) {
    Serial.print(array[i]);
    D(Serial.print(" "));
  }
  D(Serial.println());
}

void printIntArray(int* array, int size) {
  for (int i = 0; i < size; i++) {
    Serial.print(array[i]);
    D(Serial.print("\t"));
  }
  D(Serial.println());
}

void readEEPROMArray(int start, byte buffer[], int length) {
  for (int i = 0; i < length; i++) {
    buffer[i] = EEPROM.read(start + i);
  }
}



void calibrateIMU() {
  delay(1000);
  D(Serial.println("Internal sensor offsets BEFORE calibration..."));
  D(Serial.print(BMI160.getXAccelOffset()));
  D(Serial.print("\t"));
  D(Serial.print(BMI160.getYAccelOffset()));
  D(Serial.print("\t"));
  D(Serial.print(BMI160.getZAccelOffset()));
  D(Serial.print("\t"));
  D(Serial.print(BMI160.getXGyroOffset()));
  D(Serial.print("\t"));
  D(Serial.print(BMI160.getYGyroOffset()));
  D(Serial.print("\t"));
  D(Serial.println(BMI160.getZGyroOffset()));
  D(Serial.print("Starting Acceleration calibration..."));
  BMI160.autoCalibrateXAccelOffset(0);
  BMI160.autoCalibrateYAccelOffset(0);
  BMI160.autoCalibrateZAccelOffset(2);
  D(Serial.println(" Done"));
  D(Serial.println("Internal sensor offsets AFTER calibration..."));
  D(Serial.print(BMI160.getXAccelOffset()));
  D(Serial.print("\t")); // -76
  D(Serial.print(BMI160.getYAccelOffset()));
  D(Serial.print("\t")); // -2359
  D(Serial.print(BMI160.getZAccelOffset()));
  D(Serial.print("\t")); // 1688
  D(Serial.print(BMI160.getXGyroOffset()));
  D(Serial.print("\t")); // 0
  D(Serial.print(BMI160.getYGyroOffset()));
  D(Serial.print("\t")); // 0
  D(Serial.println(BMI160.getZGyroOffset()));
  D(Serial.println("Enabling Gyroscope/Acceleration offset compensation"));
  BMI160.setGyroOffsetEnabled(true);
  BMI160.setAccelOffsetEnabled(true);
  // Blinkenlights
  for (int i = 0; i < 8; i++) {
    for (int j = 0; j < 4; j++) {
      digitalWrite(leds[3 - j], HIGH);
      digitalWrite(leds[4 + j], HIGH);
      delay(30);
      digitalWrite(leds[3 - j], LOW);
      digitalWrite(leds[4 + j], LOW);
      delay(30);
    }
  }
}

// Returns max value for each DX7 parameter
const int dxLookup[] = {99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 3, 3, 7, 3, 7, 99,
                        1, 31, 99, 14, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 3, 3, 99, 99, 7, 99, 1, 31,
                        99, 14, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 3, 3, 99, 99, 7, 99, 1, 31, 99, 14,
                        99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 99, 3, 3, 99, 99, 7, 99, 1, 31, 99, 14, 99, 99,
                        99, 99, 99, 99, 99, 99, 99, 99, 99, 3, 3, 99, 99, 7, 99, 1, 31, 99, 14, 99, 99, 99, 99,
                        99, 99, 99, 99, 99, 99, 99, 3, 3, 99, 99, 7, 99, 1, 31, 99, 14, 99, 99, 99, 99, 99, 99,
                        99, 99, 31, 7, 1, 99, 99, 99, 99, 1, 4, 99
                       };


//void dxSysex(uint8_t param, uint8_t value) {
//  byte DXSYSEX[] = {0x43, 0x10, 0x00, 0x00, 0x00};
//  DXSYSEX[2] = (param >= 128 ? 0x01 : 0x00);
//  DXSYSEX[3] = 0x7f & param;
//  DXSYSEX[4] = 0x7f & value;
//  myMidi.sendSysEx(5,DXSYSEX);
//};
//


//void make_sysex_message(uint8_t param, uint8_t value, uint8_t *buf, bool voice_param) {
//  buf[0] = 0x43; // Yamaha identifier
//  buf[1] = 0x10; // Sub-status 1, channel number 1
//  buf[2] = ((voice_param ? 0 : 2) << 2) | (param >= 128 ? 1 : 0);
//  buf[3] = 0x7f & param;
//  buf[4] = 0x7f & value;
//}


/*
  def dx_sysex(self, parameter, value):
    DXSYSEX = [0x43, 0x10, 0x00, 0x00, 0x00]
    if parameter>127:
        DXSYSEX[2] = 0x01
    DXSYSEX[3] = parameter & 0x7f
    DXSYSEX[4] = value
    self._outbuf[0] = 0xf0
    self._outbuf[1] = DXSYSEX[0]
    self._outbuf[2] = DXSYSEX[1]
    self._outbuf[3] = DXSYSEX[2]
    self._outbuf[4] = DXSYSEX[3]
    self._outbuf[5] = DXSYSEX[4]
    self._outbuf[6] = 0xf7
    self._send(self._outbuf, 7)

*/

void allLeds(boolean state) {
  for (int ledscan = 0; ledscan < NUM_LEDS; ledscan++) {
    digitalWrite(leds[ledscan], state);
  }
}

void ledRandom() {
  for (int ledscan = 0; ledscan < NUM_LEDS; ledscan++) {
    digitalWrite(leds[ledscan], random(2));
  }
}


void ledAnimate(int target) {
  // LED animate
  for (int q = 0; q < NUM_LEDS; q++) {
    digitalWrite(leds[constrain(target + q, 0, NUM_LEDS)], HIGH);
    digitalWrite(leds[constrain(target - q, 0, NUM_LEDS)], HIGH);
    delay(25);
  }
  for (int q = 0; q < NUM_LEDS; q++) {
    digitalWrite(leds[constrain(target + q, 0, NUM_LEDS)], LOW);
    digitalWrite(leds[constrain(target - q, 0, NUM_LEDS)], LOW);
    delay(25);
  }
}

void checkPage() {

  // Check button state
  buttons[4].update();
  buttons[5].update();

  enum DataButtonState {
    doNothing,
    dataIncrement,
    dataDecrement,
    bothPressed
  } buttonState = doNothing;

  // Display currently active page and wipe any existing LEDS
  if (changingPage) {
    allLeds(LOW);
    digitalWrite(leds[tempPage], HIGH);
  }

  // Check on button up for tapped up/down
  if (buttons[4].rose() && !teachMode) {
    buttonState = dataDecrement;
    pageChange = millis();
  }

  if (buttons[5].rose() && !teachMode) {
    buttonState = dataIncrement;
    pageChange = millis();
  }

  // Check if R button held down
  // longHeld stops page increment/decrement when  button is released
  if (buttons[5].read() == 0
      && buttons[5].duration() > buttonHoldTime
      && longHeld == false
     ) {
    buttonState = bothPressed;
    longHeld = true;
  }

  // Reset longHeld by pressing  button
  if (buttons[5].fell() || buttons[4].fell()) longHeld = false;


  switch (buttonState) {
    case doNothing:
      break;
    case dataIncrement:
      tempPage ++;
      break;
    case dataDecrement:
      tempPage --;
      break;
    case bothPressed:
      if (teachMode == normal) teachMode = teachFader;
      else if (teachMode == teachFader) teachMode = teachIMU;
      else if (teachMode == teachIMU) teachMode = normal;
      return;
      break;
  }

  // Wrap aroung page changes
  if (tempPage < 0) tempPage = 7;
  if (tempPage > 7) tempPage = 0;

  // Ensure no page changes caused two buttons being held down are released
  if (longHeld) tempPage = page;
  if (tempPage != page) changingPage = true;

  // Time's up: do a page change if necessary
  if (millis() - pageChange > pageChangeTime && changingPage) {

    page = tempPage;
    changingPage = false;
    BANK_STORE.write(0, page); // update EEPROM with new setting
    loadSettingsFromEEPROM();
    // Send new settings to editor ??
    ledAnimate(page);
    pageChanged = true;

  }
}

void doLeds() {
  switch (teachMode) {

    case normal:
      {
        boolean ledBlinking = (millis() - lastMidiActivity < midiBlinkTime);
        for (int i = 0; i < NUM_LEDS; i++) {

          if (ledBlinking && midiBlink[i] && faderBlink && (usbChannels[i] > 0) ) {
            digitalWrite(leds[i], HIGH);
          }
          else if (ledBlinking && midiBlink[i + 8] && accelBlink && (usbChannels[i + 8] > 0)) {
            digitalWrite(leds[i], HIGH);
          }
          else {
            digitalWrite(leds[i], LOW);
          }
        }
        break;
      }
    case teachFader:
      {
        if (millis() - lastBlink > teachBlinkTime) {
          for (int i = 0; i < NUM_LEDS; i++) {
            digitalWrite(leds[i], (i < 4) && teachBlink);
          };
          lastBlink = millis();
          teachBlink = !teachBlink;
        }

        break;
      }
    case teachIMU:
      {
        if (millis() - lastBlink > teachBlinkTime) {
          for (int i = 0; i < NUM_LEDS; i++) {
            digitalWrite(leds[i], (i > 3) && teachBlink);
          };
          lastBlink = millis();
          teachBlink = !teachBlink;
        }
      }
  }
}
