#include "firmware.h"

void ledSet(uint8_t i, uint8_t val) {
  if (i >= NUM_LEDS) return;
  digitalWrite(LED_PINS[i], val > 0 ? HIGH : LOW);
}

void ledOn(uint8_t i) {
  ledSet(i, 1);
}

void ledOff(uint8_t i) {
  ledSet(i, 0);
}

void clearLeds() {
  for (uint8_t i = 0; i < NUM_LEDS; i++) {
    ledOff(i);
  }
}

void updateLeds() {
  unsigned long now = millis();

  switch (learnMode) {
    case LEARN_MODE_OFF:
      for (uint8_t i = 0; i < NUM_LEDS; i++) {
        // Millisecond timing is explicit so SAMD21 loop speed cannot change
        // the user-visible 50 ms pulse used by RP2040 v1.0.0.
        bool ledBlinking = (now - lastMidiActivity[i]) < MIDI_BLINK_TIME_MS;
        if (ledBlinking && midiBlink[i] && activeConfig.faderLed && activeBankConfig.usbAnalog[i].channel > 0) {
          ledOn(i);
        } else if (velocityLeds[i] > 0) {
          ledOn(i);
        } else {
          ledOff(i);
        }
      }
      break;
    case LEARN_MODE_FADER:
      if (now - lastBlink > TEACH_BLINK_TIME_MS) {
        for (uint8_t i = 0; i < NUM_LEDS; i++) ledSet(i, (i < 4) && teachBlink);
        lastBlink = now;
        teachBlink = !teachBlink;
      }
      break;
    case LEARN_MODE_GESTURE:
      if (now - lastBlink > TEACH_BLINK_TIME_MS) {
        for (uint8_t i = 0; i < NUM_LEDS; i++) ledSet(i, (i > 3) && teachBlink);
        lastBlink = now;
        teachBlink = !teachBlink;
      }
      break;
  }

  if (changingBank) {
    clearLeds();
    ledOn(tempBank);
  }
}

void ledAnimate(uint8_t target) {
  for (uint8_t q = 0; q < NUM_LEDS; q++) {
    ledOn(constrain(target + q, 0, NUM_LEDS - 1));
    ledOn(constrain(target - q, 0, NUM_LEDS - 1));
    delay(25);
  }
  for (uint8_t q = 0; q < NUM_LEDS; q++) {
    ledOff(constrain(target + q, 0, NUM_LEDS - 1));
    ledOff(constrain(target - q, 0, NUM_LEDS - 1));
    delay(25);
  }
}

void setLedFromNoteOn(uint8_t note, uint8_t vel) {
  if (note >= NUM_LEDS) return;
  velocityLeds[note] = vel;
}
