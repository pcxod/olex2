#ifndef __OLEX_HKL_REFLECTION__
#define __OLEX_HKL_REFLECTION__
#include "symspace.h"
BeginXlibNamespace()

const int NoFlagSet = 0xF0FA;

class TReflection: public ACollectionItem  {
  vec3i hkl;
  double I, S;
  bool Centric, Absent;
  short Multiplicity;
  int Flag;  // shelx 'batch number'
  void _init(int flag=NoFlagSet)  {
    Absent = Centric = false;
    Multiplicity = 1;
    SetTag(1);
    Flag = flag;
  }
public:
  TReflection(const TReflection& r)  {  *this = r;  }
  TReflection(int h, int k, int l) : hkl(h,k,l), I(0), S(0)  {  _init();  }
  TReflection(const vec3i& _hkl) : hkl(_hkl), I(0), S(0)  {  _init();  }
  TReflection(int h, int k, int l, double _I, double _S, int flag=NoFlagSet) :
    hkl(h,k,l), I(_I), S(_S)  {  _init(flag);  }
  TReflection(const vec3i& _hkl, double _I, double _S, int flag=NoFlagSet) :
    hkl(_hkl), I(_I), S(_S)  {  _init(flag);  }
  virtual ~TReflection()  {}
  //TReflection& operator = (const TLstRef& R);
  TReflection& operator = (const TReflection &r)  {
    hkl = r.hkl;
    I = r.I;  S = r.S;
    Centric = r.Centric;
    Absent = r.Absent;
    Multiplicity = r.Multiplicity;
    Flag = r.Flag;
    this->SetTag( r.GetTag() );
    return *this;
  }
  //bool operator == (const TReflection &r) const {  return CompareTo(r) == 0; }

  inline int GetH() const {  return hkl[0];  }
  inline void SetH(int v)   {  hkl[0] = v;  }
  inline int GetK() const {  return hkl[1];  }
  inline void SetK(int v)   {  hkl[1] = v;  }
  inline int GetL() const {  return hkl[2];  }
  inline void SetL(int v)   {  hkl[2] = v;  }

