//#include "TH2.h"
//#include <vector>
//#include "TCanvas.h"
//#include "TFile.h"
//#include "TCanvas.h"
//#include "TTree.h"
//#include <iostream>
//#include <vector> 
//#include "TMath.h"
//#include "TH1F.h"
//#include "TF1.h"
//#include "TRandom.h"
//
//using namespace::std;
//void timeMatch()
{
  //TFile *f1 = new TFile("dump_ch0_3DpieceOnly_2ChRead_1PMT_coin_2200V_0.root");
  //TFile *f2 = new TFile("dump_ch1_3DpieceOnly_2ChRead_1PMT_coin_2200V_0.root");

  TFile *f1 = new TFile("data_ch0_TeSOP_137Cs_2PMTcoin_2200V_thresh100_1min_csv_25Mar.root");
  TFile *f2 = new TFile("data_ch1_TeSOP_137Cs_2PMTcoin_2200V_thresh100_1min_csv_25Mar.root");
//  TFile *ft = new TFile("ftree.root","recreate");
  double coinwin = 10; 
  int bin = 1000; // 8192
  TH2F *e2d = new TH2F("e2d","",bin, 0, bin, bin,0,bin);

  TH2F *e2d_cor = new TH2F("e2d_cor","",bin, 0, bin, bin,0,bin);

  TTree *t1 = (TTree*)f1->Get("T");
  TTree *t2 = (TTree*)f2->Get("T");

  int evt = t1->GetEntries();
  UShort_t e1, e2;
  t1->SetBranchAddress("energy",&e1);
  t2->SetBranchAddress("energy",&e2);

  int adcCh = 8192;
  TH1F *heshort_0 = new TH1F("heshort_0","eshort, ch0",adcCh, 0, adcCh);
  TH1F *heshort_1 = new TH1F("heshort_1","eshort, ch1",adcCh, 0, adcCh);
  
  t1->Draw("energy>>heshort_0","","goff");
  t2->Draw("energy>>heshort_1","","goff");

  UShort_t flag1, flag2;
  t1->SetBranchAddress("flag",&flag1);
  t2->SetBranchAddress("flag",&flag2);
 
  Double_t tag1 = 0, tag2 = 0;

  t1->SetBranchAddress("timeStamp",&tag1);
  t2->SetBranchAddress("timeStamp",&tag2);
  int j = 0; 
  std::vector<double> checkTag1;
  std::vector<double> checkTag2;

  long int evtID = 0, evtCh0 =0, evtCh1=0;
  double tagCh0 = 0, tagCh1 = 0;
  UShort_t flagCh0, flagCh1;
  UShort_t eCh0, eCh1;
  //TTree *tt1= new TTree("newT","dump two channels");
  //tt1->Branch("evtID",evtID,"evtID/l");
  //tt1->Branch("evtCh0",evtCh0, "evtCh0/s");
  //tt1->Branch("evtCh1",evtCh1, "evtCh1/s");

  //tt1->Branch("tagCh0", tagCh0, "tagCh0/D");
  //tt1->Branch("tagCh1", tagCh1, "tagCh1/D");

  //tt1->Branch("flagCh0", flagCh0, "flagCh0/s");
  //tt1->Branch("flagCh1", flagCh1, "flagCh1/s");

  //tt1->Branch("eCh0", eCh0, "eCh0/s");
  //tt1->Branch("eCh1", eCh1, "eCh1/s");

  long int evtNum = T->GetEntries();
  for(long int i = 0;i<evtNum;i++)
  {
    t1->GetEntry(i);
    t2->GetEntry(j);
    e2d->Fill(e1,e2);
    j++;
  }

  j = 0;
  long int eventID = 0;
  for(long int i = 0;i<evtNum;i++)
  { 
    long int evt0 = i, evt1 = j;
    int check = 1;
    checkTag1.push_back(tag1);
    checkTag2.push_back(tag2);
    Double_t deltaTag1 = 0, deltaTag2 = 0;
    t1->GetEntry(i);
    t2->GetEntry(j);
    // time tag difference from previous 
//    if(i>0) deltaTag1 = checkTag1[i]-checkTag1[i-1];
//    if(j>0) deltaTag2 = checkTag2[j]-checkTag2[j-1];
    Double_t deltaT = tag2 - tag1;
    if(abs(deltaT)>coinwin) // mismatch
    {
      //cout<<"ch0 "<<i<<" ch1 "<<j<<" tag2 - tag1 = "<<deltaT<<" now process mismatch"<<endl;
      if(deltaT<0) // ch1 delay?
      { //cout<<"ch1 delay, ch0 "<<i<<" ch1 "<<j<<endl;
        if(j<evtNum) {
          j = j + 1;// go to next event
          t2->GetEntry(j);
          double deltaTcor = tag2 - tag1;
          //cout<<"after cor:  "<<tag1<<" "<<tag2<<" "<<deltaTcor<<endl;
          if(abs(deltaTcor)<10) 
          {
           cout<<"fill corrected ch1, event ch0 "<<i<<" ch1 "<<j<<endl;
           //if(e1>0 && e2>0) 
           {e2d_cor->Fill(e1, e2*1);}//26./22);}
           eCh0 = e1, eCh1 = e2;
           flagCh0 = flag1, flagCh1 = flag2;
           tagCh0 = tag1, tagCh1 = tag2;
          }
          //else {cout<<"abandon fill"<<endl;check = 0;}
        }
        //else {cout<<"reach the last data line!"<<endl;}
        //cout<<endl;
      }
      else if(deltaT>0)// ch0 delay?
      { //cout<<"ch0 delay, ch0 "<<i<<" ch1 "<<j<<endl;
        if(i<evtNum){
          i = i + 1;
          t1->GetEntry(i);
          double deltaTcor = tag2 - tag1;
          //cout<<"after cor:  "<<tag1<<" "<<tag2<<" "<<deltaTcor<<endl;
          if(abs(deltaTcor)<10)
          {
           //cout<<"fill corrected ch0, event ch0 "<<i<<" ch1 "<<j<<endl;
           //if(e1>0 && e2>0) 
           {e2d_cor->Fill(e1, e2*26./22);}//tt1->Fill();
           eCh0 = e1, eCh1 = e2;
           flagCh0 = flag1, flagCh1 = flag2;
           tagCh0 = tag1, tagCh1 = tag2;
          }
          //else {cout<<"abandon fill"<<endl;}
         }
        else {cout<<"reach the last data line!"<<endl;check = 0;}
        //cout<<endl;
      } // ch0 delay
     } // two ch mismatch

//    cout<<i<<" "<<deltaTag1<<" "<<deltaTag2<<" "<<tag2-tag1<<endl;
    else if(abs(deltaT)<coinwin) {
      //if(e1>0 && e2>0) 
      {
        e2d_cor->Fill(e1, e2*26./22);
        eCh0 = e1, eCh1 = e2;
        flagCh0 = flag1, flagCh1 = flag2;
        tagCh0 = tag1, tagCh1 = tag2;
      }
    }
    //tt1->Fill();
    //double deltaT = double(tag2-tag1);
    if(check !=0) evtID++;
    j++;
  }

 // ft->cd();tt1->Write();
//  ft->Close();

  e2d->GetXaxis()->SetTitle("ch0");
  e2d->GetYaxis()->SetTitle("ch1");

  e2d_cor->GetXaxis()->SetTitle("ch0");
  e2d_cor->GetYaxis()->SetTitle("ch1");
 
  TH1F *hch1_scaled = new TH1F("hch1_scaled","",adcCh, 0, adcCh);
  hch1_scale = heshort_1->Clone("hch1_scale");
  const int nbins = heshort_1->GetXaxis()->GetNbins();
  double new_bins[nbins+1];
//  for(int j=0; j< nbins; j++){
//    new_bins[j] = 26./22*heshort_1->GetBinLowEdge(j+1);
//    //cout<<new_bins[j]<<", ";
//  }

  hch1_scaled->SetBins(nbins, new_bins);
  hch1_scaled->SetLineColor(kBlue);

  TFile *ff = new TFile("plots_caen_coin.root","recreate");
  ff->cd();
  e2d->Write();
  TH1D *hch0proj = e2d->ProjectionX();
  TH1D *hch1proj = e2d->ProjectionY();
  hch0proj->Write();hch1proj->Write();
  ff->Close();

  TCanvas c0("c0","",800,600);
  c0.cd();
  heshort_1->SetLineColor(kRed);
  heshort_0->Draw();  
  heshort_1->Draw("same");
//  hch1_scaled->Draw("same");

  TCanvas c1("c1","",800,600);
  c1.cd();
  e2d->Draw("colz");

  TCanvas c2("c2","",800,600);
  c2.cd();
  e2d_cor->Draw("colz");

}
