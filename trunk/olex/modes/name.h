#ifndef __OLX_NAME_MODE_H
#define __OLX_NAME_MODE_H

class TNameMode : public AModeWithLabels  {
  int Index;
  olxstr Prefix, Suffix, Symbol;
  TUndoData* FirstUndo;
protected:
  class TNameModeUndo : public TUndoData {
    TXAtom* Atom;
  public:
    TNameModeUndo(IUndoAction* action, TXAtom* XA) : TUndoData(action)  {
      Atom = XA;
    }
    inline TXAtom* GetAtom() const {  return Atom;  }
  };
  class TNameModeUndoX : public TUndoData {
  public:
    TNameModeUndoX(IUndoAction* action) : TUndoData(action)  {  }
    TXAtomPList atoms;
  };

  void undo(TUndoData* data)  {
    TNameModeUndo* undo = static_cast<TNameMode::TNameModeUndo*>(data);
    TGlXApp::GetGXApp()->MarkLabel(*undo->GetAtom(), false);
    Index--;
    SetCursor();
  }
  void undoX(TUndoData* data)  {
    TNameModeUndoX* undo = static_cast<TNameMode::TNameModeUndoX*>(data);
    for( int i=0; i < undo->atoms.Count(); i++ )  {
      TGlXApp::GetGXApp()->MarkLabel(*undo->atoms[i], false);
      Index--;
    }
    SetCursor();
  }

  void SetCursor()  {
    olxstr Labl( Symbol.IsEmpty() ? olxstr('$') : Symbol );
    TGlXApp::GetMainForm()->SetUserCursor(Labl << Prefix << Index << Suffix, "name");
  }
  void Autocomplete(TXAtom& xa, TNameModeUndoX* undo = NULL)  {
    TGXApp& app = *TGlXApp::GetGXApp();
    for( int i=0; i < app.AtomCount(); i++ )
      app.GetAtom(i).Atom().SetTag(i);
    TXAtomPList outgoing;
    const TSAtom& sa = xa.Atom();
    for( int i=0; i < sa.NodeCount(); i++ )  {
      const TSAtom& nd = sa.Node(i);
      if( nd.IsDeleted() || nd.GetAtomInfo() == iQPeakIndex || nd.GetAtomInfo().GetMr() < 3.5 )
        continue;
      if( app.IsLabelMarked(app.GetAtom(nd.GetTag())) )
        continue;
      outgoing.Add( app.GetAtom(nd.GetTag()) );
    }
    if( outgoing.Count() == 1 )  {
      if( undo == NULL )  {
        undo = new TNameModeUndoX( new TUndoActionImplMF<TNameMode>(this, &TNameMode::undoX) );
        TGlXApp::GetMainForm()->GetUndoStack()->Push( undo );
        if( FirstUndo == NULL )
          FirstUndo = undo;
      }
      olxstr Labl ( Symbol.IsEmpty() ? outgoing[0]->Atom().GetAtomInfo().GetSymbol() : Symbol);
      Labl << Prefix <<  Index << Suffix;
      undo->AddAction( TGlXApp::GetGXApp()->Name(*outgoing[0], Labl, false) );
      undo->atoms.Add( outgoing[0] );
      TGlXApp::GetGXApp()->MarkLabel(*outgoing[0], true);
      Index++;
      SetCursor();
      Autocomplete( *outgoing[0], undo );
    }
  }
public:
  TNameMode(int id) : AModeWithLabels(id)  {
    FirstUndo = NULL;
  }
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
      app.GetBond(i).SetGroupable(false);
    return true;
  }
  ~TNameMode() {
    TGXApp& app = *TGlXApp::GetGXApp();
    for( int i=0; i < app.BondCount(); i++ )
      app.GetBond(i).SetGroupable(true);
    if( FirstUndo != NULL )  {  //unroll undo..
      TUndoData* ud;
      while( (ud=TGlXApp::GetMainForm()->GetUndoStack()->Pop()) != FirstUndo )
        delete ud;
      delete FirstUndo;
    }
    if( !Symbol.IsEmpty() )  // reconnect the structure according to the new atom types
      TGlXApp::GetMainForm()->executeMacro("fuse");
    
  }
  virtual bool OnObject(AGDrawObject &obj)  {
    if( EsdlInstanceOf( obj, TXAtom) )  {
      TXAtom &XA = (TXAtom&)obj;
      olxstr Labl ( Symbol.IsEmpty() ? XA.Atom().GetAtomInfo().GetSymbol() : Symbol);
      Labl << Prefix <<  Index << Suffix;
      TNameModeUndo* undo = new TNameModeUndo( new TUndoActionImplMF<TNameMode>(this, &TNameMode::undo), &XA);
      undo->AddAction( TGlXApp::GetGXApp()->Name(XA, Labl, false) );
      if( FirstUndo == NULL )
        FirstUndo = undo;
      TGlXApp::GetMainForm()->GetUndoStack()->Push( undo );
      TGlXApp::GetGXApp()->MarkLabel(XA, true);
      Index++;
      SetCursor();
      Autocomplete(XA);
      return true;
    }
    return false;
  }
};

#endif
