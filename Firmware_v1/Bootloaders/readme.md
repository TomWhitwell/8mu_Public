## Notes on building bootloaders: 

Once ARM toolchain is in place... 

SAMD Bootloader: 
* Clone https://github.com/microsoft/uf2-samdx1
* Add folder in `boards`: `MusicThing_m0_plus`
* Add `board_config.h` and `board.mk` from the Samd SRC folder here 
* Run `make -j8 BOARD=MusicThing_m0_plus` (-j8 = number of processors)
* Check the `build` folder for your build
* The `.bin` file is what you need to burn using Segger or another method 

Circuit Python: 
* Clone https://github.com/adafruit/circuitpython
* add folder in `ports/atmel-samd/boards': `MusicThing_m0_plus`
* Add `board.c mpconfigboard.h mpconfigboard.mk pins.c' from the CP SRC folder 
* Run `make -j8 BOARD=MusicThing_m0_plus` (-j8 = number of processors)
* Check the `build` folder for your build
* The `.uf2` file is what you need to drag onto MT_BOOT (or copy using cp [source] [destination] if using MacOS Ventura)

Arduino
* Not a bootloader, but a board definition that the Arduino IDE can use to compile code for 8mu 
* 8mu uses the "Music Thing Modular MusicThing m0 Plus" board definition 
* Board definitions are stored at https://github.com/TomWhitwell/board-defs
* Install: Add 
`https://raw.githubusercontent.com/TomWhitwell/board-defs/master/package_musicthing_samd21_index.json
`
to the "Additional Boards Manager URLs" list in Arduino Preferences



