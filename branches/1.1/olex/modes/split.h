#ifndef __OLX_SPLIT_MODE_H
#define __OLX_SPLIT_MODE_H

enum {
  mode_split_Disassemble,
  mode_split_ObjectsCreate
};
class TSplitMode : public AEventsDispatcher, public AMode  {
  TTypeList<AnAssociation2<size_t,size_t> > SplitAtoms;
protected:
  void UpdateSelectionCrds() const {
    TGlGroup& sel = TGlXApp::GetGXApp()->GetSelection();
    if( sel.Count() > 1 )  {
      vec3d c, cr;
      TXAtomPList atoms;
      for( size_t i=0; i < sel.Count(); i++ )  {
        if( EsdlInstanceOf(sel[i], TXAtom) )  {
          cr += ((TXAtom&)sel[i]).GetCenter();
          cr += ((TXAtom&)sel[i]).Atom().crd();
          atoms.Add( (TXAtom&)sel[i] );
        }
      }
      //if( atoms.Count() > 1 )  {
      //  cr /= atoms.Count();
      //  for( size_t i=0; i < atoms.Count(); i++ )  {
      //    c = atoms[i]->Atom().crd();
      //    c += atoms[i]->Basis.GetCenter();
      //    c -= cr;
      //    c *= atoms[i]->Basis.GetMatrix();
      //    c += cr;
      //    atoms[i]->Atom().crd() = c;
      //    atoms[i]->Basis.Reset();
      //  }
      //}
    }
  }
  void UpdateCrds() const {
    TGXApp& app = *TGlXApp::GetGXApp();
    const TAsymmUnit& au = app.XFile().GetAsymmUnit();
    UpdateSelectionCrds();
    for( size_t i=0; i < app.AtomCount(); i++ )  {
      TXAtom& xa = app.GetAtom(i);
      xa.SetMoveable(false);
      xa.SetRoteable(false);
      // summ the translations
      xa.Atom().crd() += xa.GetCenter();
      xa.NullCenter();
      vec3d c = xa.Atom().crd();
      au.CartesianToCell(c);
      xa.Atom().ccrd() = c;
      xa.Atom().CAtom().ccrd() = c;
    }
  }
  olxstr ReCon; // restraint or constraint to use for split atoms
public:
  TSplitMode(size_t id) : AMode(id)  {
    TGXApp& app = *TGlXApp::GetGXApp();
    app.OnObjectsCreate.Add(this, mode_split_ObjectsCreate, msiExit);
    app.XFile().GetLattice().OnDisassemble.Add(this, mode_split_Disassemble, msiEnter);
  }
  bool Initialise(TStrObjList& Cmds, const TParamList& Options) {
    TGXApp& app = *TGlXApp::GetGXApp();
    if( !app.CheckFileType<TIns>() )  return false;
    ReCon = Options.FindValue("r", EmptyString).ToLowerCase();
    TGlXApp::GetMainForm()->executeMacro("cursor(hand)");
    for( size_t i=0; i < app.AtomCount(); i++ )
      app.GetAtom(i).SetMoveable(true);
    return true;
  }
  ~TSplitMode()  {
    TGXApp& app = *TGlXApp::GetGXApp();
    app.OnObjectsCreate.Remove(this);
    app.XFile().GetLattice().OnDisassemble.Remove(this);
  }
  void Finalise() {
    TGXApp& app = *TGlXApp::GetGXApp();
    TIns& Ins = app.XFile().GetLastLoader<TIns>();
    RefinementModel& rm = app.XFile().GetRM();
    UpdateCrds();
    if( SplitAtoms.IsEmpty() )  return;
    TCAtomPList to_isot;
    XVar& xv = rm.Vars.NewVar(0.5);
    for( size_t i=0; i < SplitAtoms.Count(); i++ )  {
      TXAtom& xa = app.GetAtom(SplitAtoms[i].GetA());
      TXAtom& xb = app.GetAtom(SplitAtoms[i].GetB());
      to_isot.Add(xa.Atom().CAtom());
      rm.Vars.AddVarRef(xv, xa.Atom().CAtom(), catom_var_name_Sof, relation_AsVar, 1.0);
      rm.Vars.AddVarRef(xv, xb.Atom().CAtom(), catom_var_name_Sof, relation_AsOneMinusVar, 1.0);
      int part = xa.Atom().CAtom().GetPart();
      if( part == 0 )  part ++;
      xa.Atom().CAtom().SetPart(part);
      xb.Atom().CAtom().SetPart(part+1);
      TSimpleRestraint* sr = NULL;
      if( ReCon.IsEmpty() );
      else if( ReCon == "eadp" )
        sr = &rm.rEADP.AddNew();
      else if( ReCon == "isor" )
        sr = &rm.rISOR.AddNew();
      else if( ReCon == "simu" )
        sr = &rm.rSIMU.AddNew();
      if( sr != NULL )
        sr->AddAtomPair(xa.Atom().CAtom(), NULL, xb.Atom().CAtom(), NULL);
    }
    app.XFile().GetLattice().SetAnis(to_isot, false);
  }
  virtual bool Dispatch(int msg, short id, const IEObject* Sender, const IEObject* Data=NULL)  {  
    TGXApp& app = *TGlXApp::GetGXApp();
    if( msg == mode_split_ObjectsCreate )  {
      for( size_t i=0; i < app.AtomCount(); i++ )
        app.GetAtom(i).SetMoveable(true);
    }
    else if( msg == mode_split_Disassemble )  {
      UpdateCrds();
    }
    return true;
  }
  virtual bool OnObject(AGDrawObject &obj)  {
    if( EsdlInstanceOf( obj, TXAtom) )  {
      TXAtom *XA = &(TXAtom&)obj;
      bool split = true;
      for( size_t i=0; i < SplitAtoms.Count(); i++ )
        if( SplitAtoms[i].A() == XA->GetXAppId() || SplitAtoms[i].B() == XA->GetXAppId() )  {
          split = false;
          break;
        }
      if( split )  {
        TXAtom* xa = TGlXApp::GetGXApp()->AddAtom(XA);
        if( xa != NULL )  {
          xa->SetMoveable(true);
          xa->SetRoteable(true);
          SplitAtoms.AddNew(XA->GetXAppId(), xa->GetXAppId());
          int part = XA->Atom().CAtom().GetPart();
          if( part == 0 )  part ++;
          XA->Atom().CAtom().SetPart(part);
          xa->Atom().CAtom().SetPart(part+1);
          xa->Atom().crd() += 0.5;
          vec3d c = xa->Atom().crd();
          TGlXApp::GetGXApp()->XFile().GetAsymmUnit().CartesianToCell(c);
          xa->Atom().CAtom().ccrd() = c;
          xa->Atom().ccrd() = c;
          olxstr new_l = XA->Atom().GetLabel();
          olxch lc = olxstr::o_tolower(new_l.Last() );
          if( lc >= 'a' && lc <= 'z' )
            new_l[new_l.Length()-1] = ++lc;
          else
            new_l << 'a';
          xa->Atom().CAtom().SetLabel(TGlXApp::GetGXApp()->XFile().GetAsymmUnit().CheckLabel(&xa->Atom().CAtom(), new_l), false);
          if( xa->Atom().GetType() == iQPeakZ )
            xa->Atom().CAtom().SetQPeak(1.0);
          TGlXApp::GetGXApp()->XFile().GetLattice().UpdateConnectivity();
        }
      }
      else  {  // do selection then
        UpdateSelectionCrds();
        TGlXApp::GetGXApp()->GetRender().Select(*XA);
      }
      return true;
    }
    return false;
  }
};

#endif
