/*
   Ultralight Midi driver
   (c) 2023 by Tom Whitwell
   MIT License
*/


#pragma once

// activates printing of debug messages
// #define DEBUG 0

// wrap code to be executed only under DEBUG conditions in D()
#ifdef DEBUG
#define D(x) x
#else
#define D(x)
#endif

#define MAKEHEADER(cn, cin) (((cn & 0x0f) << 4) | cin)


// Class member variables, initialised at startup
const byte NOTE_OFF = 0x80;
const byte NOTE_ON = 0x90;
const byte CONT_CONT = 0xB0;
const byte SYSEX = 0xF0;
const byte SYSEX_END = 0xF7;

class uMidi
{
  public:

    // Constructor
    uMidi();

    // Initialises Midi
    void begin();
    boolean _midiThru = true;

    // Various controls
    void noteOn(byte channel, byte note, byte velocity);
    void hardNoteOn(byte channel, byte note, byte velocity);
    void noteOff(byte channel, byte note);
    void hardNoteOff(byte channel, byte note);
    void CC(byte channel, byte controller, byte level);
    void hardCC(byte channel, byte controller, byte level);
    void changeHardMidiMode(boolean _midiMode);


    // Check for messages
    void newMessage();
    boolean sysexAvailable();
    byte *returnSysex();


    // Send sysex
    void sendSysEx(int length, byte *data);

  private:
    static const int bufferSize = 124;
    byte midiBuffer[bufferSize];
    byte oldPacket[4];
    int bufferLength = -1;
    byte recievingSysex = false;
    byte recievedSysex = false;


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
    };

};
