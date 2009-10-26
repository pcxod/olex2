#ifndef __OLX_FIT_MODE_H
#define __OLX_FIT_MODE_H

class TFitMode : public AMode  {
  TXAtomPList Atoms;
protected:
  void UpdateSelectionCrds() {
    TGlGroup& sel = TGlXApp::GetGXApp()->GetSelection();
    if( sel.Count() > 1 )  {
      vec3d c, cr;
      TXAtomPList atoms;
      for( size_t i=0; i < sel.Count(); i++ )  {
        if( EsdlInstanceOf(sel[i], TXAtom) )  {
          cr += ((TXAtom&)sel[i]).Basis.GetCenter();
          cr += ((TXAtom&)sel[i]).Atom().crd();
          atoms.Add( (TXAtom&)sel[i]);
        }
      }
      if( atoms.Count() > 1 )  {
        cr /= atoms.Count();
        for( size_t i=0; i < atoms.Count(); i++ )  {
          c = atoms[i]->Atom().crd();
          c += atoms[i]->Basis.GetCenter();
          c -= cr;
          c *= atoms[i]->Basis.GetMatrix();
          c += cr;
          atoms[i]->Atom().crd() = c;
          atoms[i]->Basis.Reset();
        }
      }
    }
  }
public:
  TFitMode(int id) : AMode(id)  {}
  bool Init(TStrObjList &Cmds, const TParamList &Options) {
    TGXApp& app = *TGlXApp::GetGXApp();
    TGlXApp::GetMainForm()->executeMacro("cursor(hand)");
    return true;
  }
  ~TFitMode() {
    TGXApp& app = *TGlXApp::GetGXApp();
    vec3d c;
    TAsymmUnit& au = app.XFile().GetAsymmUnit();
    RefinementModel& rm = app.XFile().GetRM();
    UpdateSelectionCrds();
    for( size_t i=0; i < Atoms.Count(); i++ )  {
      TXAtom& xa = *Atoms[i];
      xa.SetMoveable(false);
      xa.SetRoteable(false);
      // summ the translations
      xa.Atom().crd() += xa.Basis.GetCenter();
      xa.Basis.NullCenter();
      c = xa.Atom().crd();
      au.CartesianToCell(c);
      xa.Atom().ccrd() = c;
      xa.Atom().CAtom().ccrd() = c;
    }
    //TGlXApp::GetMainForm()->executeMacro("fuse");
  }
  virtual bool OnObject(AGDrawObject &obj)  {
    if( EsdlInstanceOf( obj, TXAtom) )  {
      //TXAtom *XA = &(TXAtom&)obj;
      UpdateSelectionCrds();
      //TGlXApp::GetGXApp()->GetRender().Select(XA);
      return true;
    }
    return false;
  }
  virtual bool AddAtoms(const TXAtomPList& atoms)  {
    Atoms.AddList(atoms);
    for( size_t i=0; i < Atoms.Count(); i++ )  {
      Atoms[i]->SetRoteable(true);
      Atoms[i]->SetMoveable(true);
      TGlXApp::GetGXApp()->GetRender().Select(*Atoms[i]);
    }
    return true;
  }
};

#endif
