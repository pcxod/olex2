#ifndef __OLX_FIXC_MODE_H
#define __OLX_FIXC_MODE_H

class TFixCMode : public AModeWithLabels  {
protected:
  class TFixCModeUndo : public TUndoData {
    TXAtom* Atom;
    XVarReference* Vars[3];
  public:
    TFixCModeUndo(TXAtom* XA) :
        TUndoData(new TUndoActionImpl<TFixCModeUndo>(this, &TFixCModeUndo::undo))  {
      Atom = XA;
      RefinementModel& rm = *XA->Atom().CAtom().GetParent()->GetRefMod();
      for( int i=0; i < 3; i++ )
        Vars[i] = rm.Vars.ReleaseRef(XA->Atom().CAtom(), var_name_X+i);
    }
    ~TFixCModeUndo() {
      for( int i=0; i < 3; i++ )
        if( Vars[i] != NULL )
          delete Vars[i];
    }
    void undo(TUndoData* data)  {
      TGlXApp::GetGXApp()->MarkLabel(*Atom, false);
      RefinementModel& rm = *Atom->Atom().CAtom().GetParent()->GetRefMod();
      for( int i=0; i < 3; i++ )  {
        rm.Vars.RestoreRef(Atom->Atom().CAtom(), var_name_X+i, Vars[i]);
        Vars[i] = NULL;
      }
    }
  };

public:
  TFixCMode(int id) : AModeWithLabels(id)  {}
  bool Init(TStrObjList &Cmds, const TParamList &Options) {
    TGlXApp::GetMainForm()->executeMacro("labels -f");
    TGlXApp::GetMainForm()->SetUserCursor( "XYZ", "fix" );
    return true;
  }
  ~TFixCMode() {  }
  virtual bool OnObject(AGDrawObject &obj)  {
    if( EsdlInstanceOf( obj, TXAtom) )  {
      TXAtom& XA = (TXAtom&)obj;
      TGlXApp::GetMainForm()->GetUndoStack()->Push( new TFixCModeUndo(&XA) );
      RefinementModel& rm = *XA.Atom().CAtom().GetParent()->GetRefMod();
      for( int i=0; i < 3; i++ )
        rm.Vars.FixAtomParam(XA.Atom().CAtom(), var_name_X+i);
      TGlXApp::GetGXApp()->MarkLabel(XA, true);
      return true;
    }
    return false;
  }
};

#endif
