#/usr/bin/python
#filename: processAnaly.py
# get filenames in filelist
# change analysis code with different filename
# process analysis code and store information

import sys,os,glob,fnmatch
import subprocess
import os.path

path = os.getcwd() #path name string
file_list = os.listdir(path)
folder = "Nov16"
#if os.path.exists(folder+"/datalist.txt"):
#  subprocess.call(["rm",folder+"/datalist.txt"])

subprocess.call(["touch",folder+"/datalist.txt"])
subprocess.call("ls -1 "+folder+"/*.xml>>"+folder+"/datalist.txt",shell=True)
f1 = open(folder+"/datalist.txt",'r')
flist = f1.readlines()
namelist = []
countfile = 0

for line in flist:
  filename = line.split()[0] # get rid of '\n'
  subprocess.call(["root","-q","readMCA.C(\""+filename+"\")"])
  countfile += 1

print "processed "+str(countfile)+" files."
f1.close()
