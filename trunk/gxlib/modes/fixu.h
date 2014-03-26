/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __OLX_FIXU_MODE_H
#define __OLX_FIXU_MODE_H

class TFixUMode : public AModeWithLabels  {
  double Val;
protected:
  static bool HasInstance;
  class TFixUModeUndo : public TUndoData {
    TCAtom& Atom;
    XVarReference* Uiso, *Vars[6];
    size_t LabelIndex;
  public:
    TFixUModeUndo(TXAtom* XA)
      : TUndoData(new TUndoActionImplMF<TFixUModeUndo>(
          this, &TFixUModeUndo::undo)),
        Atom(XA->CAtom()), LabelIndex(XA->GetOwnerId())
    {
      RefinementModel& rm = *Atom.GetParent()->GetRefMod();
      Uiso = rm.Vars.ReleaseRef(Atom, catom_var_name_Uiso);
      for( int i=0; i < 6; i++ )
        Vars[i] = rm.Vars.ReleaseRef(Atom, catom_var_name_U11+i);
    }
    ~TFixUModeUndo()  {
      if( Uiso != NULL )
        delete Uiso;
      for( int i=0; i < 6; i++ )
        if( Vars[i] != NULL )
          delete Vars[i];
    }
    void undo(TUndoData* data)  {
      if( TFixUMode::HasInstance )
        TGXApp::GetInstance().MarkLabel(LabelIndex, false);
      RefinementModel& rm = *Atom.GetParent()->GetRefMod();
      rm.Vars.RestoreRef(Atom, catom_var_name_Uiso, Uiso);
      Uiso = NULL;
      for( int i=0; i < 6; i++ )  {
        rm.Vars.RestoreRef(Atom, catom_var_name_U11+i, Vars[i]);
        Vars[i] = NULL;
      }
    }
  };
#ifdef __BORLANDC__
  friend class TFixUModeUndo;
#endif
public:
  TFixUMode(size_t id) : AModeWithLabels(id)  {  HasInstance = true;  }
  bool Initialise_(TStrObjList& Cmds, const TParamList& Options) {
    Val = Cmds.IsEmpty() ? 1 : Cmds[0].ToDouble();
    olex2.processMacro("labels -f -r -h");
    if( Val == 0 )
      SetUserCursor("<U>", "fix");
    else
      SetUserCursor(Val, "fixU");
    return true;
  }
  ~TFixUMode() {  HasInstance = false;  }
  void Finalise_()  {}
  virtual bool OnObject_(AGDrawObject& obj)  {
    if( EsdlInstanceOf(obj, TXAtom) )  {
      TXAtom& XA = (TXAtom&)obj;
      gxapp.GetUndo().Push(new TFixUModeUndo(&XA));
      RefinementModel& rm = *XA.CAtom().GetParent()->GetRefMod();
      if( XA.CAtom().GetEllipsoid() == NULL )
        gxapp.SetAtomUiso(XA, Val);
      else  {
        for( int i=0; i < 6; i++ )
          rm.Vars.FixParam(XA.CAtom(), catom_var_name_U11+i);
      }
      gxapp.MarkLabel(XA, true);
      return true;
    }
    return false;
  }
};

#endif
