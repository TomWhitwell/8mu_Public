#include "firmware.h"

#include "wiring_private.h"

static const byte MIDI_TIP_PIN = 1;
static const byte MIDI_RING_PIN = 45;

// The SAMD21 board can drive either side of the TRS pair.  MIDI type A uses
// tip as the UART sink and ring as the current source; type B reverses them.
// Separate Uart instances are needed because the core fixes TX pin/pad in the
// constructor.  Only the selected instance is active at any time.
static Uart hardMidiA(&sercom2, MIDI_RING_PIN, MIDI_TIP_PIN, SERCOM_RX_PAD_0, UART_TX_PAD_2);
static Uart hardMidiB(&sercom2, MIDI_TIP_PIN, MIDI_RING_PIN, SERCOM_RX_PAD_2, UART_TX_PAD_0);
static Uart *hardMidi = &hardMidiA;
static MidiTRSMode activeTrsMode = MIDI_TRS_A;

void SERCOM2_Handler() {
  hardMidi->IrqHandler();
}

static bool isReadingSysex = false;
uint8_t sysexBuffer[MIDI_INPUT_BUFFER];
uint8_t sysexLength = 0;

void midiBegin() {
  setTrsMode(MIDI_TRS_A);
}

void setTrsMode(MidiTRSMode mode) {
  if (mode != MIDI_TRS_A && mode != MIDI_TRS_B) return;

  Uart *selected = (mode == MIDI_TRS_A) ? &hardMidiA : &hardMidiB;
  byte sourcePin = (mode == MIDI_TRS_A) ? MIDI_RING_PIN : MIDI_TIP_PIN;
  byte sinkPin = (mode == MIDI_TRS_A) ? MIDI_TIP_PIN : MIDI_RING_PIN;

  // midiBegin() calls this with the initial mode before a UART is running.
  // Later calls drain the old UART before reconfiguring the shared SERCOM.
  static bool begun = false;
  if (begun && selected == hardMidi && mode == activeTrsMode) return;
  if (begun) {
    hardMidi->flush();
    hardMidi->end();
  }

  hardMidi = selected;
  activeTrsMode = mode;
  hardMidi->begin(31250);
  pinPeripheral(sinkPin, PIO_SERCOM_ALT);
  pinMode(sourcePin, OUTPUT);
  digitalWrite(sourcePin, HIGH);
  begun = true;
}

static void hardWrite3(uint8_t status, uint8_t data1, uint8_t data2) {
  hardMidi->write(status);
  hardMidi->write(data1);
  hardMidi->write(data2);
}

void sendUsbCC(uint8_t channel, uint8_t cc, uint8_t value) {
  if (channel < 1 || channel > 16) return;
  midiEventPacket_t packet = {0x0B, uint8_t(MIDI_CC | (channel - 1)), cc, value};
  MidiUSB.sendMIDI(packet);
  MidiUSB.flush();
}

void sendUsbNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
  if (channel < 1 || channel > 16) return;
  midiEventPacket_t packet = {0x09, uint8_t(MIDI_NOTE_ON | (channel - 1)), note, velocity};
  MidiUSB.sendMIDI(packet);
  MidiUSB.flush();
}

void sendUsbNoteOff(uint8_t channel, uint8_t note) {
  if (channel < 1 || channel > 16) return;
  midiEventPacket_t packet = {0x08, uint8_t(MIDI_NOTE_OFF | (channel - 1)), note, 0};
  MidiUSB.sendMIDI(packet);
  MidiUSB.flush();
}

void sendTrsCC(uint8_t channel, uint8_t cc, uint8_t value) {
  if (channel < 1 || channel > 16) return;
  hardWrite3(uint8_t(MIDI_CC | (channel - 1)), cc, value);
}

void sendTrsNoteOn(uint8_t channel, uint8_t note, uint8_t velocity) {
  if (channel < 1 || channel > 16) return;
  hardWrite3(uint8_t(MIDI_NOTE_ON | (channel - 1)), note, velocity);
}

void sendTrsNoteOff(uint8_t channel, uint8_t note) {
  if (channel < 1 || channel > 16) return;
  hardWrite3(uint8_t(MIDI_NOTE_OFF | (channel - 1)), note, 0);
}

