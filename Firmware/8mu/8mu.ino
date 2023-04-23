/*
  Music Thing Modular 8mu Firmware
  (c)Tom Whitwell 2023
  MIT License
  Includes lots of code and inspiration from
  16n Faderbank Firmware Configuration
  (c) 2017,2018,2020 by Brian Crabtree, Sean Hellfritsch, Tom Armitage, and Brendon Cassidy
  MIT License
*/
/*
   Using library ResponsiveAnalogRead at version 1.2.1
   Using library Adafruit_SPIFlash at version 4.0.0 
   Using library Bounce2 at version 2.60
   Using library BMI160-Arduino-master
   Using library Adafruit_TinyUSB_Library at version 0.10.0
   NB: USB Stack TinyUSB
   
*/





#include "uMidi.h"
#include <ResponsiveAnalogRead.h>
#include "EEPROMFlash.h"
#include <Bounce2.h>
#include <BMI160Gen.h>
#include "config.h"


// Config variables
int MAJOR_VERSION = 0x01;
int MINOR_VERSION = 0x00;
int POINT_VERSION = 0x01;
const int DEVICE_ID = 0x04; //   0x04 = 8mu

// Create Midi
uMidi myMidi;

// Access two eeprom-like stores in on the internal flash memory
EEPROMFlash EEPROM;
EEPROMFlash BANK_STORE;

// activates printing of debug messages
// #define DEBUG 0

// wrap code to be executed only under DEBUG conditions in D()
#ifdef DEBUG
#define D(x) x
#else
#define D(x)
#endif



// Hardware variables
const int channelCount = 16;

#define NUM_LEDS 8

#if defined (Proto_3_1) || defined (Proto_2)
const byte leds[8] = {0, 1, 5, 7, 10, 11, 12, 13};
#elif defined (Proto_3_2)
const byte leds[8] = {0, 44, 5, 7, 10, 11, 12, 13};
#else
const byte leds[8] = {0, 1, 5, 7, 10, 11, 12, 13};
#endif 


#define NUM_BUTTONS 6 // all buttons 
const byte buttonPins[6] = {29, 30, 28, 2, 31, 3}; // A, B, C, D, Left, Right
Bounce * buttons = new Bounce[NUM_BUTTONS];
const int longPush = 1000;
const int buttonCount = 4; // output buttons

// Pins for faders
const byte faders[8] = {A0, A1, A2, A3, A4, A5, A10, A11};

// Holds raw readings from 6-axis accelerometer
int IMU_readings[6];

// Smooth the two types of analog input
ResponsiveAnalogRead *analog[channelCount];

// System variables
int usbChannels[channelCount];
int trsChannels[channelCount];
int usbCCs[channelCount];
int trsCCs[channelCount];
int volatile currentValue[channelCount];
int lastMidiValue[channelCount];

int buttonUSBChannels[buttonCount];
int buttonTRSChannels[buttonCount];
int buttonUSBMode[buttonCount];
int buttonTRSMode[buttonCount];
int buttonUSBParamA[buttonCount];
int buttonTRSParamA[buttonCount];
int buttonUSBParamB[buttonCount];
int buttonTRSParamB[buttonCount];

// Some legacy 16n variables here
int flip;
int faderBlink;
int accelBlink;
int i2cMaster;
int midiThru;
int midiMode = 0;
int dxMode;
const int adcResolutionBits = 12; // 12 bit ADC resolution on SAMD21
int faderMin;
int faderMax;

// run variables
bool forceMidiWrite = false;
int shiftyTemp, notShiftyTemp;
int midiInterval = 1000; // 1ms
bool shouldDoMidiRead = false;
bool shouldDoMidiWrite = false;
unsigned long midiTimerClock;
bool midiBlink[channelCount]; // should this led be blinking
const int midiBlinkTime = 50; // How long led blinks after a midi send
unsigned long lastMidiActivity;
unsigned long lastBlink; // Since blinks in teach mode
const int teachBlinkTime = 200; // speed of LED blinking when in Teach Mode
boolean teachBlink = true;


