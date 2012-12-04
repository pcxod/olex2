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
  if( olx_abs(data[0]) > 1 || olx_abs(data[1]) > 1 || olx_abs(data[2]) > 1
    || olx_abs(data[3]) > 1 )
  {
    olxstr Tmp(olxstr::FormatFloat(3, data[0]), 32);
    Tmp.stream(',') <<
      olxstr::FormatFloat(3, data[1]) <<
      olxstr::FormatFloat(3, data[2]) <<
      olxstr::FormatFloat(3, data[3]);
    return Tmp;
  }
  else  {
    return olxstr(GetRGB());
  }
}
//..............................................................................
bool TGlOption::FromString(const olxstr &S)  {
  if( S.FirstIndexOf(',') != InvalidIndex )  {
    TStrList SL(S, ',');
    if( SL.Count() != 4 )  return false;
    data[0] = SL[0].ToFloat();
    data[1] = SL[1].ToFloat();
    data[2] = SL[2].ToFloat();
    data[3] = SL[3].ToFloat();
  }
  else  {
    *this = S.SafeUInt<uint32_t>();
  }
  return true;
}