static void midiThruPacket(const midiEventPacket_t &packet) {
  if (!activeConfig.midiThru) return;

  // USB-MIDI's CIN states the message length.  Data bytes are allowed to be
  // zero, so testing their value would corrupt CC 0, value 0 and note 0.
  uint8_t cin = packet.header & 0x0F;
  uint8_t length = 3;
  if (cin == 0x05 || cin == 0x0F) {
    length = 1;
  } else if (cin == 0x02 || cin == 0x06 || cin == 0x0C || cin == 0x0D) {
    length = 2;
  }
  hardMidi->write(packet.byte1);
  if (length >= 2) hardMidi->write(packet.byte2);
  if (length >= 3) hardMidi->write(packet.byte3);
}

static void appendSysexByte(uint8_t value) {
  if (!isReadingSysex && value != 0xF0) return;

  if (value == 0xF0) {
    isReadingSysex = true;
    sysexLength = 0;
  }

  if (sysexLength < MIDI_INPUT_BUFFER) {
    sysexBuffer[sysexLength++] = value;
  }

  if (value == 0xF7) {
    isReadingSysex = false;
    processSysexBuffer();
  }
}

static void processChannelMessage(const midiEventPacket_t &packet) {
  uint8_t status = packet.byte1;
  if ((status & 0x0F) != 0) return;

  switch (status & 0xF0) {
    case 0xC0:
      programChange(packet.byte2);
      break;
    case 0x90:
      setLedFromNoteOn(packet.byte2, packet.byte3);
      break;
    case 0x80:
      setLedFromNoteOn(packet.byte2, 0);
      break;
  }
}

void midiReadTask() {
  midiEventPacket_t packet = MidiUSB.read();
  while (packet.header != 0) {
    uint8_t cin = packet.header & 0x0F;
    bool isSysexEvent = cin == 0x04 || packet.byte1 == 0xF0 ||
                        (isReadingSysex && cin >= 0x05 && cin <= 0x07);
    if (isSysexEvent) {
      appendSysexByte(packet.byte1);
      appendSysexByte(packet.byte2);
      appendSysexByte(packet.byte3);
    } else {
      processChannelMessage(packet);
      midiThruPacket(packet);
    }
    packet = MidiUSB.read();
  }
}

void sendByteArrayAsSysex(uint8_t messageId, uint8_t *byteArray, uint8_t byteArrayLength) {
  uint8_t outputMessageLength = 1 + 3 + 1 + byteArrayLength + 1;
  uint8_t outputMessage[128];
  if (outputMessageLength > sizeof(outputMessage)) return;

  outputMessage[0] = 0xF0;
  outputMessage[1] = 0x7D;
  outputMessage[2] = 0x00;
  outputMessage[3] = 0x00;
  outputMessage[4] = messageId;
  for (uint8_t i = 0; i < byteArrayLength; i++) {
    outputMessage[i + 5] = byteArray[i];
  }
  outputMessage[outputMessageLength - 1] = 0xF7;

  uint8_t usbPacketBytes[176];
  uint8_t usbPacketLength = 0;
  uint8_t index = 0;

  while ((outputMessageLength - index) > 3) {
    usbPacketBytes[usbPacketLength++] = 0x04;
    usbPacketBytes[usbPacketLength++] = outputMessage[index++];
    usbPacketBytes[usbPacketLength++] = outputMessage[index++];
    usbPacketBytes[usbPacketLength++] = outputMessage[index++];
  }

  uint8_t remaining = outputMessageLength - index;
  if (remaining == 3) {
    usbPacketBytes[usbPacketLength++] = 0x07;
    usbPacketBytes[usbPacketLength++] = outputMessage[index++];
    usbPacketBytes[usbPacketLength++] = outputMessage[index++];
    usbPacketBytes[usbPacketLength++] = outputMessage[index++];
  } else if (remaining == 2) {
    usbPacketBytes[usbPacketLength++] = 0x06;
    usbPacketBytes[usbPacketLength++] = outputMessage[index++];
    usbPacketBytes[usbPacketLength++] = outputMessage[index++];
    usbPacketBytes[usbPacketLength++] = 0;
  } else {
    usbPacketBytes[usbPacketLength++] = 0x05;
    usbPacketBytes[usbPacketLength++] = outputMessage[index++];
    usbPacketBytes[usbPacketLength++] = 0;
    usbPacketBytes[usbPacketLength++] = 0;
  }

  MidiUSB.write(usbPacketBytes, usbPacketLength);
  MidiUSB.flush();
}
