/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __OLX_GROW_MODE_H
#define __OLX_GROW_MODE_H

class TGrowMode : public AMode  {
protected:
  bool GrowShells;
  short mode;
  // the size is used to detrmine if the AU is being rebuilt
  TEBitArray detached;
public:
  TGrowMode(size_t id) : AMode(id), GrowShells(false), mode(0)  {}
  bool Initialise(TStrObjList& Cmds, const TParamList& Options) {
    bool SI = Options.Contains('s'),
         Cov = Options.Contains('c'),
         VdW = Options.Contains('v'),
         Rad = Options.Contains('r');
    GrowShells = Options.Contains("shells");
    mode = 0;
    if( SI )   mode = gmSInteractions;
    if( Cov )  mode = gmCovalent;
    else if( VdW )  {
      mode = gmVanDerWaals;
      olxstr vr = Options.FindValue('v');
      TGlXApp::GetGXApp()->SetDeltaV(vr.IsEmpty() ? 2.0 : vr.ToDouble());
    }
    else if( Rad )
      mode = gmSameAtoms;
    if( mode == 0 )  
      mode = gmCovalent;
    // the AU rebuilding mode, enfoces grow shells and covalent bonds only
    if( Options.Contains('a') )  {
      TAsymmUnit &au = TXApp::GetInstance().XFile().GetAsymmUnit();
      detached.SetSize(au.AtomCount());
      for( size_t i=0; i < au.AtomCount(); i++ )  {
        TCAtom &a = au.GetAtom(i);
        if( !a.IsDetached() && a.GetType() == iQPeakZ )  {
          a.SetDetached(true);
          detached.SetTrue(i);
        }
      }
      TGXApp::GetInstance().UpdateConnectivity();
      GrowShells = true;
      mode = gmCovalent;
    }
    const olxstr AtomsToGrow = Cmds.Text(' ');
    TGlXApp::GetMainForm()->processMacro("cursor(hand)");
    TGlXApp::GetGXApp()->SetGrowMode(mode, AtomsToGrow);
    TGlXApp::GetGXApp()->SetXGrowLinesVisible(true);
    TGlXApp::GetGXApp()->SetZoomAfterModelBuilt(false);
    return true;
  }
  void Finalise() {
    TGlXApp::GetGXApp()->SetXGrowLinesVisible(false);
    TGlXApp::GetGXApp()->SetZoomAfterModelBuilt(true);
    if( !detached.IsEmpty() )  {
      TAsymmUnit &au = TXApp::GetInstance().XFile().GetAsymmUnit();
      for( size_t i=0; i < au.AtomCount(); i++ )  {
        if( detached[i] )
          au.GetAtom(i).SetDetached(false);
      }
      TGXApp::GetInstance().XFile().GetLattice().CompaqQ();
    }
  }
  void UpdateAU(size_t ac)  {
    if( detached.IsEmpty() )  return;
    TLattice& latt = TXApp::GetInstance().XFile().GetLattice();
    for( size_t i=ac; i < latt.GetObjects().atoms.Count(); i++ )  {
      TSAtom &aa = latt.GetObjects().atoms[i];
      for( size_t j=0; j < ac; j++ )  {
        TSAtom &ab = latt.GetObjects().atoms[j];
        if( aa.CAtom().GetId() == ab.CAtom().GetId() )
          ab.SetDeleted(true);
      }
    }
    if( ac != latt.GetObjects().atoms.Count() )  {
      latt.UpdateAsymmUnit();
      latt.Init();
    }
  }
  virtual bool OnObject(AGDrawObject& obj)  {
    TLattice& latt = TXApp::GetInstance().XFile().GetLattice();
    if( EsdlInstanceOf(obj, TXGrowLine) )  {
      TXGrowLine& xl = (TXGrowLine&)obj;
      if( GrowShells && mode == gmCovalent )  {
        size_t ac = latt.GetObjects().atoms.Count();
        latt.GrowAtom(xl.CAtom(), xl.GetTransform());
        UpdateAU(ac);
      }
      else
        latt.GrowFragment(xl.CAtom().GetFragmentId(), xl.GetTransform());
      return true;
    }
    else if( EsdlInstanceOf(obj, TXAtom) )  {
      size_t ac = latt.GetObjects().atoms.Count();
      TXAtom& a = (TXAtom&)obj;
      if( !a.IsGrown() )  {
        latt.GrowAtom(a, GrowShells, NULL);
        UpdateAU(ac);
      }
      return true;
    }
    else
      return false;
  }
};

#endif
