
//Load as Music Thing m0

// NB: Encoder Channels: Turn SLOWLY when midi mapping and ensure the box in the bottom left (in Ableton) reads Relative (Signed Bit 2) - it should switch automatically when it detects an encoder.

//#define DEBUG

#ifdef DEBUG
#define DEBUG_PRINT(x)     Serial.print (x)
#define DEBUG_PRINTLN(x)  Serial.println (x)
#else
#define DEBUG_PRINT(x)
#define DEBUG_PRINTLN(x)
#endif

// include the relevant libraries
#include <ResponsiveAnalogRead.h>
#include <Bounce2.h>
#include <Encoder.h>

#include <pitchToFrequency.h>
#include <pitchToNote.h>
#include <frequencyToNote.h>
#include <MIDIUSB_Defs.h>
#include <MIDIUSB.h>



// Midi Channel and CCs for Analog outs - faders and pots, left to right
int midiCC[2][7] = {
  {32, 33, 34, 35, 36, 37, 38},
  {41, 42, 43, 44, 45, 46, 47}
};
int midiChannel[2][7] = {
  {0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0}
};
int bank = 0;

// Midi Channel and CCs for Encoders  left to right
int encoderMidiCC[2][2] = {{39, 40}, {48, 49}};
int encoderMidiChannel[2][2] = {{0, 0}, {0, 0}};
int encoderAccel = 5; // How much do encoders accelerate as they're turned - 1 is none, 10 is too much
// NB: this method sends 65 when encoder is turned right, 1 when turned left, set in Ableton as Relative Signed Bit 2

//// Midi Notes to be triggered by encoder buttons, left to right
//int encoderNote[2] = {48, 60}; // C3 and C4
//int encoderNoteChannel[2] = {0, 0};
//int encoderNoteVelocity[2] = {100, 100};

// Update delay = how many milliseconds between reads (10 is normal), also how long the LED glows when control is touched
int updateDelay = 10;
unsigned long updateCount = 0;



// Connect the analog inputs - faders and pots to an array. 0-6 goes left to right
ResponsiveAnalogRead inputs[7] =
{ ResponsiveAnalogRead (A0, true, 1),
  ResponsiveAnalogRead (A1, true, 1),
  ResponsiveAnalogRead (A2, true, 1),
  ResponsiveAnalogRead (A3, true, 1),
  ResponsiveAnalogRead (A4, true, 1),
  ResponsiveAnalogRead (D9, true, 1),
  ResponsiveAnalogRead (A5, true, 1),
};

// Connect the two buttons on the Encoders, 0 on left, 1 on right

#define NUM_BUTTONS 2
const uint8_t BUTTON_PINS[NUM_BUTTONS] = {5, PIN_WIRE_SCL};
Bounce * buttons = new Bounce[NUM_BUTTONS];

// Connect the rotary encoders

Encoder knobLeft(11, 12);
Encoder knobRight(6, 10);
long positionLeft  = -999;
long positionRight = -999;
unsigned long leftChange = 0;
unsigned long rightChange = 0;

// Set up Midi

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
}

void sendCC(byte channel, byte cc, byte value) {
  midiEventPacket_t msg = {0x0B, 0xB0 | channel, cc, value};
  MidiUSB.sendMIDI(msg);
}




void setup() {
  // begin serial so we can see analog read values through the serial monitor
#ifdef DEBUG
  Serial.begin(9600);
#endif

  pinMode(13, OUTPUT);    // sets the digital pin 13 as output

  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].attach( BUTTON_PINS[i] , INPUT_PULLUP  );       //setup the bounce instance for the current button
    buttons[i].interval(25);              // interval in ms
  }

  // Ignore knob changes that are less than 8 steps (of 1023)
  for (int i = 0; i < 7; i++) {
    inputs[i].setActivityThreshold(8);
  }
}

