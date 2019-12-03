/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_evector_H
#define __olx_evector_H
#include <math.h>
#include "ebase.h"
#include "typelist.h"
#include "tptrlist.h"
#include "emath.h"
#include "istream.h"
#ifdef QLength
  #undef QLength
#endif

BeginEsdlNamespace()

template <typename> class SharedVector;
template <typename> class ConstVector;

// using forward reference
template <typename> class TMatrix;

template <typename heir_t, typename FT> struct VecOp {
private:
  const heir_t &Self() const { return *static_cast<const heir_t*>(this); }
  const FT& Get_(size_t i) const { return Self()(i); }
  size_t Count_() const { return Self().Count(); }
public:
  template <typename AT>
  FT DotProd(const AT &v) const {
    return olx_vec::DotProd(Self(), v);
  }
  template <typename AT>
  FT QDistanceTo(const AT &v) const {
    return olx_vec::Distance(Self(), v);
  }
  template <typename AT>
  FT DistanceTo(const AT &v) const { return sqrt(QDistanceTo(v)); }

  FT QLength() const {
    return olx_vec::QLength(Self());
  }
  FT Length() const { return sqrt(QLength()); }

  template <typename AT>
  FT CAngle(const AT& v) const {
    FT dp = DotProd(v);
    const FT l = sqrt(QLength()*v.QLength());
    if (l == 0) {
      throw TDivException(__OlxSourceInfo);
    }
    return dp / l;
  }
};

template <typename FT> class TVector : public VecOp<TVector<FT>, FT> {
protected:
  size_t Fn;
  FT *FData;
public:
  TVector()
    : Fn(0), FData(0)
  {}

  TVector(FT a, FT b, FT c)
    : Fn(3)
  {
    FData = new FT[Fn];
    FData[0] = a;  FData[1] = b;  FData[2] = c;
  }

  TVector(FT a, FT b, FT c, FT d, FT e, FT f)
    : Fn(6)
  {
    FData = new FT[Fn];
    FData[0] = a;  FData[1] = b;  FData[2] = c;
    FData[3] = d;  FData[4] = e;  FData[5] = f;
  }

  template <typename AType> TVector(const TVector<AType>& V)
    : Fn(V.Count())
  {
    FData = new FT[Fn];
    for (size_t i = 0; i < Fn; i++) {
      FData[i] = V[i];
    }
  }

  template <typename AType> TVector(size_t size, const AType *V)
    : Fn(size)
  {
    FData = new FT[Fn];
    for (size_t i = 0; i < Fn; i++) {
      FData[i] = V[i];
    }
  }

  TVector(size_t size, const FT *V)
    : Fn(size)
  {
    FData = new FT[Fn];
    memcpy(FData, V, size * sizeof(FT));
  }

  TVector(const TVector& V)
    : Fn(V.Count())
  {
    FData = new FT[Fn];
    memcpy(FData, V.FData, Fn * sizeof(FT));
  }

  TVector(size_t ddim)
    : Fn(ddim)
  {
    FData = (Fn == 0 ? 0 : new FT[Fn]);
    Null();
  }

  TVector(const SharedVector<FT> &v) : FData(0) {
    TakeOver(v.Release(), true);
  }

  TVector(const ConstVector<FT> &v) : FData(0) {
    TakeOver(v.Release(), true);
  }

  template <typename vec_t>
  static ConstVector<FT> FromAny(const vec_t& V) {
    return TVector().Assign(V, V.Count());
  }

  virtual ~TVector() {
    olx_del_arr(FData);
  }

  TVector &TakeOver(TVector& l, bool do_delete = false) {
    olx_del_arr(FData);
    Fn = l.Fn;
    FData = l.FData;
    l.Fn = 0;
    l.FData = 0;
    if (do_delete) {
      delete &l;
    }
    return *this;
  }

  size_t Count() const { return Fn; }
  size_t Size() const { return Fn; }
  bool IsEmpty() const { return Count() == 0; }
  const FT& operator [](size_t offset) const {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, offset, 0, Fn);
#endif
    return FData[offset];
  }
  FT& operator [](size_t offset) {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, offset, 0, Fn);
