## Hello World files 

This folder contains basic Hello World files for 8mu. 

One for Arduino, one for Circuit Python 

Both files have the same function - a basic USB midi step sequencer, with 
buttons on the back influencing playback speed and angle of the 8mu 
controlling velocity. 

# Arduino 

Before testing this on Arduino: 
*  Add 
`https://raw.githubusercontent.com/TomWhitwell/board-defs/master/package_musicthing_samd21_index.json`
  to "Additional Boards Manager Urls" in the Arduino IDE Preferences to 
install 'MusicThing M0 Plus' into the Arduino IDE, and select that board. 
* Install the  library BMI160-Arduino-master
  https://github.com/hanyazou/BMI160-Arduino
* Select USB Stack TinyUSB in the Arduino IDE Tools Menu 

# Circuit Python 

* This folder is designed for Circuit Python 7.3.3 
* The 'libs' folder includes drivers for the BMI160 accelerometer and a 
super minimal Midi library. Nothing else should be required. 
* The BMI160 is precompiled to an .mpy file. The uncompiled version is 
there in a separate folder. 

