{
float trigger_time[10000];
int flag;
float trig_counts;
int evtNum = 10000;
int recordLength = 252;

//float attenuation = 0.25;
//float attenuation = 0.5;
float attenuation = 0.75;
//float attenuation = 1;

float delay = 1;
float threshold[10000];
float baseline[10000];
float baseline2[10000];
float energy[10000];

TFile *f0 = new TFile("../data/feb22/dump_ch0_Data_TeSOP_241Am_coin248ns_2200V_3_0.root");

    vector<TH1F*> hlistProcessWf_ch0;
    for(int i = 0;i<evtNum;i++)
    {
     TH1F *hprocess = new TH1F("hprocess","", recordLength, 0, recordLength);
     hlistProcessWf_ch0.push_back(hprocess);
     hlistProcessWf_ch0[i]->SetName(Form("hpwf%u",i));
    }

    
//trigger

vector<TH1F*> hlistSsignal_ch0;


TH1F *trig = new TH1F("triger_time","", 100, 0, 100);
TH1F *hEnergy = new TH1F("energy spectrum","", 1000, 0, 12000);


for(int i = 0; i<evtNum; i++){	
	flag = 0;
		//calculate baseline[i]	
		baseline[i] = ((TH1F*)f0->Get(Form("hwf%u",i)))->Integral(0,32)/32;
		baseline2[i] = baseline[i]*(attenuation-1);
		//cout<<"baseline2[i] = "<<baseline2[i]<<endl;
	for(int q = 0;q<recordLength;q++){
		//invert and delay and fill histogram
        hlistProcessWf_ch0[i]->SetBinContent(q,(((TH1F*)f0->Get(Form("hwf%u",i)))->GetBinContent(q+1+delay))*-1  +  attenuation * (((TH1F*)f0->Get(Form("hwf%u",i)))->GetBinContent(q+1))); 
		hlistProcessWf_ch0[i]->SetBinContent(252,baseline2[i]);  
	}
	//add a cut to find threshold
	hlistProcessWf_ch0[i]->GetXaxis()->SetRange(0,100);
	threshold[i] = (hlistProcessWf_ch0[i]->GetMaximum()- hlistProcessWf_ch0[i]->GetMinimum())/3;
	//cout<<"threshold[i] = "<<threshold[i]<<endl;

	for(int q = 0;q<recordLength;q++){
		//find trigger time
 		trig_counts = hlistProcessWf_ch0[i]->GetBinContent(q);
		if((trig_counts < (baseline2[i] - threshold[i])) and flag == 0){flag = 1;}
		if((flag == 1) and (trig_counts > baseline2[i])){trigger_time[i] = q-1;break;}  
	}

	if((trigger_time[i] == 0) or trigger_time[i] >50.){
		//cout<<"i "<<i<<" trigger_time "<<trigger_time[i]<<" baseline2[i] "<<baseline2[i]<<" threshold[i] "<<threshold[i]<<endl;
	}
	//cout<<"i = "<<i<<" trigger_time "<<trigger_time[i]<<endl;
	trig->Fill(trigger_time[i]);
	energy[i] = ((TH1F*)f0->Get(Form("hwf%u",i)))->Integral(trigger_time[i]-8,trigger_time[i]+7) - baseline[i]*15;
	if(energy[i]<0){energy[i] = 0;}
	hEnergy->Fill(energy[i]);

}


TFile *fprocess = new TFile("processedWf_a=0.75.root","recreate");
TH1F *hBaseline1 = new TH1F("baseline1","", 100, -1000, 1000);
TH1F *hBaseline2 = new TH1F("baseline2","", 100, -1000, 1000);
TH1F *hThreshold = new TH1F("threshold","", 100, -1000, -1000);
TH1F *hTrigger_time = new TH1F("trigger_time","", 100, 0, 100);

fprocess->cd();
for(int i = 0;i<hlistProcessWf_ch0.size();i++)
{
  hlistProcessWf_ch0[i]->Write();
  hTrigger_time->Fill(trigger_time[i]);
  hBaseline1->Fill(baseline[i]);
  hBaseline2->Fill(baseline2[i]);
  hThreshold->Fill(threshold[i]);

}


hTrigger_time->Write();
hBaseline1->Write();
hBaseline2->Write();
hThreshold->Write();
hEnergy->Write();
hEnergy->Draw();
fprocess->Close();
//+8ns, short gate 15ns
}
