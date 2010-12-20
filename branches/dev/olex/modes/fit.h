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
  bool Initialised;
  class OnUniqHandler : public AActionHandler {
    TFitMode& fit_mode;
  public:
    OnUniqHandler(TFitMode& fm) : fit_mode(fm)  {}
    bool Enter(const IEObject* Sender, const IEObject* Data=NULL)  {
      fit_mode.Dispatch(mode_fit_disassemble, msiEnter, NULL, NULL);
      return true;
    }
  };

  OnUniqHandler* uniq_handler;
public:
  TFitMode(size_t id) : AMode(id), Initialised(false)  {
    uniq_handler = new OnUniqHandler(*this);
    TGlXApp::GetGXApp()->OnObjectsCreate.Add(this, mode_fit_create, msiExit);
    TGlXApp::GetGXApp()->XFile().GetLattice().OnDisassemble.Add(this, mode_fit_disassemble, msiEnter);
    TGlXApp::GetGXApp()->XFile().GetLattice().OnStructureUniq.AddFirst(uniq_handler);
    TGlXApp::GetGXApp()->XFile().GetLattice().OnStructureGrow.AddFirst(uniq_handler);
    TGlXApp::GetGXApp()->EnableSelection(false);
  }
  bool Initialise(TStrObjList& Cmds, const TParamList& Options) {
    AtomsToMatch.Clear();
    TGlXApp::GetMainForm()->SetUserCursor('0', "<F>");
    TXAtomPList xatoms;
    TPtrList<AGDrawObject> xbonds;
    TGlGroup& sel = TGlXApp::GetGXApp()->GetSelection();
    for( size_t i=0; i < sel.Count(); i++ )  {
      if( EsdlInstanceOf(sel[i], TXAtom) )
        xatoms.Add((TXAtom&)sel[i]);
      else if( EsdlInstanceOf(sel[i], TXBond) )
        xbonds.Add(sel[i]);
    }
    group = &TGlXApp::GetGXApp()->GetRender().ReplaceSelection<TXGroup>();
    AddAtoms(xatoms);
    for( size_t i=0; i < xbonds.Count(); i++ )
      TGlXApp::GetGXApp()->GetRender().Select(*xbonds[i]);
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
    TAsymmUnit& au = app.XFile().GetAsymmUnit();
    RefinementModel& rm = app.XFile().GetRM();
    for( size_t i=0; i < Atoms.Count(); i++ )  {
      Atoms[i]->CAtom().ccrd() = Atoms[i]->crd();
      Atoms[i]->ccrd() = au.CartesianToCell(Atoms[i]->CAtom().ccrd());
      rm.Vars.FixParam(Atoms[i]->CAtom(), catom_var_name_Sof);
    }
    app.GetRender().ReplaceSelection<TGlGroup>();
    Initialised = false;
    // need to update symm eq etc
    app.XFile().GetLattice().Init();
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
      for( size_t i=0; i < Atoms.Count(); i++ )  {
        Atoms[i]->CAtom().ccrd() = Atoms[i]->crd();
        Atoms[i]->ccrd() = au.CartesianToCell(Atoms[i]->CAtom().ccrd());
      }
      Atoms.Clear();
      AtomsToMatch.Clear();
      TGlXApp::GetMainForm()->SetUserCursor('0', "<F>");
    }
    else if( msg == mode_fit_create )  {
      if( !EsdlInstanceOf(app.GetRender().GetSelection(), TXGroup) )
        group = &TGlXApp::GetGXApp()->GetRender().ReplaceSelection<TXGroup>();
      for( size_t i=0; i < group->Count(); i++ )  {
        if( EsdlInstanceOf(group->GetObject(i), TXAtom) )  {
          TXAtom* xa = Atoms.Add((TXAtom&)group->GetObject(i));
          xa->Atom().CAtom().ccrd() = xa->Atom().crd();
          xa->Atom().ccrd() = au.CartesianToCell(xa->Atom().CAtom().ccrd());
        }
      }
      group->Update();
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
