// Illustrates how to find peaks in histograms.
// This script generates a random number of gaussian peaks
// on top of a linear background.
// The position of the peaks is found via TSpectrum and injected
// as initial values of parameters to make a global fit.
// The background is computed and drawn on top of the original histogram.
//
// To execute this example, do
//  root > .x peaks.C  (generate 10 peaks by default)
//  root > .x peaks.C++ (use the compiler)
//  root > .x peaks.C++(30) (generates 30 peaks)
//
// To execute only the first part of the script (without fitting)
// specify a negative value for the number of peaks, eg
//  root > .x peaks.C(-20)
//
//Author: Rene Brun
#include "TH2.h"
#include <vector>
#include "TCanvas.h"
#include "TFile.h"
#include "TCanvas.h"
#include "TMath.h"
#include "TH1F.h"
#include "TF1.h"
#include "TRandom.h"
#include "TSpectrum.h"
#include "TVirtualFitter.h"

Int_t npeaks = 4;
Int_t recordLength = 252;
Int_t adcChannel = 16384;

Double_t fpeaks(Double_t *x, Double_t *par) {
      Double_t result;
      Double_t norm  = par[0];
      Double_t mean  = par[1];
      Double_t sigma = par[2];
      result = norm*TMath::Gaus(x[0],mean,sigma);
   return result;
}

void peak_waveform(){
    TFile *f0 = new TFile("dumpEvt_0.root","read");
    vector<TH1F*> hlistWaveform;
    int evtNum = 10000;
    for(int i = 0;i<evtNum;i++)
    {
     TH1F *htemp = new TH1F("htemp","", recordLength, 0, recordLength);
     hlistWaveform.push_back((TH1F*)f0->Get(Form("hwf%u",i)));
     delete htemp;
    }

    TH1F *h2 = (TH1F*)hlistWaveform[0]->Clone("h2");
    TFile *f1 = new TFile("processed_wf.root","recreate");
    // loop and integrate waveform with time gate
    vector<int> energyShort_cor;
    int shortgate[2] = {30,45};// tune parameters
    TH1F *hEshort_cor = new TH1F("hEshort_cor","energy short gate, corrected", adcChannel, 0, adcChannel);
    for(vector<TH1F*>::iterator it = hlistWaveform.begin(); it != hlistWaveform.end(); it++)
    {
      TH1F *hp = *it;
      //cout<<hp->Integral(0,recordLength)<<endl;
      //hp->Scale(1./hp->GetMaximum());
      int eshort = hp->Integral(shortgate[0],shortgate[1]);
      cout<<eshort<<endl;
      energyShort_cor.push_back(eshort);
      hEshort_cor->Fill(eshort);
    }

    TCanvas *c1 = new TCanvas("c1","c1",10,10,1000,900); 
    c1->Divide(1,2);c1->cd(1);//gPad->SetLogy();
    hEshort_cor->Draw();
    /*
    hlistWaveform[0]->Draw();

    TSpectrum *s = new TSpectrum(3);
    Int_t nfound = s->Search(hlistWaveform[0],10,"",0.05);//Search (const TH1 *hist, Double_t sigma=2, Option_t *option="", Double_t threshold=0.05)
    printf("Found %d candidate peaks to fit\n",nfound);
    //Estimate background using TSpectrum::Background
    TH1 *hb = s->Background(hlistWaveform[0],20,"same");

    c1->Update();
 
   ////estimate linear background using a fitting method
   c1->cd(2);
   //gPad->SetLogy(); 
   TF1 *fline = new TF1("fline","pol1",0,1000);
   hlistWaveform[0]->Fit("fline","qn");
   //Loop on all found peaks. Eliminate peaks at the background level
   Double_t par[3000];
   par[0] = fline->GetParameter(0);
   par[1] = fline->GetParameter(1);
   npeaks = 0;
   Float_t *xpeaks = s->GetPositionX();
   for (int p=0;p<nfound;p++) {
      Float_t xp = xpeaks[p];
      Int_t bin = hlistWaveform[0]->GetXaxis()->FindBin(xp);
      Float_t yp = hlistWaveform[0]->GetBinContent(bin);
      if (yp-TMath::Sqrt(yp) < fline->Eval(xp)) continue;
      par[0] = yp;
      par[1] = xp;
      par[2] = 3;
      npeaks++;
   }
   printf("Found %d useful peaks to fit\n",npeaks);
   printf("Now fitting: Be patient\n");
   TF1 *fit = new TF1("fit",fpeaks,-100,100,3);
   ////we may have more than the default 25 parameters
   TVirtualFitter::Fitter(h2,2);
   fit->SetParameters(par);
   fit->SetNpx(1000);
   h2->Fit("fit");  
   */
}
