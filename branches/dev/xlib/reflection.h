#ifndef __OLEX_HKL_REFLECTION__
#define __OLEX_HKL_REFLECTION__

BeginXlibNamespace()

const int NoFlagSet = 0xF0FA;

class TReflection: public ACollectionItem  {
  vec3i hkl;
  double I, S;
  bool Centric, Absent;
  short Multiplicity;
  int Flag;  // shelx 'batch number'
public:
  TReflection(const TReflection& r)  {
    *this = r;
  }
  TReflection(int h, int k, int l) : hkl(h,k,l)  {
    I = S = 0;
    Absent = Centric = false;
    Multiplicity = 1;
    SetTag(1);
    Flag = NoFlagSet;
  }
  TReflection(int h, int k, int l, double I, double S, int flag=NoFlagSet) : hkl(h,k,l) {
    this->I = I;
    this->S = S;
    Absent = Centric = false;
    Multiplicity = 1;
    SetTag(1);
    Flag = flag;
  }
  virtual ~TReflection()  {  }
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

  inline int GetH()  const  {  return hkl[0];  }
  inline void SetH(int v)   {  hkl[0] = v;  }
  inline int GetK()  const  {  return hkl[1];  }
  inline void SetK(int v)   {  hkl[1] = v;  }
  inline int GetL()  const  {  return hkl[2];  }
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
  template <class VC, class MC> void MulHkl(VC& res, const MC& mat) const {
    res[0] = (hkl[0]*mat[0][0] + hkl[1]*mat[1][0] + hkl[2]*mat[2][0]);
    res[1] = (hkl[0]*mat[0][1] + hkl[1]*mat[1][1] + hkl[2]*mat[2][1]);
    res[2] = (hkl[0]*mat[0][2] + hkl[1]*mat[1][2] + hkl[2]*mat[2][2]);
  }
  template <class MC> vec3i operator * (const MC& mat) const {
    return vec3i(
      (int)(hkl[0]*mat[0][0] + hkl[1]*mat[1][0] + hkl[2]*mat[2][0]),
      (int)(hkl[0]*mat[0][1] + hkl[1]*mat[1][1] + hkl[2]*mat[2][1]),
      (int)(hkl[0]*mat[0][2] + hkl[1]*mat[1][2] + hkl[2]*mat[2][2]));
  }
  // generates index of an equivalen reflection
  template <class V> void MulHkl(V& res, const smatd& mat) const {
    res[0] = (hkl[0]*mat.r[0][0] + hkl[1]*mat.r[1][0] + hkl[2]*mat.r[2][0]);
    res[1] = (hkl[0]*mat.r[0][1] + hkl[1]*mat.r[1][1] + hkl[2]*mat.r[2][1]);
    res[2] = (hkl[0]*mat.r[0][2] + hkl[1]*mat.r[1][2] + hkl[2]*mat.r[2][2]);
  }
  vec3i operator * (const smatd& mat) const {
    return vec3i(
      hkl[0]*mat.r[0][0] + hkl[1]*mat.r[1][0] + hkl[2]*mat.r[2][0],
      hkl[0]*mat.r[0][1] + hkl[1]*mat.r[1][1] + hkl[2]*mat.r[2][1],
      hkl[0]*mat.r[0][2] + hkl[1]*mat.r[1][2] + hkl[2]*mat.r[2][2]);
  }
  // generates an equivalent using rounding on the resulting indexes
  template <class VC, class MC> void MulHklR(VC& res, const MC& mat) const {
    res[0] = olx_round(hkl[0]*mat[0][0] + hkl[1]*mat[1][0] + hkl[2]*mat[2][0]);
    res[1] = olx_round(hkl[0]*mat[0][1] + hkl[1]*mat[1][1] + hkl[2]*mat[2][1]);
    res[2] = olx_round(hkl[0]*mat[0][2] + hkl[1]*mat[1][2] + hkl[2]*mat[2][2]);
  }
  // generates an equivalent using rounding on the resulting indexes
  template <class V> void MulHklR(V& res, const smatdd& mat) const {
    res[0] = olx_round(hkl[0]*mat.r[0][0] + hkl[1]*mat.r[1][0] + hkl[2]*mat.r[2][0]);
    res[1] = olx_round(hkl[0]*mat.r[0][1] + hkl[1]*mat.r[1][1] + hkl[2]*mat.r[2][1]);
    res[2] = olx_round(hkl[0]*mat.r[0][2] + hkl[1]*mat.r[1][2] + hkl[2]*mat.r[2][2]);
  }
//..............................................................................
  /* replaces hkl with standard hkl accroding to provided matrices. 
  Also performs the reflection analysis, namely:
  Initialises Absent flag
  */
  template <class MatList> void StandardiseFP(const MatList& ml)  {
    vec3i hklv;
    Absent = false;
    bool changes = true;
    while( changes )  {
      changes = false;
      for( size_t i=0; i < ml.Count(); i++ )  {
        MulHkl(hklv, ml[i]);
        if( (hklv[2] > hkl[2]) ||        // standardise then ...
          ((hklv[2] == hkl[2]) && (hklv[1] > hkl[1])) ||
          ((hklv[2] == hkl[2]) && (hklv[1] == hkl[1]) && (hklv[0] > hkl[0])) )    {
            hkl = hklv;
            changes = true;
        }
        else  {
          hklv *= -1;          
          if( (hklv[2] > hkl[2]) ||        // standardise then ...
            ((hklv[2] == hkl[2]) && (hklv[1] > hkl[1])) ||
            ((hklv[2] == hkl[2]) && (hklv[1] == hkl[1]) && (hklv[0] > hkl[0])) )    {
              hkl = hklv;
              changes = true;
          }
        }
      }
    }
    for( size_t i=0; i < ml.Count(); i++ )  {
      MulHkl(hklv, ml[i]);
      if( EqHkl(hklv) )  {  // only if there is no change
        const double ps = PhaseShift(ml[i]);
        Absent = (olx_abs( ps - olx_round(ps) ) > 0.01);
        if( Absent )
          break;
      }
    }
  }
  template <class MatList> void Standardise(const MatList& ml)  {
    vec3i hklv;
    Absent = false;
    bool changes = true;
    while( changes )  {
      changes = false;
      for( size_t i=0; i < ml.Count(); i++ )  {
        MulHkl(hklv, ml[i]);
        if( (hklv[2] > hkl[2]) ||        // standardise then ...
          ((hklv[2] == hkl[2]) && (hklv[1] > hkl[1])) ||
          ((hklv[2] == hkl[2]) && (hklv[1] == hkl[1]) && (hklv[0] > hkl[0])) )    {
            hkl = hklv;
            changes = true;
        }
      }
    }
    for( size_t i=0; i < ml.Count(); i++ )  {
      MulHkl(hklv, ml[i]);
      if( EqHkl(hklv) )  {  // only if there is no change
        const double ps = PhaseShift(ml[i]);
        Absent = (olx_abs( ps - olx_round(ps) ) > 0.01);
        if( Absent )
          break;
      }
    }
  }
//..............................................................................
  inline double PhaseShift(const smatd& m) const {  
    return m.t.DotProd(hkl);  
  }
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
  inline bool IsSymmetric(const smatd& m) const {
    vec3i hklv;
    MulHkl(hklv, m);
    return EqHkl(hklv);
  }
//..............................................................................
  inline bool IsCentrosymmetric(const smatd& m) const {
    vec3i hklv;
    MulHkl(hklv, m);
    return EqNegHkl(hklv);
  }
//..............................................................................
  template <class MatList> bool IsAbsent(const MatList& ml) const {
    for( size_t i=0; i < ml.Count(); i++ )  {
      if( IsSymmetric(ml[i]) )  {
        double l = PhaseShift(ml[i]);
        if( olx_abs( l - olx_round(l) ) > 0.01 )  
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
  static void SortList(TTypeList<TReflection>& lst)  {  
    lst.QuickSorter.SortSF(lst, TReflection::Compare);  
  }
  // could be list of pointers or list of const pointers
  template <class RefPList> static void SortPList(RefPList& lst)  {  
    lst.QuickSorter.SortSF(lst, TReflection::CompareP);  
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
  // returns a string: h k l ...
  TIString ToString() const  {
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
  //writes string to the provided buffer (should be at least 33 bytes long)
  char* ToCBuffer(char* bf) const  {
#ifdef _MSC_VER
    if( Flag == NoFlagSet )  sprintf_s(bf, 29, "%4i%4i%4i%8.2lf%8.2lf", hkl[0], hkl[1], hkl[2], I, S);
    else                     sprintf_s(bf, 33, "%4i%4i%4i%8.2lf%8.2lf%4i", hkl[0], hkl[1], hkl[2], I, S, Flag);
#else
    if( Flag == NoFlagSet )  sprintf(bf, "%4i%4i%4i%8.2lf%8.2lf", hkl[0], hkl[1], hkl[2], I, S);
    else                     sprintf(bf, "%4i%4i%4i%8.2lf%8.2lf%4i", hkl[0], hkl[1], hkl[2], I, S, Flag);
#endif
    return bf;
  }
//..............................................................................
  // return a string like: tag. h k l ...
  olxstr ToNString() const  {
    olxstr Str(GetTag(), 80);
    Str << '.';
    Str.Format(7, true, ' ');
    return Str + ToString();
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
  vec3d ToCart(const mat3d& m) const {
    return vec3d(
      hkl[0]*m[0][0],
      hkl[0]*m[0][1] + hkl[1]*m[1][1],
      hkl[0]*m[0][2] + hkl[1]*m[1][2] + hkl[2]*m[2][2]
    );
  }
  // standartisation functions
  static inline const TReflection& GetRef(const TReflection& r) {  return r;  }
  static inline const TReflection& GetRef(const TReflection* r) {  return *r;  }
  static inline const TReflection* GetRefP(const TReflection& r) {  return &r;  }
  static inline const TReflection* GetRefP(const TReflection* r) {  return r;  }
  static inline TReflection& Ref(TReflection& r) {  return r;  }
  static inline TReflection& Ref(TReflection* r) {  return *r;  }
  static inline TReflection* RefP(TReflection& r) {  return &r;  }
  static inline TReflection* RefP(TReflection* r) {  return r;  }
};

typedef TPtrList<TReflection> TRefPList;
typedef TTypeList<TReflection> TRefList;

EndXlibNamespace()

#endif