// Variables to track page changes and Teach Mode
int page = 0;
unsigned long pageChange = millis();
int tempPage = page;
const int pageChangeTime = 700;
const int buttonHoldTime = 700;
boolean changingPage = false; // true while the page change process is in progress - button pressed but not yet settled
boolean longHeld = true;
boolean pageChanged = true; // true after the page has changed. Only send config to editor if page has changed.
unsigned int lastSysex = 0; // allows pageChanged to timeout after some time
const int pageChangedTimeout = 10000;
// In teach mode, the IMU outputs are mapped to the faders to make it easier to use midi learn functions
enum ledModes {normal, teachFader, teachIMU} teachMode = normal;



void setup() {
  //  D(delay(5000));

  // Set resolution
  analogReadResolution(adcResolutionBits);

  // Setup smoothing and initialise variables
  for (int i = 0; i < channelCount; i++)
  {
    analog[i] = new ResponsiveAnalogRead(0, true, .001);
    analog[i] = new ResponsiveAnalogRead(0, true, .0001);
    analog[i]->setActivityThreshold(4 << (adcResolutionBits - 10));
    currentValue[i] = 0;
    lastMidiValue[i] = 0;
  }

  // Setup Leds as output pins
  for (int i = 0; i < 8; i++) {
    pinMode(leds[i], OUTPUT);
  }

  // Start Midi
  myMidi.begin();

  // Setup Buttons as input pins
  // Setup buttons after MIDI because we need to remap Pin 3 to button R
  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].attach( buttonPins[i] , INPUT_PULLUP  );
    buttons[i].interval(25);
  }
  // let the buttons settle
  delay(50);

  // Setup Acellerometer
  // Setup acellerometer after buttons to allow calibration
  BMI160.begin(BMI160GenClass::I2C_MODE);
  BMI160.setGyroRange(250);
  // Calibrate if Button A (not button B) held down on startup
  if (!buttons[0].read() && buttons[1].read()) calibrateIMU();

  // Initialise the two stores
  EEPROM.begin("EEPROM.cfg");
  BANK_STORE.begin("BANK.cfg");

  // If they don't exist, create them and fill with FFs
  // Config store = 1kb of space
  if (!EEPROM.exists()) EEPROM.create(1024);
  // Bank store = 1 byte of space
  if (!BANK_STORE.exists()) BANK_STORE.create(1);

  // If Buttons ABCD held down on startup, return memory to default
  if (!buttons[0].read() && !buttons[1].read() && !buttons[2].read() && !buttons[3].read() ) {
    initializeFactorySettingsFast();
  }
  
  // Read defaults from the virtual EEPRPOM
  checkDefaultSettings();
  loadSettingsFromEEPROM();

  // Implement new settings
  myMidi._midiThru = midiThru;
}


