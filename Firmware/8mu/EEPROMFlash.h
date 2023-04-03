/*
   8mu external flash EEPROM emulatorr
   (c) 2023 by Tom Whitwell
   MIT License
*/

#include <Arduino.h>
#include <SPI.h>
#include <SdFat.h>
#include <Adafruit_SPIFlash.h>

#define EXTERNAL_FLASH_DEVICES  W25Q64JV_IQ
#define EXTERNAL_FLASH_USE_SPI  SPI1
#define EXTERNAL_FLASH_USE_CS   SS1

Adafruit_FlashTransport_SPI flashTransport(SS1, SPI1);
Adafruit_SPIFlash flash(&flashTransport);
FatFileSystem fatfs;

class EEPROMFlash {

  public:
    String _filename;
    void begin(String filename) {
      flash.begin();
      fatfs.begin(&flash);
      _filename = filename;

    };

    uint8_t read( int idx ) {
      File eepromStore = fatfs.open(_filename, FILE_READ);
      if (eepromStore) {
        eepromStore.seek(idx);
        char content = eepromStore.read();
        eepromStore.close();
        return content;
      }
      else {
        eepromStore.close();
        return 0;
      }
    }

    void write(int idx, uint8_t val ) {
      File eepromStore = fatfs.open(_filename, FILE_WRITE);
      eepromStore.seek(idx);
      eepromStore.write(val);
      eepromStore.close();
    }

    void readArray(int start, byte buffer[], int length) {
      File eepromStore = fatfs.open(_filename, FILE_READ);
      if (eepromStore) {
        eepromStore.seek(start);
        for (int i = 0; i < length; i++) {
          buffer[i] = eepromStore.read();
        }
      }
      else {
        eepromStore.close();
      }
    }

    void writeArray(int start, byte buffer[], int length) {
      File eepromStore = fatfs.open(_filename, FILE_WRITE);
      eepromStore.seek(start);
      for (int i = 0; i < length; i++) {
        eepromStore.write(buffer[i]);
      }
      eepromStore.close();
    }

    boolean exists() {
      File eepromStore = fatfs.open(_filename, FILE_READ);
      if (eepromStore) {
        eepromStore.close();
        return 1;
      }
      else {
        eepromStore.close();
        return 0;
      }
    }

    void create(int len) {
      byte blank[len];
      for (int i = 0; i < len; i++) {
        blank[i] = 0xFF;
      };
      writeArray(0, blank, len);
    }
};
