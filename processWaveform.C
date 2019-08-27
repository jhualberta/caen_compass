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
int shortgate[2] = {32,82};// tune parameters
int gatetime = shortgate[1] - shortgate[0];
int pregate = 8;
int delayW = 1; // 1 ns for DT5751
// calibration parameters: to match CoMPASS values
double cp0 = -9.816, cp1 = 0.03104;

double calcuEnergy(TH1F* hwf, double baseline)
{
   bool flag = 0;
   double energy = 0;
   int threshold = 0;
   int trigger_time = 0;

   double baseline2 = baseline*(attenu-1);
   TH1F *hprocessWf = new TH1F("hprocessWf","",recordLength,0,recordLength);
   for(int q = 0;q<recordLength;q++){
     //invert and delay and fill histogram
     hprocessWf->SetBinContent(q,((hwf->GetBinContent(q+1+delayW))*-1  +  attenu * hwf->GetBinContent(q+1)));
     hprocessWf->SetBinContent(recordLength,baseline2);
   }
   threshold = (hprocessWf->GetMaximum()- hprocessWf->GetMinimum())/3;

   for(int q = 0;q<recordLength;q++){
     //find trigger time
     double trig_counts = hprocessWf->GetBinContent(q+1);
     if((trig_counts < (baseline2 - threshold)) && flag == 0){flag = 1;}
     if((flag == 1) && (trig_counts > baseline2) ){trigger_time = q-1;break;}
   }

   //if((trigger_time == 0) || trigger_time >50.){
       //cout<<"i "<<i<<" trigger_time "<<trigger_time[i]<<" baseline2[i] "<<baseline2[i]<<" threshold[i] "<<threshold[i]<<endl;
   //}
   //trig->Fill(trigger_time);
   energy = hwf->Integral(trigger_time-pregate,trigger_time + gatetime-pregate) - baseline*(gatetime);
   delete hprocessWf;
   return energy; 
}

