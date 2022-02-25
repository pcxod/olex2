/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __OLX_SPLIT_MODE_H
#define __OLX_SPLIT_MODE_H

/* The mode is still available but has been superseeded by the mode fit
*/

enum {
  mode_split_Disassemble,
  mode_split_ObjectsCreate
};
class TSplitMode : public AEventsDispatcher, public AMode {
  TTypeList<olx_pair_t<TCAtom*, TCAtom*> > SplitAtoms;
protected:
  void UpdateSelectionCrds() const {
    TGlGroup& sel = gxapp.GetSelection();
    if (sel.Count() > 1) {
      vec3d cr;
      TXAtomPList atoms;
      for (size_t i = 0; i < sel.Count(); i++) {
        if (sel[i].Is<TXAtom>()) {
          cr += ((TXAtom&)sel[i]).GetCenter();
          cr += ((TXAtom&)sel[i]).crd();
          atoms.Add((TXAtom&)sel[i]);
        }
      }
    }
  }
  void UpdateCrds() const {
    const TAsymmUnit& au = gxapp.XFile().GetAsymmUnit();
    UpdateSelectionCrds();
    TGXApp::AtomIterator ai = gxapp.GetAtoms();
    while (ai.HasNext()) {
      TXAtom& xa = ai.Next();
      // summ the translations
      xa.crd() += xa.GetCenter();
      xa.NullCenter();
      vec3d c = xa.crd();
      au.CartesianToCell(c);
      xa.ccrd() = c;
      xa.CAtom().ccrd() = c;
    }
  }
  olxstr ReCon; // restraint or constraint to use for split atoms
  bool doSplit;
public:
  TSplitMode(size_t id) : AMode(id) {
    gxapp.OnObjectsCreate.Add(this, mode_split_ObjectsCreate, msiExit);
    gxapp.XFile().GetLattice().OnDisassemble.Add(
      this, mode_split_Disassemble, msiEnter);
  }

  bool Initialise_(TStrObjList& Cmds, const TParamList& Options) {
    if (!gxapp.CheckFileType<TIns>())  return false;
    ReCon = Options.FindValue("r", EmptyString()).ToLowerCase();
    olex2.processMacro("cursor(hand)");
    TGXApp::AtomIterator ai = gxapp.GetAtoms();
    while (ai.HasNext()) {
      ai.Next().SetMoveable(true);
    }
    gxapp.SetZoomAfterModelBuilt(gxapp
      .GetOptions().GetBoolOption("model.center_on_update", true, true));
    doSplit = Options.GetBoolOption('s', false, true);
    return true;
  }

  ~TSplitMode() {
    gxapp.OnObjectsCreate.Remove(this);
    gxapp.XFile().GetLattice().OnDisassemble.Remove(this);
  }

  void Finalise_() {
    RefinementModel& rm = gxapp.XFile().GetRM();
    UpdateCrds();
    // if this is not done here it interferes and may cause a crash
    gxapp.XFile().GetLattice().OnDisassemble.Remove(this);
    gxapp.OnObjectsCreate.Remove(this);
    TGXApp::AtomIterator ai = gxapp.GetAtoms();
    while (ai.HasNext()) {
      ai.Next().SetMoveable(false);
    }
    if (SplitAtoms.IsEmpty() || !doSplit) {
      gxapp.XFile().GetLattice().UpdateConnectivity();
      return;
    }
    TCAtomPList to_isot;
    XVar& xv = rm.Vars.NewVar(0.75);
    for (size_t i = 0; i < SplitAtoms.Count(); i++) {
      TCAtom& a = *SplitAtoms[i].GetA();
      TCAtom& b = *SplitAtoms[i].GetB();
      to_isot.Add(a);
      const double sp = 1. / a.GetDegeneracy();
      rm.Vars.AddVarRef(xv, a, catom_var_name_Sof, relation_AsVar, sp);
      rm.Vars.AddVarRef(xv, b, catom_var_name_Sof, relation_AsOneMinusVar, sp);
      int part = a.GetPart();
      if (part == 0) {
        part++;
      }
      a.SetPart(part);
      a.SetOccu(0.75*sp);
      b.SetPart(olx_sign(part)*(olx_abs(part) + 1));
      b.SetOccu(0.25*sp);
      TSimpleRestraint* sr = 0;
      if (ReCon.IsEmpty());
      else if (ReCon == "eadp") {
        sr = &rm.rEADP.AddNew();
      }
      else if (ReCon == "isor") {
        sr = &rm.rISOR.AddNew();
      }
      else if (ReCon == "simu") {
        sr = &rm.rSIMU.AddNew();
      }
      if (sr != 0) {
        sr->AddAtomPair(a, 0, b, 0);
      }
    }
    gxapp.XFile().GetLattice().SetAnis(to_isot, false);
    gxapp.XFile().GetLattice().Uniq();
  }

  virtual bool Dispatch(int msg, short id, const IOlxObject* Sender,
    const IOlxObject* Data, TActionQueue *)
  {
    if (msg == mode_split_ObjectsCreate) {
      TGXApp::AtomIterator ai = gxapp.GetAtoms();
      while (ai.HasNext()) {
        ai.Next().SetMoveable(true);
      }
    }
    else if (msg == mode_split_Disassemble) {
      UpdateCrds();
    }
    return true;
  }

  virtual bool OnObject_(AGDrawObject &obj) {
    if (obj.Is<TXAtom>()) {
      TXAtom *XA = &(TXAtom&)obj;
      bool split = true;
      for (size_t i = 0; i < SplitAtoms.Count(); i++)
        if (*SplitAtoms[i].GetA() == XA->CAtom() ||
          *SplitAtoms[i].GetB() == XA->CAtom())
        {
          split = false;
          break;
        }
      if (split && doSplit) {
        TXAtom &xa = gxapp.AddAtom(XA);
        const TAsymmUnit& au = gxapp.XFile().GetAsymmUnit();
        TAsymmUnit::TLabelChecker lck(au);
        xa.SetMoveable(true);
        xa.SetRoteable(true);
        SplitAtoms.AddNew(&XA->CAtom(), &xa.CAtom());
        int part = XA->CAtom().GetPart();
        if (part == 0) {
          part++;
        }
        XA->CAtom().SetPart(part);
        xa.CAtom().SetPart(part + 1);
        xa.crd() += 0.5;
        vec3d c = au.Fractionalise(xa.crd());
        xa.CAtom().ccrd() = c;
        xa.ccrd() = c;
        olxstr new_l = XA->GetLabel();
        olxch lc = olxstr::o_tolower(new_l.GetLast());
        if (olxstr::o_isalpha(lc) && new_l.Length() > 1) {
          new_l[new_l.Length() - 1] = ++lc;
        }
        else {
          new_l << 'a';
        }
        xa.CAtom().SetLabel(lck.CheckLabel(xa.CAtom(), new_l, 0, true), true);
        if (xa.GetType() == iQPeakZ) {
          xa.CAtom().SetQPeak(1.0);
        }
        gxapp.XFile().GetLattice().UpdateConnectivity();
        gxapp.UpdateDuplicateLabels();
        if (TXApp::DoUseSafeAfix()) {
          gxapp.GetUndo().Push(
            gxapp.XFile().GetLattice().ValidateHGroups(true, true));
        }
      }
      else {  // do selection then
        UpdateSelectionCrds();
        gxapp.GetRenderer().Select(*XA);
      }
      return true;
    }
    return false;
  }
};

#endif
