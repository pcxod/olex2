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
  TVector3<AT> operator * (const TVector3<AT>& a) const {
    return TVector3<AT>( r*a ).operator +=(t);
  }
  
  inline bool operator == (const TSymmMat<MC,VC>& v) const {
    return r == v.r ? (t == v.t) : false;
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
  
  TVector3<VC> t;
  TMatrix33<MC> r;

  DefPropP(int, Tag)

  inline int IncTag() {  return ++Tag;  }
  inline int DecTag() {  return --Tag;  }
};

typedef TSymmMat<int,double>   smatd;
typedef TPtrList<smatd>    smatd_plist;
typedef TTypeList<smatd>   smatd_list;
typedef TSymmMat<int,double>   smatid;
typedef TSymmMat<double,double>   smatdd;

#endif
