/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __OLX_FIXC_MODE_H
#define __OLX_FIXC_MODE_H

class TFixCMode : public AModeWithLabels {
protected:
  static bool HasInstance;
  class TFixCModeUndo : public TUndoData {
    TCAtom& Atom;
    XVarReference* Vars[3];
    size_t LabelIndex;
  public:
    TFixCModeUndo(TXAtom* XA)
      : TUndoData(
        new TUndoActionImplMF<TFixCModeUndo>(this, &TFixCModeUndo::undo)),
      Atom(XA->CAtom()), LabelIndex(XA->GetOwnerId())
    {
      RefinementModel& rm = *Atom.GetParent()->GetRefMod();
      for (int i = 0; i < 3; i++)
        Vars[i] = rm.Vars.ReleaseRef(Atom, catom_var_name_X + i);
    }
    ~TFixCModeUndo() {
      for (int i = 0; i < 3; i++)
        if (Vars[i] != NULL)
          delete Vars[i];
    }
    void undo(TUndoData* data) {
      if (TFixCMode::HasInstance)
        TGXApp::GetInstance().MarkLabel(LabelIndex, false);
      RefinementModel& rm = *Atom.GetParent()->GetRefMod();
      for (int i = 0; i < 3; i++) {
        rm.Vars.RestoreRef(Atom, catom_var_name_X + i, Vars[i]);
        Vars[i] = NULL;
      }
    }
  };
#ifdef __BORLANDC__
  friend class TFixCModeUndo;
#endif
public:
  TFixCMode(size_t id) : AModeWithLabels(id) { HasInstance = true; }
  bool Initialise_(TStrObjList& Cmds, const TParamList& Options) {
    olex2.processMacro("labels -f");
    SetUserCursor("XYZ", "fix");
    return true;
  }
  ~TFixCMode() { HasInstance = false; }
  void Finalise_() {}
  virtual bool OnObject_(AGDrawObject &obj) {
    if (obj.Is<TXAtom>()) {
      TXAtom& XA = (TXAtom&)obj;
      gxapp.GetUndo().Push(new TFixCModeUndo(&XA));
      RefinementModel& rm = *XA.CAtom().GetParent()->GetRefMod();
      for (int i = 0; i < 3; i++) {
        rm.Vars.FixParam(XA.CAtom(), catom_var_name_X + i);
      }
      gxapp.MarkLabel(XA, true);
      return true;
    }
    return false;
  }
};

#endif
