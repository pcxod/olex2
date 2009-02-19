#ifndef __OLX_OCCU_MODE_H
#define __OLX_OCCU_MODE_H

class TOccuMode : public AModeWithLabels  {
  double Occu;
protected:
  class TOccuModeUndo : public TUndoData {
    TXAtom* Atom;
    double Occu;
  public:
    TOccuModeUndo(TXAtom* XA) :
        TUndoData(new TUndoActionImpl<TOccuModeUndo>(this, &TOccuModeUndo::undo))  {
      Atom = XA;
      Occu = XA->Atom().CAtom().GetParent()->GetRefMod()->Vars.GetAtomParam(XA->Atom().CAtom(), var_name_Sof);
    }
    void undo(TUndoData* data)  {
      TGlXApp::GetGXApp()->MarkLabel(*Atom, false);
      Atom->Atom().CAtom().GetParent()->GetRefMod()->Vars.SetAtomParam(Atom->Atom().CAtom(), var_name_Sof, Occu);
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
      TXAtom& XA = (TXAtom&)obj;
      TGlXApp::GetMainForm()->GetUndoStack()->Push( new TOccuModeUndo(&XA) );
      XA.Atom().CAtom().GetParent()->GetRefMod()->Vars.SetAtomParam(XA.Atom().CAtom(), var_name_Sof, Occu);
      TGlXApp::GetGXApp()->MarkLabel(XA, true);
      return true;
    }
    return false;
  }
};

#endif
