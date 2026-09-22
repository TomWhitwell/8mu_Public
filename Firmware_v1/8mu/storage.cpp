#include "firmware.h"

Adafruit_FlashTransport_SPI flashTransport(SS1, SPI1);
Adafruit_SPIFlash flash(&flashTransport);
FatFileSystem fatfs;

EEPROMFlash CONFIG_STORE;
EEPROMFlash BANK_STORE;

static bool midiChannelValid(uint8_t value) {
  return value >= 1 && value <= 16;
}

static bool midiDataValid(uint8_t value) {
  return value <= 0x7F;
}

static bool configBytesLookValid(const uint8_t *buffer, size_t length) {
  if (length < CONFIG_BYTES) return false;

  if (buffer[0] > 1 || buffer[1] > 1 || buffer[2] > 1) return false;
  uint16_t faderMin = buffer[3] | (uint16_t(buffer[4]) << 8);
  uint16_t faderMax = buffer[5] | (uint16_t(buffer[6]) << 8);
  if (faderMin > 4095 || faderMax > 4095 || faderMax <= faderMin) return false;
  if (buffer[7] > 1 || buffer[8] > 1) return false;

  for (uint8_t bank = 0; bank < BANK_COUNT; bank++) {
    size_t base = 16 + (bank * BANK_CONFIG_BYTES);

    for (uint8_t i = 0; i < ANALOG_CONTROL_COUNT; i++) {
      if (!midiChannelValid(buffer[base + i])) return false;
      if (!midiDataValid(buffer[base + 16 + i])) return false;
      if (!midiChannelValid(buffer[base + 32 + i])) return false;
      if (!midiDataValid(buffer[base + 48 + i])) return false;
    }

    for (uint8_t output = 0; output < 2; output++) {
      size_t buttonBase = base + 64 + (output * 16);
      for (uint8_t i = 0; i < OUTPUT_BUTTON_COUNT; i++) {
        size_t button = buttonBase + (i * 4);
        if (!midiChannelValid(buffer[button])) return false;
        if (buffer[button + 1] > BUTTON_MODE_CC) return false;
        if (!midiDataValid(buffer[button + 2]) || !midiDataValid(buffer[button + 3])) return false;
      }
    }
  }

  return true;
}

bool initialiseStorage(bool forceFactoryReset) {
  CONFIG_STORE.begin("8MU2040.CFG");
  BANK_STORE.begin("8MUBANK2.CFG");

  if (!flash.begin() || !fatfs.begin(&flash)) return false;

  if (!CONFIG_STORE.exists() && !CONFIG_STORE.create(1024)) return false;
  if (!BANK_STORE.exists() && !BANK_STORE.create(1)) return false;

  if (forceFactoryReset) return writeDefaultFiles();

  uint8_t buffer[CONFIG_BYTES];
  if (!readConfigBytes(buffer, sizeof(buffer))) {
    return writeDefaultFiles();
  }

  return true;
}

bool writeDefaultFiles() {
  activeConfig = makeDefaultControllerConfig();
  currentBank = 0;
  tempBank = currentBank;
  activeBankConfig = activeConfig.banks[currentBank];
  bool configWritten = writeActiveConfigToFile();
  bool bankWritten = writeBankIndexToFile(currentBank);
  return configWritten && bankWritten;
}

bool readConfigBytes(uint8_t *outBuffer, size_t bufferSize) {
  return CONFIG_STORE.readArray(0, outBuffer, bufferSize) &&
         configBytesLookValid(outBuffer, bufferSize);
}

bool writeActiveConfigToFile() {
  uint8_t buffer[CONFIG_BYTES];
  configToBytes(activeConfig, buffer);
  return CONFIG_STORE.writeArray(0, buffer, CONFIG_BYTES) &&
         CONFIG_STORE.verifyArray(0, buffer, CONFIG_BYTES);
}

bool writeBankIndexToFile(uint8_t bank) {
  return BANK_STORE.write(0, bank) && BANK_STORE.read(0) == bank;
}

uint8_t readBankIndexFromFile() {
  uint8_t bank = BANK_STORE.read(0);
  if (bank >= BANK_COUNT) {
    bank = 0;
    writeBankIndexToFile(bank);
  }
  return bank;
}
