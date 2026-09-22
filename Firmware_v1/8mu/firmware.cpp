#include "firmware.h"

const byte LED_PINS[NUM_LEDS] = {0, 44, 5, 7, 10, 11, 12, 13};
const byte BUTTON_PINS[NUM_BUTTONS] = {29, 30, 28, 2, 31, 3};
const byte FADER_PINS[FADER_COUNT] = {A0, A1, A2, A3, A4, A5, A10, A11};

Bounce buttons[NUM_BUTTONS];
ResponsiveAnalogRead *analog[FADER_COUNT];
ResponsiveAnalogRead *gestures[GESTURE_COUNT];

ControllerConfig activeConfig;
BankConfig activeBankConfig;
uint8_t currentBank = 0;
uint8_t tempBank = 0;

uint8_t lastFaderMidiValues[FADER_COUNT];
uint8_t lastGestureMidiValues[GESTURE_COUNT];
bool midiBlink[FADER_COUNT];
unsigned long lastMidiActivity[FADER_COUNT];
unsigned long lastBlink = 0;
bool teachBlink = true;
bool bankLockout = false;
bool changingBank = false;
bool bankChangePending = false;
unsigned long bankChangeRequestedAt = 0;
bool doLedAnimate = false;
bool shouldDoWriteBankFile = false;
bool shouldSendConfig = false;
LearnMode learnMode = LEARN_MODE_OFF;
int IMU_readings[6];
bool flip = false;
uint8_t velocityLeds[NUM_LEDS] = {0, 0, 0, 0, 0, 0, 0, 0};

static unsigned long lastIMUReadAt = 0;
static unsigned long lastControlScanAt = 0;
static bool longClickConsumed = false;

static uint8_t readFaderMidiValue(uint8_t faderIndex, int *scaledValueOut, bool settleFilter) {
  uint16_t rawAdcValue = analogRead(FADER_PINS[faderIndex]);
  analog[faderIndex]->update(rawAdcValue);

  if (settleFilter) {
    analog[faderIndex]->update(rawAdcValue);
  }

  int filteredRaw = analog[faderIndex]->getValue();
  filteredRaw = constrain(filteredRaw, activeConfig.faderMin, activeConfig.faderMax);
  int scaledValue = map(filteredRaw, activeConfig.faderMin, activeConfig.faderMax, 0, (1 << ADC_RESOLUTION) - 1);
  scaledValue = ((1 << ADC_RESOLUTION) - 1) - scaledValue;

  if (scaledValueOut != nullptr) {
    *scaledValueOut = scaledValue;
  }

  return scaledValue >> 5;
}

void firmwareSetup() {
  analogReadResolution(ADC_RESOLUTION);

  midiBegin();

  for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].attach(BUTTON_PINS[i], INPUT_PULLUP);
    buttons[i].interval(10);
  }
  delay(50);

  BMI160.begin(BMI160GenClass::I2C_MODE);
  BMI160.setGyroRange(250);

  for (uint8_t i = 0; i < FADER_COUNT; i++) {
    analog[i] = new ResponsiveAnalogRead(FADER_PINS[i], true, .00001);
    analog[i]->setActivityThreshold(32);
    analog[i]->setAnalogResolution(1 << ADC_RESOLUTION);
    lastFaderMidiValues[i] = 0;
    midiBlink[i] = false;
    lastMidiActivity[i] = 0;
  }

  for (uint8_t i = 0; i < GESTURE_COUNT; i++) {
    gestures[i] = new ResponsiveAnalogRead(0, true, .000001);
    gestures[i]->setActivityThreshold(32);
    gestures[i]->setAnalogResolution(1 << ADC_RESOLUTION);
    lastGestureMidiValues[i] = 0;
  }

  // ResponsiveAnalogRead configures its pin as an input.  Gesture filters do
  // not use a physical analog pin, so restore the LED pins afterwards; this
  // is especially important for LED 1, which shares pin number 0.
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    pinMode(LED_PINS[i], OUTPUT);
    ledOff(i);
  }

  bool forceFactoryReset = true;
  for (uint8_t i = 0; i < OUTPUT_BUTTON_COUNT; i++) {
    if (digitalRead(BUTTON_PINS[i]) != LOW) {
      forceFactoryReset = false;
      break;
    }
  }

  bool storageReady = initialiseStorage(forceFactoryReset);

  uint8_t configFileBuffer[CONFIG_BYTES];
  if (storageReady && readConfigBytes(configFileBuffer, sizeof(configFileBuffer))) {
    bytesToConfig(configFileBuffer, activeConfig);
    currentBank = readBankIndexFromFile();
  } else {
    // A failed or unavailable external store must never leak erased 0xFF bytes
    // into MIDI configuration or SysEx.  Continue safely with RAM defaults.
    activeConfig = makeDefaultControllerConfig();
    currentBank = 0;
  }
  tempBank = currentBank;
  activeBankConfig = activeConfig.banks[currentBank];
  flip = activeConfig.rotated;
  setTrsMode(activeConfig.midiType);

  changingBank = true;
  bankChangePending = true;
  bankChangeRequestedAt = millis();
}

