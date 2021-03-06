#include <vector>
#include <list>
#include <map>
#include <set>
#include <tuple>

#include <memory>

#include <algorithm>
#include <climits>
#include <numeric>

#include "Math/GenVector/LorentzVector.h"
#include "Math/GenVector/PtEtaPhiM4D.h"
#include "Math/GenVector/VectorUtil.h"
#include "TMath.h"
#include "TChain.h"
#include "TFileCollection.h"
#include "THashList.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TH3D.h"
#include "TString.h"
#include "TSystem.h"

#include "TreeReader/ParticleTreeMC2.hxx"

#include "myAna.h"

using std::cerr;
using std::cout;
using std::endl;

using std::vector;
using std::string;
using std::list;
using std::map;
using std::tuple;

using std::shared_ptr;

using PtEtaPhiM_t = ROOT::Math::PtEtaPhiM4D<double>;
using Hist1DMaps = std::map<std::string, std::unique_ptr<TH1D>>;
using Hist2DMaps = std::map<std::string, std::unique_ptr<TH2D>>;
using Hist3DMaps = std::map<std::string, std::unique_ptr<TH3D>>;
 
using MatchPairInfo =  tuple<size_t, size_t, double, double>;

class Particle {
  public:
    using ParticlePtr = shared_ptr<Particle>;
    Particle(int id): _id(id), _flip(0) ,_treeIdx(USHRT_MAX) {};
    Particle(const Particle& p): _id(p._id), _flip(0), _treeIdx(USHRT_MAX) {
      for (const auto& d : p._daus) {
        _daus.push_back(std::make_shared<Particle>(*d));
      }
    };
    //~Particle() { cout << "Deleted" << endl; }

    bool isStable() const { return _daus.size(); }
    int  id()       const { return _id;          }
    bool isNull() const   { return _id == 0;     }
    vector<ParticlePtr> daughters() const { return _daus; }
    void addDaughter(Particle& dau) { _daus.push_back(std::make_shared<Particle>(dau)); }
    ParticlePtr daughter(size_t i);
    ParticlePtr daughter(size_t i) const;
    void setTreeIdx(unsigned short treeIdx) { _treeIdx = treeIdx; }
    int flip() { return _flip; }
    unsigned short treeIdx() const { return _treeIdx; }
    void flipFlavor();
  protected:
    int _id;
    int _flip;
    vector<ParticlePtr> _daus;
    unsigned short _treeIdx;
};


Particle::ParticlePtr Particle::daughter(size_t i)
{ 
  if (_daus.size()) {
    return _daus[i];
  }
  else {
    cerr << "No daughter found with pdgId " << _id << endl;
    return nullptr;
  }
}

Particle::ParticlePtr Particle::daughter(size_t i) const
{ 
  if (_daus.size()) {
    return _daus.at(i);
  }
  else {
    cerr << "No daughter found with pdgId " << _id << endl;
    return nullptr;
  }
}

void Particle::flipFlavor()
{
  _id = -_id;
  for(auto& d : _daus) d->flipFlavor();
  _flip++;
}

class MatchCriterion {
  public:
    template <typename T>
      bool match (const T& reco, const T& gen);

    MatchCriterion (const float dR, const float dRelPt) :
      _deltaR(dR), _deltaRelPt(dRelPt) { };
    ~MatchCriterion() {}
    void SetDeltaRelPt(const float dRelPt) { _deltaRelPt = dRelPt; }
    void SetDeltaR(const float dR) { _deltaR = dR; }

  private:
    float _deltaR;
    float _deltaRelPt;
};
  template <typename T>
bool MatchCriterion::match (const T& reco, const T& gen)
{
  //cout << "reco:gen" << endl;
  //cout << reco.Pt() << ":" << gen.Pt() << endl;
  //cout << reco.Eta() << ":" << gen.Eta() << endl;
  //cout << reco.Phi() << ":" << gen.Phi() << endl;
  const auto dR = ROOT::Math::VectorUtil::DeltaR(reco, gen);
  const auto dRelPt = TMath::Abs(gen.Pt() - reco.Pt())/gen.Pt();
  //cout << "dR: " << dR << endl;
  //cout << "dpT: " << dRelPt << endl;
  return dR < _deltaR && dRelPt < _deltaRelPt;
}

