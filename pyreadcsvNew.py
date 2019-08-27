# read CoMPASS .csv file into root
# ussage: python pyreadcsvNew.py -i *.csv
# J.H 20 Feb, 2019
# fill all the events in one TTree file; 
# fill waveforms into divided dump files (root file has a limit number to save TH1F)
# Ref: USER MANUAL UM5960 CoMPASS, www.caen.it
import ROOT
import sys, os, getopt
from ROOT import TTree, TFile, TH1F, AddressOf
import numpy as np
import csv
from array import array
path = os.getcwd()
file_list = os.listdir(path)
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
def process_csv(inputfile):
 histosize = 252 # recordLength 
 filename = inputfile.split('.')[0]
 print "now processing ", filename, "it will take minutes!"
 f0 = open(inputfile)
 reader = csv.reader(f0)
 csvdata = list(reader)
 csvdata = csvdata[1:] #delete the table title
 totalevents = len(csvdata)
 print "total events: "+str(totalevents)
 linedivide = 20000
 divide = totalevents/linedivide+1
 if divide!=0:
   print "divide into "+str(divide)+" files: "+ str(divide-1)+"*"+str(linedivide)+" + "+str(totalevents-(divide-1)*10000)+" events."

 lineNum = 0 # eventID == line number in data 
 fN = 0# file number

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

 ff = TFile("data_"+filename+".root","recreate")
 for item in csvdata: 
   evtID[0] = lineNum
   timeStamp[0] = float(item[0])/1024
   energy[0] = int(item[1])
   energyShort[0] = int(item[2])
   flag[0] = int(item[3],16)
   tree.Fill()
   lineNum = lineNum+1

 ff.cd()
 tree.Write()
 ff.Close()

 lineNum1 = 0 
 for fN in range(divide):
   print 'process file '+str(fN)
   f = TFile("dumpWaveform_"+filename+"_"+str(fN)+".root","recreate")
   lineNum1 = 0 + fN*linedivide
   if fN<divide-1:
     csvdata_divide = csvdata[lineNum1:lineNum1 + linedivide]
   else:
     csvdata_divide = csvdata[lineNum1:]
   for item in csvdata_divide:
     waveformdata = item[4:4+histosize]
     waveformdata = map(int,waveformdata)
     #print evtID, timeStamp, energy,energyShort,flag, waveformdata
     hwaveform = TH1F("hwf","",histosize-1,0,histosize)
     hwaveform.SetName("hwf"+str(lineNum1))
     # fill waveform
     ibin=0
     for idata in waveformdata:
       hwaveform.SetBinContent(ibin+1,idata)
       ibin = ibin+1
     lineNum1 = lineNum1+1
     f.cd()
     hwaveform.Write()
     #hwaveform.BufferEmpty()
 f.Close()
 fN = fN + 1

def main(argv):
  inputfile = ''
  try:
    opts, args = getopt.getopt(argv,"hi:",["ifile="])
  except getopt.GetoptError:
    print 'pyreadcsv.py -i <inputfile>'
    sys.exit(2)
  for opt, arg in opts:
    if opt == '-h':
      print 'pyreadcsv.py -i <inputfile>'
      sys.exit()
    elif opt in ("-i", "--ifile"):
      inputfile = arg
      print 'input', inputfile
      process_csv(inputfile)
  print 'complete processing.'

if __name__ == "__main__":
   main(sys.argv[1:])
