/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __OLX_PART_MODE_H
#define __OLX_PART_MODE_H

class TPartMode : public AModeWithLabels {
  int Part;
protected:
  static bool HasInstance;
  class TPartModeUndo : public TUndoData {
    TCAtom& Atom;
    int Part;
    size_t LabelIndex;
  public:
    TPartModeUndo(TXAtom* XA)
      : TUndoData(
        new TUndoActionImplMF<TPartModeUndo>(this, &TPartModeUndo::undo)),
      Atom(XA->CAtom()), LabelIndex(XA->GetOwnerId())
    {
      Part = Atom.GetPart();
    }
    void undo(TUndoData* data) {
      if (TPartMode::HasInstance) {
        TGXApp::GetInstance().MarkLabel(LabelIndex, false);
      }
      Atom.SetPart(Part);
    }
  };
public:
  TPartMode(size_t id) : AModeWithLabels(id)
  {
    HasInstance = true;
  }
  bool Initialise_(TStrObjList& Cmds, const TParamList& Options) {
    Part = Cmds.IsEmpty() ? 0 : Cmds[0].ToInt();
    SetUserCursor(Part, "part");
    olex2.processMacro("labels -p -h");
    return true;
  }
  ~TPartMode() { HasInstance = false; }
  void Finalise_() {
    TXApp::GetInstance().XFile().GetLattice().UpdateConnectivity();
  }
  virtual bool OnObject_(AGDrawObject& obj) {
    if (obj.Is<TXAtom>()) {
      TXAtom& XA = (TXAtom&)obj;
      gxapp.GetUndo().Push(new TPartModeUndo(&XA));
      XA.CAtom().SetPart(Part);
      gxapp.MarkLabel(XA, true);
      return true;
    }
    return false;
  }
};

#endif
