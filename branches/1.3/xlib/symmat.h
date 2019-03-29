/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_symmat_H
#define __olx_xl_symmat_H
#include "threex3.h"
#include "testsuit.h"

template <class MC, class VC> class TSymmMat {
  uint32_t Id;
public:
  TMatrix33<MC> r;
  TVector3<VC> t;

  TSymmMat() : Id(~0) {}
  // copy constructor
  TSymmMat(const TSymmMat& v) :
    Id(v.Id),
    r(v.r),
    t(v.t)
  {}
  template <typename AMC, typename AVC>
  TSymmMat(const TSymmMat<AMC,AVC>& v) :
    Id(v.GetId()),
    r(v.r),
    t(v.t)
  {}
  // composing constructor
  template <typename AMC, typename AVC>
  TSymmMat(const TMatrix33<AMC>& m, const TVector3<AVC>& v)
    : Id(~0), r(m), t(v) {}

  template <typename AMC>
  TSymmMat(const TMatrix33<AMC>& m)
    : Id(~0), r(m)
  {}

  template <class AT>
  TVector3<VC> operator * (const TVector3<AT>& a) const {
    return TVector3<VC>(r*a).operator +=(t);
  }

  TVector3<VC> operator * (const TVector3<VC>& a) const {
    return TVector3<VC>(r*a).operator +=(t);
  }

  TSymmMat operator * (const TSymmMat& v) const {
    return TSymmMat(r*v.r, r*v.t+t);
  }

  TSymmMat& operator *= (const TSymmMat& v)  {
    r *= v.r;
    t = r*v.t+t;
    return *this;
  }

  //returns the transltion part of the multiplication
  TVector3<VC> MulT(const TSymmMat& v) const {  return r*v.t+t;  }

  bool operator == (const TSymmMat& v) const {
    return (r == v.r && t == v.t);
  }
  bool operator != (const TSymmMat& v) const {
    return !this->operator == (v);
  }
  /* compares rotational part directly, but does distance comparison for
  translation to prevent rounding errors influence
  */
  bool Equals(const TSymmMat& v, VC eps=1e-3) const {
    return (r == v.r && t.Equals(v.t, eps));
  }

  TSymmMat& operator = (const TSymmMat& sm)  {
    t = sm.t;
    r = sm.r;
    Id = sm.Id;
    return *this;
  }

  template <class AMC, class AVC>
  TSymmMat& operator = (const TSymmMat<AMC,AVC>& sm)  {
    t = sm.t;
    r = sm.r;
    Id = sm.GetId();
    return *this;
  }

  template <class AT> TSymmMat& operator *= (AT v) {
    r *= v;
    t *= v;
    return *this;
  }

  TSymmMat Negate() const {
    return TSymmMat(*this) *= -1;
  }

  TSymmMat& I()  {
    r.I();
    t.Null();
    return *this;
  }
  bool IsI() const  {
    return (r.IsI() && t.QLength() < 1e-6);
  }
  static TSymmMat Identity()  { return  TSymmMat(TMatrix33<MC>::Idenity()); }

  TSymmMat& Null()  {
    r.Null();
    t.Null();
    return *this;
  }

  TSymmMat Inverse() const  {
    TSymmMat rv(r.Inverse(), t*-1);
    rv.t = rv.r * rv.t;
    return rv;
  }

  static TSymmMat& Inverse(TSymmMat& m)  {
    m.r = m.r.Inverse();
    m.t = ((m.r*m.t) *= -1);
    return m;
  }

  uint32_t GetId() const { return Id;  }
  void SetId(uint8_t id)  {  Id = ((uint32_t)id << 24)|(0x00808080);  }
  void SetRawId(uint32_t id)  {  Id = id;  }
  void SetId(uint8_t id, int8_t ta, int8_t tb, int8_t tc)  {
    Id = ((uint32_t)id << 24)|
         ((uint32_t)(0x80-ta) << 16)|
         ((uint32_t)(0x80-tb) << 8)|
         (uint32_t)(0x80-tc);
  }
  void SetId(uint8_t id, const vec3i& t)  {  Id = GenerateId(id, t);  }

  bool IsFirst() const {  return IsFirst(Id);  }
  static bool IsFirst(uint32_t id) {  return id == 0x00808080;  }
  uint8_t GetContainerId() const {  return (uint8_t)(Id >> 24);  }

  static uint8_t GetContainerId(uint32_t id) {  return (uint8_t)(id >> 24);  }
  static int8_t GetTx(uint32_t id) {
    return (int8_t)(128-(int8_t)((id&0x00FF0000) >> 16));
  }
  static int8_t GetTy(uint32_t id) {
    return (int8_t)(128-(int8_t)((id&0x0000FF00) >> 8));
  }
  static int8_t GetTz(uint32_t id) {
    return (int8_t)(128-(int8_t)(id&0x000000FF));
  }
  static vec3i GetT(uint32_t id)  {
    return vec3i(GetTx(id), GetTy(id), GetTz(id));
  }
  static TSymmMat FromId(uint32_t id, const TSymmMat& ref)  {
    TSymmMat rv(ref);
    rv.t[0] += GetTx(id);
    rv.t[1] += GetTy(id);
    rv.t[2] += GetTz(id);
    rv.Id = id;
    return rv;
  }
  static uint32_t GenerateId(uint8_t id, int8_t ta, int8_t tb, int8_t tc) {
    return ((uint32_t)id<<24)|((uint32_t)(0x80-ta)<<16)|
           ((uint32_t)(0x80-tb)<<8)|(uint32_t)(0x80-tc);
  }
  static uint32_t GenerateId(uint8_t id, const vec3i& t) {
    return ((uint32_t)id<<24)|((uint32_t)(0x80-t[0])<<16)|
           ((uint32_t)(0x80-t[1])<<8)|(uint32_t)(0x80-t[2]);
  }
  static uint32_t GenerateId(uint8_t container_id, const TSymmMat& m,
    const TSymmMat& ref)
  {
    vec3i t(ref.t - m.t);
    uint32_t rv = ((uint32_t)container_id << 24);
    rv |= ((uint32_t)(0x80-t[0]) << 16);
    rv |= ((uint32_t)(0x80-t[1]) << 8);
    rv |= (uint32_t)(0x80-t[2]);
    return rv;
  }
  // normalises a fractional coordinate/translation
  static vec3d NormaliseFractional(const vec3d &t_) {
    vec3d t = t_;
    for (int j=0; j < 3; j++) {
      t[j] -= olx_floor(t[j]);
      if (t[j] < 1e-3 || t[j] > 0.999) t[j] = 0;
    }
    return t;
  }
  /* returns a vector with smaller or equal values */
  template <class list_t, class accessor_t>
  static vec3d StandardiseFractional(const vec3d &x, const list_t& ml,
    const accessor_t &acc)
  {
    vec3d v = NormaliseFractional(x);
    for( size_t i=0; i < ml.Count(); i++ )  {
      vec3d tmp = NormaliseFractional(acc(ml[i])*x);
      if( (tmp[0] < v[0]) ||  // standardise then ...
        (olx_abs(tmp[0]-v[0]) < 1e-3 && (tmp[1] < v[1])) ||
        (olx_abs(tmp[0]-v[0]) < 1e-3 &&
         olx_abs(tmp[1]-v[1]) < 1e-3 && (tmp[2] < v[2])) )
      {
        v = tmp;
      }
    }
    return v;
  }
  template <class list_t>
  static vec3d StandardiseFractional(const vec3d &x, const list_t& ml) {
    return StandardiseFractional(x, ml, ListAccessor(ml));
  }

  struct IdComparator {
    static int Compare(const TSymmMat& a, const TSymmMat& b)  {
      return olx_cmp(a.GetId(), b.GetId());
    }
    static int Compare(const TSymmMat* a, const TSymmMat* b)  {
      return olx_cmp(a->GetId(), b->GetId());
    }
  };
  struct ContainerIdComparator {
    static int Compare(const TSymmMat& a, const TSymmMat& b)  {
      return olx_cmp(a.GetContainerId(), b.GetContainerId());
    }
  };
};

typedef TSymmMat<int,double>   smatd;
typedef TPtrList<smatd>    smatd_plist;
typedef TTypeList<smatd>   smatd_list;
typedef TSymmMat<int,double>   smatid;
typedef TSymmMat<double,double>   smatdd;

#endif
