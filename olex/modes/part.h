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
      Atom(XA->Atom().CAtom()), LabelIndex(XA->GetXAppId())
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
  bool Init(TStrObjList &Cmds, const TParamList &Options) {
    Part = Cmds.IsEmpty() ? 0 : Cmds[0].ToInt();
    TGlXApp::GetMainForm()->SetUserCursor( Part, "part");
    TGlXApp::GetMainForm()->executeMacro("labels -p -h");
    return true;
  }
  ~TPartMode() {
    HasInstance = false;
    TXApp::GetInstance().XFile().GetLattice().UpdateConnectivity();
    //TGlXApp::GetMainForm()->executeMacro("fuse");
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
