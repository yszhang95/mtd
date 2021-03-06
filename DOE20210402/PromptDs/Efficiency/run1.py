#!/usr/bin/env python
import ROOT as r
r.gROOT.LoadMacro('helpers.cc+')
Phi = r.Particle(333)
Phi.selfConj(True);
Phi.addDaughter(r.Particle(-321))
Phi.addDaughter(r.Particle(+321))
Ds = r.Particle(431)
Ds.addDaughter(r.Particle(+211))
Ds.addDaughter(Phi)

#topoCut = r.ana.TopoCut(0.0, 99999., 0.6, 0.0) # old
topoCut = r.ana.TopoCut(0.0, 99999., 0.45, 0.0)
kinsCut = r.ana.KineCut(2., 1000., 0., 3.)

ts = r.TStopwatch()
ts.Start()
r.genMatchFSDsMass('DsHydJets_pT0to3p05_y0to1p05.list', 'ds_ana_mc', Ds, topoCut, kinsCut, -1, False, False, False)
ts.Stop()
ts.Print()