#endif
    return FData[offset];
  }

  const FT& operator ()(size_t offset) const {
    return operator [] (offset);
  }
  FT& operator ()(size_t offset) { return operator [] (offset); }

  const FT* GetRawData() const { return FData; }

  FT& GetLast() const {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, Fn - 1, 0, Fn);
#endif
    return FData[Fn - 1];
  }

  template <typename vec_t>
  static vec_t& Null(vec_t& v, size_t sz) {
    for (size_t i = 0; i < sz; i++) {
      v[i] = 0;
    }
    return v;
  }
  template <typename vec_t> static vec_t& Null(vec_t& v) {
    return Null(v, v.Count());
  }

  TVector& Null() {
    memset(FData, 0, sizeof(FT)*Fn);
    return *this;
  }

  bool IsNull(FT eps = FT(1e-8)) const {
    for (size_t i = 0; i < Fn; i++) {
      if (olx_abs(FData[i]) > eps) {
        return false;
      }
    }
    return true;
  }

  int Compare(const TVector& v) const {
    if (Count() != v.Count()) {
      return olx_cmp(Count(), v.Count());
    }
    const size_t l = Count();
    for (size_t i = 0; i < l; i++) {
      if (FData[i] < v[i]) {
        return -1;
      }
      if (FData[i] > v[i]) {
        return 1;
      }
    }
    return 0;
  }

  template <typename vec_t>
  static vec_t& Normalise(vec_t& v, size_t cnt) {
    const FT l = Length(v, cnt);
    if (l == 0) {
      throw TDivException(__OlxSourceInfo);
    }
    for (size_t i = 0; i < cnt; i++) {
      v[i] /= l;
    }
    return v;
  }
  template <typename vec_t>
  static vec_t& Normalise(vec_t& v) { return Normalise(v, v.Count()); }
  TVector& Normalise() { return Normalise(*this); }

  template <typename AType> TVector& operator = (const AType& a) {
    Resize(a.Count());
    for (size_t i = 0; i < Fn; i++) {
      FData[i] = a[i];
    }
    return *this;
  }

  TVector& operator = (const TVector& a) {
    Resize(a.Count());
    memcpy(FData, a.FData, Count() * sizeof(FT));
    return *this;
  }

  TVector& operator = (const SharedVector<FT> &v) {
    return TakeOver(v.Release(), true);
  }

  TVector& operator = (const ConstVector<FT> &v) {
    return TakeOver(v.Release(), true);
  }

  template <typename VC> TVector& Assign(const VC& v, size_t size) {
    Resize(size);
    for (size_t i = 0; i < size; i++) {
      FData[i] = v[i];
    }
    return *this;
  }

  ConstVector<FT> operator + (FT a) const {
    return TVector(*this) += a;
  }

  ConstVector<FT> operator - (FT a) const {
    return TVector(*this) -= a;
  }

  ConstVector<FT> operator * (FT a) const {
    return TVector(*this) *= a;
  }

  ConstVector<FT> operator / (FT a) const {
    return TVector<FT>(*this) /= a;
  }

  TVector& operator += (FT v) {
    for (size_t i = 0; i < Fn; i++) {
      FData[i] += v;
    }
    return *this;
  }

  TVector& operator -= (FT v) {
    for (size_t i = 0; i < Fn; i++) {
      FData[i] -= v;
    }
    return *this;
  }

  TVector& operator *= (FT v) {
    for (size_t i = 0; i < Fn; i++) {
      FData[i] *= v;
    }
    return *this;
  }

  TVector& operator /= (FT v) {
    if (v == 0) {
      throw TDivException(__OlxSourceInfo);
    }
    for (size_t i = 0; i < Fn; i++) {
      FData[i] /= v;
    }
    return *this;
  }

  template <typename AType>
  ConstVector<FT> operator + (const TVector<AType>& a) const {
    return TVector(*this) += a;
  }
  template <typename AType>
  ConstVector<FT> operator - (const TVector<AType>& a) const {
    return TVector(*this) -= a;
  }
  template <typename AType>
  ConstVector<FT> operator * (const TVector<AType>& a) const {
    return TVector(*this) *= a;
  }
  template <typename AType>
  ConstVector<FT> operator / (const TVector<AType>& a) const {
    return TVector(*this) /= a;
  }

  template <typename AType> TVector& operator += (const TVector<AType>& a) {
    for (size_t i = 0; i < Fn; i++) {
      FData[i] += a[i];
    }
    return *this;
  }

  template <typename AType> TVector& operator -= (const TVector<AType>& a) {
    for (size_t i = 0; i < Fn; i++) {
      FData[i] -= a[i];
    }
    return *this;
  }

  template <typename AType> TVector& operator *= (const TVector<AType>& a) {
    for (size_t i = 0; i < Fn; i++) {
      FData[i] *= a[i];
    }
    return *this;
  }

  template <typename AType> TVector& operator /= (const TVector<AType>& a) {
    for (size_t i = 0; i < Fn; i++) {
      FData[i] /= a[i];
    }
    return *this;
  }

  /* row vector */
  template <typename AType>
  ConstVector<FT> operator * (const TMatrix<AType>& a) const {
    TVector V(a.ColCount());
    return olx_vec::MulMatrix(*this, a, V);
  }

  /* row vector */
  template <typename AType> TVector& operator *= (const TMatrix<AType>& a) {
    return (*this = (*this)*a);
  }

  template <typename AType> bool operator == (const TVector<AType>& a) const {
    if (Fn != a.Count()) {
      return false;
    }
    for (size_t i = 0; i < Fn; i++) {
      if (FData[i] != a[i]) {
        return false;
      }
    }
    return true;
  }

  // for console APP only
  void Print() const {
    for (size_t i = 0; i < Fn; i++) {
      printf("%05.4e\t", FData[i]);
    }
  }

  template <typename SC> SC StrRepr() const {
    if (Fn == 0) {
      return SC();
    }
    SC rv;
    rv << FData[0];
    for (size_t i = 1; i < Fn; i++) {
      rv << ", " << FData[i];
    }
    return rv;
  }
  TIString ToString() const { return StrRepr<olxstr>(); }
  olxcstr  ToCStr() const { return StrRepr<olxcstr>(); }
  olxwstr  ToWStr() const { return StrRepr<olxwstr>(); }
  void ToStreamSafe(IOutputStream& out) const {
    uint32_t sz = (uint32_t)Fn; //TODO: check overflow
    out.Write(&sz, sizeof(sz));
    uint8_t dt_sz = sizeof(FT);
    out.Write(&dt_sz, sizeof(dt_sz));
    out.Write(FData, sizeof(FT)*Fn);
  }
  void FromStreamSafe(IInputStream& in) {
    uint32_t sz;
    uint8_t dt_sz;
    in >> sz;
    in >> dt_sz;
    if (dt_sz != sizeof(FT)) {
      throw TInvalidArgumentException(__OlxSourceInfo, "element size");
    }
    Resize(sz);
    in.Read(FData, sizeof(FT)*sz);
  }
  // unsafe - element size is not checked, length up to 32bit
  void ToStream(IOutputStream& out) const {
    uint32_t sz = (uint32_t)Fn; //TODO: check overflow
    out.Write(&sz, sizeof(sz));
    out.Write(FData, sizeof(FT)*Fn);
  }
  // unsafe - element size is not checked
  void FromStream(IInputStream& in) {
    uint32_t sz;
    in >> sz;
    Resize(sz);
    in.Read(FData, sizeof(FT)*sz);
  }
  TVector& Resize(size_t newsize, bool strict = false) {
    if (newsize <= Fn) {
      if (strict) {
        FT* ND = new FT[newsize];
        memcpy(ND, FData, newsize * sizeof(FT));
        delete[] FData;
        FData = ND;
      }
      Fn = newsize;
    }
    else if (newsize == 0) {
      if (strict) {
        delete[] FData;
        FData = 0;
      }
      Fn = 0;
    }
    else {
      if (FData != 0) {
        FT* ND = new FT[newsize];
        memcpy(ND, FData, Fn * sizeof(FT));
        memset(&ND[Fn], 0, (newsize - Fn) * sizeof(FT));
        Fn = newsize;
        delete[] FData;
        FData = ND;
      }
      else {
        Fn = newsize;
        FData = new FT[Fn];
        Null();
      }
    }
    return *this;
  }
  TVector &Transpose() {
    if (type == vec_type_col) {
      type = vec_type_row;
    }
    else if (type == vec_type_row) {
      type = vec_type_row;
    }
    else {
      throw TInvalidArgumentException(__OlxSourceInfo, "vector type");
    }
    return *this;
  }
  // for compatibility of the template interfaces
  void SetCount(size_t v) { Resize(v); }
  //------------------------------------------------------------------------------
    // searches maximum of an array
  static FT ArrayMax(const FT* a, size_t& n, size_t sz) {
    FT b;
    b = a[0];
    n = 0;
    for (size_t i = 1; i < sz; i++) {
      if (a[i] > b) {
        b = a[i];
        n = i;
      }
    }
    return b;
  }
  // searches minimum of an array
  static FT ArrayMin(const FT* a, size_t& n, size_t sz) {
    FT b = a[0];
    n = 0;
    for (size_t i = 1; i < sz; i++) {
      if (a[i] < b) {
        b = a[i];
        n = i;
      }
    }
    return b;
  }
  // can be used to evaluate polynom value
  static FT PolynomValue(const TVector& coeff, FT arg) {
    FT c = coeff[0];
    for (size_t i = 1; i < coeff.Count(); i++) {
      c += coeff[i] * pow(arg, (FT)i);
    }
    return c;
  }