void firmwareLoop() {
  midiReadTask();

  if (shouldDoWriteBankFile) {
    writeBankIndexToFile(currentBank);
    shouldDoWriteBankFile = false;
  }

  if (bankChangePending && (millis() - bankChangeRequestedAt >= PAGE_CHANGE_TIME)) {
    bankChangePending = false;
    currentBank = tempBank;
    changingBank = false;
    doLedAnimate = true;
    shouldDoWriteBankFile = true;
    shouldSendConfig = true;
    activeBankConfig = activeConfig.banks[currentBank];
  }

  if (doLedAnimate) {
    ledAnimate(currentBank);
    doLedAnimate = false;
  }

  if (shouldSendConfig) {
    sendCurrentConfig();
    shouldSendConfig = false;
  }

  updateControls();

  if (learnMode == LEARN_MODE_OFF) {
    updateIMU();
  }

  updateLeds();
}

void updateControls(bool force) {
  for (uint8_t i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].update();
  }

  for (uint8_t i = 0; i < OUTPUT_BUTTON_COUNT; i++) {
    if (buttons[i].fell()) sendButtonDown(i);
    if (buttons[i].rose()) sendButtonUp(i);
  }

  if (buttons[4].rose()) handleBankDec();
  if (buttons[5].rose() && !longClickConsumed) handleBankInc();

  if (!buttons[5].read() && buttons[5].currentDuration() > LEARN_LONG_PRESS_MS && !longClickConsumed) {
    handleLearnModeChange();
    longClickConsumed = true;
  }
  if (buttons[5].rose()) {
    longClickConsumed = false;
  }

  if (!force && (micros() - lastControlScanAt < CONTROL_SCAN_INTERVAL_US)) {
    return;
  }
  lastControlScanAt = micros();

  for (uint8_t i = 0; i < FADER_COUNT; i++) {
    int scaledValue = 0;
    uint8_t candidateMidiValue = readFaderMidiValue(i, &scaledValue, force);
    uint8_t newMidiValue = lastFaderMidiValues[i];

    if (candidateMidiValue == 0 || candidateMidiValue == 127 || abs(int(candidateMidiValue) - int(lastFaderMidiValues[i])) > 1) {
      newMidiValue = candidateMidiValue;
    } else if (candidateMidiValue > lastFaderMidiValues[i]) {
      int upperThreshold = (candidateMidiValue << 5) + MIDI_VALUE_HYSTERESIS;
      if (scaledValue >= upperThreshold) {
        newMidiValue = candidateMidiValue;
      }
    } else if (candidateMidiValue < lastFaderMidiValues[i]) {
      int lowerThreshold = (lastFaderMidiValues[i] << 5) - MIDI_VALUE_HYSTERESIS;
      if (scaledValue <= lowerThreshold) {
        newMidiValue = candidateMidiValue;
      }
    }

    if (newMidiValue != lastFaderMidiValues[i] || force) {
      lastFaderMidiValues[i] = newMidiValue;
      midiBlink[i] = true;
      lastMidiActivity[i] = millis();

      uint8_t usbOutputValue = lastFaderMidiValues[i];
      uint8_t trsOutputValue = lastFaderMidiValues[i];
      uint8_t controlIndex = i;

      if (activeConfig.rotated) {
        usbOutputValue = 127 - usbOutputValue;
        trsOutputValue = 127 - trsOutputValue;
        controlIndex = 7 - i;
      }

      if (learnMode == LEARN_MODE_GESTURE) {
        controlIndex += 8;
      }

      sendUsbCC(activeBankConfig.usbAnalog[controlIndex].channel, activeBankConfig.usbAnalog[controlIndex].cc, usbOutputValue);
      sendTrsCC(activeBankConfig.trsAnalog[controlIndex].channel, activeBankConfig.trsAnalog[controlIndex].cc, trsOutputValue);
    }
  }
}

