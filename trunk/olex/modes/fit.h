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
protected:
  TGXApp::GroupData group_data;
public:
  TFitMode(size_t id) : AMode(id)  {
    TGlXApp::GetGXApp()->OnObjectsCreate.Add(this, mode_fit_create, msiExit);
    TGlXApp::GetGXApp()->XFile().GetLattice().OnDisassemble.Add(this, mode_fit_disassemble, msiEnter);
    group_data.parent_id = -3;
  }
  bool Initialise(TStrObjList& Cmds, const TParamList& Options) {
    AtomsToMatch.Clear();
    TGlXApp::GetMainForm()->SetUserCursor('0', "<F>");
    group = &TGlXApp::GetGXApp()->GetRender().ReplaceSelection<TXGroup>();
    return true;
  }
  ~TFitMode()  {
    TGlXApp::GetGXApp()->OnObjectsCreate.Remove(this);
    TGlXApp::GetGXApp()->XFile().GetLattice().OnDisassemble.Remove(this);
  }
  void Finalise() {
    TGXApp& app = *TGlXApp::GetGXApp();
    TAsymmUnit& au = app.XFile().GetAsymmUnit();
    RefinementModel& rm = app.XFile().GetRM();
    for( size_t i=0; i < Atoms.Count(); i++ )  {
      Atoms[i]->Atom().CAtom().ccrd() = Atoms[i]->Atom().crd();
      Atoms[i]->Atom().ccrd() = au.CartesianToCell(Atoms[i]->Atom().CAtom().ccrd());
      rm.Vars.FixParam(Atoms[i]->Atom().CAtom(), catom_var_name_Sof);
      Atoms[i]->Atom().CAtom().SetPart(0);
    }
    app.GetRender().ReplaceSelection<TGlGroup>();
    app.XFile().GetLattice().UpdateConnectivity();
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
    TGXApp& app = *TGlXApp::GetGXApp();
    if( msg == mode_fit_disassemble )  {
      if( !EsdlInstanceOf(app.GetRender().GetSelection(), TXGroup) )
        return true;
      TGXApp::GetInstance().StoreGroup(*group, group_data);
      Atoms.Clear();
      AtomsToMatch.Clear();
      TGlXApp::GetMainForm()->SetUserCursor('0', "<F>");
    }
    else if( msg == mode_fit_create )  {
      if( group_data.parent_id == -3 )  return true;
      if( !EsdlInstanceOf(app.GetRender().GetSelection(), TXGroup) )
        group = &TGlXApp::GetGXApp()->GetRender().ReplaceSelection<TXGroup>();
      TGXApp::GetInstance().RestoreGroup(*group, group_data);
      for( size_t i=0; i < group->Count(); i++ )  {
        if( EsdlInstanceOf(group->GetObject(i), TXAtom) )
          Atoms.Add((TXAtom&)group->GetObject(i));
      }
      group->Update();
      group->SetSelected(true);
      group->SetVisible(true);
      group_data.atoms.Clear();
      group_data.bonds.Clear();
      group_data.parent_id = -3;
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
    for( size_t i=0; i < atoms.Count(); i++ )
      atoms[i]->Atom().CAtom().SetPart(DefNoPart);
    return true;
  }
};

#endif
