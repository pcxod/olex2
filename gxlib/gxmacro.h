/******************************************************************************
* Copyright (c) 2004-2025 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gxlib_xmacro_H
#define __olx_gxlib_xmacro_H
#include "gxapp.h"

class GXLibMacros : public IOlxObject {
protected:
  TGXApp &app;
public:
  GXLibMacros(TGXApp &app) : app(app) {}
  DefMacro(Grow)
  DefMacro(Pack)
  DefMacro(Name)
  DefMacro(CalcFourier)
  DefMacro(CalcPatt)
  DefMacro(CalcVoid)
  DefMacro(Mask)
  DefMacro(ARad)
  DefMacro(ADS)
  DefMacro(AZoom)
  DefMacro(BRad)
  DefMacro(TelpV)
  DefMacro(Info)
  DefMacro(Label)
  DefMacro(Labels)
  DefMacro(ShowH)
  DefMacro(ShowQ)
  DefMacro(Detach)
  DefMacro(ShowP)
  DefMacro(ShowR)
  DefMacro(Cell)
  DefMacro(Basis)
  DefMacro(Sel)
  DefMacro(Esd)
  DefMacro(Undo)
  DefMacro(LstGO)
  DefMacro(Kill)
  DefMacro(Inv)

  DefMacro(Qual)
  DefMacro(Load)
  DefMacro(Matr)
  DefMacro(Line)
  DefMacro(Angle)
  DefMacro(Mpln)
  DefMacro(Lpln)
  DefMacro(Cent)
  DefMacro(SetView)
  DefMacro(ChemDraw)
  DefMacro(PiM)
  DefMacro(Group)
  DefMacro(Poly)

  DefMacro(WBox)
  DefMacro(Center)
  DefMacro(Direction)
  DefMacro(Individualise)
  DefMacro(Collectivise)
  DefMacro(Match)

  DefMacro(Uniq)
  DefMacro(Fmol)

  DefMacro(SetMaterial)
  DefMacro(DefineVar)
  DefMacro(ProjSph)
  DefMacro(OFileDel)
  DefMacro(OFileSwap)
  DefFunc(OFileCount)
  DefMacro(TwinView)
  DefMacro(CalcSurf)
  DefMacro(Legend)
  DefMacro(AdjustStyle)
  DefMacro(TLS)
  DefMacro(Udiff)
  DefMacro(MSDSView)
  DefMacro(ScaleN)
  DefMacro(PiPi)

  DefFunc(MatchFiles)
  DefFunc(ExtraZoom)
  DefFunc(SelName)
  DefFunc(GetMaterial)
  DefFunc(FBond)
  DefFunc(ObjectSettings)
  DefFunc(Visible)

  static int QPeakSortA(const TCAtom &a, const TCAtom &b);
  static int QPeakSortD(const TCAtom &a, const TCAtom &b) {
    return QPeakSortA(b, a);
  }

  void Export(TLibrary& lib);
};
//---------------------------------------------------------------------------
#endif
