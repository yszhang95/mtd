#include "PromptD.h"
#include "myAna.h"
int whichY(const float& y)
{
   for(int i=0; i<ana::nuOfY; i++){
      if(y>ana::ybin[i] && y<ana::ybin[i+1]) return i;
      }
   return -1;
}

inline float invBetaPion(const float& p){
   return std::sqrt(1 + std::pow(ana::massPion/p,2));
}

inline float invBetaKaon(const float& p){
   return std::sqrt(1 + std::pow(ana::massKaon/p,2));
}


void setPalette()
{
   gStyle->SetPalette(55);
   const Int_t NRGBs = 5;
   const Int_t NCont = 255;
   Double_t stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
   Double_t red[NRGBs]   = { 0.00, 0.00, 0.87, 1.00, 0.51 };
   Double_t green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
   Double_t blue[NRGBs]  = { 0.51, 1.00, 0.12, 0.00, 0.00 };
   TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
   gStyle->SetNumberContours(NCont);
}

void invBetaPlotBTL()
{
   bool draw = true;
   bool draw2D = false;
   bool projection = false;
   bool profile= true;

   TCanvas *c[4];
   TFile* f1 = new TFile("matchPromptD0Tree_fullSample_reRECO.root");
   TChain* tp = new TChain("PromptD"); 
   tp->Add("matchPromptD0Tree_fullSample_reRECO.root");
   PromptD *t = new PromptD(tp);
   std::cout << t->GetEntries() << std::endl;

   TH2F* hdInvBetaPionVsPDau1D = new TH2F("hdInvBetaPionVsPDau1D", "hdInvBetaPionVsPDau1D" , 100, 0, 10, 1000, -0.1, 0.1);
   TH2F* hdInvBetaKaonVsPDau2D = new TH2F("hdInvBetaKaonVsPDau2D", "hdInvBetaKaonVsPDau2D", 100, 0, 10, 1000, -0.1, 0.1);
   TH2F* hdInvBetaKaonVsPDau1Dbar = new TH2F("hdInvBetaKaonVsPDau1Dbar", "hdInvBetaKaonVsPDau1Dbar", 100, 0, 10, 1000, -0.1, 0.1);
   TH2F* hdInvBetaPionVsPDau2Dbar = new TH2F("hdInvBetaPionVsPDau2Dbar", "hdInvBetaPionVsPDau2Dbar", 100, 0, 10, 1000, -0.1, 0.1);

   TH2F* hdInvBetaPionVsPBTL = new TH2F("hdInvBetaPionVsPBTL", "hdInvBetaPionVsPBTL", 50, 0, 10, 1000, -0.1, 0.1);
   TH2F* hdInvBetaKaonVsPBTL = new TH2F("hdInvBetaKaonVsPBTL", "hdInvBetaKaonVsPBTL", 50, 0, 10, 1000, -0.1, 0.1);
   TH2F* hdInvBetaPionVsPBTLDraw = new TH2F("hdInvBetaPionVsPBTLDraw", "hdInvBetaPionVsPBTL", 1000, 0, 10, 1000, -0.1, 0.1);
   TH2F* hdInvBetaKaonVsPBTLDraw = new TH2F("hdInvBetaKaonVsPBTLDraw", "hdInvBetaKaonVsPBTL", 1000, 0, 10, 1000, -0.1, 0.1);

   for(Long64_t ientry=0; ientry<t->GetEntries(); ientry++){
      t->GetEntry(ientry);
      if(t->y<-3 || t->y>3) continue;

      const float pD1 = t->pTD1 * std::cosh(t->EtaD1);
      const float pD2 = t->pTD2 * std::cosh(t->EtaD2);

      if(fabs(t->EtaD1)>1.5) continue;
      if(fabs(t->EtaD2)>1.5) continue;

      // require eta<1.4 ? pT > 0.8 : p > 0.7
      if(fabs(t->EtaD1) < 1.4 ? t->pTD1 <= 0.7 : pD1 <= 0.7) continue;
      if(fabs(t->EtaD2) < 1.4 ? t->pTD2 <= 0.7 : pD2 <= 0.7) continue;

      if(t->flavor == 1 && t->isMtdDau1) hdInvBetaPionVsPDau1D->Fill(pD1, 1./t->beta1_PV - invBetaPion(pD1));
      if(t->flavor == 1 && t->isMtdDau2) hdInvBetaKaonVsPDau2D->Fill(pD2, 1./t->beta2_PV - invBetaKaon(pD2));
      if(t->flavor == -1 && t->isMtdDau1) hdInvBetaKaonVsPDau1Dbar->Fill(pD1, 1./t->beta1_PV - invBetaKaon(pD1));
      if(t->flavor == -1 && t->isMtdDau2) hdInvBetaPionVsPDau2Dbar->Fill(pD2, 1./t->beta2_PV - invBetaPion(pD2));

      if(t->flavor == 1 && t->isMtdDau1) hdInvBetaPionVsPBTL->Fill(pD1, 1./t->beta1_PV - invBetaPion(pD1));
      if(t->flavor == -1 && t->isMtdDau2) hdInvBetaPionVsPBTL->Fill(pD2, 1./t->beta2_PV - invBetaPion(pD2));
      if(t->flavor == 1 && t->isMtdDau1) hdInvBetaPionVsPBTLDraw->Fill(pD1, 1./t->beta1_PV - invBetaPion(pD1));
      if(t->flavor == -1 && t->isMtdDau2) hdInvBetaPionVsPBTLDraw->Fill(pD2, 1./t->beta2_PV - invBetaPion(pD2));

      if(t->flavor == -1 && t->isMtdDau1) hdInvBetaKaonVsPBTL->Fill(pD1, 1./t->beta1_PV - invBetaKaon(pD1));
      if(t->flavor == 1 && t->isMtdDau2) hdInvBetaKaonVsPBTL->Fill(pD2, 1./t->beta2_PV - invBetaKaon(pD2));
      if(t->flavor == -1 && t->isMtdDau1) hdInvBetaKaonVsPBTLDraw->Fill(pD1, 1./t->beta1_PV - invBetaKaon(pD1));
      if(t->flavor == 1 && t->isMtdDau2) hdInvBetaKaonVsPBTLDraw->Fill(pD2, 1./t->beta2_PV - invBetaKaon(pD2));
   }
   if(draw){
      setPalette();
      for(int i=0; i<4; i++){
         c[i] = new TCanvas(Form("c%d", i),Form("c%d", i), 450, 500);
         c[i]->SetLogz();
         c[i]->SetLeftMargin(0.16);
         gStyle->SetOptStat(0);
      }
   }
   if(draw && draw2D){
	   c[0]->cd();
	   hdInvBetaPionVsPDau1D->GetXaxis()->SetTitle("p (GeV)");
	   hdInvBetaPionVsPDau1D->GetYaxis()->SetTitle("1/#beta - 1/#beta_{#pi}");
	   hdInvBetaPionVsPDau1D->SetMinimum(1.0);
	   hdInvBetaPionVsPDau1D->Draw("COLZ");
	   c[1]->cd();
	   hdInvBetaKaonVsPDau2D->GetXaxis()->SetTitle("p (GeV)");
	   hdInvBetaKaonVsPDau2D->GetYaxis()->SetTitle("1/#beta - 1/#beta_{K}");
	   hdInvBetaKaonVsPDau2D->SetMinimum(1.0);
	   hdInvBetaKaonVsPDau2D->Draw("COLZ");
	   c[2]->cd();
	   hdInvBetaKaonVsPDau1Dbar->GetXaxis()->SetTitle("p (GeV)");
	   hdInvBetaKaonVsPDau1Dbar->GetYaxis()->SetTitle("1/#beta - 1/#beta_{K}");
	   hdInvBetaKaonVsPDau1Dbar->SetMinimum(1.0);
	   hdInvBetaKaonVsPDau1Dbar->Draw("COLZ");
	   c[3]->cd();
	   hdInvBetaPionVsPDau2Dbar->GetXaxis()->SetTitle("p (GeV)");
	   hdInvBetaPionVsPDau2Dbar->GetYaxis()->SetTitle("1/#beta - 1/#beta_{#pi}");
	   hdInvBetaPionVsPDau2Dbar->SetMinimum(1.0);
	   hdInvBetaPionVsPDau2Dbar->Draw("COLZ");
	   //std::cout << hdInvBetaPionVsPDau1D->ProjectionY()->Integral()/hdInvBetaPionVsPDau1D->ProjectionY()->Integral(0, 100000) << std::endl;
	   //std::cout << hdInvBetaKaonVsPDau2D->ProjectionY()->Integral()/hdInvBetaKaonVsPDau2D->ProjectionY()->Integral(0, 100000) << std::endl;
	   //std::cout << hdInvBetaPionVsPDau2Dbar->ProjectionY()->Integral()/hdInvBetaPionVsPDau2Dbar->ProjectionY()->Integral(0, 100000) << std::endl;
	   //std::cout << hdInvBetaKaonVsPDau1Dbar->ProjectionY()->Integral()/hdInvBetaKaonVsPDau1Dbar->ProjectionY()->Integral(0, 100000) << std::endl;
   }

   if(draw && projection){
      c[0]->cd();
      TH1D* hdInvBetaDau1D =  hdInvBetaPionVsPDau1D->ProjectionY("dau1D0", 1, 30);
      hdInvBetaDau1D->Fit("gaus");
      c[1]->cd();
      TH1D* hdInvBetaDau2D =  hdInvBetaKaonVsPDau2D->ProjectionY("dau2D0", 1, 30);
      hdInvBetaDau2D->Fit("gaus");
      c[3]->cd();
      TH1D* hdInvBetaDau1Dbar =  hdInvBetaKaonVsPDau1Dbar->ProjectionY("dau1D0bar", 1, 30);
      hdInvBetaDau1Dbar->Fit("gaus");
      c[1]->cd();
      TH1D* hdInvBetaDau2Dbar =  hdInvBetaPionVsPDau2Dbar->ProjectionY("dau2D0bar", 1, 30);
      hdInvBetaDau2Dbar->Fit("gaus");
   }
   if(draw && profile){
      TLatex* ltx = new TLatex();
      c[0]->cd();
      TProfile* hdInvBetaPion = hdInvBetaPionVsPBTL->ProfileX("Pion", 1, -1, "i");
      hdInvBetaPion->GetYaxis()->SetRangeUser(-0.001, 0.001);
      hdInvBetaPion->GetYaxis()->SetTitle("Mean of 1/beta - 1/beta_{#pi}");
      hdInvBetaPion->GetXaxis()->SetTitle("p (GeV)");
      hdInvBetaPion->Draw();
      ltx->DrawLatexNDC(0.72, 0.65, "| #eta | < 1.5");

      c[1]->cd();
      TProfile* hdInvBetaPionstd = hdInvBetaPionVsPBTL->ProfileX("Pionstd", 1, -1, "s");
      hdInvBetaPionstd->GetYaxis()->SetRangeUser(-0.03, 0.03);
      hdInvBetaPionstd->GetYaxis()->SetTitle("Mean of 1/beta - 1/beta_{#pi}");
      hdInvBetaPionstd->GetXaxis()->SetTitle("p (GeV)");
      hdInvBetaPionstd->Draw();
      ltx->DrawLatexNDC(0.72, 0.65, "| #eta | < 1.5");

      c[2]->cd();
      TProfile* hdInvBetaKaon = hdInvBetaKaonVsPBTL->ProfileX("Kion", 1, -1, "i");
      hdInvBetaKaon->GetYaxis()->SetRangeUser(-0.001, 0.001);
      hdInvBetaKaon->GetYaxis()->SetTitle("Mean of 1/beta - 1/beta_{K}");
      hdInvBetaKaon->GetXaxis()->SetTitle("p (GeV)");
      hdInvBetaKaon->Draw();
      ltx->DrawLatexNDC(0.72, 0.65, "| #eta | < 1.5");

      c[3]->cd();
      TProfile* hdInvBetaKaonstd = hdInvBetaKaonVsPBTL->ProfileX("Kionstd", 1, -1, "s");
      hdInvBetaKaonstd->GetYaxis()->SetRangeUser(-0.03, 0.03);
      hdInvBetaKaonstd->GetYaxis()->SetTitle("Mean of 1/beta - 1/beta_{K}");
      hdInvBetaKaonstd->GetXaxis()->SetTitle("p (GeV)");
      hdInvBetaKaonstd->Draw();
      ltx->DrawLatexNDC(0.72, 0.65, "| #eta | < 1.5");

      TGraph* gPion = new TGraph(50);
      TGraph* gKaon = new TGraph(50);

      TLatex *latex = new TLatex();
      latex->SetTextSize(0.03);

      for(int i=0; i<50; i++){
         gPion->SetPoint(i, hdInvBetaPionstd->GetBinCenter(i+1), hdInvBetaPionstd->GetBinError(i+1));
         gKaon->SetPoint(i, hdInvBetaKaonstd->GetBinCenter(i+1), hdInvBetaKaonstd->GetBinError(i+1));
      }

      TF1* fExpPion = new TF1("fExpPion_dInvBetaRMS", "[0]+[1]*exp(-[2]*x)", 0.8, 10);
      TF1* fPoly2Pion = new TF1("fPoly2Pion_dInvBetaRMS", "[0]+[1]*x +[2]*x*x", 0.8, 10);

      fExpPion->SetParameters(0.003, 0.006, 0.11);
      fPoly2Pion->SetParameters(0.00977323, 0.000449226, -0.00017);
      fPoly2Pion->FixParameter(1, 0);
      fPoly2Pion->SetLineColor(kBlue);
      gPion->Fit(fExpPion, "Q R", "", 0.8, 10);
      gPion->Fit(fExpPion, "Q R", "", 0.8, 10);
      gPion->Fit(fExpPion, "Q R", "", 0.8, 10);
      gPion->Fit(fExpPion, "R", "", 0.8, 10);
      gPion->Fit(fPoly2Pion, "Q R", "", 0.8, 10);
      gPion->Fit(fPoly2Pion, "Q R", "", 0.8, 10);
      gPion->Fit(fPoly2Pion, "Q R", "", 0.8, 10);
      gPion->Fit(fPoly2Pion, "R", "", 0.8, 10);

      c[0]->cd();
      gStyle->SetOptStat(0);
      //gStyle->SetOptFit(1);
      gPion->SetMarkerColor(kBlack);
      gPion->SetMarkerSize(0.8);
      gPion->SetMarkerStyle(kOpenCircle);
      gPion->GetHistogram()->SetMaximum(0.045);
      gPion->GetHistogram()->SetMinimum(0.0);
      gPion->GetYaxis()->SetTitle("RMS");
      gPion->GetXaxis()->SetTitle("p (GeV)");
      gPion->SetTitle("RMS of diff_1/beta_Pion");
      gPion->Draw("pa");
      fExpPion->Draw("same");
      fPoly2Pion->Draw("same");
      TLegend *l1 = new TLegend(0.75, 0.8, 0.9, 0.9);
      l1->AddEntry(fExpPion, "exp", "l");
      l1->AddEntry(fPoly2Pion, "poly2", "l");
      l1->Draw();
      latex->DrawLatexNDC(0.4, 0.85,"p0 + p1 * exp(-p2*x)");
      latex->DrawLatexNDC(0.2, 0.69, Form("exp: p0=%.3e", fExpPion->GetParameter(0)));
      latex->DrawLatexNDC(0.2, 0.62, Form("exp: p1=%.3e", fExpPion->GetParameter(1)));
      latex->DrawLatexNDC(0.2, 0.55, Form("exp: p2=%.3e", fExpPion->GetParameter(2)));
      latex->DrawLatexNDC(0.4, 0.78,"p0 + p1*x + p2*x*x");
      latex->DrawLatexNDC(0.55, 0.69, Form("poly2: p0=%.3e", fPoly2Pion->GetParameter(0)));
      latex->DrawLatexNDC(0.55, 0.62, Form("poly2: p1=%.3e", fPoly2Pion->GetParameter(1)));
      latex->DrawLatexNDC(0.55, 0.55, Form("poly2: p2=%.3e", fPoly2Pion->GetParameter(2)));
      ltx->DrawLatexNDC(0.72, 0.65, "| #eta | < 1.5");

      TF1* fExpKaon = new TF1("fExpKaon_dInvBetaRMS", "[0]+[1]*exp(-[2]*x)", 0.8, 10);
      TF1* fPoly2Kaon = new TF1("fPoly2Kaon_dInvBetaRMS", "[0]+[1]*x + [2]*x*x", 0.8, 10);

      fExpKaon->SetParameters(0.005, 0.016, 0.36);
      fPoly2Kaon->SetParameters(0.014, 0.0, 0.0001);
      fPoly2Kaon->FixParameter(1, 0);
      fPoly2Kaon->SetLineColor(kBlue);
      gKaon->Fit(fExpKaon, "Q R", "", 0.8, 10);
      gKaon->Fit(fExpKaon, "Q R", "", 0.8, 10);
      gKaon->Fit(fExpKaon, "Q R", "", 0.8, 10);
      gKaon->Fit(fExpKaon, "R", "", 0.8, 10);
      gKaon->Fit(fPoly2Kaon, "Q R", "", 0.8, 10);
      gKaon->Fit(fPoly2Kaon, "Q R", "", 0.8, 10);
      gKaon->Fit(fPoly2Kaon, "Q R", "", 0.8, 10);
      gKaon->Fit(fPoly2Kaon, "R", "", 0.8, 10);
      c[2]->cd();
      gStyle->SetOptStat(0);
      //gStyle->SetOptFit(1);
      gKaon->SetMarkerColor(kBlack);
      gKaon->SetMarkerSize(0.8);
      gKaon->SetMarkerStyle(kOpenCircle);
      gKaon->GetHistogram()->SetMaximum(0.045);
      gKaon->GetHistogram()->SetMinimum(0.0);
      gKaon->GetYaxis()->SetTitle("RMS");
      gKaon->GetXaxis()->SetTitle("p (GeV)");
      gKaon->SetTitle("RMS of diff_1/beta_Kaon");
      gKaon->Draw("pa");
      fExpKaon->Draw("same");
      fPoly2Kaon->Draw("same");
      TLegend *l2 = new TLegend(0.75, 0.8, 0.9, 0.9);
      l2->AddEntry(fExpKaon, "exp", "l");
      l2->AddEntry(fPoly2Kaon, "poly2", "l");
      l2->Draw();
      latex->DrawLatexNDC(0.4, 0.85,"p0 + p1 * exp(-p2*x)");
      latex->DrawLatexNDC(0.2, 0.69, Form("exp: p0=%.3e", fExpKaon->GetParameter(0)));
      latex->DrawLatexNDC(0.2, 0.62, Form("exp: p1=%.3e", fExpKaon->GetParameter(1)));
      latex->DrawLatexNDC(0.2, 0.55, Form("exp: p2=%.3e", fExpKaon->GetParameter(2)));
      latex->DrawLatexNDC(0.4, 0.78,"p0 + p1*x + p2*x*x");
      latex->DrawLatexNDC(0.55, 0.69, Form("poly2: p0=%.3e", fPoly2Kaon->GetParameter(0)));
      latex->DrawLatexNDC(0.55, 0.62, Form("poly2: p1=%.3e", fPoly2Kaon->GetParameter(1)));
      latex->DrawLatexNDC(0.55, 0.55, Form("poly2: p2=%.3e", fPoly2Kaon->GetParameter(2)));
      ltx->DrawLatexNDC(0.72, 0.65, "| #eta | < 1.5");

      TFile fFunc("fFuncDInvBeta.root", "recreate");

      fExpPion->Write();
      fExpKaon->Write();
      fPoly2Pion->Write();
      fPoly2Kaon->Write();

      TCanvas* c1 = new TCanvas("cPion", "cPion", 575, 500);
      c1->SetLogz();
      c1->SetLeftMargin(0.16);
      c1->SetRightMargin(0.1);
      gStyle->SetOptStat(0);
	   hdInvBetaPionVsPBTLDraw->GetXaxis()->SetTitle("p (GeV)");
	   hdInvBetaPionVsPBTLDraw->GetYaxis()->SetTitle("1/#beta - 1/#beta_{#pi}");
	   hdInvBetaPionVsPBTLDraw->SetMinimum(1.0);
      hdInvBetaPionVsPBTLDraw->SetTitle("");
	   hdInvBetaPionVsPBTLDraw->Draw("COLZ");
      ltx->SetTextSize(0.05);
      ltx->DrawLatexNDC(0.3, 0.93, "Phase II Simulation #sqrt{s} = 5.5 TeV");
      ltx->DrawLatexNDC(0.75, 0.85, "CMS");
      ltx->SetTextSize(0.035);
      ltx->DrawLatexNDC(0.72, 0.8, "Preliminary");
      ltx->DrawLatexNDC(0.72, 0.74, "D -> K + #pi");
      ltx->DrawLatexNDC(0.72, 0.65, "| #eta | < 1.5");

      TCanvas* c2 = new TCanvas("cKaon", "cKaon", 575, 500);
      c2->SetLogz();
      c2->SetLeftMargin(0.16);
      c2->SetRightMargin(0.1);
      gStyle->SetOptStat(0);
	   hdInvBetaKaonVsPBTLDraw->GetXaxis()->SetTitle("p (GeV)");
	   hdInvBetaKaonVsPBTLDraw->GetYaxis()->SetTitle("1/#beta - 1/#beta_{K}");
	   hdInvBetaKaonVsPBTLDraw->SetMinimum(1.0);
      hdInvBetaKaonVsPBTLDraw->SetTitle("");
	   hdInvBetaKaonVsPBTLDraw->Draw("COLZ");
      ltx->SetTextSize(0.05);
      ltx->DrawLatexNDC(0.3, 0.93, "Phase II Simulation #sqrt{s} = 5.5 TeV");
      ltx->DrawLatexNDC(0.75, 0.85, "CMS");
      ltx->SetTextSize(0.035);
      ltx->DrawLatexNDC(0.72, 0.8, "Preliminary");
      ltx->DrawLatexNDC(0.72, 0.74, "D -> K + #pi");
      ltx->DrawLatexNDC(0.72, 0.65, "| #eta | < 1.5");

      std::cout << hdInvBetaPionVsPBTL->ProjectionY("pion", 0, 12)->Integral(420, 580)/ hdInvBetaPionVsPBTL->ProjectionY("pion", 0, 12)->Integral(0, 100000)<< std::endl;
      std::cout << hdInvBetaKaonVsPBTL->ProjectionY("kaon", 0, 12)->Integral(420, 580)/ hdInvBetaKaonVsPBTL->ProjectionY("kaon", 0, 12)->Integral(0, 100000)<< std::endl;

      TF1* fPionRes = new TF1("fPionRes", "[0]*(x-2)*(x-2) + [1]", 0.7, 2);
      TCanvas* cPionMean = new TCanvas("cPionMean", "", 575, 500);
      cPionMean->SetLeftMargin(0.16);
      hdInvBetaPion->GetYaxis()->SetRangeUser(-0.005, 0.005);
      hdInvBetaPion->SetTitle("");
      hdInvBetaPion->GetYaxis()->SetTitle("Mean of 1/beta - 1/beta_{#pi}");
      hdInvBetaPion->GetXaxis()->SetTitle("p (GeV)");
      hdInvBetaPion->Draw();
      hdInvBetaPion->Fit(fPionRes, "Q R", "", 0.8, 2 );
      hdInvBetaPion->Fit(fPionRes, "Q R", "", 0.8, 2 );
      hdInvBetaPion->Fit(fPionRes, "R", "", 0.8, 2 );
      ltx->SetTextSize(0.05);
      ltx->DrawLatexNDC(0.3, 0.93, "Phase II Simulation #sqrt{s} = 5.5 TeV");
      ltx->DrawLatexNDC(0.75, 0.85, "CMS");
      ltx->SetTextSize(0.035);
      ltx->DrawLatexNDC(0.72, 0.8, "Preliminary");
      ltx->DrawLatexNDC(0.72, 0.74, "D -> K + #pi");
      ltx->DrawLatexNDC(0.72, 0.65, "| #eta | < 1.5");
      ltx->DrawLatexNDC(0.4, 0.7, "p0 * (x-2)^2 + p1");
      ltx->DrawLatexNDC(0.4, 0.35, Form("p0: %.2e", fPionRes->GetParameter(0)));
      ltx->DrawLatexNDC(0.4, 0.31, Form("p1: %.2e", fPionRes->GetParameter(1)));

      TF1* fKaonRes = new TF1("fKaonRes", "[0]*(x-2)*(x-2) + [1]", 0.7, 3);
      TCanvas* cKaonMean = new TCanvas("cKaonMean", "", 575, 500);
      cKaonMean->SetLeftMargin(0.16);
      hdInvBetaKaon->GetYaxis()->SetRangeUser(-0.005, 0.005);
      hdInvBetaKaon->SetTitle("");
      hdInvBetaKaon->GetYaxis()->SetTitle("Mean of 1/beta - 1/beta_{K}");
      hdInvBetaKaon->GetXaxis()->SetTitle("p (GeV)");
      hdInvBetaKaon->Draw();
      hdInvBetaKaon->Fit(fKaonRes, "Q R", "", 0.8, 2 );
      hdInvBetaKaon->Fit(fKaonRes, "Q R", "", 0.8, 2 );
      hdInvBetaKaon->Fit(fKaonRes, "R", "", 0.8, 2 );
      ltx->SetTextSize(0.05);
      ltx->DrawLatexNDC(0.3, 0.93, "Phase II Simulation #sqrt{s} = 5.5 TeV");
      ltx->DrawLatexNDC(0.75, 0.85, "CMS");
      ltx->SetTextSize(0.035);
      ltx->DrawLatexNDC(0.72, 0.8, "Preliminary");
      ltx->DrawLatexNDC(0.72, 0.74, "D -> K + #pi");
      ltx->DrawLatexNDC(0.72, 0.65, "| #eta | < 1.5");
      ltx->DrawLatexNDC(0.4, 0.7, "p0 * (x-2)^2 + p1");
      ltx->DrawLatexNDC(0.4, 0.35, Form("p0: %.2e", fKaonRes->GetParameter(0)));
      ltx->DrawLatexNDC(0.4, 0.31, Form("p1: %.2e", fKaonRes->GetParameter(1)));
   }
}
