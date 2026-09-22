import usb_midi
import board
from digitalio import DigitalInOut, Direction, Pull
from busio import UART


#Ultralight Midi
#inspired by Winterbloom_SmolMIDI
NOTE_OFF = const(0x80)
NOTE_ON = const(0x90)
CC = const(0xB0)
SYSEX = const(0xF0)
SYSEX_END = const(0xF7)
# CLOCK = const(0xF8)
# TICK = const(0xF9)
# START = const(0xFA)
# CONTINUE = const(0xFB)
# STOP = const(0xFC)


def hardMidi(type):
    ## Setup hardware midi
    ## Korg = UART to tip, source to ring = 0
    ## Arturia = UART to ring, source to tip = 1
    if type == 0:
        tip = board.D20
        ring = board.D9 
    else: 
        tip = board.D9
        ring =  board.D20
    midiSource =  DigitalInOut(ring)
    midiSource.direction = Direction.OUTPUT
    midiSource.value = True
    return UART(tip, None, baudrate=31250) 
    
def usbMidiIn():
    return usb_midi.ports[0]

def usbMidiOut():
    return usb_midi.ports[1]


def noteOn(note,velocity):
    return bytearray([NOTE_ON, note, velocity])

def noteOff(note):
    return bytearray([NOTE_OFF, note, 0])

def cc(controller, level):
    return bytearray([CC, controller, level])
    
def sysex(payload):
    return bytearray([SYSEX, payload, SYSEX_END])

def sysexPayload(payload):
    buffer = bytearray(len(payload)+2)
    buffer[0] = SYSEX
    for i in range(len(payload)):
        buffer[i+1] = payload[i]
    buffer[len(buffer)-1] = SYSEX_END
    return buffer


