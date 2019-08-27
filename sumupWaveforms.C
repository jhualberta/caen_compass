// CAEN CoMPASS files are saved as: two data_ch*.root files for each channels, each has TTree which saves timeStamp, energy, ...
// Dumped waveforms are divided and saved into a serials of dump_ch*.root files
// each has a number of linedivide=20000 TH1F waveforms; the last file has totalEventNumber-(dividefile-1)*linedivide waveforms

// This code calculates energies through dumped waveforms. It goes through all the dump_ch*.root files, also go through TTree
// to extract trigger flags and timeStamps. Custom calculated energies as well as CAEN saved flags and timeStamps
// are filled into new TTree for each channels. 

//Author: Jie Hu 
#include "TH2.h"
#include <vector>
#include "TCanvas.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TTree.h"
#include "TMath.h"
#include "TH1F.h"
#include "TF1.h"
#include "TRandom.h"
#include "TSpectrum.h"
#include "TVirtualFitter.h"

double coarseGain = 80;
Int_t npeaks = 4;
Int_t recordLength = 252;
Int_t adcChannel = 8192;
double attenu = 0.75;//25%
int shortgate[2] = {32,47};// tune parameters
int delayW = 1; // 1 ns for DT5751

void sumupWaveforms() {

   TString sample = "labPPO_137Cs_2PMTcoin_csv_5Apr";
   TString chN = "_ch0_";
   TFile *fdataCh0 = new TFile("data"+chN+sample+".root");

   TTree *t1 = (TTree*)fdataCh0->Get("T");
 
   int evtNumCh0 = t1->GetEntries();

   int linedivide = 20000; // number of divided waveforms files

   // calculate channel 0 
   int dividefile = evtNumCh0/linedivide + 1;
   int lastfileEvt0 = 0, lastfileEvt1 = 0;
   if (dividefile!=0)
   lastfileEvt0 = evtNumCh0-(dividefile-1)*linedivide;

   Double_t tag1 = 0, tag2 = 0;
   UShort_t fg1 = 0, fg2 = 0;
   TFile *f1 = new TFile("sumup"+chN+"rmSaturate_"+sample+".root","recreate");

   TFile *f00 = new TFile("dumpWaveform"+chN+sample+"_0.root","read");
   TH1F *hsumup = (TH1F*)f00->Get("hwf0");

   int totalEvt0 = t1->GetEntries();

   int lineNum0, lineNum1, linestart; 
   for(int ifile = 0; ifile<dividefile; ifile++) // loop on files
   {
     TString ifN; ifN.Form("%d",ifile);
     TFile *fs1 = new TFile("dumpWaveform"+chN+sample+"_"+ifN+".root","read");

     linestart = ifile*linedivide;

     //suppose before the last file, all files from 2 channels are same amount
     if(ifile<dividefile-1) {
       vector<TH1F*> hlistWaveform0;
       vector<TH1F*> processListWaveform0;
       for(int i = linestart; i<linestart+linedivide; i++) // loop events in one file, for channel 0
       {
        if(i%5000==0) cout<<i<<" "<<endl;

        TH1F *htemp = new TH1F("htemp","", recordLength, 0, recordLength);
        hlistWaveform0.push_back((TH1F*)fs1->Get(Form("hwf%u",i)));
        delete htemp;

        int ihist0 = 0, ihist1 = 0;
        double baseLine = hlistWaveform0[ihist0]->Integral(0,32)/32;
        if(hlistWaveform0[ihist0]->GetMaximum()/baseLine<1.1 || hlistWaveform0[ihist0]->GetMaximum()>1000) // flat waveform, E=0 
        {  }
        else {
          hsumup->Add(hlistWaveform0[ihist0]); 
        } //check flat waveform
        ihist0++;
      }//loop events
       hlistWaveform0.clear();
     } // before last file     
     
    
     if (ifile == dividefile-1)
     {  
       vector<TH1F*> hlistWaveform0;
 
       int ihist0 = 0;
       for(int i = linestart; i<linestart+lastfileEvt0; i++) // loop events in one file, for channel 0
       {
        //if(i%5000==0) cout<<i<<" "<<lineNum0<<" "<<lastNum0<<endl;
        t1->GetEntry(i);

        TH1F *htemp = new TH1F("htemp","", recordLength, 0, recordLength);
        hlistWaveform0.push_back((TH1F*)fs1->Get(Form("hwf%u",i)));
        delete htemp;
 
        double baseLine = hlistWaveform0[ihist0]->Integral(0,32)/32;
        if(hlistWaveform0[ihist0]->GetMaximum()/baseLine<1.1 || hlistWaveform0[ihist0]->GetMaximum()>1000 ) // flat waveform, E=0 
        { }
        else {
          hsumup->Add(hlistWaveform0[ihist0]);
        } //check flat waveform
        ihist0++;
       }
       hlistWaveform0.clear();

     } // loop events in last file

     fs1->Close();
   } // loop files
   f1->cd(); 
   hsumup->Write();
   f1->Close();
}