void loop() {
  
  // Poll to check and send midi every midiInterval
  if (micros() - midiTimerClock > midiInterval) {
    midiTimerClock = micros();
    writeMidi();
    // Check for Sysex Configuration messages
    myMidi.newMessage();
    if (myMidi.sysexAvailable()) processIncomingSysex(myMidi.returnSysex(), sizeof(myMidi.returnSysex()));
    // update LEDs
    doLeds();
    
  }

  // Check buttons for page/bank changes
  checkPage();

  // Read Faders
  for (int j = 0; j < 8; j++) {
    int thisFader = abs((7 * flip) - j);
    analog[j]->update(analogRead(faders[thisFader]));
    int temp = analog[j]->getValue();
    temp = constrain(temp, faderMin, faderMax);
    temp = map(temp, faderMin, faderMax, 0, (1 << adcResolutionBits)-1);
    if (!flip) temp = ((1 << adcResolutionBits)-1 )- temp;
    currentValue[j] = temp;
  }

  // Read Accelerometer
  BMI160.readMotionSensor(IMU_readings[3], IMU_readings[4], IMU_readings[5], IMU_readings[0], IMU_readings[1], IMU_readings[2]);
  for (int j = 0; j < 8 ; j++) {
    analog[j + 8]->update(gesture(j));
    int temp = analog[j + 8]->getValue();
    temp = constrain(temp, faderMin, faderMax);
    temp = map(temp, faderMin, faderMax, 0, (1 << adcResolutionBits)-1);
    currentValue[j + 8] = temp;
  }


  // Read buttons
  for (int i = 0; i < buttonCount; i++) {
    // Check what's happend to the button
    buttons[i].update();

    // When the button goes down, send noteOns or CCs according to mode
    if (buttons[i].fell()) {
      switch (buttonUSBMode[i]) {
        case 0:
          // send usb CC
          myMidi.CC(buttonUSBChannels[i], buttonUSBParamA[i], buttonUSBParamB[i]);
          break;
        case 1:
          // send usb note
          myMidi.noteOn(buttonUSBChannels[i], buttonUSBParamA[i], buttonUSBParamB[i]);
          break;
      }

      switch (buttonTRSMode[i]) {
        case 0:
          // send TRS CC
          myMidi.hardCC(buttonTRSChannels[i], buttonTRSParamA[i], buttonTRSParamB[i]);
          break;
        case 1:
          // send TRS note
          myMidi.hardNoteOn(buttonTRSChannels[i], buttonTRSParamA[i], buttonTRSParamB[i]);
          break;
      }
    }

    // When the button comes up, just need to send note offs
    if (buttons[i].rose()) {
      if (buttonUSBMode[i] == 1) {
        // send usb noteOff
        myMidi.noteOff(buttonUSBChannels[i], buttonUSBParamA[i]);
      }

      if (buttonTRSMode[i] == 1) {
        // send TRS noteOff
        myMidi.hardNoteOff(buttonTRSChannels[i], buttonTRSParamA[i]);
      }
    }
  }

}

void writeMidi() {
  for (int q = 0; q < channelCount; q++)
  {

    midiBlink[q] = false;
    notShiftyTemp = currentValue[q];

    // shift for MIDI precision (0-127)
    shiftyTemp = notShiftyTemp >> 5;

    // if there was a change in the midi value
    if ((shiftyTemp != lastMidiValue[q]) || forceMidiWrite)
    {

      switch (teachMode) {

        case normal:

          // send the message over USB
          // myMidi will not send on channel 0, so channel 0 = off 
          myMidi.CC(usbChannels[q], usbCCs[q], shiftyTemp);
          // send the message over physical midi
          myMidi.hardCC(trsChannels[q], trsCCs[q], shiftyTemp);
          // store the shifted value for future comparison
          lastMidiValue[q] = shiftyTemp;
          //      D(Serial.printf("Fader[%d] Chan: %d CC: %d val: %d\n", q, usbChannels[q], usbCCs[q], shiftyTemp));
          midiBlink[q] = true;
          lastMidiActivity = millis();
          break;

        case teachFader:
          // In fader mode, only send fader movements, ignore IMU movements
          if (q < 8) {
            // send the message over USB
            myMidi.CC(usbChannels[q], usbCCs[q], shiftyTemp);
            // send the message over physical midi
            myMidi.hardCC(trsChannels[q], trsCCs[q], shiftyTemp);
            // store the shifted value for future comparison
            lastMidiValue[q] = shiftyTemp;
            //      D(Serial.printf("Fader[%d] Chan: %d CC: %d val: %d\n", q, usbChannels[q], usbCCs[q], shiftyTemp));
          }


          break;

        case teachIMU:
          //in IMU mode, send fader movements through the IMU channels to make it easier to set up midi learn
          if (q < 8 ) {

            // send the message over USB
            myMidi.CC(usbChannels[q + 8], usbCCs[q + 8], shiftyTemp);
            // send the message over physical midi
            myMidi.hardCC(trsChannels[q + 8], trsCCs[q + 8], shiftyTemp);
            // store the shifted value for future comparison
            lastMidiValue[q] = shiftyTemp;
            //      D(Serial.printf("Fader[%d] Chan: %d CC: %d val: %d\n", q, usbChannels[q], usbCCs[q], shiftyTemp));


          }
          break;


      }
    }

  }
  forceMidiWrite = false;
}
