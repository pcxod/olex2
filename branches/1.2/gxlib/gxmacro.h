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

class GXLibMacros  {
protected:
  static int QPeakSortA(const TCAtom &a, const TCAtom &b);
  static int QPeakSortD(const TCAtom &a, const TCAtom &b) {
    return QPeakSortA(b, a);
  }
public:
  static DefMacro(Grow)
  static DefMacro(Pack)
  static DefMacro(Name)
  static DefMacro(CalcFourier)
  static DefMacro(CalcPatt)
  static DefMacro(Mask)
  static DefMacro(ARad)
  static DefMacro(ADS)
  static DefMacro(AZoom)
  static DefMacro(BRad)
  static DefMacro(TelpV)
  static DefMacro(Info)
  static DefMacro(Label)
  static DefMacro(Labels)
  static DefMacro(ShowH)
  static DefMacro(ShowQ)

  static DefMacro(Qual)
  static DefMacro(Load)
  static DefMacro(Matr)
  static DefMacro(Line)
  static DefMacro(Mpln)
  static DefMacro(Cent)
  static DefMacro(SetView)

//  static DefFunc(Lst)
  static void Export(class TLibrary& lib);
};
//---------------------------------------------------------------------------
#endif
