#ifndef __OLX_NAME_MODE_H
#define __OLX_NAME_MODE_H

class TNameMode : public AModeWithLabels  {
  int Index;
  olxstr Prefix, Suffix, Symbol;
protected:
  class TNameModeUndo : public TUndoData {
    TXAtom* Atom;
  public:
    TNameModeUndo(IUndoAction* action, TXAtom* XA) : TUndoData(action)  {
      Atom = XA;
    }
    inline TXAtom* GetAtom() const {  return Atom;  }
  };

  void undo(TUndoData* data)  {
    TNameModeUndo* undo = static_cast<TNameMode::TNameModeUndo*>(data);
    TGlXApp::GetGXApp()->MarkLabel(*undo->GetAtom(), false);
    Index--;
    SetCursor();
  }
  void SetCursor()  {
    olxstr Labl( Symbol.IsEmpty() ? olxstr('$') : Symbol );
    TGlXApp::GetMainForm()->SetUserCursor(Labl << Prefix << Index << Suffix, "name");
  }
public:
  TNameMode(int id) : AModeWithLabels(id)  {}
  bool Init(TStrObjList &Cmds, const TParamList &Options) {
    Index = Cmds.IsEmpty() ? 1 : Cmds[0].ToInt();
    Prefix = Options.FindValue('p');
    Suffix = Options.FindValue('s');
    Symbol = Options.FindValue('t');  // type
    bool typeSet = false;

    if( Cmds.IsEmpty() && !Symbol.IsEmpty() )
      Index = TGlXApp::GetGXApp()->GetNextAvailableLabel(Symbol);

    SetCursor();

    TGlXApp::GetMainForm()->executeMacro("labels -l");
    TGXApp& app = *TGlXApp::GetGXApp();
    for( int i=0; i < app.BondCount(); i++ )
      app.GetBond(i).Groupable(false);
    return true;
  }
  ~TNameMode() {
    TGXApp& app = *TGlXApp::GetGXApp();
    for( int i=0; i < app.BondCount(); i++ )
      app.GetBond(i).Groupable(true);
    if( !Symbol.IsEmpty() )  // reconnect the structure according to the new atom types
      TGlXApp::GetMainForm()->executeMacro("fuse");
  }
  virtual bool OnObject(AGDrawObject &obj)  {
    if( EsdlInstanceOf( obj, TXAtom) )  {
      TXAtom &XA = (TXAtom&)obj;
      olxstr Labl ( Symbol.IsEmpty() ? XA.Atom().GetAtomInfo().GetSymbol() : Symbol);
      Labl << Prefix <<  Index << Suffix;
      TGlXApp::GetMainForm()->GetUndoStack()->Push(
        new TNameModeUndo(
          new TUndoActionImpl<TNameMode>(this, &TNameMode::undo), &XA) )->AddAction(
            TGlXApp::GetGXApp()->Name(XA, Labl, false) );
      TGlXApp::GetGXApp()->MarkLabel(XA, true);
      Index++;
      SetCursor();
      return true;
    }
    return false;
  }
};

#endif
