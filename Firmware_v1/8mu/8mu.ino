/*
 * SAMD21 hardware adapter for the 8mu2040 firmware.
 *
 * Arduino supplies the SAMD21 build and USB-MIDI runtime; the firmware model,
 * config format, bank system, and editor protocol are ported from 8mu2040-main.
 */

#include "firmware.h"

void setup() {
  firmwareSetup();
}

void loop() {
  firmwareLoop();
}
