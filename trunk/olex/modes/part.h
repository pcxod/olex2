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

class TPartMode : public AModeWithLabels  {
  int Part;
protected:
  static bool HasInstance;
  class TPartModeUndo : public TUndoData {
    TCAtom& Atom;
    int Part;
    size_t LabelIndex;
  public:
    TPartModeUndo(TXAtom* XA) : TUndoData( new TUndoActionImplMF<TPartModeUndo>(this, &TPartModeUndo::undo)),
      Atom(XA->CAtom()), LabelIndex(XA->GetOwnerId())
    {
      Part = Atom.GetPart();
    }
    void undo(TUndoData* data)  {
      if( TPartMode::HasInstance )
        TGlXApp::GetGXApp()->MarkLabel(LabelIndex, false);
      Atom.SetPart( Part );
    }
  };
#ifdef __BORLANDC__
  friend class TPartModeUndo;
#endif
public:
  TPartMode(size_t id) : AModeWithLabels(id)  {  HasInstance = true;  }
  bool Initialise(TStrObjList& Cmds, const TParamList& Options) {
    Part = Cmds.IsEmpty() ? 0 : Cmds[0].ToInt();
    TGlXApp::GetMainForm()->SetUserCursor( Part, "part");
    TGlXApp::GetMainForm()->executeMacro("labels -p -h");
    return true;
  }
  ~TPartMode() {  HasInstance = false;  }
  void Finalise()  {  TXApp::GetInstance().XFile().GetLattice().UpdateConnectivity();  }
  virtual bool OnObject(AGDrawObject& obj)  {
    if( EsdlInstanceOf(obj, TXAtom) )  {
      TXAtom& XA = (TXAtom&)obj;
      TGlXApp::GetMainForm()->GetUndoStack()->Push(new TPartModeUndo(&XA));
      XA.CAtom().SetPart(Part);
      TGlXApp::GetGXApp()->MarkLabel(XA, true);
      return true;
    }
    return false;
  }
};

#endif
