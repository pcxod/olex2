#ifndef __olx_symmat
#define __olx_symmat
#include "threex3.h"

template <class MC, class VC> class TSymmMat {
  int Tag;
public:
  TSymmMat() : Tag(-1) {  }
  // copy constructor
  template <typename AMC, typename AVC> 
  TSymmMat(const TSymmMat<AMC,AVC>& v) : 
    r(v.r), 
    t(v.t),
    Tag(-1) {  }
  // composing constructor
  template <typename AMC, typename AVC> 
  TSymmMat(const TMatrix33<AMC>& m, const TVector3<AVC>& v) :
    r(m), t(v), Tag(-1) {  }

  template <class AT> 
  inline TVector3<VC> operator * (const TVector3<AT>& a) const {
    return TVector3<VC>( r*a ).operator +=(t);
  }
  
  inline TSymmMat<MC,VC> operator * (const TSymmMat<MC,VC>& v) const {
    return TSymmMat<MC,VC>(r*v.r, v*t);
  }

  inline TSymmMat<MC,VC>& operator *= (const TSymmMat<MC,VC>& v)  {
    r *= v.r;
    t = v * t;
    return *this;
  }

  inline bool operator == (const TSymmMat<MC,VC>& v) const {
    return (r == v.r && t == v.t);
  }
  /* compares rotational part directly, but does distance comparison for translation 
  to prevent rounding errors influence*/
  bool EqualExt(const TSymmMat<MC,VC>& v) const {
    return (r == v.r && t.QDistanceTo(v.t) < 1e-6);
  }
  
  inline TSymmMat& operator = (const TSymmMat& sm)  {
    t = sm.t;
    r = sm.r;
    Tag = sm.Tag;
    return *this;
  }

  template <class AMC, class AVC> 
  inline TSymmMat& operator = (const TSymmMat<AMC,AVC>& sm)  {
    t = sm.t;
    r = sm.r;
    Tag = sm.Tag;
    return *this;
  }
  
  template <class AT> 
  inline void operator *= (AT v) {
    r *= v;
    t *= v;
  }
  
  inline TSymmMat<MC,VC>& I()  {
    r.I();
    t.Null();
    return *this;
  }
  inline bool IsI() const  {
    return (r.IsI() && t.QLength() < 1e-6);
  }
  
  inline TSymmMat<MC,VC>& Null()  {
    r.Null();
    t.Null();
    return *this;
  }
  
  inline TSymmMat<MC,VC> Inverse() const  {
    TSymmMat<MC,VC> rv(r.Inverse(), t*-1);
    rv.t = rv.r * rv.t;
    return rv;
  }
  
  static inline TSymmMat<MC,VC>& Inverse(TSymmMat<MC,VC>& m)  {
    m.r = m.r.Inverse();
    m.t = ((m.r*m.t) *= -1);
    return m;
  }

  TVector3<VC> t;
  TMatrix33<MC> r;

  DefPropP(int, Tag)

  inline int IncTag() {  return ++Tag;  }
  inline int DecTag() {  return --Tag;  }

  // Tag dependent reference operations
  struct Ref  {
    int tag;
    TVector3<VC> t;
    Ref(int _tag, const TVector3<VC>& _t) : tag(_tag), t(_t) {}
    Ref(const Ref& r) : tag(r.tag), t(r.t) {}
    Ref& operator = (const Ref& r)  {
      tag = r.tag;
      t = r.t;
      return *this;
    }
  };
  Ref GetRef() const {  return Ref(Tag, t);  }
  bool operator == (const Ref& ref) const  {
    return (Tag == ref.tag && t.QDistanceTo(ref.t) < 1e-6);
  }
};

typedef TSymmMat<int,double>   smatd;
typedef TPtrList<smatd>    smatd_plist;
typedef TTypeList<smatd>   smatd_list;
typedef TSymmMat<int,double>   smatid;
typedef TSymmMat<double,double>   smatdd;

#endif
