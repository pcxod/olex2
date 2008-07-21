#ifndef __OLEX_HKL_REFLECTION__
#define __OLEX_HKL_REFLECTION__

BeginXlibNamespace()

const int NoFlagSet = 0xF0FA;

class TReflection: public ACollectionItem  {
  int H, K, L;
  double I, S;
  bool Centric, Absent;
  short Multiplicity;
  int Flag;  // shelx 'batch number'
public:
//  TReflectionN(const struct TLstRef& r)  {
//    FH = r.H;  FK = r.K;  FL = r.L;
//    i = r.DF;  Sigma = r.Res;
//  }
  TReflection(const TReflection& r)  {
    *this = r;
  }
  TReflection(int h, int k, int l)  {
    H = h;  K = k;  L = l;
    I = S = 0;
    Absent = Centric = false;
    Multiplicity = 1;
    Flag = NoFlagSet;
  }
  TReflection(int h, int k, int l, double I, double S, int flag=NoFlagSet)  {
    H = h;  K = k;  L = l;
    this->I = I;
    this->S = S;
    Absent = Centric = false;
    Multiplicity = 1;
    Flag = flag;
  }
  virtual ~TReflection()  {  }
  //TReflection& operator = (const TLstRef& R);
  TReflection& operator = (const TReflection &r)  {
    H = r.H;  K = r.K;  L = r.L;
    I = r.I;  S = r.S;
    Centric = r.Centric;
    Absent = r.Absent;
    Multiplicity = r.Multiplicity;
    Flag = r.Flag;
    this->SetTag( r.GetTag() );
    return *this;
  }
  //bool operator == (const TReflection &r) const {  return CompareTo(r) == 0; }

  inline int GetH()  const  {  return H;  }
  inline void SetH(int v)   {  H = v;  }
  inline int GetK()  const  {  return K;  }
  inline void SetK(int v)   {  K = v;  }
  inline int GetL()  const  {  return L;  }
  inline void SetL(int v)   {  L = v;  }

