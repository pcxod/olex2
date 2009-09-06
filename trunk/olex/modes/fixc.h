#ifndef __OLX_FIXC_MODE_H
#define __OLX_FIXC_MODE_H

class TFixCMode : public AModeWithLabels  {
protected:
  static bool HasInstance;
  class TFixCModeUndo : public TUndoData {
    TCAtom& Atom;
    XVarReference* Vars[3];
    int LabelIndex;
  public:
    TFixCModeUndo(TXAtom* XA) : TUndoData(new TUndoActionImplMF<TFixCModeUndo>(this, &TFixCModeUndo::undo)),
      Atom(XA->Atom().CAtom()), LabelIndex(XA->GetXAppId())
    {
      RefinementModel& rm = *Atom.GetParent()->GetRefMod();
      for( int i=0; i < 3; i++ )
        Vars[i] = rm.Vars.ReleaseRef(Atom, catom_var_name_X+i);
    }
    ~TFixCModeUndo() {
      for( int i=0; i < 3; i++ )
        if( Vars[i] != NULL )
          delete Vars[i];
    }
    void undo(TUndoData* data)  {
      if( TFixCMode::HasInstance )
        TGlXApp::GetGXApp()->MarkLabel(LabelIndex, false);
      RefinementModel& rm = *Atom.GetParent()->GetRefMod();
      for( int i=0; i < 3; i++ )  {
        rm.Vars.RestoreRef(Atom, catom_var_name_X+i, Vars[i]);
        Vars[i] = NULL;
      }
    }
  };
#ifdef __BORLANDC__
  friend class TFixCModeUndo;
#endif
public:
  TFixCMode(int id) : AModeWithLabels(id)  { HasInstance = true;  }
  bool Init(TStrObjList &Cmds, const TParamList &Options) {
    TGlXApp::GetMainForm()->executeMacro("labels -f");
    TGlXApp::GetMainForm()->SetUserCursor( "XYZ", "fix" );
    return true;
  }
  ~TFixCMode() {  HasInstance = false;  }
  virtual bool OnObject(AGDrawObject &obj)  {
    if( EsdlInstanceOf( obj, TXAtom) )  {
      TXAtom& XA = (TXAtom&)obj;
      TGlXApp::GetMainForm()->GetUndoStack()->Push( new TFixCModeUndo(&XA) );
      RefinementModel& rm = *XA.Atom().CAtom().GetParent()->GetRefMod();
      for( int i=0; i < 3; i++ )
        rm.Vars.FixParam(XA.Atom().CAtom(), catom_var_name_X+i);
      TGlXApp::GetGXApp()->MarkLabel(XA, true);
      return true;
    }
    return false;
  }
};

#endif