public:
  typedef FT list_item_type;
  typedef FT number_type;
  typedef ConstVector<FT> const_vec_type;
};

typedef TVector<float> evecf;
typedef TVector<double> evecd;
typedef TVector<int> eveci;
typedef TVector<size_t> evecsz;

typedef TTypeList<eveci> eveci_list;
typedef TTypeList<evecf> evecf_list;
typedef TTypeList<evecd> evecd_list;

typedef TPtrList<eveci> eveci_plist;
typedef TPtrList<evecf> evecf_plist;
typedef TPtrList<evecd> evecd_plist;

template <typename FT>
class SharedVector
  : public shared_array<TVector<FT>, FT>,
  public VecOp<SharedVector<FT>, FT>
{
  typedef TVector<FT> arr_t;
  typedef shared_array<arr_t, FT> parent_t;
public:
  SharedVector() {}
  SharedVector(const SharedVector &l) : parent_t(l) {}
  SharedVector(arr_t *arr) : parent_t(arr) {}
  SharedVector(arr_t &arr) : parent_t(arr) {}
  SharedVector &operator = (const SharedVector &l) {
    parent_t::operator = (l);
    return *this;
  }
public:
  typedef FT number_type;
};

template <typename FT>
class ConstVector
  : public const_list<TVector<FT> >,
  public VecOp<ConstVector<FT>, FT>
{
  typedef TVector<FT> vec_t;
  typedef const_list<vec_t> parent_t;
public:
  ConstVector(const ConstVector &l) : parent_t(l) {}
  ConstVector(vec_t *arr) : parent_t(arr) {}
  ConstVector(vec_t &arr) : parent_t(arr) {}
  ConstVector &operator = (const ConstVector &l) {
    parent_t::operator = (l);
    return *this;
  }
public:
  typedef FT number_type;
};
  
EndEsdlNamespace()
#endif
