//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "gloption.h"
#include "estrlist.h"

//..............................................................................
TIString TGlOption::ToString() const  {
  if( fabs(data[0]) >= 1 || fabs(data[1]) >= 1 || fabs(data[2]) >= 1 || fabs(data[3]) >= 1 )  {
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
  if( S.FirstIndexOf(',') != -1 )  {
    TStrList SL(S, ',');
    if( SL.Count() != 4 )  return false;
    data[0] = (float)SL[0].ToDouble();
    data[1] = (float)SL[1].ToDouble();
    data[2] = (float)SL[2].ToDouble();
    data[3] = (float)SL[3].ToDouble();
  }
  else  {
    *this = S.ToInt();
  }
  return true;
}

