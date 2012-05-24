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
public:
  static DefMacro(Grow)
  static DefMacro(Pack)
  static DefMacro(Name)
  static DefMacro(CalcFourier)
  static DefMacro(CalcPatt)

  static DefMacro(Qual)

//  static DefFunc(Lst)
  static void Export(class TLibrary& lib);
};
//---------------------------------------------------------------------------
#endif
