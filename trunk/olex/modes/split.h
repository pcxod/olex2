#ifndef __OLX_SPLIT_MODE_H
#define __OLX_SPLIT_MODE_H

class TSplitMode : public AMode  {
  TTypeList< AnAssociation2<TXAtom*, TXAtom*> > SplitAtoms;
protected:
public:
  TSplitMode(int id) : AMode(id)  {}
  bool Init(TStrObjList &Cmds, const TParamList &Options) {
    if( !TGlXApp::GetGXApp()->CheckFileType<TIns>() )  return false;
    TGlXApp::GetMainForm()->executeMacro("cursor(hand)");
    TXAtomPList Atoms;
    TGlXApp::GetGXApp()->FindXAtoms(EmptyString, Atoms, false);
    for( int i=0; i < Atoms.Count(); i++ )  {
      if( Atoms[i]->Atom().GetAtomInfo() != iQPeakIndex )
        Atoms[i]->Moveable(true);
    }
    return true;
  }
  ~TSplitMode() {
    TXAtomPList Atoms;
    TVPointD c;
    TIns *Ins = (TIns*)TGlXApp::GetGXApp()->XFile().GetLastLoader();
    Ins->AddVar(0.5);
    int Var = Ins->Vars().Count()*10+1;
    TGlXApp::GetGXApp()->FindXAtoms(EmptyString, Atoms, false);
    for( int i=0; i < Atoms.Count(); i++ )  {
      Atoms[i]->Moveable(false);
      // summ the translations
      Atoms[i]->Atom().Center() += Atoms[i]->Basis.GetCenter();
      Atoms[i]->Basis.NullCenter();
      c = Atoms[i]->Atom().Center();
      TGlXApp::GetGXApp()->XFile().GetAsymmUnit().CartesianToCell(c);
      Atoms[i]->Atom().CCenter() = c;
      Atoms[i]->Atom().CAtom().CCenter() = c;
    }
    for( int i=0; i < SplitAtoms.Count(); i++ )  {
      SplitAtoms[i].A()->Atom().CAtom().SetOccpVar( Var );
      SplitAtoms[i].B()->Atom().CAtom().SetOccpVar( -Var );
      int part = SplitAtoms[i].A()->Atom().CAtom().GetPart();
      if( part == 0 )  part ++;
      SplitAtoms[i].A()->Atom().CAtom().SetPart( part );
      SplitAtoms[i].B()->Atom().CAtom().SetPart( part+1 );
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
          TVPointD c;
          xa->Moveable(true);
          SplitAtoms.AddNew(XA, xa);
          xa->Atom().Center() += 0.5;
          c = xa->Atom().Center();
          TGlXApp::GetGXApp()->XFile().GetAsymmUnit().CartesianToCell(c);
          xa->Atom().CAtom().CCenter() = c;
          xa->Atom().CAtom().Label() = TGlXApp::GetGXApp()->XFile().GetAsymmUnit().CheckLabel(&xa->Atom().CAtom(), XA->Atom().GetLabel()+'b');
        }
      }
      else  // do selection then
        TGlXApp::GetGXApp()->GetRender().Select(XA);
      return true;
    }
    return false;
  }
};

#endif
