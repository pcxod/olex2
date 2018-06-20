/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __OLX_OCCU_MODE_H
#define __OLX_OCCU_MODE_H

class TOccuMode : public AModeWithLabels {
  double Occu;
protected:
  static bool HasInstance;
  class TOccuModeUndo : public TUndoData {
    TCAtom& Atom;
    double Occu;
    size_t LabelIndex;
  public:
    TOccuModeUndo(TXAtom* XA)
      : TUndoData(new TUndoActionImplMF<TOccuModeUndo>(
        this, &TOccuModeUndo::undo)),
      Atom(XA->CAtom()), LabelIndex(XA->GetOwnerId())
    {
      Occu = Atom.GetParent()->GetRefMod()->Vars.GetParam(
        Atom, catom_var_name_Sof);
    }
    void undo(TUndoData* data) {
      if (TOccuMode::HasInstance)
        TGXApp::GetInstance().MarkLabel(LabelIndex, false);
      Atom.GetParent()->GetRefMod()->Vars.SetParam(
        Atom, catom_var_name_Sof, Occu);
    }
  };
public:
  TOccuMode(size_t id) : AModeWithLabels(id)
  {
    HasInstance = true;
  }
  bool Initialise_(TStrObjList& Cmds, const TParamList& Options) {
    Occu = Cmds.IsEmpty() ? 0 : Cmds[0].ToDouble();
    SetUserCursor(Occu, "occu");
    olex2.processMacro("labels -ao");
    return true;
  }
  ~TOccuMode() { HasInstance = false; }
  void Finalise_() {}
  virtual bool OnObject_(AGDrawObject& obj) {
    if (obj.Is<TXAtom>()) {
      TXAtom& XA = (TXAtom&)obj;
      gxapp.GetUndo().Push(new TOccuModeUndo(&XA));
      XA.CAtom().GetParent()->GetRefMod()->Vars.SetParam(
        XA.CAtom(), catom_var_name_Sof, Occu);
      gxapp.MarkLabel(XA, true);
      return true;
    }
    return false;
  }
};

#endif