void refreshFaderMidiValues() {
  for (uint8_t i = 0; i < FADER_COUNT; i++) {
    lastFaderMidiValues[i] = readFaderMidiValue(i, nullptr, true);
  }
}

void updateIMU() {
  if (millis() - lastIMUReadAt < IMU_SCAN_INTERVAL_MS) return;
  lastIMUReadAt = millis();

  BMI160.readMotionSensor(IMU_readings[3], IMU_readings[4], IMU_readings[5], IMU_readings[0], IMU_readings[1], IMU_readings[2]);

  for (uint8_t i = 0; i < GESTURE_COUNT; i++) {
    gestures[i]->update(gesture(i));
    int temp = gestures[i]->getValue();
    temp = constrain(temp, 0, 4095);
    temp = map(temp, 0, 4095, 0, (1 << ADC_RESOLUTION) - 1);
    uint8_t newMidiValue = temp >> 5;

    if (newMidiValue != lastGestureMidiValues[i]) {
      lastGestureMidiValues[i] = newMidiValue;
      uint8_t controlIndex = i + 8;
      sendUsbCC(activeBankConfig.usbAnalog[controlIndex].channel, activeBankConfig.usbAnalog[controlIndex].cc, newMidiValue);
      sendTrsCC(activeBankConfig.trsAnalog[controlIndex].channel, activeBankConfig.trsAnalog[controlIndex].cc, newMidiValue);
    }
  }
}

void programChange(uint8_t bank) {
  if (bank >= BANK_COUNT) return;
  shouldSendConfig = true;
  currentBank = bank;
  tempBank = bank;
  shouldDoWriteBankFile = true;
  changingBank = false;
  bankChangePending = false;
  doLedAnimate = true;
  activeBankConfig = activeConfig.banks[currentBank];
}

void handleBankInc() {
  if (bankLockout) return;
  if (learnMode == LEARN_MODE_OFF) incTempBankBy(1);
}

void handleBankDec() {
  if (bankLockout) return;
  if (learnMode == LEARN_MODE_OFF) incTempBankBy(-1);
}

void incTempBankBy(int8_t delta) {
  tempBank = (tempBank + delta + BANK_COUNT) % BANK_COUNT;
  changingBank = true;
  bankChangePending = true;
  bankChangeRequestedAt = millis();
}

void handleLearnModeChange() {
  switch (learnMode) {
    case LEARN_MODE_OFF:
      learnMode = LEARN_MODE_FADER;
      break;
    case LEARN_MODE_FADER:
      learnMode = LEARN_MODE_GESTURE;
      break;
    case LEARN_MODE_GESTURE:
      learnMode = LEARN_MODE_OFF;
      clearLeds();
      break;
  }
}

void sendButtonDown(uint8_t buttonIndex) {
  if (activeConfig.rotated) buttonIndex = 3 - buttonIndex;

  ButtonConfig usbConfig = activeBankConfig.usbButton[buttonIndex];
  if (usbConfig.mode == BUTTON_MODE_NOTE) {
    sendUsbNoteOn(usbConfig.channel, usbConfig.paramA, usbConfig.paramB);
  } else {
    sendUsbCC(usbConfig.channel, usbConfig.paramA, usbConfig.paramB);
  }

  ButtonConfig trsConfig = activeBankConfig.trsButton[buttonIndex];
  if (trsConfig.mode == BUTTON_MODE_NOTE) {
    sendTrsNoteOn(trsConfig.channel, trsConfig.paramA, trsConfig.paramB);
  } else {
    sendTrsCC(trsConfig.channel, trsConfig.paramA, trsConfig.paramB);
  }
}

void sendButtonUp(uint8_t buttonIndex) {
  if (activeConfig.rotated) buttonIndex = 3 - buttonIndex;

  ButtonConfig usbConfig = activeBankConfig.usbButton[buttonIndex];
  if (usbConfig.mode == BUTTON_MODE_NOTE) {
    sendUsbNoteOff(usbConfig.channel, usbConfig.paramA);
  }

  ButtonConfig trsConfig = activeBankConfig.trsButton[buttonIndex];
  if (trsConfig.mode == BUTTON_MODE_NOTE) {
    sendTrsNoteOff(trsConfig.channel, trsConfig.paramA);
  }
}
