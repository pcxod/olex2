#ifndef __olx_symmat
#define __olx_symmat
#include "threex3.h"

template <class T> class TSymmMat {
  int Tag;
public:
  TSymmMat() { Tag = -1;  }
  template <class AT>
  TSymmMat(const TSymmMat<AT>& v) : r(v.r), t(v.t) { Tag = -1;  }

  template <class AT> TVector3<AT>  operator * (const TVector3<AT>& a) const  {
    return TVector3<AT>( r*a ).operator +=(t);
  }
  inline bool operator == (const TSymmMat<T>& v) const {
    return r == v.r ? (t == v.t) : false;
  }

  template <class AT> inline void operator *= (AT v) {
    r *= v;
    t *= v;
  }
  inline TSymmMat<T>& Null()  {
    r.Null();
    t.Null();
    return *this;
  }
  TVector3<T> t;
  TMatrix33<T> r;

  DefPropP(int, Tag)

  inline int IncTag() {  return ++Tag;  }
  inline int DecTag() {  return --Tag;  }
};

typedef TSymmMat<double>   smatd;
typedef TPtrList<smatd>    smatd_plist;
typedef TTypeList<smatd>   smatd_list;

#endif