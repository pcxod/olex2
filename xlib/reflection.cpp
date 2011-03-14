#include "reflection.h"

#ifdef __GNUC__
  const int TReflection::NoBatchSet;
#endif

vec3i TReflection::Standardise(const vec3i& _hkl, const SymSpace::InfoEx& info)  {
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
bool TReflection::IsAbsent(const vec3i& hkl, const SymSpace::InfoEx& info)  {
  bool absent = false;
  for( size_t i=0; i < info.matrices.Count(); i++ )  {
    vec3i hklv = hkl*info.matrices[i].r;
    if( hkl == hklv || (info.centrosymmetric && hkl == -hklv) )  {
      const double ps = info.matrices[i].t.DotProd(hkl);
      if( !(absent = (olx_abs( ps - olx_round(ps) ) > 0.01)) )  {
        for( size_t j=0; j < info.vertices.Count(); j++ )  {
          const double ps = (info.matrices[i].t+info.vertices[j]).DotProd(hkl);
          if( absent = (olx_abs( ps - olx_round(ps) ) > 0.01) )
            return true;
        }
      }
      if( absent )  return true;
    }
  }
  if( !absent )  {  // check for Identity and centering
    for( size_t i=0; i < info.vertices.Count(); i++ )  {
      const double ps = info.vertices[i].DotProd(hkl);
      if( absent = (olx_abs( ps - olx_round(ps) ) > 0.01) )
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
      Flag = Toks[6].ToInt();
    return true;
  }
  return false;
}
//..............................................................................
bool TReflection::FromNString(const olxstr& str)  {
  TStrList Toks(str, ' ');
  if( Toks.Count() > 5 )  {
    if( Toks[0].CharAt(Toks[0].Length()-1) != '.' )  return false;
    Toks[0].SetLength(Toks[0].Length()-1);
    SetTag(Toks[0].ToInt());
    SetOmitted(GetTag() < 0);
    hkl[0] = Toks[1].ToInt();
    hkl[1] = Toks[2].ToInt();
    hkl[2] = Toks[3].ToInt();
    I = Toks[4].ToDouble();
    S = Toks[5].ToDouble();
    if( Toks.Count() > 6 )
      Flag = Toks[6].ToInt();
    return true;
  }
  return false;
}
  // returns a string: h k l I S [f]
//..............................................................................
TIString TReflection::ToString() const {
  static char bf[128];
#ifdef _MSC_VER
  if( Flag == NoBatchSet )  sprintf_s(bf, 128, "%4i%4i%4i%8.2lf%8.2lf", hkl[0], hkl[1], hkl[2], I, S);
  else                     sprintf_s(bf, 128, "%4i%4i%4i%8.2lf%8.2lf%4i", hkl[0], hkl[1], hkl[2], I, S, Flag);
#else
  if( Flag == NoBatchSet )  sprintf(bf, "%4i%4i%4i%8.2lf%8.2lf", hkl[0], hkl[1], hkl[2], I, S);
  else                     sprintf(bf, "%4i%4i%4i%8.2lf%8.2lf%4i", hkl[0], hkl[1], hkl[2], I, S, Flag);
#endif
  return olxstr(bf);
}
//..............................................................................
char* TReflection::ToCBuffer(char* bf, size_t sz, double k) const {
#ifdef _MSC_VER
  if( Flag == NoBatchSet )  sprintf_s(bf, sz, "%4i%4i%4i%8.2lf%8.2lf", hkl[0], hkl[1], hkl[2], I*k, S*k);
  else                     sprintf_s(bf, sz, "%4i%4i%4i%8.2lf%8.2lf%4i", hkl[0], hkl[1], hkl[2], I*k, S*k, Flag);
#else
  if( Flag == NoBatchSet )  sprintf(bf, "%4i%4i%4i%8.2lf%8.2lf", hkl[0], hkl[1], hkl[2], I*k, S*k);
  else                     sprintf(bf, "%4i%4i%4i%8.2lf%8.2lf%4i", hkl[0], hkl[1], hkl[2], I*k, S*k, Flag);
#endif
  return bf;
}
//..............................................................................
