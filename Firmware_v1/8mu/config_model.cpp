#include "firmware.h"

BankConfig makeDefaultBankConfig(uint8_t channel) {
  BankConfig bank;
  for (uint8_t i = 0; i < ANALOG_CONTROL_COUNT; i++) {
    bank.usbAnalog[i] = {channel, uint8_t(34 + i)};
    bank.trsAnalog[i] = {channel, uint8_t(34 + i)};
  }
  const uint8_t notes[OUTPUT_BUTTON_COUNT] = {36, 48, 60, 72};
  for (uint8_t i = 0; i < OUTPUT_BUTTON_COUNT; i++) {
    bank.usbButton[i] = {channel, BUTTON_MODE_NOTE, notes[i], 64};
    bank.trsButton[i] = {channel, BUTTON_MODE_NOTE, notes[i], 64};
  }
  return bank;
}

ControllerConfig makeDefaultControllerConfig() {
  ControllerConfig config;
  config.faderLed = true;
  config.accelLed = false;
  config.rotated = false;
  config.faderMin = 15;
  config.faderMax = 4080;
  config.midiThru = true;
  config.midiType = MIDI_TRS_A;
  for (uint8_t i = 0; i < BANK_COUNT; i++) {
    // RP2040 v1.0.0 deliberately defaults every bank to MIDI channel 1.
    config.banks[i] = makeDefaultBankConfig(1);
  }
  return config;
}

size_t bankToBytes(const BankConfig &bank, uint8_t *out) {
  if (!out) return 0;
  size_t idx = 0;

  for (uint8_t i = 0; i < ANALOG_CONTROL_COUNT; i++) out[idx++] = bank.usbAnalog[i].channel;
  for (uint8_t i = 0; i < ANALOG_CONTROL_COUNT; i++) out[idx++] = bank.usbAnalog[i].cc;
  for (uint8_t i = 0; i < ANALOG_CONTROL_COUNT; i++) out[idx++] = bank.trsAnalog[i].channel;
  for (uint8_t i = 0; i < ANALOG_CONTROL_COUNT; i++) out[idx++] = bank.trsAnalog[i].cc;

  for (uint8_t i = 0; i < OUTPUT_BUTTON_COUNT; i++) {
    out[idx++] = bank.usbButton[i].channel;
    out[idx++] = static_cast<uint8_t>(bank.usbButton[i].mode);
    out[idx++] = bank.usbButton[i].paramA;
    out[idx++] = bank.usbButton[i].paramB;
  }

  for (uint8_t i = 0; i < OUTPUT_BUTTON_COUNT; i++) {
    out[idx++] = bank.trsButton[i].channel;
    out[idx++] = static_cast<uint8_t>(bank.trsButton[i].mode);
    out[idx++] = bank.trsButton[i].paramA;
    out[idx++] = bank.trsButton[i].paramB;
  }

  return idx == BANK_CONFIG_BYTES ? idx : 0;
}

size_t bytesToBankConfig(uint8_t *in, BankConfig &bank) {
  if (!in) return 0;
  size_t idx = 0;

  for (uint8_t i = 0; i < ANALOG_CONTROL_COUNT; i++) bank.usbAnalog[i].channel = in[idx++];
  for (uint8_t i = 0; i < ANALOG_CONTROL_COUNT; i++) bank.usbAnalog[i].cc = in[idx++];
  for (uint8_t i = 0; i < ANALOG_CONTROL_COUNT; i++) bank.trsAnalog[i].channel = in[idx++];
  for (uint8_t i = 0; i < ANALOG_CONTROL_COUNT; i++) bank.trsAnalog[i].cc = in[idx++];

  for (uint8_t i = 0; i < OUTPUT_BUTTON_COUNT; i++) {
    bank.usbButton[i].channel = in[idx++];
    bank.usbButton[i].mode = static_cast<ButtonMode>(in[idx++]);
    bank.usbButton[i].paramA = in[idx++];
    bank.usbButton[i].paramB = in[idx++];
  }

  for (uint8_t i = 0; i < OUTPUT_BUTTON_COUNT; i++) {
    bank.trsButton[i].channel = in[idx++];
    bank.trsButton[i].mode = static_cast<ButtonMode>(in[idx++]);
    bank.trsButton[i].paramA = in[idx++];
    bank.trsButton[i].paramB = in[idx++];
  }

  return idx == BANK_CONFIG_BYTES ? idx : 0;
}

size_t deviceConfigToBytes(const ControllerConfig &config, uint8_t bank, uint8_t *out) {
  if (!out) return 0;

  out[0] = config.faderLed ? 1 : 0;
  out[1] = config.accelLed ? 1 : 0;
  out[2] = config.rotated ? 1 : 0;
  out[3] = 0;
  out[4] = 0;
  out[5] = 0;
  out[6] = 0;
  out[7] = config.midiThru ? 1 : 0;
  out[8] = static_cast<uint8_t>(config.midiType);
  for (uint8_t i = 9; i < 15; i++) out[i] = 0;
  out[15] = bank;
  return 16;
}

