#pragma once

#include <Arduino.h>
#include <Bounce2.h>
#include <BMI160Gen.h>
#include <MIDIUSB.h>

#include "ResponsiveAnalogRead.h"
#include "EEPROMFlash.h"

#define FIRMWARE_VERSION_MAJOR 1
#define FIRMWARE_VERSION_MINOR 5
#define FIRMWARE_VERSION_POINT 2
#define DEVICE_INDEX 6

#define FADER_COUNT 8
#define GESTURE_COUNT 8
#define ANALOG_CONTROL_COUNT 16
#define NUM_LEDS 8
#define NUM_BUTTONS 6
#define OUTPUT_BUTTON_COUNT 4
#define ADC_RESOLUTION 12
#define BANK_COUNT 8
#define BANK_CONFIG_BYTES 96
#define CONFIG_BYTES (16 + (BANK_COUNT * BANK_CONFIG_BYTES))
#define MIDI_INPUT_BUFFER 128
#define PAGE_CHANGE_TIME 700
#define MIDI_CC 0xB0
#define MIDI_NOTE_ON 0x90
#define MIDI_NOTE_OFF 0x80
#define MIDI_VALUE_HYSTERESIS 16
#define CONTROL_SCAN_INTERVAL_US 1000
#define IMU_SCAN_INTERVAL_MS 5
#define MIDI_BLINK_TIME_MS 50
#define TEACH_BLINK_TIME_MS 200
#define LEARN_LONG_PRESS_MS 1000

enum LearnMode {
  LEARN_MODE_OFF,
  LEARN_MODE_FADER,
  LEARN_MODE_GESTURE
};

enum MidiTRSMode {
  MIDI_TRS_A,
  MIDI_TRS_B
};

enum ButtonMode {
  BUTTON_MODE_NOTE,
  BUTTON_MODE_CC
};

struct AnalogConfig {
  uint8_t channel;
  uint8_t cc;
};

struct ButtonConfig {
  uint8_t channel;
  ButtonMode mode;
  uint8_t paramA;
  uint8_t paramB;
};

struct BankConfig {
  AnalogConfig usbAnalog[ANALOG_CONTROL_COUNT];
  AnalogConfig trsAnalog[ANALOG_CONTROL_COUNT];
  ButtonConfig usbButton[OUTPUT_BUTTON_COUNT];
  ButtonConfig trsButton[OUTPUT_BUTTON_COUNT];
};

struct ControllerConfig {
  bool faderLed;
  bool accelLed;
  bool rotated;
  uint16_t faderMin;
  uint16_t faderMax;
  bool midiThru;
  MidiTRSMode midiType;
  BankConfig banks[BANK_COUNT];
};

extern const byte LED_PINS[NUM_LEDS];
extern const byte BUTTON_PINS[NUM_BUTTONS];
extern const byte FADER_PINS[FADER_COUNT];

extern Bounce buttons[NUM_BUTTONS];
extern ResponsiveAnalogRead *analog[FADER_COUNT];
extern ResponsiveAnalogRead *gestures[GESTURE_COUNT];

extern ControllerConfig activeConfig;
extern BankConfig activeBankConfig;
extern uint8_t currentBank;
extern uint8_t tempBank;

extern uint8_t lastFaderMidiValues[FADER_COUNT];
extern uint8_t lastGestureMidiValues[GESTURE_COUNT];
extern bool midiBlink[FADER_COUNT];
extern unsigned long lastMidiActivity[FADER_COUNT];
extern unsigned long lastBlink;
extern bool teachBlink;
extern bool bankLockout;
extern bool changingBank;
extern bool bankChangePending;
extern unsigned long bankChangeRequestedAt;
extern bool doLedAnimate;
extern bool shouldDoWriteBankFile;
extern bool shouldSendConfig;
extern LearnMode learnMode;
extern int IMU_readings[6];
extern bool flip;
extern uint8_t velocityLeds[NUM_LEDS];
extern uint8_t sysexBuffer[MIDI_INPUT_BUFFER];
extern uint8_t sysexLength;

extern EEPROMFlash CONFIG_STORE;
extern EEPROMFlash BANK_STORE;

void firmwareSetup();
void firmwareLoop();

size_t bankToBytes(const BankConfig &bank, uint8_t *out);
size_t bytesToBankConfig(uint8_t *in, BankConfig &bank);
size_t deviceConfigToBytes(const ControllerConfig &config, uint8_t bank, uint8_t *out);
size_t configToBytes(const ControllerConfig &config, uint8_t *out);
size_t bytesToConfig(uint8_t *in, ControllerConfig &config);
void updateConfig(uint8_t *incomingSysex, uint8_t incomingSysexLength);

ControllerConfig makeDefaultControllerConfig();
BankConfig makeDefaultBankConfig(uint8_t channel);

bool initialiseStorage(bool forceFactoryReset = false);
bool writeDefaultFiles();
bool readConfigBytes(uint8_t *outBuffer, size_t bufferSize);
bool writeActiveConfigToFile();
bool writeBankIndexToFile(uint8_t bank);
uint8_t readBankIndexFromFile();

void midiBegin();
void midiReadTask();
void sendUsbCC(uint8_t channel, uint8_t cc, uint8_t value);
void sendUsbNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
void sendUsbNoteOff(uint8_t channel, uint8_t note);
void sendTrsCC(uint8_t channel, uint8_t cc, uint8_t value);
void sendTrsNoteOn(uint8_t channel, uint8_t note, uint8_t velocity);
void sendTrsNoteOff(uint8_t channel, uint8_t note);
void setTrsMode(MidiTRSMode mode);

void sendByteArrayAsSysex(uint8_t messageId, uint8_t *byteArray, uint8_t byteArrayLength);
void sendCurrentConfig();
void sendCurrentBank();
void sendHeartbeat();
void sendFaderPositions();
void processSysexBuffer();

void updateControls(bool force = false);
void refreshFaderMidiValues();
void updateIMU();
void programChange(uint8_t bank);
void handleBankInc();
void handleBankDec();
void incTempBankBy(int8_t delta);
void handleLearnModeChange();
void sendButtonDown(uint8_t buttonIndex);
void sendButtonUp(uint8_t buttonIndex);

void ledSet(uint8_t i, uint8_t val);
void ledOn(uint8_t i);
void ledOff(uint8_t i);
void clearLeds();
void updateLeds();
void ledAnimate(uint8_t target);
void setLedFromNoteOn(uint8_t note, uint8_t vel);

int gesture(byte num);
