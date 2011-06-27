/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "gloption.h"
#include "estrlist.h"
#include "emath.h"

TIString TGlOption::ToString() const  {
  if( olx_abs(data[0]) >= 1 || olx_abs(data[1]) >= 1 || olx_abs(data[2]) >= 1 || olx_abs(data[3]) >= 1 )  {
    olxstr Tmp(olxstr::FormatFloat(3, data[0]), 24);
    Tmp << ',' << olxstr::FormatFloat(3, data[1]) <<
           ',' << olxstr::FormatFloat(3, data[2]) <<
           ',' << olxstr::FormatFloat(3, data[3]);
    return Tmp;
  }
  else  {
    return olxstr((unsigned int)GetRGB());
  }
}
//..............................................................................
bool TGlOption::FromString(const olxstr &S)  {
  if( S.FirstIndexOf(',') != InvalidIndex )  {
    TStrList SL(S, ',');
    if( SL.Count() != 4 )  return false;
    data[0] = (float)SL[0].ToDouble();
    data[1] = (float)SL[1].ToDouble();
    data[2] = (float)SL[2].ToDouble();
    data[3] = (float)SL[3].ToDouble();
  }
  else  {
    *this = S.SafeUInt<uint32_t>();
  }
  return true;
}

