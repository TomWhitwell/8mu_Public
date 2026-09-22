#include "firmware.h"

void sendCurrentConfig() {
  const uint8_t configDataLength = 4 + 16 + BANK_CONFIG_BYTES;
  uint8_t currentConfigData[configDataLength];
  uint8_t bankBuf[BANK_CONFIG_BYTES];
  uint8_t deviceBuf[16];

  bankToBytes(activeConfig.banks[currentBank], bankBuf);
  deviceConfigToBytes(activeConfig, currentBank, deviceBuf);

  // Hardware-family signature in bytes reserved by the 8mu configuration
  // protocol. Keeping DEVICE_INDEX at 6 preserves compatibility with existing
  // editors and with Radio Music; updated editors can use this signature to
  // distinguish the SAMD21 8mu v1 from the shipped RP2040 8mu v2.
  deviceBuf[9] = 'S';
  deviceBuf[10] = 'A';
  deviceBuf[11] = 'M';
  deviceBuf[12] = 'D';

  currentConfigData[0] = DEVICE_INDEX;
  currentConfigData[1] = FIRMWARE_VERSION_MAJOR;
  currentConfigData[2] = FIRMWARE_VERSION_MINOR;
  currentConfigData[3] = FIRMWARE_VERSION_POINT;

  for (uint8_t i = 0; i < 16; i++) {
    currentConfigData[i + 4] = deviceBuf[i];
  }

  for (uint8_t i = 0; i < BANK_CONFIG_BYTES; i++) {
    currentConfigData[i + 4 + 16] = bankBuf[i];
  }

  sendByteArrayAsSysex(0x0F, currentConfigData, configDataLength);
}

void sendCurrentBank() {
  uint8_t data[1];
  data[0] = currentBank;
  sendByteArrayAsSysex(0x2B, data, 1);
}

void sendHeartbeat() {
  uint8_t dataArray[5];
  dataArray[0] = DEVICE_INDEX;
  dataArray[1] = FIRMWARE_VERSION_MAJOR;
  dataArray[2] = FIRMWARE_VERSION_MINOR;
  dataArray[3] = FIRMWARE_VERSION_POINT;
  dataArray[4] = 0x20;

  sendByteArrayAsSysex(0x20, dataArray, 5);
}

void sendFaderPositions() {
  refreshFaderMidiValues();
  sendByteArrayAsSysex(0x12, lastFaderMidiValues, FADER_COUNT);
}

void processSysexBuffer() {
  if (sysexLength < 6) return;
  if (!(sysexBuffer[0] == 0xF0 && sysexBuffer[1] == 0x7D && sysexBuffer[2] == 0x00 && sysexBuffer[3] == 0x00)) {
    return;
  }

  switch (sysexBuffer[4]) {
    case 0x1F:
      sendCurrentConfig();
      break;
    case 0x1D:
      currentBank = 0;
      tempBank = 0;
      bankLockout = true;
      activeBankConfig = makeDefaultBankConfig(1);
      break;
    case 0x0E:
      updateConfig(sysexBuffer, sysexLength);
      break;
    case 0x1A:
      // Factory reset is deliberately supported on SAMD21 because its
      // external filesystem survives a UF2 firmware reinstall.
      writeDefaultFiles();
      flip = activeConfig.rotated;
      setTrsMode(activeConfig.midiType);
      bankLockout = false;
      changingBank = false;
      bankChangePending = false;
      doLedAnimate = true;
      shouldSendConfig = true;
      break;
    case 0x21:
      if (!bankLockout) {
        currentBank = 0;
        tempBank = 0;
        bankLockout = true;
        activeBankConfig = makeDefaultBankConfig(1);
      }
      sendHeartbeat();
      break;
    case 0x12:
      sendFaderPositions();
      break;
    case 0x30:
      // Temporary host override; like RP2040 v1.0.0, this is not persisted.
      activeConfig.faderLed = false;
      break;
    case 0x31:
      // Restore MIDI activity blinking until power cycle or a config update.
      activeConfig.faderLed = true;
      break;
  }
}
