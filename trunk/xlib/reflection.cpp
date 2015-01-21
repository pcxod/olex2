/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "reflection.h"

#ifdef __GNUC___
  const int16_t TReflection::NoBatchSet;
  static const uint32_t
    FlagMask, FlagLen,
    MultMask, MultLen, MultOff,
    BatchMask, BatchLen = 16, BatchOff;
  static const uint16_t bitCentric, bitAbsent, bitOmitted;
#endif

vec3i TReflection::Standardise(const vec3i& _hkl, const SymmSpace::InfoEx& info)  {
  vec3i hkl = _hkl;
  if( info.centrosymmetric )  {
    vec3i hklv = -hkl, new_hkl = hkl;
    if( (hklv[2] > hkl[2]) ||
      ((hklv[2] == hkl[2]) && (hklv[1] > hkl[1])) ||
      ((hklv[2] == hkl[2]) && (hklv[1] == hkl[1]) && (hklv[0] > hkl[0])) )
    {
      new_hkl = hklv;
    }
    for( size_t i=0; i < info.matrices.Count(); i++ )  {
      hklv = hkl*info.matrices[i].r;
      if( (hklv[2] > new_hkl[2]) ||
        ((hklv[2] == new_hkl[2]) && (hklv[1] > new_hkl[1])) ||
        ((hklv[2] == new_hkl[2]) && (hklv[1] == new_hkl[1]) && (hklv[0] > new_hkl[0])) )
      {
        new_hkl = hklv;
      }
      hklv *= -1;
      if( (hklv[2] > new_hkl[2]) ||
        ((hklv[2] == new_hkl[2]) && (hklv[1] > new_hkl[1])) ||
        ((hklv[2] == new_hkl[2]) && (hklv[1] == new_hkl[1]) && (hklv[0] > new_hkl[0])) )
      {
        new_hkl = hklv;
      }
    }
    hkl = new_hkl;
  }
  else  {
    vec3i new_hkl = hkl;
    for( size_t i=0; i < info.matrices.Count(); i++ )  {
      vec3i hklv = hkl*info.matrices[i].r;
      if( (hklv[2] > new_hkl[2]) ||
        ((hklv[2] == new_hkl[2]) && (hklv[1] > new_hkl[1])) ||
        ((hklv[2] == new_hkl[2]) && (hklv[1] == new_hkl[1]) && (hklv[0] > new_hkl[0])) )
      {
        new_hkl = hklv;
      }
    }
    hkl = new_hkl;
  }
  return hkl;
}
//..............................................................................
bool TReflection::IsAbsent(const vec3i& hkl, const SymmSpace::InfoEx& info)  {
  bool absent = false;
  for( size_t i=0; i < info.matrices.Count(); i++ )  {
    vec3i hklv = hkl*info.matrices[i].r;
    if( hkl == hklv || (info.centrosymmetric && hkl == -hklv) )  {
      const double ps = info.matrices[i].t.DotProd(hkl);
      if( !(absent = (olx_abs( ps - olx_round(ps) ) > 0.01)) )  {
        for( size_t j=0; j < info.vertices.Count(); j++ )  {
          const double ps = (info.matrices[i].t+info.vertices[j]).DotProd(hkl);
          if( (absent = (olx_abs( ps - olx_round(ps) ) > 0.01)) )
            return true;
        }
      }
      if( absent )  return true;
    }
  }
  if( !absent )  {  // check for Identity and centering
    for( size_t i=0; i < info.vertices.Count(); i++ )  {
      const double ps = info.vertices[i].DotProd(hkl);
      if( (absent = (olx_abs( ps - olx_round(ps) ) > 0.01)) )
        return true;
    }
  }
  return false;
}
//..............................................................................
bool TReflection::FromString(const olxstr& Str)  {
  TStrList Toks(Str, ' ');
  if( Toks.Count() > 5 )  {
    hkl[0] = Toks[1].ToInt();
    hkl[1] = Toks[2].ToInt();
    hkl[2] = Toks[3].ToInt();
    I = Toks[4].ToDouble();
    S = Toks[5].ToDouble();
    if( Toks.Count() > 6 )
      SetBatch(Toks[6].ToInt());
    return true;
  }
  return false;
}
//..............................................................................
bool TReflection::FromNString(const olxstr& str) {
  TStrList Toks(str, ' ');
  if (Toks.Count() > 5) {
    if( Toks[0].GetLast() != '.' )  return false;
    Toks[0].SetLength(Toks[0].Length()-1);
    SetTag(Toks[0].ToInt());
    SetOmitted(GetTag() < 0);
    hkl[0] = Toks[1].ToInt();
    hkl[1] = Toks[2].ToInt();
    hkl[2] = Toks[3].ToInt();
    I = Toks[4].ToDouble();
    S = Toks[5].ToDouble();
    if (Toks.Count() > 6)
      SetBatch(Toks[6].ToInt());
    return true;
  }
  return false;
}
  // returns a string: h k l I S [f]
