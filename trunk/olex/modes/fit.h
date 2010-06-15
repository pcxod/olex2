#ifndef __OLX_FIT_MODE_H
#define __OLX_FIT_MODE_H
#include "xgroup.h"

class TFitMode : public AMode  {
  TXGroup* group;
  TXAtomPList Atoms;
protected:
public:
  TFitMode(size_t id) : AMode(id)  {}
  bool Init(TStrObjList &Cmds, const TParamList &Options) {
    TGXApp& app = *TGlXApp::GetGXApp();
    TGlXApp::GetMainForm()->executeMacro("cursor(hand)");
    group = &app.GetRender().ReplaceSelection<TXGroup>();
    return true;
  }
  ~TFitMode() {
    TGXApp& app = *TGlXApp::GetGXApp();
    TAsymmUnit& au = app.XFile().GetAsymmUnit();
    RefinementModel& rm = app.XFile().GetRM();
    for( size_t i=0; i < Atoms.Count(); i++ )  {
      TXAtom& xa = *Atoms[i];
      xa.Atom().crd() = (xa.Atom().crd()-group->GetRotationCenter())*group->GetMatrix() +
        group->GetRotationCenter() + group->GetCenter();
      vec3d c = xa.Atom().crd();
      xa.Atom().ccrd() = au.CartesianToCell(c);
      xa.Atom().CAtom().ccrd() = c;
    }
    app.GetRender().ReplaceSelection<TGlGroup>();
  }
  virtual bool OnObject(AGDrawObject &obj)  {
    if( EsdlInstanceOf( obj, TXAtom) )  {
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