void loop() {

  // Read the analog inputs, print out any changes
  for (int i = 0; i < 7; i++) {
    inputs[i].update();
    if (inputs[i].hasChanged()) {
      DEBUG_PRINT(i);
      DEBUG_PRINT("=");
      DEBUG_PRINTLN(inputs[i].getValue());
      sendCC(midiChannel[bank][i], midiCC[bank][i], inputs[i].getValue() >> 3);
      digitalWrite(13, !bank);
    }
    MidiUSB.flush();

  }





  // Read the buttons, print any changes
  for (int i = 0; i < NUM_BUTTONS; i++)  {
    // Update the Bounce instance :
    buttons[i].update();
    // If it fell, start relevant note
    if ( buttons[i].fell() ) {
      DEBUG_PRINT("Button ");
      DEBUG_PRINT (i);
      DEBUG_PRINTLN (" pushed");
      bank = i;
      digitalWrite(13, !bank);
    }}



    //  // Read the buttons, print any changes
    //  for (int i = 0; i < NUM_BUTTONS; i++)  {
    //    // Update the Bounce instance :
    //    buttons[i].update();
    //    // If it fell, start relevant note
    //    if ( buttons[i].fell() ) {
    //      DEBUG_PRINT("Button ");
    //      DEBUG_PRINT (i);
    //      DEBUG_PRINTLN (" pushed");
    //      noteOn(encoderNoteChannel[i], encoderNote[i], encoderNoteVelocity[i]);
    //      digitalWrite(13, !bank);
    //    }
    //
    //    if ( buttons[i].rose() ) {
    //      DEBUG_PRINT("Button ");
    //      DEBUG_PRINT (i);
    //      DEBUG_PRINTLN (" un-pushed");
    //      noteOff(encoderNoteChannel[i], encoderNote[i], 0);
    //      digitalWrite(13, !bank);
    //    }
    //
    //    MidiUSB.flush();
    //  }

    // Read the encoders, print any changes
    long newLeft, newRight;
    int changeDivider = (64 - updateDelay) / encoderAccel; //
    newLeft = knobLeft.read();
    newRight = knobRight.read();

    if (newLeft   > positionLeft  ) { // /4 because encoders spit out 4 pulses for every detent click
      DEBUG_PRINT("Left Up ");
      int change = millis() - leftChange;
      change = 65 + (max(64 - change, 0) / changeDivider);
      DEBUG_PRINTLN(change);
      leftChange = millis();
      sendCC(encoderMidiChannel[bank][0], encoderMidiCC[bank][0], change);
      positionLeft = newLeft;
      digitalWrite(13, !bank);
    }
    if (newRight  > positionRight) {
      DEBUG_PRINT("Right Up ");
      int change = millis() - rightChange;
      change = 65 + (max(64 - change, 0) / changeDivider);
      DEBUG_PRINTLN(change);
      rightChange = millis();
      sendCC(encoderMidiChannel[bank][1], encoderMidiCC[bank][1], change);
      positionRight = newRight;
      digitalWrite(13, !bank);
    }

    if (newLeft   < positionLeft  ) {
      DEBUG_PRINT("Left Down ");
      int change = millis() - leftChange;
      change = 1 + (max(64 - change, 0) / changeDivider);
      DEBUG_PRINTLN(change);
      leftChange = millis();
      sendCC(encoderMidiChannel[bank][0], encoderMidiCC[bank][0], change);
      positionLeft = newLeft;
      digitalWrite(13, !bank);
    }


    if (newRight  < positionRight ) {
      DEBUG_PRINT("Right Up ");
      int change = millis() - rightChange;
      change = 1 + (max(64 - change, 0) / changeDivider);
      DEBUG_PRINTLN(change);
      rightChange = millis();
      sendCC(encoderMidiChannel[bank][1], encoderMidiCC[bank][1], change);
      positionRight = newRight;
      digitalWrite(13, !bank);
    }
    MidiUSB.flush();

    // Wait for a little time to pass
    while (millis() - updateCount < updateDelay);
    updateCount = millis();

    // Turn off the LED if a change has turned it on
    digitalWrite(13, bank);

  }
