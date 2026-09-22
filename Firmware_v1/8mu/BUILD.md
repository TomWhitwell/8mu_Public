# Building 8mu v1 firmware 1.5.2

The release build uses an isolated copy of the Music Thing SAMD platform. Do
not patch the globally installed Arduino core: copy it to a temporary hardware
directory and apply `arduino-core-serial-suffix.patch` there.

## Tested toolchain

- Arduino IDE 1.8.19 `arduino-builder`
- Music Thing SAMD platform 0.1.13
- Arduino ARM GCC 4.8.3-2014q1
- Adafruit CMSIS 5.4.0
- Arduino CMSIS-Atmel 1.2.0
- Bounce2 2.71
- MIDIUSB 1.0.5
- Adafruit SPIFlash 4.3.4
- SdFat Adafruit Fork 2.2.3
- BMI160 Arduino library

The commands below assume this repository is at
`/Users/tom/Documents/GitHub/8mu_Public` and the Arduino packages and libraries
are installed in the standard locations for the `tom` account. Adjust those
paths for another machine.

## Prepare an isolated patched platform

```sh
BUILD_ROOT="$(mktemp -d /private/tmp/8mu-samd21-152.XXXXXX)"
mkdir -p "$BUILD_ROOT/hardware/music thing/samd"
cp -R \
  "/Users/tom/Library/Arduino15/packages/music thing/hardware/samd/0.1.13" \
  "$BUILD_ROOT/hardware/music thing/samd/0.1.13"
patch -d "$BUILD_ROOT/hardware/music thing/samd/0.1.13" -p1 \
  < "/Users/tom/Documents/GitHub/8mu_Public/Firmware/8mu/arduino-core-serial-suffix.patch"
mkdir -p "$BUILD_ROOT/build" "$BUILD_ROOT/cache"
```

## Compile

```sh
/Applications/Arduino.app/Contents/Java/arduino-builder \
  -compile -logger=machine -warnings=all -ide-version=10819 \
  -hardware /Applications/Arduino.app/Contents/Java/hardware \
  -hardware "$BUILD_ROOT/hardware" \
  -tools /Applications/Arduino.app/Contents/Java/tools-builder \
  -tools /Applications/Arduino.app/Contents/Java/hardware/tools/avr \
  -tools /Users/tom/Library/Arduino15/packages \
  -built-in-libraries /Applications/Arduino.app/Contents/Java/libraries \
  -libraries /Users/tom/Documents/Arduino/libraries \
  '-prefs=runtime.tools.arm-none-eabi-gcc.path=/Users/tom/Library/Arduino15/packages/arduino/tools/arm-none-eabi-gcc/4.8.3-2014q1' \
  '-prefs=runtime.tools.CMSIS-5.4.0.path=/Users/tom/Library/Arduino15/packages/adafruit/tools/CMSIS/5.4.0' \
  '-prefs=runtime.tools.CMSIS-Atmel-1.2.0.path=/Users/tom/Library/Arduino15/packages/arduino/tools/CMSIS-Atmel/1.2.0' \
  '-prefs=build.usb_product="Music Thing 8mu"' \
  -fqbn='music thing:samd:musicthing_musicthing_m0_plus:usbstack=arduino,debug=off' \
  -build-path "$BUILD_ROOT/build" \
  -build-cache "$BUILD_ROOT/cache" \
  /Users/tom/Documents/GitHub/8mu_Public/Firmware/8mu/8mu.ino
```

## Convert to UF2

The existing bootloader expects the application at address `0x2000`.

```sh
python3 \
  /Users/tom/Library/Arduino15/packages/rp2040/hardware/rp2040/3.9.2/tools/uf2conv.py \
  -c -b 0x2000 -f SAMD21 \
  -o "$BUILD_ROOT/8mu_samd21_v1.5.2.uf2" \
  "$BUILD_ROOT/build/8mu.ino.bin"
```

The hardware-tested release UF2 has this SHA-256 digest:

```text
496806e1ab22713c7d998eb024705fe8fe56d260e1d93853c606088927d0c87f
```

This digest verifies the published release asset. The legacy SAMD platform
embeds the absolute `SPI.cpp` source path in the application for assertion
messages, so a build made in a differently named temporary directory will have
a different binary digest even when the source and generated machine code are
otherwise equivalent.
