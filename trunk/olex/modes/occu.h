#ifndef __OLX_OCCU_MODE_H
#define __OLX_OCCU_MODE_H

class TOccuMode : public AModeWithLabels  {
  double Occu;
protected:
  class TOccuModeUndo : public TUndoData {
    TXAtom* Atom;
    double Occu, Var;
  public:
    TOccuModeUndo(TXAtom* XA) :
        TUndoData(new TUndoActionImpl<TOccuModeUndo>(this, &TOccuModeUndo::undo))  {
      Atom = XA;
      Occu = XA->Atom().CAtom().GetOccp();
      Var = XA->Atom().CAtom().GetOccpVar();
    }
    void undo(TUndoData* data)  {
      TGlXApp::GetGXApp()->MarkLabel(Atom, false);
      Atom->Atom().CAtom().SetOccp( Occu );
      Atom->Atom().CAtom().SetOccpVar( Var );
    }
  };

public:
  TOccuMode(int id) : AModeWithLabels(id)  {}
  bool Init(TStrObjList &Cmds, const TParamList &Options) {
    Occu = Cmds.IsEmpty() ? 0 : Cmds[0].ToDouble();
    TGlXApp::GetMainForm()->SetUserCursor( Occu, "occu");
    TGlXApp::GetMainForm()->executeMacro("labels -ao");
    return true;
  }
  ~TOccuMode() {  }
  virtual bool OnObject(AGDrawObject &obj)  {
    if( EsdlInstanceOf( obj, TXAtom) )  {
      TXAtom *XA = &(TXAtom&)obj;
      TGlXApp::GetMainForm()->GetUndoStack()->Push( new TOccuModeUndo(XA) );
      if( fabs(Occu) > 10 )  {
        int iv = (int)Occu/10;  iv *= 10;
        if( iv != 10 )
          XA->Atom().CAtom().SetOccpVar( Occu );
        else
          XA->Atom().CAtom().SetOccpVar( 10 );
        XA->Atom().CAtom().SetOccp( fabs(Occu - iv) );
      }
      else  {
        XA->Atom().CAtom().SetOccpVar( 0 );
        XA->Atom().CAtom().SetOccp( Occu );
      }
      TGlXApp::GetGXApp()->MarkLabel(XA, true);
      return true;
    }
    return false;
  }
};

#endif