size_t configToBytes(const ControllerConfig &config, uint8_t *out) {
  if (!out) return 0;

  out[0] = config.faderLed ? 1 : 0;
  out[1] = config.accelLed ? 1 : 0;
  out[2] = config.rotated ? 1 : 0;
  out[3] = config.faderMin & 0xff;
  out[4] = (config.faderMin >> 8) & 0xff;
  out[5] = config.faderMax & 0xff;
  out[6] = (config.faderMax >> 8) & 0xff;
  out[7] = config.midiThru ? 1 : 0;
  out[8] = static_cast<uint8_t>(config.midiType);
  for (uint8_t i = 9; i < 16; i++) out[i] = 0;

  for (uint8_t bank = 0; bank < BANK_COUNT; bank++) {
    bankToBytes(config.banks[bank], &out[16 + (bank * BANK_CONFIG_BYTES)]);
  }

  return CONFIG_BYTES;
}

size_t bytesToConfig(uint8_t *in, ControllerConfig &config) {
  if (!in) return 0;
  size_t idx = 0;

  config.faderLed = in[idx++] != 0;
  config.accelLed = in[idx++] != 0;
  config.rotated = in[idx++] != 0;
  config.faderMin = static_cast<uint16_t>(in[idx++]);
  config.faderMin |= static_cast<uint16_t>(in[idx++]) << 8;
  config.faderMax = static_cast<uint16_t>(in[idx++]);
  config.faderMax |= static_cast<uint16_t>(in[idx++]) << 8;
  config.midiThru = in[idx++] != 0;
  config.midiType = static_cast<MidiTRSMode>(in[idx++]);

  if (config.faderMax <= config.faderMin) {
    config.faderMin = 15;
    config.faderMax = 4080;
  }

  for (uint8_t bank = 0; bank < BANK_COUNT; bank++) {
    bytesToBankConfig(&in[16 + (bank * BANK_CONFIG_BYTES)], config.banks[bank]);
  }

  return CONFIG_BYTES;
}

void updateConfig(uint8_t *incomingSysex, uint8_t incomingSysexLength) {
  if (incomingSysexLength < 122) return;

  const uint8_t dataStartIndex = 9;
  activeConfig.faderLed = incomingSysex[dataStartIndex + 0] != 0;
  activeConfig.accelLed = incomingSysex[dataStartIndex + 1] != 0;
  activeConfig.rotated = incomingSysex[dataStartIndex + 2] != 0;
  activeConfig.midiThru = incomingSysex[dataStartIndex + 7] != 0;
  activeConfig.midiType = static_cast<MidiTRSMode>(incomingSysex[dataStartIndex + 8]);
  setTrsMode(activeConfig.midiType);

  const uint8_t usbChannelOffset = 16;
  const uint8_t usbCCOffset = 48;
  const uint8_t trsChannelOffset = 32;
  const uint8_t trsCCOffset = 64;

  for (uint8_t i = 0; i < ANALOG_CONTROL_COUNT; i++) {
    activeConfig.banks[currentBank].usbAnalog[i].channel = incomingSysex[dataStartIndex + usbChannelOffset + i];
    activeConfig.banks[currentBank].usbAnalog[i].cc = incomingSysex[dataStartIndex + usbCCOffset + i];
    activeConfig.banks[currentBank].trsAnalog[i].channel = incomingSysex[dataStartIndex + trsChannelOffset + i];
    activeConfig.banks[currentBank].trsAnalog[i].cc = incomingSysex[dataStartIndex + trsCCOffset + i];
  }

  const uint8_t usbButtonOffset = 80;
  const uint8_t trsButtonOffset = 96;
  for (uint8_t i = 0; i < OUTPUT_BUTTON_COUNT; i++) {
    activeConfig.banks[currentBank].usbButton[i].channel = incomingSysex[dataStartIndex + usbButtonOffset + (i * 4) + 0];
    activeConfig.banks[currentBank].usbButton[i].mode = static_cast<ButtonMode>(incomingSysex[dataStartIndex + usbButtonOffset + (i * 4) + 1]);
    activeConfig.banks[currentBank].usbButton[i].paramA = incomingSysex[dataStartIndex + usbButtonOffset + (i * 4) + 2];
    activeConfig.banks[currentBank].usbButton[i].paramB = incomingSysex[dataStartIndex + usbButtonOffset + (i * 4) + 3];

    activeConfig.banks[currentBank].trsButton[i].channel = incomingSysex[dataStartIndex + trsButtonOffset + (i * 4) + 0];
    activeConfig.banks[currentBank].trsButton[i].mode = static_cast<ButtonMode>(incomingSysex[dataStartIndex + trsButtonOffset + (i * 4) + 1]);
    activeConfig.banks[currentBank].trsButton[i].paramA = incomingSysex[dataStartIndex + trsButtonOffset + (i * 4) + 2];
    activeConfig.banks[currentBank].trsButton[i].paramB = incomingSysex[dataStartIndex + trsButtonOffset + (i * 4) + 3];
  }

  activeBankConfig = activeConfig.banks[currentBank];
  writeActiveConfigToFile();
}
