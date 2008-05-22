#ifndef __OLX_FIXC_MODE_H
#define __OLX_FIXC_MODE_H

class TFixCMode : public AModeWithLabels  {
protected:
  class TFixCModeUndo : public TUndoData {
    TXAtom* Atom;
    double Vars[3];
  public:
    TFixCModeUndo(TXAtom* XA) :
        TUndoData(new TUndoActionImpl<TFixCModeUndo>(this, &TFixCModeUndo::undo))  {
      Atom = XA;
      for( int i=0; i < 3; i++ )
        Vars[i] = XA->Atom().CAtom().FixedValues()[TCAtom::CrdFixedValuesOffset + i];
    }
    void undo(TUndoData* data)  {
      TGlXApp::GetGXApp()->MarkLabel(Atom, false);
      for( int i=0; i < 3; i++ )
        Atom->Atom().CAtom().FixedValues()[TCAtom::CrdFixedValuesOffset + i] = Vars[i];
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
      TXAtom *XA = &(TXAtom&)obj;
      TGlXApp::GetMainForm()->GetUndoStack()->Push( new TFixCModeUndo(XA) );
      for( int i=0; i < 3; i++ )
        XA->Atom().CAtom().FixedValues()[TCAtom::CrdFixedValuesOffset + i] = 10;
      TGlXApp::GetGXApp()->MarkLabel(XA, true);
      return true;
    }
    return false;
  }
};

#endif
