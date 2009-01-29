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
    TGXApp& app = *TGlXApp::GetGXApp();
    if( !app.CheckFileType<TIns>() )  return false;
    ReCon = Options.FindValue("r", EmptyString).ToLowerCase();
    TGlXApp::GetMainForm()->executeMacro("cursor(hand)");
    for( int i=0; i < app.AtomCount(); i++ )  {
      //if( Atoms[i]->Atom().GetAtomInfo() != iQPeakIndex )
      app.GetAtom(i).Moveable(true);
    }
    return true;
  }
  ~TSplitMode() {
    TGXApp& app = *TGlXApp::GetGXApp();
    vec3d c;
    TIns& Ins = app.XFile().GetLastLoader<TIns>();
    TAsymmUnit& au = app.XFile().GetAsymmUnit();
    RefinementModel& rm = app.XFile().GetRM();
    UpdateSelectionCrds();
    for( int i=0; i < app.AtomCount(); i++ )  {
      TXAtom& xa = app.GetAtom(i);
      xa.Moveable(false);
      xa.Roteable(false);
      // summ the translations
      xa.Atom().crd() += xa.Basis.GetCenter();
      xa.Basis.NullCenter();
      c = xa.Atom().crd();
      au.CartesianToCell(c);
      xa.Atom().ccrd() = c;
      xa.Atom().CAtom().ccrd() = c;
    }
    if( SplitAtoms.IsEmpty() )  return;

    TCAtomPList to_isot;
    XVar& xv = rm.Vars.NewVar(0.5);
    for( int i=0; i < SplitAtoms.Count(); i++ )  {
      to_isot.Add(&SplitAtoms[i].A()->Atom().CAtom());
      rm.Vars.AddVarRef(xv, SplitAtoms[i].A()->Atom().CAtom(), var_name_Sof, relation_AsVar);
      rm.Vars.AddVarRef(xv, SplitAtoms[i].B()->Atom().CAtom(), var_name_Sof, relation_AsOneMinusVar);
      int part = SplitAtoms[i].A()->Atom().CAtom().GetPart();
      if( part == 0 )  part ++;
      SplitAtoms[i].A()->Atom().CAtom().SetPart( part );
      SplitAtoms[i].B()->Atom().CAtom().SetPart( part+1 );
      TSimpleRestraint* sr = NULL;
      if( ReCon.IsEmpty() );
      else if( ReCon == "eadp" )
        sr = &rm.rEADP.AddNew();
      else if( ReCon == "isor" )
        sr = &rm.rISOR.AddNew();
      else if( ReCon == "simu" )
        sr = &rm.rSIMU.AddNew();
      if( sr != NULL )
        sr->AddAtomPair(SplitAtoms[i].A()->Atom().CAtom(), NULL, SplitAtoms[i].B()->Atom().CAtom(), NULL);
      //TGlXApp::GetMainForm()->executeMacro(
      //  olxstr("addins EADP ") << SplitAtoms[i].A()->Atom().GetLabel() << ' ' <<
      //  SplitAtoms[i].B()->Atom().GetLabel());
    }
    app.XFile().GetLattice().SetAnis(to_isot, false);
    //TGlXApp::GetMainForm()->executeMacro("fuse");
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
