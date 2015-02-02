/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xlib_reflection
#define __olx_xlib_reflection
#include "symmspace.h"
BeginXlibNamespace()

class TReflection: public ACollectionItem  {
public:
  static const int16_t NoBatchSet = 0xFE;
  static const uint32_t
    FlagMask  = 0x000000FF, FlagLen  = 8,
    MultMask  = 0x0000FF00, MultLen  = 8, MultOff = FlagLen,
    BatchMask = 0xFFFF0000, BatchLen = 16, BatchOff = FlagLen+MultLen;
  static const uint16_t
    bitCentric = 0x0001,
    bitAbsent  = 0x0002,
    bitOmitted = 0x0004;
private:
  vec3i hkl;
  double I, S;
  // first 8 bits - flags, next 8 - multiplicity, then batch number
  uint32_t Flags;
  void _init(int batch=NoBatchSet)  {
    _reset_flags(0, 1, batch);
    SetTag(1);
  }
  void _reset_flags(int flags, int mult, int batch)  {
    Flags = flags|(mult<<MultOff)|(batch<<BatchOff);
  }
public:
  TReflection(const TReflection& r)  {  *this = r;  }
  TReflection(const TReflection& r, int bacth_n)  {
    *this = r;
    SetBatch(bacth_n);
  }
  TReflection(const TReflection& r, const vec3i& _hkl, int batch_n=NoBatchSet)
  {
    *this = r;
    hkl = _hkl;
    SetBatch(batch_n);
  }
  TReflection(int h, int k, int l) : hkl(h,k,l), I(0), S(0)  {  _init();  }
  TReflection(const vec3i& _hkl) : hkl(_hkl), I(0), S(0)  {  _init();  }
  TReflection(int h, int k, int l, double _I, double _S, int batch=NoBatchSet)
    : hkl(h,k,l), I(_I), S(_S)  {  _init(batch);  }
  TReflection(const vec3i& _hkl, double _I, double _S, int batch=NoBatchSet)
    : hkl(_hkl), I(_I), S(_S)  {  _init(batch);  }
  virtual ~TReflection()  {}
  TReflection& operator = (const TReflection &r)  {
    hkl = r.hkl;
    I = r.I;
    S = r.S;
    Flags = r.Flags;
    SetTag(r.GetTag());
    return *this;
  }
  inline int GetH() const {  return hkl[0];  }
  inline void SetH(int v)   {  hkl[0] = v;  }
  inline int GetK() const {  return hkl[1];  }
  inline void SetK(int v)   {  hkl[1] = v;  }
  inline int GetL() const {  return hkl[2];  }
  inline void SetL(int v)   {  hkl[2] = v;  }

