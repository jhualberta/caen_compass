double asymGaus(double *x, double *par)
{
  //0: mu, 1:sigma, 2: shaping, 3: scale
  double arg = -par[2]*(x[0]-par[0])/(sqrt(2)*par[1]);// erfc(arg)
  double asym = par[3]*TMath::Gaus(x[0],par[0],par[1],true)*TMath::Erfc(arg);
  return asym;
}

double expo(double *x, double *par)
{
  //0:scale, 1:slope, 2:shift to y 
  double exp = par[1]*TMath::Exp(par[0]*x[0])+par[2];
  return exp;
}

double gausexp(double *x, double *par)
{
  double gaus = par[3]*TMath::Gaus(x[0],par[0],par[1],true)*TMath::Erfc(-par[2]*(x[0]-par[0])/(sqrt(2)*par[1]));
  double expo = par[5]*TMath::Exp(par[4]*x[0])+par[6];
  double gausexp = gaus+expo;
  return par[7]*gausexp+par[8];
}

double lightyield(double *x, double *par)
{
  double arg = par[9]*x[0]+par[10];
  double gaus = par[3]*TMath::Gaus(arg,par[0],par[1],true)*TMath::Erfc(-par[2]*(arg-par[0])/(sqrt(2)*par[1]));
  double expo = par[5]*TMath::Exp(par[4]*arg)+par[6];
  double gausexp = gaus+expo;
  return par[11]*(par[7]*gausexp+par[8])+par[12];
}

void findLightYield(double *total, double *total1, int fitstartbin, int cutoffbin)
{
  TF1 *f0 = new TF1("f0",gausexp,fitstartbin,cutoffbin,9);
  TF1 *f1 = new TF1("f1",gausexp,fitstartbin,cutoffbin,9);
  f0->SetParameters(total);
  f1->SetLineColor(kRed);
  f1->SetParameters(total1);
  // match with single pe maximum
  double scale = f0->GetMaximum();//Integral(fitstartbin,cutoffbin);//GetMaximum();
  TH1F *fithist = new TH1F("fithist","light yiled histogram",cutoffbin-fitstartbin, fitstartbin, cutoffbin);
  for(int i =0;i<cutoffbin-fitstartbin;i++)
  {
   fithist->SetBinContent(i+1,f1->Eval(fitstartbin+i));
  }
  fithist->Scale(scale/fithist->GetMaximum());
  //light yield fit
//  fitstartbin = 200, cutoffbin = 400;
  TF1 *flightyield = new TF1("fLY",lightyield,fitstartbin,cutoffbin,13);
  for(int i =0;i<9; i++) flightyield->SetParameter(i,total[i]);
  flightyield->SetParameter(9,1.0);
  flightyield->SetParameter(10,1);
  flightyield->SetParameter(11,1);
  flightyield->SetParameter(12,1);

  fithist->Fit(fLY,"R");
  flightyield->SetLineColor(kGreen+1);
  flightyield->SetParNames("lightyield","x shift","scale","y shift");
  TCanvas *c2 = new TCanvas("c2","",800,600);
  f0->Draw();fithist->Draw("same");f1->Draw("same");flightyield->Draw("same");
}

