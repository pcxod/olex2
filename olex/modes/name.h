/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __OLX_NAME_MODE_H
#define __OLX_NAME_MODE_H

class TNameMode : public AModeWithLabels  {
  size_t Index;
  olxstr Prefix, Suffix, Symbol;
  TUndoData* FirstUndo;
  bool AutoComplete;
protected:
  static TNameMode* Instance;
  class TNameModeUndo : public TUndoData {
    TSizeList LabelIndeces;
  public:
    TNameModeUndo()
      : TUndoData(new TUndoActionImplMF<TNameModeUndo>(
          this, &TNameModeUndo::undo))
    {}
    TNameModeUndo(TXAtom& xa)
      : TUndoData(new TUndoActionImplMF<TNameModeUndo>(
          this, &TNameModeUndo::undo))
    {
      AddAtom(xa);
    }
    void AddAtom(TXAtom& xa)  {  LabelIndeces.Add(xa.GetOwnerId());  }
    void undo(TUndoData* data)  {
      if( TNameMode::Instance != NULL )  {
        for( size_t i=0; i < LabelIndeces.Count(); i++ )  {
          TGlXApp::GetGXApp()->MarkLabel(LabelIndeces[i], false);
          TNameMode::Instance->Index--;
        }
        TNameMode::Instance->SetCursor();
      }
    }
  };
#ifdef __BORLANDC__
  friend class TNameModeUndo;
#endif
  void SetCursor()  {
    olxstr Labl( Symbol.IsEmpty() ? olxstr('$') : Symbol );
    TGlXApp::GetMainForm()->SetUserCursor(Labl << Prefix << Index << Suffix, "name");
  }
  void Autocomplete(TXAtom& xa, TNameModeUndo* undo)  {
    TGXApp& app = *TGlXApp::GetGXApp();
    TXAtomPList outgoing;
    for( size_t i=0; i < xa.NodeCount(); i++ )  {
      TXAtom& nd = static_cast<TXAtom&>(xa.Node(i));
      if( nd.IsDeleted() || nd.GetType() < 3.5 ) // H,D,Q
        continue;
      if( app.IsLabelMarked(nd) )
        continue;
      // 2009.07.17 --
      if( xa.GetType() != nd.GetType() )
        continue;
      // 2009.08.03 --
      if( xa.CAtom().GetPart() != nd.CAtom().GetPart() )
        continue;
      outgoing.Add(nd);
    }
    if( outgoing.Count() == 1 )  {
      olxstr Labl (Symbol.IsEmpty() ? outgoing[0]->GetType().symbol : Symbol);
      Labl << Prefix <<  Index << Suffix;
      undo->AddAction(TGlXApp::GetGXApp()->Name(*outgoing[0], Labl, false));
      undo->AddAtom( *outgoing[0] );
      TGlXApp::GetGXApp()->MarkLabel(*outgoing[0], true);
      Index++;
      SetCursor();
      Autocomplete( *outgoing[0], undo );
    }
  }
public:
  TNameMode(size_t id) : AModeWithLabels(id)  {  Instance = this;  }
  bool Initialise(TStrObjList& Cmds, const TParamList& Options) {
    Index = Cmds.IsEmpty() ? 1 : Cmds[0].ToInt();
    Prefix = Options.FindValue('p');
    Suffix = Options.FindValue('s');
    Symbol = Options.FindValue('t');  // type
    AutoComplete = Options.Contains('a');
    bool typeSet = false;
    // validate if type is correct
    if( !Symbol.IsEmpty() && !XElementLib::IsElement(Symbol) )
      throw TInvalidArgumentException(__OlxSourceInfo, "element type");
    if( Cmds.IsEmpty() && !Symbol.IsEmpty() )
      Index = TGlXApp::GetGXApp()->GetNextAvailableLabel(Symbol);

    SetCursor();
    olxstr labels("labels -l");
    if( Symbol.Equalsi('H') || Symbol.Equalsi('D') )
      labels << " -h";
    TGlXApp::GetMainForm()->processMacro(labels);
    TGXApp& app = *TGlXApp::GetGXApp();
    TGXApp::BondIterator bi = app.GetBonds();
    while( bi.HasNext() )
      bi.Next().SetSelectable(false);
    return true;
  }
  ~TNameMode() {  Instance = NULL;  }
  void Finalise()  {
    TGXApp& app = *TGlXApp::GetGXApp();
    TGXApp::BondIterator bi = app.GetBonds();
    while( bi.HasNext() )
      bi.Next().SetSelectable(true);
    app.XFile().GetLattice().UpdateConnectivity();
  }
  virtual bool OnObject(AGDrawObject& obj)  {
    if( EsdlInstanceOf(obj, TXAtom) )  {
      TXAtom &XA = (TXAtom&)obj;
      olxstr Labl(Symbol.IsEmpty() ? XA.GetType().symbol : Symbol);
      Labl << Prefix <<  Index << Suffix;
      TNameModeUndo* undo = new TNameModeUndo(XA);
      undo->AddAction(TGlXApp::GetGXApp()->Name(XA, Labl, false));
      TGlXApp::GetMainForm()->GetUndoStack()->Push(undo);
      TGlXApp::GetGXApp()->MarkLabel(XA, true);
      Index++;
      SetCursor();
      if( AutoComplete )
        Autocomplete(XA, undo);
      return true;
    }
    return false;
  }
};

#endif