  template <class VC>
  bool EqHkl(const VC& v) const {
    return ((int)v[0] == hkl[0]) && ((int)v[1] == hkl[1]) &&
            ((int)v[2] == hkl[2]);
  }
  template <class VC>
   bool EqNegHkl (const VC& v) const {
    return ((int)v[0] == -hkl[0]) && ((int)v[1] == -hkl[1]) &&
      ((int)v[2] == -hkl[2]);
   }
  template <class VC, class MC>
  static VC MulHkl(const VC& hkl, const MC& mat) {
    return VC((hkl[0]*mat[0][0] + hkl[1]*mat[1][0] + hkl[2]*mat[2][0]),
      (hkl[0]*mat[0][1] + hkl[1]*mat[1][1] + hkl[2]*mat[2][1]),
      (hkl[0]*mat[0][2] + hkl[1]*mat[1][2] + hkl[2]*mat[2][2]));
  }
  static vec3i MulHkl(const vec3i& hkl, const smatd& mat) {
    return MulHkl(hkl, mat.r);
  }
  // generates an equivalent using rounding on the resulting indexes
  template <class VC, class MC>
  static vec3i MulHklR(const VC &hkl, const MC& mat) {
    return vec3i(
      olx_round(hkl[0]*mat[0][0] + hkl[1]*mat[1][0] + hkl[2]*mat[2][0]),
      olx_round(hkl[0]*mat[0][1] + hkl[1]*mat[1][1] + hkl[2]*mat[2][1]),
      olx_round(hkl[0]*mat[0][2] + hkl[1]*mat[1][2] + hkl[2]*mat[2][2]));
  }
  vec3i operator * (const smatd& mat) const { return MulHkl(hkl, mat.r); }
  template <class MC> vec3i operator * (const MC& mat) const {
    return MulHkl(hkl, mat);
  }
  template <class MC> vec3i MulHklR(const MC& mat) const {
    return MulHklR(hkl, mat);
  }
  /* replaces hkl with standard hkl accroding to provided matrices.
  Also performs the reflection analysis, namely:
  Initialises Absent flag
  */
  template <class MatList> void Standardise(const MatList& ml)  {
    hkl = Standardise(hkl, ml);
    SetAbsent(IsAbsent(hkl, ml));
  }
//..............................................................................
  template <class MatList> static vec3i Standardise(const vec3i& _hkl,
    const MatList& ml)
  {
    vec3i new_hkl = _hkl;
    for( size_t i=0; i < ml.Count(); i++ )  {
      vec3i hklv = _hkl*ml[i].r;
        if( (hklv[2] > new_hkl[2]) ||
          ((hklv[2] == new_hkl[2]) && (hklv[1] > new_hkl[1])) ||
          ((hklv[2] == new_hkl[2]) && (hklv[1] == new_hkl[1]) &&
           (hklv[0] > new_hkl[0])) )
      {
        new_hkl = hklv;
      }
    }
    return new_hkl;
  }
//..............................................................................
  template <class MatList> static bool IsAbsent(const vec3i& hkl,
    const MatList& ml)
  {
    for( size_t i=0; i < ml.Count(); i++ )  {
      vec3i hklv = hkl*ml[i].r;
      if( hkl == hklv )  {  // only if there is no change
        const double ps = ml[i].t.DotProd(hkl);
        if( olx_abs( ps - olx_round(ps) ) > 0.01 )
          return true;
      }
    }
    return false;
  }
//..............................................................................
  template <class MatList> bool IsAbsent(const MatList& ml) const {
    return IsAbsent(hkl, ml);
  }
//..............................................................................
  void Standardise(const SymmSpace::InfoEx& info)  {
    hkl = Standardise(hkl, info);
    SetAbsent(IsAbsent(hkl, info));
  }
//..............................................................................
  static vec3i Standardise(const vec3i& _hkl, const SymmSpace::InfoEx& info);
//..............................................................................
  static bool IsAbsent(const vec3i& hkl, const SymmSpace::InfoEx& info);
//..............................................................................
  inline double PhaseShift(const smatd& m) const {  return m.t.DotProd(hkl);  }
//..............................................................................
  /* analyses if this reflection is centric, systematically absent and its
  multiplicity
  */
  template <class MatList> void Analyse(const MatList& ml)  {
    _reset_flags(0, 1, GetBatch());
    for( size_t i=0; i < ml.Count(); i++ )  {
      vec3i hklv = MulHkl(hkl, ml[i]);
      if (EqHkl(hklv)) {  // symmetric reflection
        IncMultiplicity();
        if (!IsAbsent()) {
          const double l = PhaseShift(ml[i]);
          SetAbsent(olx_abs(l - olx_round(l)) > 0.01);
        }
      }
      else if (!IsCentric() && EqNegHkl(hklv))  // centric reflection
        SetCentric(true);
    }
  }
//..............................................................................
  void Analyse(const SymmSpace::InfoEx& info);
//..............................................................................
  bool IsSymmetric(const smatd& m) const {
    return EqHkl(MulHkl(hkl, m));
  }
//..............................................................................
  bool IsCentrosymmetric(const smatd& m) const {
    return EqNegHkl(MulHkl(hkl, m));
  }
//..............................................................................
  int CompareTo(const TReflection &r) const {
    int res = GetBatch() - r.GetBatch();  // prioritise by batch number
    if( res == 0 )  {
      res = hkl[2] - r.hkl[2];
      if( res == 0 )  {
        res = hkl[1] - r.hkl[1];
        if( res == 0 )
          res = hkl[0] - r.hkl[0];
      }
    }
    return res;
  }
  static int Compare(const TReflection &a, const TReflection &b)  {
    return a.CompareTo(b);
  }
  // could be list of pointers or list of const pointers
  template <class RefPList> static void SortList(RefPList& lst)  {
    QuickSorter::SortSF(lst, &TReflection::Compare);
  }
  //..............................................................................
  int CompareToIndices(const TReflection &r) const {
    int res = hkl[2] - r.hkl[2];
    if (res == 0)  {
      res = hkl[1] - r.hkl[1];
      if (res == 0)
        res = hkl[0] - r.hkl[0];
    }
    return res;
  }
  static int CompareIndices(const TReflection &a, const TReflection &b)  {
    return a.CompareToIndices(b);
  }
  // could be list of pointers or list of const pointers
  template <class RefPList> static void SortListByIndices(RefPList& lst)  {
    QuickSorter::SortSF(lst, &TReflection::CompareIndices);
  }
  //..............................................................................
  bool operator == (const vec3i &hkl_) const {
    return hkl == hkl_;
  }
//..............................................................................
  // these values are intialised by Analyse
  DefPropBFIsSet(Centric, Flags, bitCentric)
  DefPropBFIsSet(Absent, Flags, bitAbsent)
  DefPropBFIsSet(Omitted, Flags, bitOmitted)
//..............................................................................
  uint8_t GetMultiplicity() const {
    return (uint8_t)((Flags&0xff00)>>MultOff);
  }
  void SetMultiplicity(uint8_t v)  {
    Flags = (Flags&~MultMask)|((uint32_t)v<<MultOff);
  }
  void IncMultiplicity(int v=1)  {  SetMultiplicity(GetMultiplicity()+v);  }
//..............................................................................
  int16_t GetBatch() const {  return (int16_t)((Flags&BatchMask)>>BatchOff);  }
  void SetBatch(int16_t v)  {
    Flags = (Flags&~BatchMask)|((uint32_t)v<<BatchOff);
  }
  bool IsBatchSet() const { return GetBatch() != NoBatchSet; }
//..............................................................................
  vec3i& GetHkl()  {  return hkl;  }
  const vec3i& GetHkl() const {  return hkl;  }
  TReflection& SetHkl(const vec3i& _hkl)  {  hkl = _hkl;  return *this;  }
  DefPropP(double, I)
  DefPropP(double, S)
//..............................................................................
  // returns a string: h k l I S [f]
  TIString ToString() const;
  //writes string to the provided buffer (should be at least 29 bytes long)
  char* ToCBuffer(char* bf, size_t sz, double k) const;
//..............................................................................
  // return a string like: tag. h k l I S [f]
  olxstr ToNString() const {
    olxstr Str(IsOmitted() ? '-' : '+', 80);
    Str << olx_abs(GetTag()) << '.';
    return (Str.RightPadding(10, ' ', true) << ToString());
  }
//..............................................................................
  bool FromString(const olxstr& Str);
  bool FromNString(const olxstr& str);
  // a top diagonal matrix is expected
  vec3d ToCart(const mat3d& m) const {  return ToCart(hkl, m);  }
//..............................................................................
  static vec3d ToCart(const vec3d& hkl, const mat3d& m) {
    return vec3d(
      hkl[0]*m[0][0],
      hkl[0]*m[0][1] + hkl[1]*m[1][1],
      hkl[0]*m[0][2] + hkl[1]*m[1][2] + hkl[2]*m[2][2]
    );
  }
//..............................................................................
  template <class Ref> static double GetF(const Ref& _r) {
    const TReflection& r = olx_ref::get(_r);
    return r.GetI() <= 0 ? 0 : sqrt(r.GetI());
  }
  static double GetF(const double& _f) {  return _f;  }
  template <class Ref> static double GetFsq(const Ref& _r) {
    return olx_ref::get(_r).GetI();
  }
  static double GetFsq(const double& _fsq) {  return _fsq;  }
  template <class Ref> static const vec3i& GetHkl(const Ref& _r) {
    return olx_ref::get(_r).GetHkl();
  }
  static const vec3i& GetHkl(const vec3i& _hkl) {  return _hkl;  }
  static vec3i& GetHkl(vec3i& _hkl) {  return _hkl;  }
};

