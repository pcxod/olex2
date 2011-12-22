/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __OLX_FIT_MODE_H
#define __OLX_FIT_MODE_H
#include "xgroup.h"
#include "match.h"

enum  {
  mode_fit_create,
  mode_fit_disassemble
};
class TFitMode : public AEventsDispatcher, public AMode  {
  TXGroup* group;
  TXAtomPList Atoms, AtomsToMatch;
  vec3d_alist original_crds;
  bool Initialised, DoSplit;
  class OnUniqHandler : public AActionHandler {
    TFitMode& fit_mode;
  public:
    OnUniqHandler(TFitMode& fm) : fit_mode(fm)  {}
    bool Enter(const IEObject* Sender, const IEObject* Data=NULL)  {
      fit_mode.Dispatch(mode_fit_disassemble, msiEnter, NULL, NULL);
      return true;
    }
  };
  double AngleInc;
  OnUniqHandler* uniq_handler;
public:
  TFitMode(size_t id) : AMode(id), Initialised(false), AngleInc(0), DoSplit(false)  {
    uniq_handler = new OnUniqHandler(*this);
    TGlXApp::GetGXApp()->OnObjectsCreate.Add(this, mode_fit_create, msiExit);
    TGlXApp::GetGXApp()->XFile().GetLattice().OnDisassemble.Add(this, mode_fit_disassemble, msiEnter);
    TGlXApp::GetGXApp()->XFile().GetLattice().OnStructureUniq.AddFirst(uniq_handler);
    TGlXApp::GetGXApp()->XFile().GetLattice().OnStructureGrow.AddFirst(uniq_handler);
    TGlXApp::GetGXApp()->EnableSelection(false);
  }
  bool Initialise(TStrObjList& Cmds, const TParamList& Options) {
    if( TGlXApp::GetGXApp()->XFile().GetLattice().IsGenerated() )  {
      TBasicApp::NewLogEntry(logError) << "Unavailable for grown structures";
      return false;
    }
    DoSplit = Options.Contains('s');
    AtomsToMatch.Clear();
    TGlXApp::GetMainForm()->SetUserCursor('0', "<F>");
    TXAtomPList xatoms = TGlXApp::GetGXApp()->GetSelection().Extract<TXAtom>();
    original_crds.SetCount(xatoms.Count());
    for( size_t i=0; i < xatoms.Count(); i++ )
      original_crds[i] = xatoms[i]->crd();
    group = &TGlXApp::GetGXApp()->GetRender().ReplaceSelection<TXGroup>();
    AngleInc = Options.FindValue("r", "0").ToDouble();
    group->SetAngleInc(AngleInc*M_PI/180);
    AddAtoms(xatoms);
    group->SetOrgiginalCrds(original_crds);
    return (Initialised = true);
  }
  ~TFitMode()  {
    TGlXApp::GetGXApp()->OnObjectsCreate.Remove(this);
    TGlXApp::GetGXApp()->XFile().GetLattice().OnDisassemble.Remove(this);
    TGlXApp::GetGXApp()->XFile().GetLattice().OnStructureUniq.Remove(uniq_handler);
    TGlXApp::GetGXApp()->XFile().GetLattice().OnStructureGrow.Remove(uniq_handler);
    delete uniq_handler;
    TGlXApp::GetGXApp()->EnableSelection(true);
  }
  void Finalise() {
    TGXApp& app = *TGlXApp::GetGXApp();
    app.GetRender().ReplaceSelection<TGlGroup>();
    Initialised = false;
    RefinementModel& rm = app.XFile().GetRM();
    TAsymmUnit& au = app.XFile().GetAsymmUnit();
    XVar& xv = rm.Vars.NewVar(0.75);
    if( DoSplit )  {
      TCAtomPList to_iso;
      for( size_t i=0; i < Atoms.Count(); i++ )  {
        if( Atoms[i]->crd().QDistanceTo(original_crds[i]) < 1e-3 )
          continue;
        TXAtom* nxa = app.AddAtom(Atoms[i]);
        if( nxa == NULL )  continue;
        TCAtom& na = nxa->CAtom();
        // set parts
        int part = Atoms[i]->CAtom().GetPart();
        if( part == 0 )  part ++;
        Atoms[i]->CAtom().SetPart(part);
        na.SetPart(part+1);
        // link occupancies
        const double sp = 1./Atoms[i]->CAtom().GetDegeneracy();
        rm.Vars.AddVarRef(xv, Atoms[i]->CAtom(), catom_var_name_Sof, relation_AsVar, sp);
        rm.Vars.AddVarRef(xv, na, catom_var_name_Sof, relation_AsOneMinusVar, sp);
        Atoms[i]->CAtom().SetOccu(0.75*sp);
        na.SetOccu(0.25*sp);
        // set label
        olxstr new_l = Atoms[i]->GetLabel();
        olxch lc = olxstr::o_tolower(new_l.GetLast());
        if( olxstr::o_isalpha(lc) )
          new_l[new_l.Length()-1] = ++lc;
        else
          new_l << 'a';
        na.SetLabel(au.CheckLabel(&na, new_l), false);
        if( na.GetType() == iQPeakZ )
          na.SetQPeak(1.0);
        // set coordinates
        na.ccrd() = au.Fractionalise(Atoms[i]->crd());
        Atoms[i]->CAtom().ccrd() = au.Fractionalise(original_crds[i]);
        to_iso.Add(Atoms[i]->CAtom());
      }
      app.XFile().GetLattice().SetAnis(to_iso, false);
    }
    else  {
      for( size_t i=0; i < Atoms.Count(); i++ )
        Atoms[i]->CAtom().ccrd() = au.Fractionalise(Atoms[i]->crd());
      app.XFile().GetLattice().Init();
    }
  }
  virtual bool OnObject(AGDrawObject &obj)  {
    if( EsdlInstanceOf(obj, TXAtom) )  {
      if( AtomsToMatch.IsEmpty() && Atoms.IndexOf((TXAtom&)obj) == InvalidIndex )
        return true;
      AtomsToMatch.Add((TXAtom&)obj);
      TGlXApp::GetMainForm()->SetUserCursor(AtomsToMatch.Count(), "<F>");
      if( (AtomsToMatch.Count()%2) == 0 )  {
        TMatchMode::FitAtoms(AtomsToMatch, "<F>", false);
        group->UpdateRotationCenter();
      }
    }
    return true;
  }
  virtual bool Dispatch(int msg, short id, const IEObject* Sender, const IEObject* Data=NULL)  {  
    if( !Initialised )  return false;
    TGXApp& app = *TGlXApp::GetGXApp();
    TAsymmUnit& au = app.XFile().GetAsymmUnit();
    if( msg == mode_fit_disassemble )  {
      if( !EsdlInstanceOf(app.GetRender().GetSelection(), TXGroup) )
        return true;
      for( size_t i=0; i < Atoms.Count(); i++ )
        Atoms[i]->CAtom().ccrd() = au.Fractionalise(Atoms[i]->crd());
      Atoms.Clear();
      AtomsToMatch.Clear();
      TGlXApp::GetMainForm()->SetUserCursor('0', "<F>");
    }
    else if( msg == mode_fit_create )  {
      if( !EsdlInstanceOf(app.GetRender().GetSelection(), TXGroup) )  {
        group = &TGlXApp::GetGXApp()->GetRender().ReplaceSelection<TXGroup>();
        group->SetAngleInc(AngleInc*M_PI/180);
      }
      Atoms = group->Extract<TXAtom>();
      group->Update();
      group->SetOrgiginalCrds(original_crds);
      group->SetSelected(true);
    }
    return true;
  }
  virtual bool OnKey(int keyId, short shiftState)  {
    if( shiftState == 0 && keyId == WXK_ESCAPE )  {
      if( AtomsToMatch.IsEmpty() )  return false;
      AtomsToMatch.Delete(AtomsToMatch.Count()-1);
      TGlXApp::GetMainForm()->SetUserCursor(AtomsToMatch.Count(), "<F>");
      return true;
    }
    return false;
  }
  virtual bool AddAtoms(const TXAtomPList& atoms)  {
    Atoms.AddList(atoms);
    group->AddAtoms(atoms);
    group->SetSelected(true);
    return true;
  }
};

#endif
