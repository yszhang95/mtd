#include "myAna.h"

void calSig30_50()
{
   TGaxis::SetMaxDigits(3);
   TFile* f1 = new TFile("genPt_reRECO.root");
   TFile* f2 = new TFile("HEPData-ins1616207-v1.root");
   TFile* f3 = new TFile("lamC3PMassHists_reRECO_all.root");
   //TFile* f4 = new TFile("hyjetsMassHists_reRECO_all.root");
   TFile* f4 = new TFile("hyjetsMassHists_reRECO_woPID_200.root");
   TFile* f5 = new TFile("hscale30_50.root");
   TFile* f6 = new TFile("hyjetsMassHists_reRECO_wPID_4950.root");

   std::ofstream fwMB("sig30_50.txt");
   std::ofstream fwoMB("sigWo30_50.txt");

   TH1D* hGenPtMidY;
   TH1F* hData;
   TH3D* hSignal;
   TH3D* hBkg;
   TH3D* hSignalMtd;
   TH3D* hBkgMtd;

   f1->GetObject("hGenPtMidY", hGenPtMidY);

   f2->GetObject("Table 5/Hist1D_y1", hData);
   f3->GetObject("hMassVsPtVsY", hSignal);
   f4->GetObject("hMassVsPtVsY30_50", hBkg);
   f3->GetObject("hMassVsPtVsYMtd", hSignalMtd);
   f6->GetObject("hMassVsPtVsY30_50Mtd", hBkgMtd);

   // try to scale gen spectrum
   //
   TH3D *hScale = (TH3D*) f5->Get("hScale");

   TH3D *hSignalScale = new TH3D();
   *hSignalScale =  (*hSignal) * (*hScale);

   TH3D *hSignalMtdScale = new TH3D();
   *hSignalMtdScale = (*hSignalMtd) * (*hScale);

   TH1D* hSig[ana::nuOfPt];
   TH1D* hSigMtd[ana::nuOfPt];

   TH1D* hS[ana::nuOfPt];
   TH1D* hSMtd[ana::nuOfPt];

   TH1D* hB[ana::nuOfPt];
   TH1D* hBMtd[ana::nuOfPt];

   for(int ipt=0; ipt<ana::nuOfPt; ipt++){
   //for(int ipt=0; ipt<2; ipt++){

      TLatex* ltx = new TLatex();

      hSig[ipt] = new TH1D(Form("hSig%d", ipt), "", ana::nuOfY, ana::ybin);
      hSigMtd[ipt] = new TH1D(Form("hSigMtd%d", ipt), "", ana::nuOfY, ana::ybin);

      hS[ipt] = new TH1D(Form("hS%d", ipt), "", ana::nuOfY, ana::ybin);
      hSMtd[ipt] = new TH1D(Form("hSMtd%d", ipt), "", ana::nuOfY, ana::ybin);

      hB[ipt] = new TH1D(Form("hB%d", ipt), "", ana::nuOfY, ana::ybin);
      hBMtd[ipt] = new TH1D(Form("hBMtd%d", ipt), "", ana::nuOfY, ana::ybin);

      double pTCutMin = ana::ptbin[ipt];
      double pTCutMax = ana::ptbin[ipt+1];

      for(int iy=0; iy<ana::nuOfY; iy++){

         double yCutMin = ana::ybin[iy];
         double yCutMax = ana::ybin[iy+1];

         double massCutMin = ana::mass_lw[iy];
         double massCutMax = ana::mass_up[iy];

         double sig, s, b, sErr, bErr;
         double sigMtd, sMtd, bMtd, sMtdErr, bMtdErr;

         calSig30_50(hSignalScale, hBkg, 
               yCutMin, yCutMax, pTCutMin, pTCutMax, massCutMin, massCutMax, 
               sig, s, b, sErr, bErr);
         calSig30_50_4751(hSignalMtdScale, hBkgMtd, 
               yCutMin, yCutMax, pTCutMin, pTCutMax, massCutMin, massCutMax, 
               sigMtd, sMtd, bMtd, sMtdErr, bMtdErr);

         hSig[ipt]->SetBinContent(iy+1, sig);
         hSigMtd[ipt]->SetBinContent(iy+1, sigMtd);

         hS[ipt]->SetBinContent(iy+1, s);
         hSMtd[ipt]->SetBinContent(iy+1, sMtd);
         hS[ipt]->SetBinError(iy+1, sErr);
         hSMtd[ipt]->SetBinError(iy+1, sMtdErr);

         hB[ipt]->SetBinContent(iy+1, b);
         hBMtd[ipt]->SetBinContent(iy+1, bMtd);
         hB[ipt]->SetBinError(iy+1, bErr);
         hBMtd[ipt]->SetBinError(iy+1, bMtdErr);

         fwMB << sigMtd << ", ";
         fwoMB << sig << ", " ;
         std::cout << sigMtd << std::endl;
      }

      fwMB << std::endl;
      fwoMB << std::endl;

      if(ipt<2) continue;

      TCanvas* c1 = new TCanvas(Form("c1_%d", ipt), "", 600, 500);
      gStyle->SetOptStat(0);
      hSigMtd[ipt]->GetYaxis()->SetTitle("Significance");
      hSigMtd[ipt]->GetXaxis()->SetTitle("y");
      hSigMtd[ipt]->GetYaxis()->CenterTitle();
      hSigMtd[ipt]->GetXaxis()->CenterTitle();
      hSigMtd[ipt]->GetYaxis()->SetTitleSize(0.04);
      hSigMtd[ipt]->GetXaxis()->SetTitleSize(0.04);
      double max = hSigMtd[ipt]->GetMaximum();
      hSigMtd[ipt]->GetYaxis()->SetRangeUser(0, max*1.3);
      hSigMtd[ipt]->SetLineColor(kRed);
      hSigMtd[ipt]->Draw();
      hSig[ipt]->Draw("same");
      TLegend* lgd = new TLegend(0.7, 0.8, 0.90, 0.90);
      lgd->AddEntry(hSigMtd[ipt], "MTD", "lp");
      lgd->AddEntry(hSig[ipt], "noMTD", "lp");
      lgd->Draw();
      ltx->SetTextSize(0.05);
      ltx->DrawLatexNDC(0.1, 0.93, "L^{PbPb}_{int} = 3 nb^{-1}  Phase II Simulation #sqrt{s} = 5.5 TeV");
      ltx->DrawLatexNDC(0.35, 0.75, "30-50 5B events");
      ltx->SetTextSize(0.035);
      ltx->DrawLatexNDC(0.6, 0.3, Form("%.1f #leq p_{T} < %.1f GeV", ana::ptbin[ipt], ana::ptbin[ipt+1]));
      c1->SaveAs(Form("LamCSigAlice30-50New/sig_pT%.1f_%.1f.png", ana::ptbin[ipt], ana::ptbin[ipt+1]));
      c1->SaveAs(Form("LamCSigAlice30-50New/sig_pT%.1f_%.1f.pdf", ana::ptbin[ipt], ana::ptbin[ipt+1]));

      TCanvas* c2 = new TCanvas(Form("c2_%d", ipt), "", 600, 500);
      c2->SetLeftMargin(0.15);
      c2->SetBottomMargin(0.15);
      gStyle->SetOptStat(0);
      hSMtd[ipt]->SetLineColor(kRed);
      hS[ipt]->GetYaxis()->SetTitle("S");
      hS[ipt]->GetXaxis()->SetTitle("y");
      hS[ipt]->GetYaxis()->CenterTitle();
      hS[ipt]->GetXaxis()->CenterTitle();
      hS[ipt]->GetYaxis()->SetTitleSize(0.04);
      hS[ipt]->GetXaxis()->SetTitleSize(0.04);
      double maxS = hS[ipt]->GetMaximum();
      hS[ipt]->GetYaxis()->SetRangeUser(0, maxS*1.3);
      hS[ipt]->Draw("e");
      hSMtd[ipt]->Draw("esame");
      TLegend* lgds = new TLegend(0.7, 0.8, 0.90, 0.90);
      lgds->AddEntry(hSMtd[ipt], "MTD", "lp");
      lgds->AddEntry(hS[ipt], "noMTD", "lp");
      lgds->Draw();
      ltx->SetTextSize(0.05);
      ltx->DrawLatexNDC(0.35, 0.7, "30-50 5B events");
      ltx->DrawLatexNDC(0.1, 0.95, "L^{PbPb}_{int} = 3 nb^{-1}  Phase II Simulation #sqrt{s} = 5.5 TeV");
      ltx->SetTextSize(0.035);
      ltx->DrawLatexNDC(0.6, 0.3, Form("%.1f #leq p_{T} < %.1f GeV", ana::ptbin[ipt], ana::ptbin[ipt+1]));
      c2->SaveAs(Form("LamCSigAlice30-50New/scaled_s_pT%.1f_%.1f.png", ana::ptbin[ipt], ana::ptbin[ipt+1]));
      c2->SaveAs(Form("LamCSigAlice30-50New/scaled_s_pT%.1f_%.1f.pdf", ana::ptbin[ipt], ana::ptbin[ipt+1]));

      TCanvas* c3 = new TCanvas(Form("c3_%d", ipt), "", 600, 500);
      c3->SetLeftMargin(0.15);
      c3->SetBottomMargin(0.15);
      gStyle->SetOptStat(0);
      hB[ipt]->GetYaxis()->SetTitle("B");
      hB[ipt]->GetXaxis()->SetTitle("y");
      hB[ipt]->GetYaxis()->SetTitleSize(0.04);
      hB[ipt]->GetXaxis()->SetTitleSize(0.04);
      hB[ipt]->GetYaxis()->CenterTitle();
      hB[ipt]->GetXaxis()->CenterTitle();
      hBMtd[ipt]->SetLineColor(kRed);
      double maxB = hB[ipt]->GetMaximum();
      hB[ipt]->GetYaxis()->SetRangeUser(0, maxB*1.3);
      hB[ipt]->Draw("e");
      hBMtd[ipt]->Draw("esame");
      TLegend* lgdb = new TLegend(0.7, 0.8, 0.90, 0.90);
      lgdb->AddEntry(hBMtd[ipt], "MTD", "lp");
      lgdb->AddEntry(hB[ipt], "noMTD", "lp");
      lgdb->Draw();
      ltx->SetTextSize(0.05);
      ltx->DrawLatexNDC(0.35, 0.7, "30-50 5B events");
      ltx->DrawLatexNDC(0.1, 0.95, "L^{PbPb}_{int} = 3 nb^{-1}  Phase II Simulation #sqrt{s} = 5.5 TeV");
      ltx->SetTextSize(0.035);
      ltx->DrawLatexNDC(0.6, 0.3, Form("%.1f #leq p_{T} < %.1f GeV", ana::ptbin[ipt], ana::ptbin[ipt+1]));
      c3->SaveAs(Form("LamCSigAlice30-50New/scaled_b_pT%.1f_%.1f.png", ana::ptbin[ipt], ana::ptbin[ipt+1]));
      c3->SaveAs(Form("LamCSigAlice30-50New/scaled_b_pT%.1f_%.1f.pdf", ana::ptbin[ipt], ana::ptbin[ipt+1]));
   }
}