class IMillerIndexList {
public:
  virtual size_t Count() const = 0;
  virtual vec3i operator [] (size_t i) const = 0;
};

// generates miller indices in given range
class MillerIndexArray : public IMillerIndexList {
  vec3i min_i, max_i;
  size_t h_sz, hk_sz, hkl_sz;
public:
  MillerIndexArray(const vec3i& mi, const vec3i& mx) : min_i(mi), max_i(mx) {
    h_sz = (mx[0]-mi[0]+1);
    hk_sz = h_sz*(mx[1]-mi[1]+1);
    hkl_sz = hk_sz*(mx[2]-mi[2]+1);
  }
  size_t Count() const {  return hkl_sz;  }
  vec3i operator [] (size_t i) const {
    size_t pi = i/hk_sz, ipi = i%hk_sz;
    return vec3i(
      min_i[0]+(int)(ipi%h_sz),
      min_i[1]+(int)(ipi/h_sz),
      min_i[2]+(int)pi);
  }
};
// could be wrapper around list of vec3i or TReflection
template <class RefList> class MillerIndexList : public IMillerIndexList {
  const RefList& src;
public:
  MillerIndexList(const RefList& r) : src(r)  {}
  size_t Count() const {  return src.Count();  }
  vec3i operator [] (size_t i) const {  return TReflection::GetHkl(src[i]); }
};

typedef TPtrList<TReflection> TRefPList;
typedef TTypeList<TReflection> TRefList;

EndXlibNamespace()

#endif
