#ifndef __olx_symmat
#define __olx_symmat
#include "threex3.h"

template <class MC, class VC> class TSymmMat {
  int Tag;
public:
  TSymmMat() { Tag = -1;  }
  template <class AMC, class AVC> TSymmMat(const TSymmMat<AMC,AVC>& v) : 
    r(v.r), 
    t(v.t) { Tag = -1;  }

  template <class AT> TVector3<AT>  operator * (const TVector3<AT>& a) const  {
    return TVector3<AT>( r*a ).operator +=(t);
  }
  inline bool operator == (const TSymmMat<MC,VC>& v) const {
    return r == v.r ? (t == v.t) : false;
  }

  template <class AT> inline void operator *= (AT v) {
    r *= v;
    t *= v;
  }
  inline TSymmMat<MC,VC>& I()  {
    r.I();
    t.Null();
    return *this;
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
