#ifndef __OLX_OCCU_MODE_H
#define __OLX_OCCU_MODE_H

class TOccuMode : public AModeWithLabels  {
  double Occu;
protected:
  static bool HasInstance;
  class TOccuModeUndo : public TUndoData {
    TCAtom& Atom;
    double Occu;
    int LabelIndex;
  public:
    TOccuModeUndo(TXAtom* XA) : TUndoData(new TUndoActionImplMF<TOccuModeUndo>(this, &TOccuModeUndo::undo)),
      Atom(XA->Atom().CAtom()), LabelIndex(XA->GetXAppId())
    {
      Occu = Atom.GetParent()->GetRefMod()->Vars.GetParam(Atom, catom_var_name_Sof);
    }
    void undo(TUndoData* data)  {
      if( TOccuMode::HasInstance )
        TGlXApp::GetGXApp()->MarkLabel(LabelIndex, false);
      Atom.GetParent()->GetRefMod()->Vars.SetParam(Atom, catom_var_name_Sof, Occu);
    }
  };
#ifdef __BORLANDC__
  friend class TOccuModeUndo;
#endif
public:
  TOccuMode(int id) : AModeWithLabels(id)  {  HasInstance = true;  }
  bool Init(TStrObjList &Cmds, const TParamList &Options) {
    Occu = Cmds.IsEmpty() ? 0 : Cmds[0].ToDouble();
    TGlXApp::GetMainForm()->SetUserCursor( Occu, "occu");
    TGlXApp::GetMainForm()->executeMacro("labels -ao");
    return true;
  }
  ~TOccuMode() {  HasInstance = false;  }
  virtual bool OnObject(AGDrawObject &obj)  {
    if( EsdlInstanceOf( obj, TXAtom) )  {
      TXAtom& XA = (TXAtom&)obj;
      TGlXApp::GetMainForm()->GetUndoStack()->Push( new TOccuModeUndo(&XA) );
      XA.Atom().CAtom().GetParent()->GetRefMod()->Vars.SetParam(XA.Atom().CAtom(), catom_var_name_Sof, Occu);
      TGlXApp::GetGXApp()->MarkLabel(XA, true);
      return true;
    }
    return false;
  }
};

#endif
