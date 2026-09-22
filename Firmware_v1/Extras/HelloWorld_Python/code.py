# Music Thing Modular 8mu Firmware - Hello World
# 
#   This is a simple midi step sequencer
#   Positions of 8 faders map to 8 steps, output on midi channel 1 via USB
#   Leds show the active step
#   Accelerometer measures if the front of 8mu is lifted, and maps that to Midi velocity
#   Buttons on the back influence clock speed




from time import sleep
import board
import uMidi 
from digitalio import DigitalInOut, Direction, Pull
from analogio import AnalogIn
from BMI160 import startIMU

## BMI160 is a big library, so is pre-compiled as an .mpy file 
## Other libraries are left uncompiled so they're easier to read 

# remap function recreates Arduinos' map function 
def remap(x, in_min, in_max, out_min, out_max):
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min

## Setup LEDs 
leds = [DigitalInOut(board.D12), DigitalInOut(board.D22), DigitalInOut(board.D16), DigitalInOut(board.D21), DigitalInOut(board.D18), DigitalInOut(board.D17), DigitalInOut(board.D19), DigitalInOut(board.D13)]
for led in leds: 
    led.direction = Direction.OUTPUT

# Setup Faders 
faders = [AnalogIn(board.A0), AnalogIn(board.A1), AnalogIn(board.A2), AnalogIn(board.A3), AnalogIn(board.A4),AnalogIn(board.A7), AnalogIn(board.A5), AnalogIn(board.A6)]

## Setup Buttons 
buttons = [DigitalInOut(board.MOSI),DigitalInOut(board.SCK),DigitalInOut(board.MISO),DigitalInOut(board.D15)]
for button in buttons:
    button.direction = Direction.INPUT
    button.pull = Pull.UP

# Setup Accelerometer 
accel = startIMU(True)

# Setup Midi 
midiOut = uMidi.usbMidiOut()

counter = 0
bpm = 120 

while True: 
    # Light LED 
    leds[counter].value = True
    
    # Read faders 
    # Faders read 0-65535, high value is at the bottom 
    read = faders[counter].value 
    
    # Convert reading to midi note & invert so low = high 
    # And also make it an integer  
    note = int(remap(read, 0, 65535, 81, 33)) 
    
    # Read accelerometer 
    # movements[0] = raise front of device
    movements = accel.getGestures8()
    velocity = int(remap(movements[0],0,16000, 40, 127))

    # Map buttons to BPM 
    # Buttons pulled high, so low = pushed 
    if (not buttons[0].value):
        bpm = 30
    elif (not buttons[1].value):
        bpm = 60
    elif (not buttons[2].value):
        bpm = 240
    elif (not buttons[3].value):
        bpm = 480
    else:
        bpm = 120

    # Write midi note on 
    # uMidi formats the message 
    # midiOut is a direct connection to the USB midi port 
    midiOut.write(uMidi.noteOn(note,velocity))

    sleep(15/bpm)
    
    # End midi note 
    midiOut.write(uMidi.noteOff(note))

    # Turn active LED off
    leds[counter].value = False
    counter += 1
    if counter>=8:
        counter = 0 
    
def remap(x, in_min, in_max, out_min, out_max):
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min
    
    