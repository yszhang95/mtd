#include "TMath.h"
#include <iostream>
#include "TH1.h"
#include "TH2.h"
#include "TH2F.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TChain.h"
#include "THashList.h"
#include "TLatex.h"
#include "TFileCollection.h"
#include "TVector3.h"
#include "TNtuple.h"
#include "TFile.h"

#include "myAna.h"

int whichY(const float& y)
{
   for(int i=0; i<ana::nuOfY; i++){
      if(y>ana::ybin[i] && y<ana::ybin[i+1]) return i;
   }
   return -1;
}

void fillGen()
{
   TH1F::SetDefaultSumw2(true);

   TChain *t = new TChain("d0ana_mc/genCandidateNtuple");
   //TFileCollection* fc = new TFileCollection("dum", "", "d0signal.list");
   //TFileCollection* fc = new TFileCollection("dum", "", "newD0Signal.list");
   TFileCollection* fc = new TFileCollection("dum", "", "PromptD0.list");
   t->AddFileInfoList(fc->GetList()); 
   
   std::cout << "total entries: " << t->GetEntries() << std::endl;

   TH1D* hGenPtMidY = new TH1D("hGenPtMidY", "", 400, 0, 40);
   TH1D* hGenPt = new TH1D("hGenPt", "|y|<0.5", 400, 0, 40);

   float pT_gen;
   float y_gen;
   int status_gen;
   int dau1ID_gen;
   int dau2ID_gen;

   t->SetBranchAddress("pT_gen", &pT_gen);
   t->SetBranchAddress("y_gen", &y_gen);
   t->SetBranchAddress("status_gen", &status_gen);
   t->SetBranchAddress("DauID1_gen", &dau1ID_gen);
   t->SetBranchAddress("DauID2_gen", &dau2ID_gen);

   int n=0;
   int nStatus2=0;
   int nStatus2_kpi=0;
   for(Long64_t ientry=0; ientry<t->GetEntries(); ++ientry){
      t->GetEntry(ientry);

      if(std::fabs(y_gen)>1) continue;

      if(status_gen == 2) nStatus2++;
      if(status_gen==2 && ((std::fabs(dau1ID_gen) == 211 && std::fabs(dau2ID_gen) == 321) || (std::fabs(dau1ID_gen) == 321 && std::fabs(dau2ID_gen) == 211)))
         nStatus2_kpi++;

      if((std::fabs(dau1ID_gen) == 211 && std::fabs(dau2ID_gen) == 321) || (std::fabs(dau1ID_gen) == 321 && std::fabs(dau2ID_gen) == 211))
      {
         hGenPtMidY->Fill(pT_gen);
         if(std::fabs(y_gen) < 0.5) hGenPt->Fill(pT_gen);
      }
      if((std::fabs(dau1ID_gen) == 211 && std::fabs(dau2ID_gen) == 321) || (std::fabs(dau1ID_gen) == 321 && std::fabs(dau2ID_gen) == 211))
         n++;
   }
   std::cout << "pass" << n << std::endl;
   std::cout << "status2" << nStatus2 << std::endl;
   std::cout << "status2 pass" << nStatus2_kpi << std::endl;
   TFile* fout;
   fout = new TFile("genPt_reRECO.root", "recreate");
   hGenPtMidY->Write();
   hGenPt->Write();
}