bool checkDecayChain(Particle& par, unsigned short genIdx, const ParticleTreeMC& p, bool isFirstGen=true)
{
  // check pdg Id
  if (par.id() * p.gen_pdgId().at(genIdx) < 0 && isFirstGen) par.flipFlavor();
  if (par.id() != p.gen_pdgId().at(genIdx)) return false;

  // get daughters out
  const auto dauIdxs = p.gen_dauIdx().at(genIdx);

  // check number of daughters
  if (par.daughters().size() != dauIdxs.size()) return false;

  // if no daughters, both of these two have the same decay chain
  if (dauIdxs.size() == 0) {
      par.setTreeIdx(genIdx);
      return true;
  }

  // fill pdgIds and check them
  std::multiset<int> genDauPdgId; // check dau ID
  std::multiset<int> desiredPdgId; // check dau ID
  std::vector<int> vec_desiredPdgId; // for later permutation
  for (const auto idx : dauIdxs) {
    genDauPdgId.insert(p.gen_pdgId().at(idx));
  }
  for (const auto dau : par.daughters()) {
    desiredPdgId.insert(dau->id());
    vec_desiredPdgId.push_back(dau->id());
  }
  bool sameIds = desiredPdgId == genDauPdgId;
  if (!sameIds) return false; // they do not have the same set of decay products
  
  // if they have same daughter Ids
  // check their id order
  std::vector<unsigned int> idxs(vec_desiredPdgId.size());
  std::iota(std::begin(idxs), std::end(idxs), 0);
  do {
    bool sameOrder = true;
    for (size_t i=0;  i<idxs.size(); i++) {
      auto dIdx = dauIdxs[idxs[i]];
      sameOrder = sameOrder && (p.gen_pdgId().at(dIdx) == vec_desiredPdgId[i]);
    }
    if(sameOrder) {
      break;
    }
  } while(std::next_permutation(idxs.begin(), idxs.end()));

  // associated them if they are compatible
  bool sameChain = true;
  for (size_t i=0;  i<idxs.size(); i++) {
    sameChain = sameChain && checkDecayChain(*par.daughter(i), dauIdxs[idxs[i]], p, false);
    if (sameChain) par.daughter(i)->setTreeIdx( dauIdxs[idxs[i]] );
  }

  return sameChain;
}

PtEtaPhiM_t getRecoP4(size_t idx, const ParticleTreeMC& p)
{
  return PtEtaPhiM_t (
            p.cand_pT()[idx],
            p.cand_eta()[idx],
            p.cand_phi()[idx],
            p.cand_mass()[idx]
            );
}
PtEtaPhiM_t getRecoDauP4(size_t idx, size_t idau, const ParticleTreeMC& p)
{
  return PtEtaPhiM_t (
            p.cand_pTDau()[idx].at(idau),
            p.cand_etaDau()[idx].at(idau),
            p.cand_phiDau()[idx].at(idau),
            p.cand_massDau()[idx].at(idau)
            );
}
PtEtaPhiM_t getGenP4(size_t idx, const ParticleTreeMC& p)
{
  return PtEtaPhiM_t (
            p.gen_pT()[idx],
            p.gen_eta()[idx],
            p.gen_phi()[idx],
            p.gen_mass()[idx]
            );
}

