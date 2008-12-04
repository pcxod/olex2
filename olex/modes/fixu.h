#ifndef __OLX_FIXU_MODE_H
#define __OLX_FIXU_MODE_H

class TFixUMode : public AModeWithLabels  {
  double Val;
protected:
  class TFixUModeUndo : public TUndoData {
    TXAtom* Atom;
    XVarReference* Uiso, *Vars[6];

  public:
    TFixUModeUndo(TXAtom* XA) :
        TUndoData(new TUndoActionImpl<TFixUModeUndo>(this, &TFixUModeUndo::undo))  {
      Atom = XA;
      RefinementModel& rm = *XA->Atom().CAtom().GetParent()->GetRefMod();
      Uiso = rm.Vars.ReleaseRef(XA->Atom().CAtom(), var_name_Uiso);
      for( int i=0; i < 6; i++ )
        Vars[i] = rm.Vars.ReleaseRef(XA->Atom().CAtom(), var_name_U11+i);
    }
    ~TFixUModeUndo()  {
      if( Uiso != NULL )  delete Uiso;
      for( int i=0; i < 6; i++ )
        if( Vars[i] != NULL )
          delete Vars[i];
    }
    void undo(TUndoData* data)  {
      TGlXApp::GetGXApp()->MarkLabel(Atom, false);
      RefinementModel& rm = *Atom->Atom().CAtom().GetParent()->GetRefMod();
      rm.Vars.RestoreRef(Atom->Atom().CAtom(), var_name_Uiso, Uiso);
      Uiso = NULL;
      for( int i=0; i < 6; i++ )  {
        rm.Vars.RestoreRef(Atom->Atom().CAtom(), var_name_U11+i, Vars[i]);
        Vars[i] = NULL;
      }
    }
  };

public:
  TFixUMode(int id) : AModeWithLabels(id)  {}
  bool Init(TStrObjList &Cmds, const TParamList &Options) {
    Val = Cmds.IsEmpty() ? 1 : Cmds[0].ToDouble();
    TGlXApp::GetMainForm()->executeMacro("labels -f");
    if( Val == 0 )
      TGlXApp::GetMainForm()->SetUserCursor( "<U>", "fix" );
    else
      TGlXApp::GetMainForm()->SetUserCursor( Val, "fixU" );
    return true;
  }
  ~TFixUMode() {  }
  virtual bool OnObject(AGDrawObject &obj)  {
    if( EsdlInstanceOf( obj, TXAtom) )  {
      TXAtom *XA = &(TXAtom&)obj;
      TGlXApp::GetMainForm()->GetUndoStack()->Push( new TFixUModeUndo(XA) );
      RefinementModel& rm = *XA->Atom().CAtom().GetParent()->GetRefMod();
      if( XA->Atom().CAtom().GetEllipsoid() == NULL )  {
        rm.Vars.SetAtomParam(XA->Atom().CAtom(), var_name_Uiso, Val);
      }
      else  {
        for( int i=0; i < 6; i++ )
          rm.Vars.FixAtomParam(XA->Atom().CAtom(), var_name_U11+i);
      }
      TGlXApp::GetGXApp()->MarkLabel(XA, true);
      return true;
    }
    return false;
  }
};

#endif
