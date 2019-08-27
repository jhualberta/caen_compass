# 25 July 2019 : Jie Hu 
# convert .bin data file to .root file
# translate CoMPASS data structure to ROOT tree
# ussage: python pyreadcsvNew.py -i *.csv
# also fill waveforms into divided dump files (root file has a limit number to save TH1F)
# Ref: USER MANUAL UM5960 CoMPASS, www.caen.it
import ROOT
import sys, os, getopt
from ROOT import TTree, TFile, TH1F, AddressOf
import numpy as np
from array import array
path = os.getcwd()
file_list = os.listdir(path)
#import binascii
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
# trigger flag labels
'''
especially: 16384 -- fine time stamp event, 32768 -- piled-up 

1 0x1 A dead-time is occurred before this event
2 0x2 Time stamp roll-over
4 0x4 Time stamp reset from external
8 0x8 Fake event
16 0x10 A memory full is occurred before this event
32 0x20 A trigger lost is occurred before this event
64 0x40 N triggers have been lost (N can be set through bits[17:16] of register 0x1n84 [RD4])
128 0x80 The event is saturating inside the gate - clipping
256 0x100 1024 triggers have been counted
1024 0x400 The input is saturating
2048 0x800 N triggers have been counted (N can be set through bits[17:16] of register 0x1n84 [RD4])
4096 0x1000 Event not matched in the time correlation filter but visualized in the waveform
16384 0x4000 Event with fine time stamp
32768 0x8000 Piled-up event
'''
def process_bin(inputfile):
   fname = inputfile.split('.')[0]
   print "now processing ", fname, "it will take minutes!"
   ff = TFile("data_"+fname+".root","recreate")
   evtID = array('L',[0]) #unsigned int
   timeStamp = array('d',[0]) #unsigned double 
   energy = array('I',[0])
   energyShort = array('I',[0])
   flag = array('I',[0])
   tree = TTree('T','dump CAEN data')
   tree.Branch('eventID',evtID,'eventID/l')
   tree.Branch("timeStamp", timeStamp, "timeStamp/D") #ULong64_
   tree.Branch("energy", energy, "energy/s")
   tree.Branch("energyShort", energyShort, "energyShort/s")
   tree.Branch("flag", flag, "flag/s")
   
   totalEvt = 0
   waveformdata = []
   boardNumber = 0 # default
   channelNumber = 0 # default
   histosize = 252 # ns, the length of timing waveform, default
   with open(inputfile, "rb") as binfile :
     size = ushort*2+uQ+ushort*2+uint+uint
     countDataChunk = 0
     flagBegin = 1
     while True:
       chunk1 = binfile.read(size)
       if not chunk1: 
           break # if board information is recorded, timing histogram should be there
       board, channel, time0, elong0, eshort0, flag0, nSample = struct.unpack('<HHqHHII', chunk1)  # "<" for big-endian, CAEN uses inverted format
       if flagBegin: # for the board information, only need to check one event and assume the remainings are same
           print "Checking... board number:", board, "channel:",channel, "waveform length [ns]:", nSample
       flagBegin = 0
       hist = [struct.unpack('H', binfile.read(2))[0] for i in range(nSample)] # loop timing histogram
       waveformdata.append(hist)
       #print board, channel, time, flag, nSample
       #print hist
       evtID[0] = countDataChunk 
       timeStamp[0] = float(time0)/1024 # ps to ns
       energy[0] = elong0
       energyShort[0] = eshort0
       flag[0] = flag0
       tree.Fill()
       countDataChunk = countDataChunk+1
   totalEvt = countDataChunk
   print "total entries: ", countDataChunk 
   ff.cd()
   tree.Write()
   ff.Close()

   # dump waveforms into several .root files, each contains up to 20000 events
   fN = 0# file number
   linedivide = 20000
   divide = totalEvt/10000+1
   lineNum1 = 0
   for fN in range(divide):
     print 'process file '+str(fN)
     f = TFile("dumpWaveform_"+fname+"_"+str(fN)+".root","recreate")
     lineNum1 = 0 + fN*linedivide
     if fN<divide-1:
       data_divide = waveformdata[lineNum1:lineNum1 + linedivide]
     else:
       data_divide = waveformdata[lineNum1:]
     for entry in data_divide:
       #print evtID, timeStamp, energy,energyShort,flag, waveformdata
       hwaveform = TH1F("hwf","",histosize-1,0,histosize)
       hwaveform.SetName("hwf"+str(lineNum1))
       # fill waveform
       ibin=0
       for idata in entry:
         hwaveform.SetBinContent(ibin+1,idata)
         ibin = ibin+1
       lineNum1 = lineNum1+1
       f.cd()
       hwaveform.Write()
       #hwaveform.BufferEmpty()
   f.Close()

def main(argv):
  inputfile = ''
  try:
    opts, args = getopt.getopt(argv,"hi:",["ifile="])
  except getopt.GetoptError:
    print 'python pyReadByte.py -i <inputfile>'
    sys.exit(2)
  for opt, arg in opts:
    if opt == '-h':
      print 'python pyReadByte.py -i <inputfile>'
      sys.exit()
    elif opt in ("-i", "--ifile"):
      inputfile = arg
      print 'input', inputfile
      process_bin(inputfile)
  print 'complete processing.'

if __name__ == "__main__":
   main(sys.argv[1:])
