#ifndef __OLX_SPLIT_MODE_H
#define __OLX_SPLIT_MODE_H

class TSplitMode : public AMode  {
  TTypeList< AnAssociation2<TXAtom*, TXAtom*> > SplitAtoms;
protected:
  void UpdateSelectionCrds() {
    TGlGroup* sel = TGlXApp::GetGXApp()->GetRender().Selection();
    if( sel->Count() > 1 )  {
      vec3d c, cr;
      TXAtomPList atoms;
      for( int i=0; i < sel->Count(); i++ )  {
        if( EsdlInstanceOf(*sel->Object(i), TXAtom) )  {
          cr += ((TXAtom*)sel->Object(i))->Basis.GetCenter();
          cr += ((TXAtom*)sel->Object(i))->Atom().crd();
          atoms.Add( (TXAtom*)sel->Object(i) );
        }
      }
      if( atoms.Count() > 1 )  {
        cr /= atoms.Count();
        for( int i=0; i < atoms.Count(); i++ )  {
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
  olxstr ReCon; // restraint or constraint to use for split atoms
public:
  TSplitMode(int id) : AMode(id)  {}
  bool Init(TStrObjList &Cmds, const TParamList &Options) {
    if( !TGlXApp::GetGXApp()->CheckFileType<TIns>() )  return false;
    ReCon = Options.FindValue("r", EmptyString).ToLowerCase();
    TGlXApp::GetMainForm()->executeMacro("cursor(hand)");
    TXAtomPList Atoms;
    TGlXApp::GetGXApp()->FindXAtoms(EmptyString, Atoms, false);
    for( int i=0; i < Atoms.Count(); i++ )  {
      //if( Atoms[i]->Atom().GetAtomInfo() != iQPeakIndex )
      Atoms[i]->Moveable(true);
    }
    return true;
  }
  ~TSplitMode() {
    TXAtomPList Atoms;
    vec3d c;
    TIns& Ins = TGlXApp::GetGXApp()->XFile().GetLastLoader<TIns>();
    Ins.AddVar(0.5);
    int Var = Ins.Vars().Count()*10+1;
    UpdateSelectionCrds();
    TGlXApp::GetGXApp()->FindXAtoms(EmptyString, Atoms, false);
    TAsymmUnit& au = TGlXApp::GetGXApp()->XFile().GetAsymmUnit();
    for( int i=0; i < Atoms.Count(); i++ )  {
      Atoms[i]->Moveable(false);
      Atoms[i]->Roteable(false);
      // summ the translations
      Atoms[i]->Atom().crd() += Atoms[i]->Basis.GetCenter();
      Atoms[i]->Basis.NullCenter();
      c = Atoms[i]->Atom().crd();
      TGlXApp::GetGXApp()->XFile().GetAsymmUnit().CartesianToCell(c);
      Atoms[i]->Atom().ccrd() = c;
      Atoms[i]->Atom().CAtom().ccrd() = c;
    }
    for( int i=0; i < SplitAtoms.Count(); i++ )  {
      SplitAtoms[i].A()->Atom().CAtom().SetOccpVar( Var );
      SplitAtoms[i].B()->Atom().CAtom().SetOccpVar( -Var );
      int part = SplitAtoms[i].A()->Atom().CAtom().GetPart();
      if( part == 0 )  part ++;
      SplitAtoms[i].A()->Atom().CAtom().SetPart( part );
      SplitAtoms[i].B()->Atom().CAtom().SetPart( part+1 );
      TSimpleRestraint* sr = NULL;
      if( ReCon.IsEmpty() );
      else if( ReCon == "eadp" )
        sr = &au.EquivalentU().AddNew();
      else if( ReCon == "isor" )
        sr = &au.RestranedUaAsUi().AddNew();
      else if( ReCon == "simu" )
        sr = &au.SimilarU().AddNew();
      if( sr != NULL )
        sr->AddAtomPair(SplitAtoms[i].A()->Atom().CAtom(), NULL, SplitAtoms[i].B()->Atom().CAtom(), NULL);
      //TGlXApp::GetMainForm()->executeMacro(
      //  olxstr("addins EADP ") << SplitAtoms[i].A()->Atom().GetLabel() << ' ' <<
      //  SplitAtoms[i].B()->Atom().GetLabel());
    }
    TGlXApp::GetMainForm()->executeMacro("fuse");
  }
  virtual bool OnObject(AGDrawObject &obj)  {
    if( EsdlInstanceOf( obj, TXAtom) )  {
      TXAtom *XA = &(TXAtom&)obj;
      bool split = true;
      for( int i=0; i < SplitAtoms.Count(); i++ )
        if( SplitAtoms[i].A() == XA || SplitAtoms[i].B() == XA )  {
          split = false;
          break;
        }
      if( split )  {
        TXAtom* xa = TGlXApp::GetGXApp()->AddAtom( XA );
        if( xa != NULL )  {
          vec3d c;
          xa->Moveable(true);
          xa->Roteable(true);
          SplitAtoms.AddNew(XA, xa);
          xa->Atom().crd() += 0.5;
          c = xa->Atom().crd();
          TGlXApp::GetGXApp()->XFile().GetAsymmUnit().CartesianToCell(c);
          xa->Atom().CAtom().ccrd() = c;
          xa->Atom().CAtom().Label() = TGlXApp::GetGXApp()->XFile().GetAsymmUnit().CheckLabel(&xa->Atom().CAtom(), XA->Atom().GetLabel()+'b');
        }
      }
      else  {  // do selection then
        UpdateSelectionCrds();
        TGlXApp::GetGXApp()->GetRender().Select(XA);
      }
      return true;
    }
    return false;
  }
};

#endif
