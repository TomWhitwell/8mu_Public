#pragma once

/*
   SAMD21 external flash file-store adapter.

   This is the hardware/storage shim used by the RP2040-shaped SAMD21 port;
   the config bytes and filenames are defined by the new firmware model.
*/

#include <Arduino.h>
#include <SPI.h>
#include <SdFat.h>
#include <Adafruit_SPIFlash.h>

#define EXTERNAL_FLASH_DEVICES  W25Q64JV_IQ
#define EXTERNAL_FLASH_USE_SPI  SPI1
#define EXTERNAL_FLASH_USE_CS   SS1

extern Adafruit_FlashTransport_SPI flashTransport;
extern Adafruit_SPIFlash flash;
extern FatFileSystem fatfs;

class EEPROMFlash {

  public:
    String _filename;
    void begin(String filename) {
      _filename = filename;
    };

    uint8_t read( int idx ) {
      uint8_t value = 0xFF;
      File eepromStore = fatfs.open(_filename, FILE_READ);
      if (eepromStore) {
        if (eepromStore.seek(idx)) {
          int content = eepromStore.read();
          if (content >= 0) {
            value = (uint8_t)content;
          }
        }
        eepromStore.close();
      }
      return value;
    }

    bool write(int idx, uint8_t val ) {
      File eepromStore = fatfs.open(_filename, O_RDWR | O_CREAT);
      if (!eepromStore) return false;
      bool ok = eepromStore.seek(idx) && eepromStore.write(val) == 1;
      eepromStore.flush();
      eepromStore.close();
      return ok;
    }

    bool readArray(int start, byte buffer[], int length) {
      for (int i = 0; i < length; i++) {
        buffer[i] = 0xFF;
      }

      File eepromStore = fatfs.open(_filename, FILE_READ);
      if (!eepromStore) return false;
      if (!eepromStore.seek(start)) {
        eepromStore.close();
        return false;
      }

      int bytesRead = 0;
      for (; bytesRead < length; bytesRead++) {
        int content = eepromStore.read();
        if (content < 0) break;
        buffer[bytesRead] = (byte)content;
      }
      eepromStore.close();
      return bytesRead == length;
    }

    bool writeArray(int start, const byte buffer[], int length) {
      File eepromStore = fatfs.open(_filename, O_RDWR | O_CREAT);
      if (!eepromStore) return false;
      if (!eepromStore.seek(start)) {
        eepromStore.close();
        return false;
      }

      bool ok = true;
      for (int i = 0; i < length; i++) {
        if (eepromStore.write(buffer[i]) != 1) {
          ok = false;
          break;
        }
      }
      eepromStore.flush();
      eepromStore.close();
      return ok;
    }

    bool verifyArray(int start, byte buffer[], int length) {
      byte verifyBuffer[length];
      if (!readArray(start, verifyBuffer, length)) return false;
      for (int i = 0; i < length; i++) {
        if (verifyBuffer[i] != buffer[i]) {
          return false;
        }
      }
      return true;
    }

    boolean exists() {
      File eepromStore = fatfs.open(_filename, FILE_READ);
      if (eepromStore) {
        eepromStore.close();
        return 1;
      }
      return 0;
    }

    bool create(int len) {
      byte blank[len];
      for (int i = 0; i < len; i++) {
        blank[i] = 0xFF;
      };
      return writeArray(0, blank, len);
    }
};
