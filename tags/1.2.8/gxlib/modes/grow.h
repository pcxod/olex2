/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __OLX_GROW_MODE_H
#define __OLX_GROW_MODE_H
#include "xgrowline.h"
#include "label_corrector.h"

class TGrowMode : public AMode {
protected:
  bool GrowShells;
  short mode, part;
  // the size is used to detrmine if the AU is being rebuilt
  TEBitArray detached;
public:
  TGrowMode(size_t id)
    : AMode(id), GrowShells(false), mode(0), part(0)
  {}
  bool Initialise_(TStrObjList& Cmds, const TParamList& Options) {
    bool SI = Options.Contains('s'),
         Cov = Options.Contains('c'),
         VdW = Options.Contains('v'),
         Rad = Options.Contains('r');
    GrowShells = Options.Contains("shells");
    mode = 0;
    if (SI)   mode = gmSInteractions;
    if (Cov)  mode = gmCovalent;
    else if (VdW) {
      mode = gmVanDerWaals;
      olxstr vr = Options.FindValue('v');
      gxapp.SetDeltaV(vr.IsEmpty() ? 2.0 : vr.ToDouble());
    }
    else if (Rad) {
      mode = gmSameAtoms;
    }
    if (mode == 0) {
      mode = gmCovalent;
    }
    if (Options.Contains('p')) {
      if (mode != gmCovalent || Options.Contains('a')) {
        TBasicApp::NewLogEntry(logError) << "Incompatible values";
        return false;
      }
      olxstr pv = Options.FindValue('p');
      part = pv.IsEmpty() ? -1 : pv.ToInt();
      GrowShells = true;
    }
    // the AU rebuilding mode, enfoces grow shells and covalent bonds only
    if (Options.Contains('a')) {
      TAsymmUnit &au = gxapp.XFile().GetAsymmUnit();
      detached.SetSize(au.AtomCount());
      for (size_t i=0; i < au.AtomCount(); i++) {
        TCAtom &a = au.GetAtom(i);
        if (!a.IsDetached() && a.GetType() == iQPeakZ) {
          a.SetDetached(true);
          detached.SetTrue(i);
        }
      }
      gxapp.UpdateConnectivity();
      GrowShells = true;
      mode = gmCovalent;
    }
    const olxstr AtomsToGrow = Cmds.Text(' ');
    olex2.processMacro("cursor(hand)");
    gxapp.SetGrowMode(mode, AtomsToGrow);
    gxapp.SetXGrowLinesVisible(true);
    gxapp.SetZoomAfterModelBuilt(false);
    return true;
  }
  void Finalise_() {
    gxapp.SetXGrowLinesVisible(false);
    gxapp.SetZoomAfterModelBuilt(true);
    if (!detached.IsEmpty()) {
      TAsymmUnit &au = gxapp.XFile().GetAsymmUnit();
      for (size_t i=0; i < au.AtomCount(); i++) {
        if (detached[i])
          au.GetAtom(i).SetDetached(false);
      }
      gxapp.XFile().GetLattice().CompaqQ();
    }
    if (part != 0) {
      TLattice& latt = gxapp.XFile().GetLattice();
      LabelCorrector lc(latt.GetAsymmUnit(), TXApp::GetMaxLabelLength(),
        TXApp::DoRenameParts());
      for (size_t i=0; i < latt.GetObjects().atoms.Count(); i++) {
        TSAtom &a = latt.GetObjects().atoms[i];
        a.CAtom().SetOccu(a.CAtom().GetOccu()/2);
        if (!a.GetMatrix().IsFirst()) {
          TCAtom &ca = latt.GetAsymmUnit().NewAtom();
          ca.ccrd() = a.ccrd();
          ca.SetType(a.GetType());
          ca.SetPart((int8_t)part);
          ca.SetOccu(a.CAtom().GetOccu());
          ca.SetLabel(a.GetLabel(), false);
          lc.Correct(ca);
          gxapp.XFile().GetRM().Vars.FixParam(ca, catom_var_name_Sof);
        }
      }
      latt.Init();
    }
  }
  void UpdateAU(size_t ac) {
    if (detached.IsEmpty())  return;
    TLattice& latt = gxapp.XFile().GetLattice();
    for (size_t i = ac; i < latt.GetObjects().atoms.Count(); i++) {
      TSAtom &aa = latt.GetObjects().atoms[i];
      for (size_t j = 0; j < ac; j++) {
        TSAtom &ab = latt.GetObjects().atoms[j];
        if (aa.CAtom().GetId() == ab.CAtom().GetId())
          ab.SetDeleted(true);
      }
    }
    if (ac != latt.GetObjects().atoms.Count()) {
      latt.UpdateAsymmUnit();
      latt.Init();
    }
  }
  virtual bool OnObject_(AGDrawObject& obj) {
    TLattice& latt = gxapp.XFile().GetLattice();
    if (EsdlInstanceOf(obj, TXGrowLine)) {
      TXGrowLine& xl = (TXGrowLine&)obj;
      if (GrowShells && (mode == gmCovalent || mode == gmVanDerWaals)) {
        size_t ac = latt.GetObjects().atoms.Count();
        latt.GrowAtom(xl.CAtom(), xl.GetTransform());
        UpdateAU(ac);
      }
      else {
        latt.GrowFragment(xl.CAtom().GetFragmentId(), xl.GetTransform());
      }
      return true;
    }
    else if (EsdlInstanceOf(obj, TXAtom)) {
      size_t ac = latt.GetObjects().atoms.Count();
      TXAtom& a = (TXAtom&)obj;
      if (!a.IsGrown()) {
        latt.GrowAtom(a, GrowShells, NULL);
        UpdateAU(ac);
      }
      return true;
    }
    else
      return false;
  }
};

#endif
