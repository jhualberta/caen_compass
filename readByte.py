# 05 July 2019 : Jie Hu 
# convert .bin data file to .root file
# translate CoMPASS data structure to ROOT tree

import binascii
import numpy as np
import struct
from struct import *
fname = 'ch0_LU151_UA-8_bin_attenu_Coin_16July.bin'
ushort = 2 #H, unsigned short
uint = 4 #I, unsigned int
uQ = 8 #Q, unsigned long
'''
format:
for each event:
     board  channel timeStamp Elong Eshort flag nSample waveform
byte 2       2         8     2      2    4      4       2*252

nSample is set to 252, therefore a data chunk for an event is 528 bytes 
'''
with open(fname, "rb") as binfile :
    # < for big-endian
   size = ushort*2+uQ+ushort*2+uint+uint
   while True:
     chunk1 = binfile.read(size)
     if not chunk1: break # if board information is recorded, timing histogram should be there
     board, channel, time, elong, eshort, flag, nSample = struct.unpack('<HHqHHII', chunk1)  # "<" for big-endian, CAEN uses inverted format
     hist = [struct.unpack('H', binfile.read(2))[0] for i in range(nSample)] # loop timing histogram
     print board, channel, time, flag, nSample
     print hist
