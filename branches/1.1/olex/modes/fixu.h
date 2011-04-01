#ifndef __OLX_FIXU_MODE_H
#define __OLX_FIXU_MODE_H

class TFixUMode : public AModeWithLabels  {
  double Val;
protected:
  static bool HasInstance;
  class TFixUModeUndo : public TUndoData {
    TCAtom& Atom;
    XVarReference* Uiso, *Vars[6];
    size_t LabelIndex;
  public:
    TFixUModeUndo(TXAtom* XA) : TUndoData(new TUndoActionImplMF<TFixUModeUndo>(this, &TFixUModeUndo::undo)),
      Atom(XA->CAtom()), LabelIndex(XA->GetOwnerId())
    {
      RefinementModel& rm = *Atom.GetParent()->GetRefMod();
      Uiso = rm.Vars.ReleaseRef(Atom, catom_var_name_Uiso);
      for( int i=0; i < 6; i++ )
        Vars[i] = rm.Vars.ReleaseRef(Atom, catom_var_name_U11+i);
    }
    ~TFixUModeUndo()  {
      if( Uiso != NULL )  
        delete Uiso;
      for( int i=0; i < 6; i++ )
        if( Vars[i] != NULL )
          delete Vars[i];
    }
    void undo(TUndoData* data)  {
      if( TFixUMode::HasInstance )
        TGlXApp::GetGXApp()->MarkLabel(LabelIndex, false);
      RefinementModel& rm = *Atom.GetParent()->GetRefMod();
      rm.Vars.RestoreRef(Atom, catom_var_name_Uiso, Uiso);
      Uiso = NULL;
      for( int i=0; i < 6; i++ )  {
        rm.Vars.RestoreRef(Atom, catom_var_name_U11+i, Vars[i]);
        Vars[i] = NULL;
      }
    }
  };
#ifdef __BORLANDC__
  friend class TFixUModeUndo;
#endif
public:
  TFixUMode(size_t id) : AModeWithLabels(id)  {  HasInstance = true;  }
  bool Initialise(TStrObjList& Cmds, const TParamList& Options) {
    Val = Cmds.IsEmpty() ? 1 : Cmds[0].ToDouble();
    TGlXApp::GetMainForm()->executeMacro("labels -f -r -h");
    if( Val == 0 )
      TGlXApp::GetMainForm()->SetUserCursor("<U>", "fix" );
    else
      TGlXApp::GetMainForm()->SetUserCursor(Val, "fixU" );
    return true;
  }
  ~TFixUMode() {  HasInstance = false;  }
  void Finalise()  {}
  virtual bool OnObject(AGDrawObject& obj)  {
    if( EsdlInstanceOf(obj, TXAtom) )  {
      TXAtom& XA = (TXAtom&)obj;
      TGlXApp::GetMainForm()->GetUndoStack()->Push(new TFixUModeUndo(&XA));
      RefinementModel& rm = *XA.CAtom().GetParent()->GetRefMod();
      if( XA.CAtom().GetEllipsoid() == NULL )
        TXApp::GetInstance().SetAtomUiso(XA, Val);
      else  {
        for( int i=0; i < 6; i++ )
          rm.Vars.FixParam(XA.CAtom(), catom_var_name_U11+i);
      }
      TGlXApp::GetGXApp()->MarkLabel(XA, true);
      return true;
    }
    return false;
  }
};

#endif
