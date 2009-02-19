#ifndef __OLX_PART_MODE_H
#define __OLX_PART_MODE_H

class TPartMode : public AModeWithLabels  {
  int Part;
protected:
  class TPartModeUndo : public TUndoData {
    TXAtom* Atom;
    int Part;
  public:
    TPartModeUndo(TXAtom* XA) :
        TUndoData( new TUndoActionImpl<TPartModeUndo>(this, &TPartModeUndo::undo))  {
      Atom = XA;
      Part = XA->Atom().CAtom().GetPart();
    }
    void undo(TUndoData* data)  {
      TGlXApp::GetGXApp()->MarkLabel(*Atom, false);
      Atom->Atom().CAtom().SetPart( Part );
    }
  };

public:
  TPartMode(int id) : AModeWithLabels(id)  {}
  bool Init(TStrObjList &Cmds, const TParamList &Options) {
    Part = Cmds.IsEmpty() ? 0 : Cmds[0].ToInt();
    TGlXApp::GetMainForm()->SetUserCursor( Part, "part");
    TGlXApp::GetMainForm()->executeMacro("labels -p -h");
    return true;
  }
  ~TPartMode() {
    TGlXApp::GetMainForm()->executeMacro("fuse");
  }
  virtual bool OnObject(AGDrawObject &obj)  {
    if( EsdlInstanceOf( obj, TXAtom) )  {
      TXAtom& XA = (TXAtom&)obj;
      TGlXApp::GetMainForm()->GetUndoStack()->Push( new TPartModeUndo(&XA) );
      XA.Atom().CAtom().SetPart(Part);
      TGlXApp::GetGXApp()->MarkLabel(XA, true);
      return true;
    }
    return false;
  }
};

#endif