//..............................................................................
TIString TReflection::ToString() const {
  olx_array_ptr<olxch> bf(new olxch[128]);
  const olxch *f1 = olxT("%4i%4i%4i%8.2lf%8.2lf");
  const olxch *f2 = olxT("%4i%4i%4i%8.2lf%8.2lf%4i");
  int16_t batch = GetBatch();
#ifdef _UNICODE
#ifdef _MSC_VER
  if (batch == NoBatchSet)
    swprintf_s(bf, 128, f1, hkl[0], hkl[1], hkl[2], I, S);
  else
    swprintf_s(bf, 128, f2, hkl[0], hkl[1], hkl[2], I, S, batch);
#else
#ifdef __WIN32__ // gcc on windows - different signature!
  if (batch == NoBatchSet)
    swprintf(bf, f1, hkl[0], hkl[1], hkl[2], I, S);
  else
    swprintf(bf, f2, hkl[0], hkl[1], hkl[2], I, S, batch);
#else
  if (batch == NoBatchSet)
    swprintf(bf, 128, f1, hkl[0], hkl[1], hkl[2], I, S);
  else
    swprintf(bf, 128, f2, hkl[0], hkl[1], hkl[2], I, S, batch);
#endif
#endif
#else
#ifdef _MSC_VER
  if (batch == NoBatchSet)
    sprintf_s(bf, 128, f1, hkl[0], hkl[1], hkl[2], I, S);
  else
    sprintf_s(bf, 128, f2, hkl[0], hkl[1], hkl[2], I, S, batch);
#else
  if (batch == NoBatchSet)
    sprintf(bf, f1, hkl[0], hkl[1], hkl[2], I, S);
  else
    sprintf(bf, f2, hkl[0], hkl[1], hkl[2], I, S, batch);
#endif
#endif
  return olxstr::FromExternal(bf.release());
}
//..............................................................................
char* TReflection::ToCBuffer(char* bf, size_t sz, double k) const {
  const char *f1 = "%4i%4i%4i%8.2lf%8.2lf";
  const char *f2 = "%4i%4i%4i%8.2lf%8.2lf%4i";
  int16_t batch = GetBatch();
#ifdef _MSC_VER
  if (batch == NoBatchSet) {
    sprintf_s(bf, sz, f1, hkl[0], hkl[1], hkl[2], I*k, S*k);
  }
  else {
    sprintf_s(bf, sz, f2, hkl[0], hkl[1], hkl[2], I*k, S*k, batch);
  }
#else
  if (batch == NoBatchSet) {
    sprintf(bf, f1, hkl[0], hkl[1], hkl[2], I*k, S*k);
  }
  else {
    sprintf(bf, f2, hkl[0], hkl[1], hkl[2], I*k, S*k, batch);
  }
#endif
  return bf;
}
//..............................................................................
void TReflection::Analyse(const SymmSpace::InfoEx& info)  {
  _reset_flags(0, 1, GetBatch());
  for( size_t i=0; i < info.matrices.Count(); i++ )  {
    vec3i hklv = hkl*info.matrices[i].r;
    if( hkl == hklv )  {
      IncMultiplicity((int)(1+info.vertices.Count()));
      if( !IsAbsent() )  {
        const double ps = info.matrices[i].t.DotProd(hkl);
        bool absent = (olx_abs( ps - olx_round(ps) ) > 0.01);
        if( !absent )  {
          for( size_t j=0; j < info.vertices.Count(); j++ )  {
            const double ps =
              (info.matrices[i].t+info.vertices[j]).DotProd(hkl);
            if( (absent = (olx_abs( ps - olx_round(ps) ) > 0.01)) )
              SetAbsent(true);
          }
        }
        else
          SetAbsent(true);
      }
    }
    else if( !info.centrosymmetric && hkl == -hklv )
      SetCentric(true);
  }
  if( info.centrosymmetric )
    SetCentric(true);
}
//..............................................................................
