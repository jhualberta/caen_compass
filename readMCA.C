// Example to read and parse any xml file, supported by TXMLEngine class
// The input file, produced by xmlnewfile.C macro is used
// If you need full xml syntax support, use TXMLParser instead
//Author: Sergey Linev
//Jie Hu: change for reading CAEN MCA .n42 data   
#include "TXMLEngine.h"
#include <iostream>
#include "TFile.h"
#include "TH1F.h"
#include <string>
using namespace std;

void RetrieveXMLdata(TXMLEngine* xml, XMLNodePointer_t node, TH1F* h);
double livetime = 0, realtime = 0;
void readMCA(const char* filename)
{
   TH1F *h1 = new TH1F("h1","h1",16000,0,16000);
   TString sname(filename);
   TString format("root");
   TString savefile = sname(0,sname.Index(".")+1);
   savefile = savefile+format;
   TFile *f1 = new TFile(savefile,"recreate");
   // First create engine
   TXMLEngine* xml = new TXMLEngine;
   //time_t real_date_t;
    
   // Now try to parse xml file
   // Only file with restricted xml syntax are supported
   XMLDocPointer_t xmldoc = xml->ParseFile(filename);
   if (xmldoc==0) {
      delete xml;
      return;  
   }
   // take access to main node   
   XMLNodePointer_t mainnode = xml->DocGetRootElement(xmldoc);
   RetrieveXMLdata(xml,mainnode,h1);
   if(realtime != 0) {
     double liveratio = livetime/realtime;
     cout<<"deadtime "<<liveratio<<endl;
     cout<<"histogram scaled to livetime "<<endl;
     TH1F *h2 = new TH1F("h2","scaled to livetime",16000,0,16000);
     h2 = (TH1F*)h1->Clone("h2");
     h2->Scale(1./liveratio);
   }
   f1->cd();
   h2->Write();
   // Release memory before exit
   xml->FreeDoc(xmldoc);
   delete xml;
   f1->Close();   
}

double convertTime(const char* content)
{
  TString printTime(content); // P00Y00M00DT00H06M23.796S
  TString sday = printTime(printTime.Index('M')+1,2);
  TString shour = printTime(printTime.Index('T')+1,2);
  TString smin = printTime(printTime.Index('H')+1,2);
  TString ssecond = printTime(printTime.Last('M')+1,printTime.Index('S')-printTime.Last('M')-1);
  double day = sday.Atof();double hour = shour.Atof();double minute = smin.Atof();double second = ssecond.Atof();
  double time_duration = day*24*60*60+hour*60*60+minute*60+second;
  cout<<time_duration<<" seconds or "<<time_duration/60<<" minutes"<<endl;
  return time_duration;
}


void RetrieveXMLdata(TXMLEngine* xml, XMLNodePointer_t node, TH1F *h1) 
{   
    int channels=0,i=0,flag=0;
    std::string s=xml->GetNodeName(node);
    if(s=="RealTimeDuration")
    {
      cout<<"spectrum "<<s<<endl;
      const char* content = xml->GetNodeContent(node);
      realtime = convertTime(content);
    }

    if(s=="LiveTimeDuration")
    {
      cout<<"spectrum "<<s<<endl;
      const char* content = xml->GetNodeContent(node);
      livetime = convertTime(content);
    }

    if(s=="ChannelData")
    { 
     std::cout<<"processing "<<s<<std::endl;
     const char* content = xml->GetNodeContent(node);
     if(content!=0) {
      int len=strlen(content);
      int num=0;
      for (;i<len;i++) 
      {if(content[i]==' ' || content[i]=='\n') num++;}
      const int number=num;
      unsigned int mydata[number];
      mydata[0]=atof(content);
      int j=0; 
      int k=1;
      int interval=0;
      for (;j<len;j++) {
        if(content[j]==' ' || content[j]=='\n') { 
          interval=j;
          mydata[k]=atof(content+interval);
          h1->SetBinContent(k,mydata[k]);
          k++;
        }
     }
    } //if content
    h1->Write();
    flag=1; 
    }
    //char title[400];
    //sprintf(title,"CAEN-MCA Data, %7.2lf s live time",GetLiveTime());
    //channels=16384;
    //char hn[400];
    //SetData(new TH1I(hn,title, GetChannels(),0,GetChannels()+1));
    //int entries = 0;
    //for (i = 0 ; i<channels ; i++){GetData()->SetBinContent(1+i,mydata[i]);entries += mydata[i];} 
    //GetData()->SetEntries(entries);} 
    //flag=1;
    //}
    else if(s=="StartDateTime") { 
      start_time= xml->GetNodeContent(node); 
      char stemp[17];
      for(i=0;i<17;i++) {
        if(start_time[i]!='T')
        stemp[i]=start_time[i];
        else stemp[i]=' ';
      }
    }
   XMLNodePointer_t child = xml->GetChild(node); 
   while (child!=0 && flag!=1)  
   {
     RetrieveXMLdata(xml, child, h1); 
     child = xml->GetNext(child); 
   }  
}