int genMatchBMass(const TString& inputList, const TString& treeDir,
    Particle Bmeson,
    Long64_t nentries=-1, TString type="")
{
  type.ToLower();
  bool sortByPt(false), sortByR(false);
  if (type == "dpt") sortByPt = true;
  else if (type == "dr") sortByR = true;
  else if (type != "") { cout << "Wrong input variable for sort type" << endl; return -1; }
  TString basename(gSystem->BaseName(inputList));
  const auto firstPos = basename.Index(".list");
  basename.Replace(firstPos, 5, "_");
  basename += nentries > 0 ? Form("%s%lld_", treeDir.Data(), nentries) : (treeDir + "_AllEntries_");

  basename += sortByPt ? "sortBydPt_" : "";
  basename += sortByR  ? "sortBydR_"  : "";
  basename += "B_DPi.root";
  TFile ofile("output/"+basename, "recreate");
  cout << "Created " << ofile.GetName() << endl;

  TFileCollection tf("tf", "", inputList);
  TChain t(treeDir+"/ParticleTree");
  t.AddFileInfoList(tf.GetList());
  ParticleTreeMC2 p(&t);

  if(nentries < 0) nentries = p.GetEntries();
  cout << "Tree " << treeDir << "/ParticleTree in " << inputList
    << " has " << nentries << " entries." << endl;;

  MatchCriterion matchCriterion(0.03, 0.5);

  Hist1DMaps hBMassMatch;
  hBMassMatch["dR0.03"]= std::move(std::unique_ptr<TH1D>(
        new TH1D("hBMassMatch0p03", "B^{#pm}, matching FS momenta, dR<0.03, dPt<0.5;Mass (GeV);Events", 200, 5.0, 5.6)));

  Long64_t nMultipleMatch = 0;
  for (Long64_t ientry=0; ientry<nentries; ientry++) {
    if (ientry % 50000 == 0) cout << "pass " << ientry << endl;
    //cout << "Events " << ientry << endl;
    p.GetEntry(ientry);
    auto gensize = p.gen_mass().size();
    auto recosize = p.cand_mass().size();

    auto pdgId = p.cand_pdgId();
    auto gen_pdgId = p.gen_pdgId();

    auto recoCharge = p.cand_charge();
    int nmatch = 0;
    for (size_t ireco=0; ireco<recosize; ireco++) {
      // begin B
      if (pdgId[ireco] == 521) {
        //cout << "record matchGEN " << p.cand_matchGEN().at(ireco) << endl;
        //cout << "record cand_genIdx " << p.cand_genIdx().at(ireco) << endl;
        //cout << "record cand_idx " << ireco << endl;
        bool matchGEN = false;
        PtEtaPhiM_t recoB (
            p.cand_pT()[ireco],
            p.cand_eta()[ireco],
            p.cand_phi()[ireco],
            p.cand_mass()[ireco]
            );
        auto dauIdx = p.cand_dauIdx().at(ireco);
        // 0 for pi, 1 for D0
        auto gDauIdx = p.cand_dauIdx().at(dauIdx.at(1));

        int Dflavor = -1;
        if (p.cand_massDau()[dauIdx.at(1)].at(0) <
            p.cand_massDau()[dauIdx.at(1)].at(1) ) Dflavor = 1;
        if ( Dflavor *
             recoCharge.at(dauIdx.at(0)) > 0) continue;
        std::vector<PtEtaPhiM_t> p4FS;
        p4FS.push_back(getRecoDauP4(ireco, 0, p)); // pi from B
        if (Dflavor<0) {
          p4FS.push_back(getRecoDauP4(dauIdx.at(1), 1, p)); // pi from D
          p4FS.push_back(getRecoDauP4(dauIdx.at(1), 0, p)); // k from D
        } else {
          p4FS.push_back(getRecoDauP4(dauIdx.at(1), 0, p)); // pi from D
          p4FS.push_back(getRecoDauP4(dauIdx.at(1), 1, p)); // k from D
        }

        for (size_t igen=0; igen<gensize; igen++) {
          if (abs(gen_pdgId[igen]) != 521) continue;
          auto gen_dauIdx = p.gen_dauIdx().at(igen);
          /*
          cout << "gen B idx: " << igen << endl;
          cout << "gen B chain" << endl;
          for (auto& e : gen_dauIdx) {
            cout << p.gen_pdgId().at(e) << endl;
            if(abs(p.gen_pdgId().at(e)) == 421 ) {
              for (const auto & ee : p.gen_dauIdx().at(e))
                cout << p.gen_pdgId().at(ee) << endl;
            }
          }
          */
          PtEtaPhiM_t genB (
              p.gen_pT()[igen],
              p.gen_eta()[igen],
              p.gen_phi()[igen],
              p.gen_mass()[igen]
              );
          Particle Bmeson_copy(Bmeson);
          bool sameChain = checkDecayChain(Bmeson_copy, igen, p);
          if (!sameChain) continue;
          std::vector<PtEtaPhiM_t> p4GenFS;
          p4GenFS.push_back(getGenP4(Bmeson_copy.daughter(0)->treeIdx(), p)); // pi from B
          p4GenFS.push_back(getGenP4(Bmeson_copy.daughter(1)->daughter(0)->treeIdx(), p)); // pi from D
          p4GenFS.push_back(getGenP4(Bmeson_copy.daughter(1)->daughter(1)->treeIdx(), p)); // k from D
          if (!matchGEN) {
            bool matchFS = true;
            for (size_t i=0; i<p4GenFS.size(); i++) {
              bool match = matchCriterion.match(p4FS[i], p4GenFS[i]);
              matchFS = matchFS && match;
            }
            matchGEN = matchFS;
          }
          if(matchGEN) break;
        }
        if (matchGEN) hBMassMatch["dR0.03"]->Fill(p.cand_mass().at(ireco));
        if (matchGEN) nmatch++;
      }
    }
    if (nmatch > 1) { ++nMultipleMatch; cout << "Find one multiple match event" << endl; }
  }
  cout << nMultipleMatch << " events has more than one matched RECO B meson" << endl;
  ofile.cd();
  for (const auto& e : hBMassMatch) e.second->Write();

  return 0;
}