void processWaveform() {
   TString fileName("TeSOP_137Cs_2PMTcoin_2200V_thresh100_1min_csv_25Mar");

   TString datafileCh0 = "data_ch0_"+ fileName + ".root";
   TFile *fdataCh0 = new TFile(datafileCh0);

   TString datafileCh1 = "data_ch1_"+ fileName + ".root";
   TFile *fdataCh1 = new TFile(datafileCh1);

   TTree *t1 = (TTree*)fdataCh0->Get("T");
   TTree *t2 = (TTree*)fdataCh1->Get("T");
 
   int evtNumCh0 = t1->GetEntries();
   int evtNumCh1 = t2->GetEntries();

   int linedivide = 20000; // number of divided waveforms files

   // calculate channel 0 
   int dividefile = evtNumCh0/linedivide + 1;
   int lastfileEvt0 = 0, lastfileEvt1 = 0;
   if (dividefile!=0)
   lastfileEvt0 = evtNumCh0-(dividefile-1)*linedivide;

   // calculate channel 1 
   dividefile = evtNumCh1/linedivide + 1;
   if (dividefile!=0)
   lastfileEvt1 = evtNumCh1-(dividefile-1)*linedivide;

   Double_t tag0 = 0, tag1 = 0;
   UShort_t trig0 = 0, trig1 = 0;
   ULong64_t eventID0 = 0, eventID1 = 0;

   TFile *f1 = new TFile(TString("Processed_"+fileName+".root"),"recreate");
   t1->SetBranchAddress("timeStamp",&tag0);
   t2->SetBranchAddress("timeStamp",&tag1);

   t1->SetBranchAddress("eventID",&eventID0);
   t2->SetBranchAddress("eventID",&eventID1);

   t1->SetBranchAddress("flag",&trig0);
   t2->SetBranchAddress("flag",&trig1);

   int totalEvt0 = t1->GetEntries();
   int totalEvt1 = t2->GetEntries();

   TTree *tNew0 = new TTree("Tch0","dump processed CAEN data, ch0");
   TTree *tNew1 = new TTree("Tch1","dump processed CAEN data, ch1");

   double timeStamp0, timeStamp1;
   double energy0, energy1;
   double eCor0, eCor1;
   UShort_t flag0, flag1;


   tNew0->Branch("eventID0", &eventID0, "eventID0/L");
   tNew0->Branch("timeStamp0", &timeStamp0, "timeStamp0/D");
   tNew0->Branch("energy0", &energy0, "energy0/D");
   tNew0->Branch("eCor0", &eCor0, "eCor0/D");
   tNew0->Branch("flag0", &flag0, "flag0/s");

   tNew1->Branch("eventID1", &eventID1, "eventID1/L");
   tNew1->Branch("timeStamp1", &timeStamp1, "timeStamp1/D");
   tNew1->Branch("energy1", &energy1, "energy1/D");
   tNew1->Branch("eCor1", &eCor1, "eCor1/D");
   tNew1->Branch("flag1", &flag1, "flag1/s");

   int lineNum0, lineNum1, linestart = 0; 
   cout<<"Now start to process, it takes minutes, be patient ..."<<endl;
   for(int ifile = 0; ifile<dividefile; ifile++) // loop on files
   {
     cout<<"process dumpwave_"<<ifile<<endl;
     TString ifN; ifN.Form("%d",ifile); 
     TString fileDump0 = "dumpWaveform_ch0_"+fileName+"_"+ifN+".root";
     TString fileDump1 = "dumpWaveform_ch1_"+fileName+"_"+ifN+".root"; 

     TFile *fs1 = new TFile(fileDump0,"read");
     TFile *fs2 = new TFile(fileDump1,"read"); 
     linestart = ifile*linedivide;

     //assume before the last file, all files from 2 channels are same amount
     if(ifile < dividefile-1) {
       vector<TH1F*> hlistWaveform0;
       vector<TH1F*> hlistWaveform1;

       vector<TH1F*> processListWaveform0;
       vector<TH1F*> processListWaveform1;
       for(int i = linestart; i<linestart+linedivide; i++) // loop events in one file, for channel 0
       {
        if(i%5000==0) cout<<i<<" "<<endl;
        t1->GetEntry(i);
        timeStamp0 = tag0;
        flag0 = trig0;

        t2->GetEntry(i);
        timeStamp1 = tag1;
        flag1 = trig1;

        TH1F *htemp = (TH1F*)fs1->Get(Form("hwf%u",i));
        TH1F *htemp1 = (TH1F*)fs2->Get(Form("hwf%u",i));

        int ihist0 = 0, ihist1 = 0;
        double baseLine = htemp->Integral(0,32)/32;

        if(htemp->GetMaximum()/baseLine<1.1) // flat waveform, E=0 
        { energy0 = 0; }
        else {
          int substract = baseLine*(shortgate[1]-shortgate[0]);
          int eshort = htemp->Integral(shortgate[0],shortgate[1])-substract;
          energy0 = ( cp0 + eshort*cp1 );
          eCor0 = calcuEnergy(htemp, baseLine);
          eCor0 = cp0 + cp1*eCor0;
        } //check flat waveform
        tNew0->Fill();
        ihist0++;
        double baseLine1 = htemp1->Integral(0,32)/32;
        if(htemp1->GetMaximum()/baseLine1<1.1)
        { energy1 = 0;}
        else {
          int substract1 = baseLine1*(shortgate[1]-shortgate[0]);
          int eshort1 = htemp1->Integral(shortgate[0],shortgate[1])-substract1;
          energy1 = (cp0 + cp1*eshort1);
          eCor1 = calcuEnergy(htemp1,baseLine1);
          eCor1 = cp0 + cp1*eCor1;
        } // calculate energy
        //delete *htemp;
        //delete *htemp1;
        tNew1->Fill();
        ihist1++;
      }//loop events
       //hlistWaveform0.clear(),hlistWaveform1.clear();
     } // before last file     
     
    
     if (ifile == dividefile-1)
     {  
       cout<<"now process last dump file "<<endl;
       vector<TH1F*> hlistWaveform0;
       vector<TH1F*> hlistWaveform1;
 
       int ihist0 = 0;
       for(int i = linestart; i<linestart+lastfileEvt0; i++) // loop events in one file, for channel 0
       {
        //if(i%5000==0) cout<<i<<" "<<lineNum0<<" "<<lastNum0<<endl;
        t1->GetEntry(i);
        timeStamp0 = tag0;
        flag0 = trig0;

        TH1F *htemp = (TH1F*)fs1->Get(Form("hwf%u",i));
 
        double baseLine = htemp->Integral(0,32)/32;
        if(htemp->GetMaximum()/baseLine<1.1) // flat waveform, E=0
        { energy0 = 0; }
        else {
          int substract = baseLine*(shortgate[1]-shortgate[0]);
          int chj0 = 0;//index of processed waveforms
          int eshort = htemp->Integral(shortgate[0],shortgate[1])-substract;
          //cout<<eshort<<endl;
          energy0 = (cp0+cp1*eshort);
          eCor0 = calcuEnergy(htemp, baseLine);
          eCor0 = cp0+cp1*eCor0;
        } //check flat waveform
        
        tNew0->Fill();
        ihist0++;
       }
       hlistWaveform0.clear();

       int ihist1 = 0;
       for(int j = linestart; j<linestart+lastfileEvt1; j++) // loop events in one file, for channel 0
       {
        t2->GetEntry(j);
        timeStamp1 = tag1;
        flag1 = trig1;

        TH1F *htemp1 = (TH1F*)fs2->Get(Form("hwf%u",j));

        double baseLine1 = htemp1->Integral(0,32)/32;
        if(htemp1->GetMaximum()/baseLine1<1.1) // flat waveform, E=0 
        { energy1 = 0; }
        else {
          int substract1 = baseLine1*(shortgate[1]-shortgate[0]);
          int chj1 = 0;//index of processed waveforms
          int eshort1 = htemp1->Integral(shortgate[0],shortgate[1])-substract1;
          //cout<<eshort<<endl;
          energy1 = (cp0 + cp1*eshort1);
          eCor1 = calcuEnergy(htemp1, baseLine1);
          eCor1 = cp0 + cp1*eCor1;
        } //check flat waveform
        
        tNew1->Fill();
        ihist1++;
       }
     } // loop events in last file

     fs1->Close();fs2->Close();
   } // loop files
   f1->cd(); 
   tNew0->Write(); tNew1->Write();
   f1->Close();
}
