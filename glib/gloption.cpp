//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "gloption.h"
#include "math.h"
#include "estrlist.h"
#include "egc.h"
#include <memory.h>

UseGlNamespace();
//..............................................................................
//..............................................................................

TGlOption::TGlOption()
{
  FV[0] = FV[1] = FV[2] = 0;
  FV[3] = 1;
}
//..............................................................................
TGlOption::TGlOption(int RGB)  {  *this = RGB;  }
//..............................................................................
int TGlOption::GetRGB() const  {  return (int)RGBA(255*FV[0], 255*FV[1], 255*FV[2], 255*FV[3]);  }
//..............................................................................
void TGlOption::operator =(int c)
{
  FV[0] = GetRValue(c);
  FV[1] = GetGValue(c);
  FV[2] = GetBValue(c);
  FV[3] = GetAValue(c);
  // normailse
  FV[0] /= 255;
  FV[1] /= 255;
  FV[2] /= 255;
  FV[3] /= 255;
}
//..............................................................................
void TGlOption::operator =(float *c)  {  memcpy( FV, c, sizeof(float)*4);  }
//..............................................................................
bool TGlOption::operator == (const TGlOption &S) const
{
  float diff, summ;
  for( int i=0; i < 4; i++ )
  {
    diff = FV[i] - S.Data()[i];
    summ = fabs(FV[i]) + fabs(S.Data()[i]);
    if( summ )  if( fabs(diff/summ) > 0.001 )  return false; // %0.1 deviation
  }
  return true;
}
//..............................................................................
void TGlOption::operator = (const TGlOption &S)  {  memcpy( FV, S.Data(), 4*sizeof(float) );  }
//..............................................................................
void TGlOption::operator -= (const TGlOption& v)
{
  for( int i=0; i < 4; i++ )
    FV[i] -= v.Data()[i];
}
//..............................................................................
void TGlOption::operator += (const TGlOption& v)
{
  for( int i=0; i < 4; i++ )    FV[i] += v.Data()[i];
}
//..............................................................................
void TGlOption::operator *= (double v) {
  for( int i=0; i < 4; i++ )    FV[i] = (float)(FV[i]*v);
}
//..............................................................................
TGlOption TGlOption::operator * (double v) const  {
  TGlOption Res;
  for( int i=0; i < 4; i++ )    Res[i] = (float)(FV[i]*v);
  return Res;
}
//..............................................................................
TGlOption TGlOption::operator + (const TGlOption& v)  const  {
  TGlOption Res;
  for( int i=0; i < 4; i++ )    Res[i] = (float)(FV[i] + v.Data()[i]);
  return Res;
}
//..............................................................................
bool TGlOption::IsEmpty() const  {  return (fabs(FV[0])+fabs(FV[1])+fabs(FV[2]) == 0.0) ? true : false;  }
//..............................................................................
void TGlOption::Clear()  {  memset(FV, 0, 4*sizeof(float));  }
//..............................................................................
TIString TGlOption::ToString() const  {
  if( fabs(FV[0]) >= 1 || fabs(FV[1]) >= 1 || fabs(FV[2]) >= 1 || fabs(FV[3]) >= 1 )  {
    olxstr Tmp(olxstr::FormatFloat(3, FV[0]), 24);
    Tmp << ',' << olxstr::FormatFloat(3, FV[1]) <<
           ',' << olxstr::FormatFloat(3, FV[2]) <<
           ',' << olxstr::FormatFloat(3, FV[3]);
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
    FV[0] = (float)SL[0].ToDouble();
    FV[1] = (float)SL[1].ToDouble();
    FV[2] = (float)SL[2].ToDouble();
    FV[3] = (float)SL[3].ToDouble();
  }
  else  {
    *this = S.ToInt();
  }
  return true;
}