  template <class VC>
    inline bool EqHkl (const VC& v) const {
      return ( ((int)v[0] == H) && ((int)v[1] == K) && ((int)v[2] == L) );
    }
  template <class VC>
    inline bool EqNegHkl (const VC& v) const {
      return ( ((int)v[0] == -H) && ((int)v[1] == -K) && ((int)v[2] == -L) );
    }
  // generates symmetry equivalent miller index and stores it in res
  template <class VC, class MC>
  void MulHkl(VC& res, const MC& mat) const {
    res[0] = (H*mat[0][0] + K*mat[1][0] + L*mat[2][0]);
    res[1] = (H*mat[0][1] + K*mat[1][1] + L*mat[2][1]);
    res[2] = (H*mat[0][2] + K*mat[1][2] + L*mat[2][2]);
  }
  // generates symmetry equivalent miller index and stores it in res, uses transposed matrix
  template <class VC, class MC>
  void MulHklT(VC& res, const MC& mat) const {
    res[0] = (H*mat[0][0] + K*mat[0][1] + L*mat[0][2]);
    res[1] = (H*mat[1][0] + K*mat[1][1] + L*mat[1][2]);
    res[2] = (H*mat[2][0] + K*mat[2][1] + L*mat[2][2]);
  }
  template <class CC>
  void MulHkl(TVector3<CC>& res, const smatd& mat) const {
    res[0] = (CC)(H*mat.r[0][0] + K*mat.r[1][0] + L*mat.r[2][0]);
    res[1] = (CC)(H*mat.r[0][1] + K*mat.r[1][1] + L*mat.r[2][1]);
    res[2] = (CC)(H*mat.r[0][2] + K*mat.r[1][2] + L*mat.r[2][2]);
  }
  // generates symmetry equivalent miller index and stores it in res, uses transposed matrix
  template <class CC>
  void MulHklT(TVector3<CC>& res, const smatd& mat) const {
    res[0] = (CC)(H*mat.r[0][0] + K*mat.r[0][1] + L*mat.r[0][2]);
    res[1] = (CC)(H*mat.r[1][0] + K*mat.r[1][1] + L*mat.r[1][2]);
    res[2] = (CC)(H*mat.r[2][0] + K*mat.r[2][1] + L*mat.r[2][2]);
  }
//..............................................................................
  /* replaces hkl with standard hkl accroding to provieded matrices, calculates
   reflection multiplicity and if it is centric or systematically absent
  */
  void Standardise(const smatd_list &ml, bool CheckInversion)  {
    vec3i hklv;
    for(int i=0; i < ml.Count(); i++ )  {
      MulHkl(hklv, ml[i]);
      if( (hklv[2] > L) ||        // sdandardise then ...
          ((hklv[2] == L) && (hklv[1] > K)) ||
          ((hklv[2] == L) && (hklv[1] == K) && (hklv[0] > H)) )    {
          H = hklv[0];  K = hklv[1];  L = hklv[2];
      }
      else if( EqHkl(hklv) )  {  // symmetric reflection
        if( !Absent )  {
          double l = PhaseShift(ml[i]);
          Absent = (fabs( l - Round(l) ) > 0.01);
        }
      }

      if( CheckInversion )  {
        hklv *= -1;
        if( (hklv[2] > L) ||
          ((hklv[2] == L) && (hklv[1] > K) ) ||
          ((hklv[2] == L) && (hklv[1] == K) && (hklv[0] > H)) )   {
          H = hklv[0];  K = hklv[1];  L = hklv[2];
        }
      }
    }
  }
//..............................................................................
  inline double PhaseShift(const smatd& m) const {
    return H*m.t[0] +  K*m.t[1] + L*m.t[2];
  }
//..............................................................................
  /* analyses if this reflection is centric, systematically absent and its multiplicity */
  void Analyse(const smatd_list &ml)  {
    vec3i hklv;
    Multiplicity = 1;
    Centric = false;
    Absent = false;
    for(int i=0; i < ml.Count(); i++ )  {
      MulHkl(hklv, ml[i]);
      if( EqHkl(hklv) )  {  // symmetric reflection
        IncDegeneracy();
        if( !Absent )  {
          double l = PhaseShift(ml[i]);
          Absent = (fabs( l - Round(l) ) > 0.01);
        }
      }
      else if( EqNegHkl(hklv) )  {  // centrocymmetric reflection
        IncDegeneracy();
        Centric = true;
      }
    }
  }
//..............................................................................
  inline bool IsSymmetric(const smatd& m) const {
    vec3i hklv;
    MulHkl(hklv, m);
    return EqHkl(hklv);
  }
  static bool IsSymmetric(const TReflection& r, const smatd_list& ml)  {
    vec3i hklv;
    for( int i=0; i < ml.Count(); i++ )  {
      r.MulHkl(hklv, ml[i]);
      if( r.EqHkl(hklv) )  return true;
    }
    return false;
  }
//..............................................................................
  inline bool IsCentrosymmetric(const smatd& m) const {
    vec3i hklv;
    MulHkl(hklv, m);
    return EqNegHkl(hklv);
  }
  inline static bool IsCentrosymmetric(const TReflection& r, const smatd_list &ml)  {
    vec3i hklv;
    for( int i=0; i < ml.Count(); i++ )  {
      r.MulHkl(hklv, ml[i]);
      if( r.EqNegHkl(hklv) )  return true;
    }
    return false;
  }
//..............................................................................
  static bool IsSystematicallyAbsent(const TReflection& r, const smatd_list &ml)  {
    for( int i=0; i < ml.Count(); i++ )  {
      if( r.IsSymmetric(ml[i]) )  {
        double l = r.PhaseShift(ml[i]);
        if( fabs( l - Round(l) ) > 0.01 )  return true;
      }
    }
    return false;
  }
//..............................................................................
  int CompareTo(const TReflection &r ) const {
    int res = L - r.L;
    if( res == 0 )  {
      res = K - r.K;
      if( res == 0 )
        res = H - r.H;
    }
    return res;
  }
  static int Compare(const TReflection &a, const TReflection &b)  {
    return a.CompareTo(b);
  }
  static int CompareP(const TReflection *a, const TReflection *b)  {
    return a->CompareTo(*b);
  }
  static void SortList(TTypeList<TReflection>& lst)  {  lst.QuickSorter.SortSF(lst, TReflection::Compare);  }
  static void SortPList(TPtrList<TReflection>& lst)  {  lst.QuickSorter.SortSF(lst, TReflection::CompareP);  }
//..............................................................................
  // these values are intialised by Hkl file function AnalyseReflections
  inline void SetCentric(bool v)         {  Centric = v;  }
  inline bool IsCentric()          const {  return Centric;  }
  inline bool IsAbsent()           const {  return Absent;  }
  inline short GetDegeneracy()     const {  return Multiplicity;  }
  inline void SetDegeneracy(short v)     {  Multiplicity = v;  }
  inline void IncDegeneracy()            {  Multiplicity++;  }
//..............................................................................
  inline short GetFlag()           const {  return Flag;  }
  inline void SetFlag(short v)           {  Flag = v;  }
//..............................................................................
  inline double GetI()             const {  return I;  }
  inline void SetI(double v)             {  I = v;  }
  inline double GetS()             const {  return S;  }
  inline void SetS(double v)             {  S = v;  }
//..............................................................................
  // returns a string: h k l ...
  TIString ToString() const  {
    char bf[128];
#ifdef _MSC_VER
    if( Flag == NoFlagSet )  sprintf_s(bf, 128, "%4i%4i%4i%8.2lf%8.2lf", H, K, L, I, S);
    else                     sprintf_s(bf, 128, "%4i%4i%4i%8.2lf%8.2lf%4i", H, K, L, I, S, Flag);
#else
    if( Flag == NoFlagSet )  sprintf(bf, "%4i%4i%4i%8.2lf%8.2lf", H, K, L, I, S);
    else                     sprintf(bf, "%4i%4i%4i%8.2lf%8.2lf%4i", H, K, L, I, S, Flag);
#endif
    //olxstr Res(H);
    //olxstr Str( Res.Format(4, false, ' ') );
    //Res = K;  Str << Res.Format(4, false, ' ');
    //Res = L;  Str << Res.Format(4, false, ' ');
    //Res = olxstr::FormatFloat(2, I);  Str << Res.Format(8, false, ' ');
    //Res = olxstr::FormatFloat(2, S);  Str << Res.Format(8, false, ' ');
    //if( Flag != NoFlagSet )  {
    //  Res = Flag;  Str << Res.Format(4, false, ' '); 
    //}
    //return Str;
    return olxstr(bf);
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
      H = Toks[1].ToInt();
      K = Toks[2].ToInt();
      L = Toks[3].ToInt();
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

      H = Toks[1].ToInt();
      K = Toks[2].ToInt();
      L = Toks[3].ToInt();
      I = Toks[4].ToDouble();
      S = Toks[5].ToDouble();
      if( Toks.Count() > 6 )
        Flag = Toks[6].ToInt();
      return true;
    }
    return false;
  }
};

typedef TPtrList<TReflection> TRefPList;
typedef TTypeList<TReflection> TRefList;

class TSimpleMerger  {
public:
  static TReflection* Merge(const TRefPList& rl)  {
    if( rl.Count() == 0 )  throw TInvalidArgumentException(__OlxSourceInfo, "empty reflexion list");
    if( rl.Count() == 1 )  return new TReflection( *rl[0] );
    double mi = 0, ms = 0;
    for( int l=0; l < rl.Count(); l++ )  {
      mi += rl[l]->GetI();
      ms += 1./QRT(rl[l]->GetS());
//      ms += QRT(rl[l]->GetS());
    }
    mi /= rl.Count();
//    ms = sqrt(ms/rl.Count());
    ms = 1./sqrt(ms);
    return new TReflection( rl[0]->GetH(), rl[0]->GetK(), rl[0]->GetL(), mi, ms);
  }
};

EndXlibNamespace()

#endif
