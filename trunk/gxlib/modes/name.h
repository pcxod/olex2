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
#include "analysis.h"

class TNameMode : public AModeWithLabels  {
  size_t Index;
  olxstr Prefix, Suffix, Symbol;
  bool Lock, NameResidues;
  TUndoData* FirstUndo;
  short AutoComplete;
protected:
  static TNameMode* Instance;
  class TNameModeUndo : public TUndoData {
    TSizeList LabelIndices;
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
    void AddAtom(TXAtom& xa)  {  LabelIndices.Add(xa.GetOwnerId());  }
    void undo(TUndoData* data)  {
      if (TNameMode::Instance != NULL) {
        for (size_t i=0; i < LabelIndices.Count(); i++) {
          TGXApp::GetInstance().MarkLabel(LabelIndices[i], false);
          TNameMode::Instance->Index--;
        }
        TNameMode::Instance->SetCursor();
      }
    }
  };
  class TLockUndo : public TUndoData {
    TCAtom &a;
    bool v;
  public:
    TLockUndo(TCAtom& a)
      : TUndoData(new TUndoActionImplMF<TLockUndo>(
          this, &TLockUndo::undo)),
        a(a), v(a.IsFixedType())
    {}
    void undo(TUndoData* data) {
      a.SetFixedType(v);
      TGXApp::AtomIterator ai = TGXApp::GetInstance().GetAtoms();
      while (ai.HasNext()) // update the locked atom view if any
        ai.Next().Update();
    }
  };
#ifdef __BORLANDC__
  friend class TNameModeUndo;
#endif
  void SetCursor()  {
    olxstr Labl = Symbol.IsEmpty() ? olxstr('$') : Symbol;
    SetUserCursor(Labl << Prefix << Index << Suffix, "name");
  }
  void DoNumber(TXAtom &a, TNameModeUndo* undo) {
    olxstr Labl =  Symbol.IsEmpty() ? a.GetType().symbol : Symbol;
    Labl << Prefix <<  Index << Suffix;
    undo->AddAction(TGXApp::GetInstance().Name(a, Labl, false));
    undo->AddAtom(a);
    TGXApp::GetInstance().MarkLabel(a, true);
    Index++;
  }
  static int RSorter(const TXAtom &a, const TXAtom &b) {
    if (a.CAtom().IsRingAtom()) {
      if (b.CAtom().IsRingAtom()) return 0;
      return -1;
    }
    if (b.CAtom().IsRingAtom())
      return 1;
    return 0;
  }
  void Autocomplete(TXAtom& xa, TNameModeUndo* undo, bool recursion=false) {
    if (!recursion) {
      TGXApp::AtomIterator ai = gxapp.GetAtoms();
      while (ai.HasNext()) {
        TXAtom &a = ai.Next();
        a.SetTag(gxapp.IsLabelMarked(a) ? 1 : 0);
      }
    }
    if (recursion && xa.GetTag() == 1) return;
    xa.SetTag(1);
    TXAtomPList outgoing;
    for (size_t i=0; i < xa.NodeCount(); i++) {
      TXAtom& nd = xa.Node(i);
      if (nd.IsDeleted() || nd.GetType() < 2 || nd.GetTag() == 1) // H,D,Q
        continue;
      if ((AutoComplete&2) == 2 && xa.GetType() != nd.GetType())
        continue;
      if ((AutoComplete&4) == 4 && xa.CAtom().GetPart() != nd.CAtom().GetPart())
        continue;
      outgoing.Add(nd);
    }
    bool set_cursor=false;
    if (outgoing.Count() == 1) {
      DoNumber(*outgoing[0], undo);
      Autocomplete(*outgoing[0], undo, true);
      set_cursor=true;
    }
    else if((AutoComplete&8) == 8) {
      TXAtomPList to_number;
      QuickSorter::SortSF(outgoing, &TNameMode::RSorter);
      for (size_t i=0; i < outgoing.Count(); i++) {
        TXAtom &a = *outgoing[i];
        if (a.GetTag() == 1) continue;
        size_t cnt=0;
        for (size_t i=0; i < a.NodeCount(); i++) {
          TXAtom& nd = a.Node(i);
          if (nd.IsDeleted() || nd.GetType() < 2 || nd.GetTag() == 1) continue;
          if ((AutoComplete&2) == 2 && xa.GetType() != nd.GetType())
            continue;
          if ((AutoComplete&4) == 4 && xa.CAtom().GetPart() != nd.CAtom().GetPart())
            continue;
          cnt++;
        }
        if (a.CAtom().IsRingAtom()) {
          set_cursor=true;
          DoNumber(a, undo);
          Autocomplete(a, undo, true);
        }
        else {
          if (cnt == 0) {
            set_cursor=true;
            DoNumber(a, undo);
          }
          else if (a.GetTag() == 0)
            to_number << a;
        }
      }
      for (size_t i=0; i < to_number.Count(); i++) {
        if (to_number[i]->GetTag() == 1) continue;
        set_cursor=true;
        DoNumber(*to_number[i], undo);
        Autocomplete(*to_number[i], undo, true);
      }
    }
    if (!recursion && set_cursor)
      SetCursor();
  }
public:
  TNameMode(size_t id) : AModeWithLabels(id)  {  Instance = this;  }
  bool Initialise_(TStrObjList& Cmds, const TParamList& Options) {
    Index = Cmds.IsEmpty() ? 1 : Cmds[0].ToInt();
    Prefix = Options.FindValue('p');
    Suffix = Options.FindValue('s');
    Symbol = Options.FindValue('t');  // type
    Lock = Options.GetBoolOption('l');
    AutoComplete = (short)Options.FindValue('a', '0').ToInt();
    NameResidues = Options.GetBoolOption('r');
    // validate if type is correct
    if( !Symbol.IsEmpty() && !XElementLib::IsElement(Symbol) )
      throw TInvalidArgumentException(__OlxSourceInfo, "element type");
    if (Cmds.IsEmpty() && !Symbol.IsEmpty())
      Index = gxapp.GetNextAvailableLabel(Symbol);

    SetCursor();
    olxstr labels("labels -l");
    if( Symbol.Equalsi('H') || Symbol.Equalsi('D') )
      labels << " -h";
    olex2.processMacro(labels);
    TGXApp::BondIterator bi = gxapp.GetBonds();
    while( bi.HasNext() )
      bi.Next().SetSelectable(false);
    if ((AutoComplete&8) == 8) {
      gxapp.SetQPeaksVisible(false);
      using namespace olx_analysis;
      TTypeList<fragments::fragment> frags = fragments::extract(
        gxapp.XFile().GetAsymmUnit());
      for (size_t i=0; i < frags.Count(); i++) {
        TCAtomPList r_atoms;
        frags[i].breadth_first_tags(InvalidIndex, &r_atoms);
        TTypeList<fragments::ring> rings = frags[i].get_rings(r_atoms);
        for (size_t j=0; j < rings.Count(); j++) {
          rings[j].atoms.ForEach(TCAtom::FlagSetter(catom_flag_RingAtom, true));
        }
      }
    }
    return true;
  }
  ~TNameMode() {  Instance = NULL;  }
  void Finalise_()  {
    TGXApp::BondIterator bi = gxapp.GetBonds();
    while( bi.HasNext() )
      bi.Next().SetSelectable(true);
    gxapp.XFile().GetLattice().UpdateConnectivity();
  }
  virtual bool OnObject_(AGDrawObject& obj)  {
    if( EsdlInstanceOf(obj, TXAtom) )  {
      TXAtom &XA = (TXAtom&)obj;
      olxstr Labl(Symbol.IsEmpty() ? XA.GetType().symbol : Symbol);
      Labl << Prefix <<  Index << Suffix;
      TNameModeUndo* undo = new TNameModeUndo(XA);
      undo->AddAction(gxapp.Name(XA, Labl, false));
      if (NameResidues) {
        undo->AddAction(gxapp.SynchroniseResidues(TXAtomPList() << XA));
      }
      if (Lock) {
        undo->AddAction(new TLockUndo(XA.CAtom()));
        XA.CAtom().SetFixedType(true);
        XA.Update();
      }
      gxapp.GetUndo().Push(undo);
      gxapp.MarkLabel(XA, true);
      Index++;
      SetCursor();
      if (AutoComplete != 0)
        Autocomplete(XA, undo);
      return true;
    }
    return false;
  }
};

#endif