void fitDistilledLAB()
{
  TFile *f1 = new TFile("Nov22/rootfile/Munish_LABPPO_241Am_test2_22Nov_2018.root");
  TFile *f2 = new TFile("Nov22/rootfile/Distilled_LABPPO_241Am_test3_22Nov_2018.root");
  ROOT::Math::MinimizerOptions::SetDefaultMinimizer("GSLMultiFit");
  h1 = (TH1F*)f1->Get("h2");
  h2 = (TH1F*)f2->Get("h2");
  const int totalPar = 9;
  int cutoffbin = 700, startbin = 0;
  TH1F *hsample1 = new TH1F("hsample1","sample1",cutoffbin-startbin,startbin,cutoffbin);
  TH1F *hsample2 = new TH1F("hsample2","sample2",cutoffbin-startbin,startbin,cutoffbin);
  TH1F *hdiff = new TH1F("hdiff","diff",cutoffbin-startbin,startbin,cutoffbin);

  for(int i = 0;i<cutoffbin-startbin;i++)
  {
    hsample1->SetBinContent(i+1,h1->GetBinContent(i+1));
    hsample2->SetBinContent(i+1,h2->GetBinContent(i+1));
  }

  hsample2->SetLineColor(kYellow+2);

  double integral1 = hsample1->Integral();
  double integral2 = hsample2->Integral();
  // normalize the histogram
//  hsample1->Scale(1./integral1);
//  hsample2->Scale(1./integral2);
  // match the single pe peak
  double scale = hsample1->GetMaximum();
  hsample2->Scale(scale/hsample2->GetMaximum());
  
//-------------------------- Fit sample1----------------------------
  int fitstartbin = 120;// range for total fit function
  int range1 = 520;
  TF1 *fasymgaus = new TF1("fasymgaus",asymGaus,fitstartbin,range1,4);
  fasymgaus->SetParameters(hsample1->GetXaxis()->GetBinCenter(hsample1->GetMaximumBin()),100,1,scale);
  hsample1->Fit(fasymgaus,"QR");

  TF1 *fexpo = new TF1("fexpo",expo,range1,cutoffbin,3);
  fexpo->SetParameters(-1e-3,100, hsample1->GetBinContent(range1));
  hsample1->Fit(fexpo,"QR+");
  double par_total[totalPar];
//  TF1 *fgausexp = new TF1("fgausexp","fasymgaus(0)+fexpo(4)",startbin,cutoffbin,7);
  TF1 *fgausexp = new TF1("fgausexp",gausexp,fitstartbin,cutoffbin,totalPar);
  fasymgaus->GetParameters(&par_total[0]);
  fexpo->GetParameters(&par_total[4]);
  par_total[7] = 1, par_total[8] = 1;
  fgausexp->SetParameters(par_total);
  fgausexp->SetLineColor(kBlue);
  hsample1->Fit(fgausexp,"QR+");
  double chi2 = fgausexp->GetChisquare();
  double ndf = fgausexp->GetNDF();
  fgausexp->SetParNames("#mu","#sigma","s","gaus scale C1","#lambda","b","expo scale C2","total scale C3","total y shift b");
  cout<<"chi/ndf "<<chi2<<"/"<<ndf<<"="<<chi2*hsample2->GetMaximum()/scale/ndf<<endl;

//-------------------------- Fit sample2----------------------------

  range1 = 600;
  fitstartbin = 120;
  TF1 *fasymgaus1 = new TF1("fasymgaus1",asymGaus,fitstartbin,range1,4);
  fasymgaus1->SetParameters(hsample2->GetXaxis()->GetBinCenter(hsample2->GetMaximumBin()),100,1,hsample2->GetMaximum());
  hsample2->Fit(fasymgaus1,"QR");
  TF1 *fexpo1 = new TF1("fexpo1",expo, range1, cutoffbin,3);
  fexpo1->SetParameters(-1e-3,100,hsample2->GetBinContent(range1));
  hsample2->Fit(fexpo1,"QR+");
  double par_total1[totalPar];
  TF1 *fgausexp1 = new TF1("fgausexp1",gausexp,fitstartbin,cutoffbin,totalPar);
  fgausexp1->SetLineColor(kRed);
  fasymgaus1->GetParameters(&par_total1[0]);
  fexpo1->GetParameters(&par_total1[4]);
  par_total1[7] = 1, par_total1[8] = 1;
  fgausexp1->SetParameters(par_total1);
  hsample2->Fit(fgausexp1,"QR+");
  chi2 = fgausexp1->GetChisquare();
  ndf = fgausexp1->GetNDF();
  cout<<"chi/ndf "<<chi2*hsample2->GetMaximum()/scale<<"/"<<ndf<<"="<<chi2*hsample2->GetMaximum()/scale/ndf<<endl;
// h1->Scale(1./h1->Integral());
//  h2->Scale(1./h2->Integral());
//  h3->Scale(1./h3->Integral());
  fgausexp1->SetParNames("#mu","#sigma","s","gaus scale C1","#lambda","b","expo scale C2","total scale C3","total y shift b");
  TCanvas *c = new TCanvas("c","241Am",800,600);
  c->cd();
  hsample1->Draw();hsample2->Draw("same");
  //hdiff = hsample2->Clone();
  //hdiff->SetDirectory(0);
  //hdiff->Add(hsample1,-1);
  //hdiff->Draw("same");
  double parfitTotal[totalPar], parfitTotal1[totalPar];
  fgausexp->GetParameters( &parfitTotal[0] );
  fgausexp1->GetParameters( &parfitTotal1[0] );

  findLightYield(&parfitTotal[0], &parfitTotal1[0], fitstartbin, cutoffbin);

}