int genMatchFSBMass(const TString& inputList, const TString& treeDir,
    Particle Bmeson, Long64_t nentries=-1, const bool doRecoGenMatch=true,
    const bool doTrkQA=true, TString type="")
{
  type.ToLower();
  bool sortByPt(false), sortByR(false);
  if (type == "dpt") sortByPt = true;
  else if (type == "dr") sortByR = true;
  else if (type != "") { cout << "Wrong input variable for sort type" << endl; return -1; }
  TString basename(gSystem->BaseName(inputList));
  const auto firstPos = basename.Index(".list");
  basename.Replace(firstPos, 5, "_");
  basename += nentries > 0 ? Form("%s%lld_", treeDir.Data(), nentries) : (treeDir + "_AllEntries_");

  basename += sortByPt ? "sortBydPt_" : "";
  basename += sortByR  ? "sortBydR_"  : "";
  basename += "B_DPi_BottomToTop.root";
  TFile ofile("output/"+basename, "recreate");
  cout << "Created " << ofile.GetName() << endl;

  TFileCollection tf("tf", "", inputList);
  TChain t(treeDir+"/ParticleTree");
  t.AddFileInfoList(tf.GetList());
  ParticleTreeMC2 p(&t);

  if(nentries < 0) nentries = p.GetEntries();
  cout << "Tree " << treeDir << "/ParticleTree in " << inputList
    << " has " << nentries << " entries." << endl;;

  MatchCriterion  matchCriterion(0.05, 0.5);

  Hist1DMaps hBMassMatch;
  hBMassMatch["dR0.03"]= std::move(std::unique_ptr<TH1D>(
        new TH1D("hBMassMatch0p03", "B^{#pm}, matching FS momenta, dR<0.03, dPt<0.5;Mass (GeV);Events", 200, 5.0, 5.6)));

  Hist2DMaps hTrkBetaInvVsP;
  hTrkBetaInvVsP["PionFromB"] = std::move(std::unique_ptr<TH2D>(
        new TH2D("hTrkBetaInvVsPVsPPionFromB", "Pion from B^{#pm};p (GeV);1/#beta", 200, 0., 8., 200, 0.9, 1.9)));
  hTrkBetaInvVsP["PionFromD"] = std::move(std::unique_ptr<TH2D>(
        new TH2D("hTrkBetaInvVsPVsPPionFromD", "Pion from neutral D;p (GeV);1/#beta", 200, 0., 8., 200, 0.9, 1.9)));
  hTrkBetaInvVsP["KaonFromD"] = std::move(std::unique_ptr<TH2D>(
        new TH2D("hTrkBetaInvVsPVsPKaonFromD", "Kaon from neutral D;p (GeV);1/#beta", 200, 0., 8., 200, 0.9, 1.9)));
  hTrkBetaInvVsP["AllTracks"] = std::move(std::unique_ptr<TH2D>(
        new TH2D("hTrkBetaInvVsPVsP", "Tracks;p (GeV);1/#beta", 200, 0., 8., 200, 0.9, 1.9)));

  Long64_t nMultipleMatch = 0;
  for (Long64_t ientry=0; ientry<nentries; ientry++) {
    if (ientry % 5000 == 0) cout << "pass " << ientry << endl;
    p.GetEntry(ientry);
    // track QA, doTrkQA should be true
    size_t nTrk = p.trk_candIdx().size();
    if (!doTrkQA) nTrk = 0;
    for (size_t iTrk=0; iTrk<nTrk; iTrk++) {
      if (p.trk_tMTDErr().at(iTrk) < 0) continue;
      double dT = p.trk_tMTD().at(iTrk) - p.bestvtxT();
      double betaInv = dT * ana::c_cm_ns/ p.trk_pathLength().at(iTrk); // 
      if (abs(betaInv - 1./p.trk_beta().at(iTrk)) > 1e-2) cerr << "Wrong beta value" << endl;
      double pT = p.cand_pT().at(p.trk_candIdx().at(iTrk).at(0));
      double eta = p.cand_eta().at(p.trk_candIdx().at(iTrk).at(0));
      hTrkBetaInvVsP["AllTracks"]->Fill(pT * std::cosh(eta), betaInv);
    }
    // reco-gen matching, if you do not want to do, set doRecoGenMatch to false, and it will make p4GenFS and p4RecoFS empty
    auto gensize = p.gen_mass().size();
    auto recosize = p.cand_mass().size();

    auto pdgId = p.cand_pdgId();
    auto gen_pdgId = p.gen_pdgId();

    auto recoCharge = p.cand_charge();

    map<size_t, PtEtaPhiM_t> p4GenFS;
    for (size_t igen=0; igen<gensize; igen++) {
      if (!doRecoGenMatch) break;
      if (abs(gen_pdgId[igen]) != 521) continue;
      auto gen_dauIdx = p.gen_dauIdx().at(igen);
      Particle Bmeson_copy(Bmeson);
      bool sameChain = checkDecayChain(Bmeson_copy, igen, p);
      if (!sameChain) continue;
      p4GenFS[Bmeson_copy.daughter(0)->treeIdx()] = getGenP4(Bmeson_copy.daughter(0)->treeIdx(), p); // pi from B
      p4GenFS[Bmeson_copy.daughter(1)->daughter(0)->treeIdx()] = getGenP4(Bmeson_copy.daughter(1)->daughter(0)->treeIdx(), p); // pi from D
      p4GenFS[Bmeson_copy.daughter(1)->daughter(1)->treeIdx()] = getGenP4(Bmeson_copy.daughter(1)->daughter(1)->treeIdx(), p); // K from D
    }
    map<size_t, PtEtaPhiM_t> p4RecoFS;
    for (size_t ireco=0; ireco<recosize; ireco++) {
      if (!doRecoGenMatch) break;
      if (p.cand_status().at(ireco) != 1) continue; // stable
      p4RecoFS[ireco] = getRecoP4(ireco, p); // K from D
    }

    vector<bool> recoLock(recosize, 0);
    vector<bool> genLock (gensize,  0);

    vector<MatchPairInfo> matchGenInfo;
    for (const auto& gen : p4GenFS) {
      for (const auto& reco : p4RecoFS) {
        if (matchCriterion.match(reco.second, gen.second)) {
          const auto dR = ROOT::Math::VectorUtil::DeltaR(reco.second, gen.second);
          const auto dRelPt = TMath::Abs(gen.second.Pt() - reco.second.Pt())/gen.second.Pt();
          matchGenInfo.push_back( std::make_tuple(reco.first, gen.first, dR, dRelPt) );
        }
      }
    }
    // sort by pT
    std::sort(matchGenInfo.begin(), matchGenInfo.end(), [](MatchPairInfo i, MatchPairInfo j) { return std::get<3>(i) < std::get<3>(j); } );

    map<size_t, size_t> matchPairs;

    for (auto e : matchGenInfo) {
      const auto ireco = std::get<0>(e);
      const auto igen  = std::get<1>(e);
      if (!recoLock.at(ireco) && !genLock.at(igen)) {
        recoLock.at(ireco) = 1;
        genLock.at(igen)   = 1;
        matchPairs.insert(map<size_t, size_t>::value_type(ireco, igen));
      }
    }

    // loop over particles
    for (size_t ireco=0; ireco<recosize; ireco++) {
      // begin B
      if (pdgId[ireco] == 521) {
        auto dauIdx = p.cand_dauIdx().at(ireco);
        // 0 for pi, 1 for D0
        auto gDauIdx = p.cand_dauIdx().at(dauIdx.at(1));

        int Dflavor = -1;
        if (p.cand_massDau()[dauIdx.at(1)].at(0) <
            p.cand_massDau()[dauIdx.at(1)].at(1) ) Dflavor = 1;
        if ( Dflavor *
             recoCharge.at(dauIdx.at(0)) > 0) continue;  // check flavor
        // check reco-gen match
        bool matchGEN = true;
        if (doRecoGenMatch) {
          matchGEN = matchGEN && ( matchPairs.find( dauIdx.at(0) ) != matchPairs.end() );
          matchGEN = matchGEN && ( matchPairs.find( gDauIdx.at(0) ) != matchPairs.end() );
          matchGEN = matchGEN && ( matchPairs.find( gDauIdx.at(1) ) != matchPairs.end() );
          //bool isSwap = true; // need to be done
        }
        if (!matchGEN) continue;
        if (doRecoGenMatch) {
          hBMassMatch["dR0.03"]->Fill(p.cand_mass().at(ireco));
          // track QA
          {// pi from B
            double mom = p.cand_pT().at(dauIdx.at(0)) * std::cosh(p.cand_eta().at(dauIdx.at(0)));
            auto trkIdx = p.cand_trkIdx().at(dauIdx.at(0));
            hTrkBetaInvVsP["PionFromB"]->Fill(mom, 1./p.trk_beta().at(trkIdx));
          }
          {// pi from D
            if (Dflavor<0) {
              double mom = p.cand_pT().at(gDauIdx.at(1)) * std::cosh(p.cand_eta().at(gDauIdx.at(1)));
              auto trkIdx = p.cand_trkIdx().at(gDauIdx.at(1));
              hTrkBetaInvVsP["PionFromD"]->Fill(mom, 1./p.trk_beta().at(trkIdx));
            } else {
              double mom = p.cand_pT().at(gDauIdx.at(0)) * std::cosh(p.cand_eta().at(gDauIdx.at(0)));
              auto trkIdx = p.cand_trkIdx().at(gDauIdx.at(0));
              hTrkBetaInvVsP["PionFromD"]->Fill(mom, 1./p.trk_beta().at(trkIdx));
            }
          }
          {// K from D
            if (Dflavor>0) {
              double mom = p.cand_pT().at(gDauIdx.at(1)) * std::cosh(p.cand_eta().at(gDauIdx.at(1)));
              auto trkIdx = p.cand_trkIdx().at(gDauIdx.at(1));
              hTrkBetaInvVsP["KaonFromD"]->Fill(mom, 1./p.trk_beta().at(trkIdx));
            } else {
              double mom = p.cand_pT().at(gDauIdx.at(0)) * std::cosh(p.cand_eta().at(gDauIdx.at(0)));
              auto trkIdx = p.cand_trkIdx().at(gDauIdx.at(0));
              hTrkBetaInvVsP["KaonFromD"]->Fill(mom, 1./p.trk_beta().at(trkIdx));
            }
          }
          // track QA end
        }
      } // end B
    }
  }
  cout << nMultipleMatch << " events has more than one matched RECO B meson" << endl;
  ofile.cd();
  for (const auto& e : hBMassMatch) e.second->Write();
  for (const auto& e : hTrkBetaInvVsP) e.second->Write();

  return 0;
}