  template <class VC>
    inline bool EqHkl (const VC& v) const {
      return ( ((int)v[0] == hkl[0]) && ((int)v[1] == hkl[1]) && ((int)v[2] == hkl[2]) );
    }
  template <class VC>
    inline bool EqNegHkl (const VC& v) const {
      return ( ((int)v[0] == -hkl[0]) && ((int)v[1] == -hkl[1]) && ((int)v[2] == -hkl[2]) );
    }
  // generates symmetry equivalent miller index and stores it in res
  template <class VC, class MC> VC& MulHkl(VC& res, const MC& mat) const {
    res[0] = (hkl[0]*mat[0][0] + hkl[1]*mat[1][0] + hkl[2]*mat[2][0]);
    res[1] = (hkl[0]*mat[0][1] + hkl[1]*mat[1][1] + hkl[2]*mat[2][1]);
    res[2] = (hkl[0]*mat[0][2] + hkl[1]*mat[1][2] + hkl[2]*mat[2][2]);
    return res;
  }
  template <class MC> vec3i MulHkl(const MC& mat) const {
    return vec3i(
      hkl[0]*mat[0][0] + hkl[1]*mat[1][0] + hkl[2]*mat[2][0],
      hkl[0]*mat[0][1] + hkl[1]*mat[1][1] + hkl[2]*mat[2][1],
      hkl[0]*mat[0][2] + hkl[1]*mat[1][2] + hkl[2]*mat[2][2]);
  }
  vec3i MulHkl(const smatd& mat) const {  return MulHkl(mat.r);  }
  template <class MC> vec3i operator * (const MC& mat) const {
    return vec3i(
      hkl[0]*mat[0][0] + hkl[1]*mat[1][0] + hkl[2]*mat[2][0],
      hkl[0]*mat[0][1] + hkl[1]*mat[1][1] + hkl[2]*mat[2][1],
      hkl[0]*mat[0][2] + hkl[1]*mat[1][2] + hkl[2]*mat[2][2]);
  }
  // generates index of an equivalen reflection
  template <class VC> VC& MulHkl(VC& res, const smatd& mat) const {  return MulHkl(res, mat.r);  }
  vec3i operator * (const smatd& mat) const {
    return vec3i(
      hkl[0]*mat.r[0][0] + hkl[1]*mat.r[1][0] + hkl[2]*mat.r[2][0],
      hkl[0]*mat.r[0][1] + hkl[1]*mat.r[1][1] + hkl[2]*mat.r[2][1],
      hkl[0]*mat.r[0][2] + hkl[1]*mat.r[1][2] + hkl[2]*mat.r[2][2]);
  }
  // generates an equivalent using rounding on the resulting indexes
  template <class VC, class MC> VC& MulHklR(VC& res, const MC& mat) const {
    res[0] = olx_round(hkl[0]*mat[0][0] + hkl[1]*mat[1][0] + hkl[2]*mat[2][0]);
    res[1] = olx_round(hkl[0]*mat[0][1] + hkl[1]*mat[1][1] + hkl[2]*mat[2][1]);
    res[2] = olx_round(hkl[0]*mat[0][2] + hkl[1]*mat[1][2] + hkl[2]*mat[2][2]);
    return res;
  }
  template <class MC> vec3i MulHklR(const MC& mat) const {
    return vec3i(
      olx_round(hkl[0]*mat[0][0] + hkl[1]*mat[1][0] + hkl[2]*mat[2][0]),
      olx_round(hkl[0]*mat[0][1] + hkl[1]*mat[1][1] + hkl[2]*mat[2][1]),
      olx_round(hkl[0]*mat[0][2] + hkl[1]*mat[1][2] + hkl[2]*mat[2][2]));
  }
  // generates an equivalent using rounding on the resulting indexes
  template <class VC> VC& MulHklR(VC& res, const smatdd& mat) const {  return MulHklR(res, mat.r);  }
//..............................................................................
  /* replaces hkl with standard hkl accroding to provided matrices. 
  Also performs the reflection analysis, namely:
  Initialises Absent flag
  */
  template <class MatList> void Standardise(const MatList& ml)  {
    hkl = Standardise(hkl, ml);
    Absent = IsAbsent(hkl, ml);
  }
//..............................................................................
  template <class MatList> static vec3i Standardise(const vec3i& _hkl, const MatList& ml)  {
    vec3i new_hkl = _hkl;
    bool absent = false;
    for( size_t i=0; i < ml.Count(); i++ )  {
      vec3i hklv = _hkl*ml[i].r;
        if( (hklv[2] > new_hkl[2]) ||
          ((hklv[2] == new_hkl[2]) && (hklv[1] > new_hkl[1])) ||
          ((hklv[2] == new_hkl[2]) && (hklv[1] == new_hkl[1]) && (hklv[0] > new_hkl[0])) )
      {
        new_hkl = hklv;
      }
    }
    return new_hkl;
  }
//..............................................................................
  template <class MatList> static bool IsAbsent(const vec3i& hkl, const MatList& ml)  {
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
//..............................................................................
  void Standardise(const SymSpace::InfoEx& info)  {
    hkl = Standardise(hkl, info);
    Absent = IsAbsent(hkl, info);
  }
//..............................................................................
  static vec3i Standardise(const vec3i& _hkl, const SymSpace::InfoEx& info)  {
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
  static bool IsAbsent(const vec3i& hkl, const SymSpace::InfoEx& info)  {
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
  inline double PhaseShift(const smatd& m) const {  return m.t.DotProd(hkl);  }
//..............................................................................
  /* analyses if this reflection is centric, systematically absent and its multiplicity */
  template <class MatList> void Analyse(const MatList& ml)  {
    vec3i hklv;
    Multiplicity = 1;
    Centric = false;
    Absent = false;
    for( size_t i=0; i < ml.Count(); i++ )  {
      MulHkl(hklv, ml[i]);
      if( EqHkl(hklv) )  {  // symmetric reflection
        IncMultiplicity();
        if( !Absent )  {
          const double l = PhaseShift(ml[i]);
          Absent = (olx_abs( l - olx_round(l) ) > 0.01);
        }
      }
      else if( !Centric && EqNegHkl(hklv) )  // centric reflection
        Centric = true;
    }
  }
//..............................................................................
  inline bool IsSymmetric(const smatd& m) const {  return EqHkl(MulHkl(m.r));  }
//..............................................................................
  inline bool IsCentrosymmetric(const smatd& m) const {  return EqNegHkl(MulHkl(m.r));  }
//..............................................................................
  template <class MatList> bool IsAbsent(const MatList& ml) const {
    for( size_t i=0; i < ml.Count(); i++ )  {
      if( IsSymmetric(ml[i]) )  {
        const double l = PhaseShift(ml[i]);
        if( olx_abs(l - olx_round(l)) > 0.01 )  
          return true;
      }
    }
    return false;
  }
//..............................................................................
  int CompareTo(const TReflection &r) const {
    int res = Flag - r.Flag;  // prioritise by batch number
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
  static int CompareP(const TReflection *a, const TReflection *b)  {
    return a->CompareTo(*b);
  }
  // could be list of pointers or list of const pointers
  template <class RefPList> static void SortList(RefPList& lst)  {  
    lst.QuickSorter.SortSF(lst, &TReflection::CompareP);  
  }
//..............................................................................
  // these values are intialised by Analyse
  DefPropBIsSet(Centric)
  DefPropBIsSet(Absent)
  DefPropP(short, Multiplicity)
  inline void IncMultiplicity()  {  Multiplicity++;  }
//..............................................................................
  DefPropP(int, Flag)
//..............................................................................
  vec3i& GetHkl()  {  return hkl;  }
  const vec3i& GetHkl() const {  return hkl;  }
  TReflection& SetHkl(const vec3i& _hkl)  {  hkl = _hkl;  return *this;  }
  
  DefPropP(double, I)
  DefPropP(double, S)
//..............................................................................
  // returns a string: h k l I S [f]
  TIString ToString() const {
    static char bf[128];
#ifdef _MSC_VER
    if( Flag == NoFlagSet )  sprintf_s(bf, 128, "%4i%4i%4i%8.2lf%8.2lf", hkl[0], hkl[1], hkl[2], I, S);
    else                     sprintf_s(bf, 128, "%4i%4i%4i%8.2lf%8.2lf%4i", hkl[0], hkl[1], hkl[2], I, S, Flag);
#else
    if( Flag == NoFlagSet )  sprintf(bf, "%4i%4i%4i%8.2lf%8.2lf", hkl[0], hkl[1], hkl[2], I, S);
    else                     sprintf(bf, "%4i%4i%4i%8.2lf%8.2lf%4i", hkl[0], hkl[1], hkl[2], I, S, Flag);
#endif
    return olxstr(bf);
  }
  //writes string to the provided buffer (should be at least 29 bytes long)
  char* ToCBuffer(char* bf, size_t sz, double k) const {
#ifdef _MSC_VER
    if( Flag == NoFlagSet )  sprintf_s(bf, sz, "%4i%4i%4i%8.2lf%8.2lf", hkl[0], hkl[1], hkl[2], I*k, S*k);
    else                     sprintf_s(bf, sz, "%4i%4i%4i%8.2lf%8.2lf%4i", hkl[0], hkl[1], hkl[2], I*k, S*k, Flag);
#else
    if( Flag == NoFlagSet )  sprintf(bf, "%4i%4i%4i%8.2lf%8.2lf", hkl[0], hkl[1], hkl[2], I*k, S*k);
    else                     sprintf(bf, "%4i%4i%4i%8.2lf%8.2lf%4i", hkl[0], hkl[1], hkl[2], I*k, S*k, Flag);
#endif
    return bf;
  }
//..............................................................................
  // return a string like: tag. h k l I S [f]
  olxstr ToNString() const {
    olxstr Str(GetTag(), 80);
    Str << '.';
    Str.Format(7, true, ' ');
    return Str << ToString();
  }
//..............................................................................
  bool FromString(const olxstr& Str)  {
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
  bool FromNString(const olxstr& str)  {
    TStrList Toks(str, ' ');
    if( Toks.Count() > 5 )  {
      if( Toks[0].CharAt(Toks[0].Length()-1) != '.' )  return false;
      Toks[0].SetLength(Toks[0].Length()-1);

      SetTag(Toks[0].ToInt());

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
  // a top diagonal matrix is expected
  vec3d ToCart(const mat3d& m) const {  return ToCart(hkl, m);  }
  static vec3d ToCart(const vec3d& hkl, const mat3d& m) {
    return vec3d(
      hkl[0]*m[0][0],
      hkl[0]*m[0][1] + hkl[1]*m[1][1],
      hkl[0]*m[0][2] + hkl[1]*m[1][2] + hkl[2]*m[2][2]
    );
  }

  template <class Ref> static double GetF(const Ref& _r) {
    const TReflection& r = olx_get_ref(_r);
    return r.GetI() <= 0 ? 0 : sqrt(r.GetI());
  }
  static double GetF(const double& _f) {  return _f;  }
  template <class Ref> static double GetFsq(const Ref& _r) {
    return olx_get_ref(_r).GetI();
  }
  static double GetFsq(const double& _fsq) {  return _fsq;  }
  template <class Ref> static const vec3i& GetHkl(const Ref& _r) {
    return olx_get_ref(_r).GetHkl();
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
