/******************************************************************************
* Copyright (c) 2004-2012 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gxlib_xmacro_H
#define __olx_gxlib_xmacro_H
#include "gxapp.h"

class GXLibMacros : public IEObject {
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
  DefMacro(ShowP)
  DefMacro(Cell)
  DefMacro(Basis)
  DefMacro(Sel)
  DefMacro(Esd)
  DefMacro(Undo)
  DefMacro(LstGO)
  DefMacro(Kill)

  DefMacro(Qual)
  DefMacro(Load)
  DefMacro(Matr)
  DefMacro(Line)
  DefMacro(Mpln)
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

  DefFunc(MatchFiles)
  DefFunc(ExtraZoom)
  DefFunc(SelName)
  DefFunc(GetMaterial)

  static int QPeakSortA(const TCAtom &a, const TCAtom &b);
  static int QPeakSortD(const TCAtom &a, const TCAtom &b) {
    return QPeakSortA(b, a);
  }

  void Export(TLibrary& lib);
};
//---------------------------------------------------------------------------
#endif
