#ifndef __olx_ref_model_H
#define __olx_ref_model_H
#include "asymmunit.h"
#include "xscatterer.h"
#include "experimental.h"
#include "leq.h"
#include "refmerge.h"
#include "afixgroup.h"
#include "exyzgroup.h"
#include "reflection.h"
#include "fragment.h"
#include "symmlib.h"
#include "edict.h"

BeginXlibNamespace()

static const double 
  def_HKLF_s  = 1,
  def_HKLF_wt = 1,
  def_HKLF_m  = 0,
  def_OMIT_s  = -2,
  def_OMIT_2t = 180.0,
  def_SHEL_hr = 0,
  def_SHEL_lr = 100;  // ['infinity' in A]
static const short 
 def_MERG   = 2,
 def_TWIN_n = 2;

class RefinementModel : public IXVarReferencerContainer, public IXVarReferencer {
  // in INS file is EQUV command
  olxdict<olxstr,smatd,olxstrComparator<false> > UsedSymm;
  olxdict<olxstr,XScatterer*, olxstrComparator<false> > SfacData;  // label + params
  olxdict<int, Fragment*, TPrimitiveComparator> Frags;
protected:
  olxstr HKLSource;
  olxstr RefinementMethod,  // L.S. or CGLS
         SolutionMethod;
  int MERG, HKLF;
  mat3d HKLF_mat;
  double HKLF_s, HKLF_wt, HKLF_m;
  double OMIT_s, OMIT_2t;
  double SHEL_lr, SHEL_hr;
  mat3d TWIN_mat;
  int TWIN_n;
  bool TWIN_set, OMIT_set, MERG_set, HKLF_set, SHEL_set, OMITs_Modified;
  vec3i_list Omits;
  TDoubleList BASF;
  TPtrList<XVarReference> BASF_Vars;
  olxstr VarRefrencerId;
  olxdict<olxstr, IXVarReferencerContainer*, olxstrComparator<false> > RefContainers;
  void SetDefaults();
  TTypeListExt<class InfoTab, IEObject> InfoTables;
public:
  // needs to be extended for the us of the batch numbers...
  struct HklStat : public MergeStats {
    double MaxD, MinD, LimDmin, LimDmax, 
      OMIT_s, OMIT_2t, SHEL_lr, SHEL_hr, MaxI, MinI;
    int MERG;
    //vec3i maxInd, minInd;
    int FilteredOff, // by LimD, OMIT_2t, SHEL_hr, SHEL_lr
      OmittedReflections, // refs after 0 0 0
      TotalReflections, // reflections read = OmittedRefs + TotalRefs
      IntensityTransformed;  // by OMIT_s
    HklStat()  {
      SetDefaults();
    }
    HklStat( const HklStat& hs ) {
      this->operator = (hs);
    }
    HklStat& operator = (const HklStat& hs)  {
      MergeStats::operator = (hs);
      MaxD = hs.MaxD;         MinD = hs.MinD; 
      OMIT_s = hs.OMIT_s;     OMIT_2t = hs.OMIT_2t;
      SHEL_lr = hs.SHEL_lr;   SHEL_hr = hs.SHEL_hr;
      LimDmin = hs.LimDmin;   LimDmax = hs.LimDmax;
      MaxI = hs.MaxI;         MinI = hs.MinI;
      FilteredOff = hs.FilteredOff;
      IntensityTransformed = hs.IntensityTransformed;
      TotalReflections = hs.TotalReflections;
      OmittedReflections = hs.OmittedReflections;
      MERG = hs.MERG;
      return *this;
    }
    HklStat& operator = (const MergeStats& ms)  {
      MergeStats::operator = (ms);
      return *this;
    }
    void SetDefaults()  {
      MergeStats::SetDefaults();
      MaxD = MinD = LimDmax = LimDmin = 0;
      MaxI = MinI = 0;
      FilteredOff = IntensityTransformed = OmittedByUser = 0;
      TotalReflections = OmittedReflections = 0;
      MERG = def_MERG;
      OMIT_s = def_OMIT_s;
      OMIT_2t = def_OMIT_2t;
      SHEL_lr = def_SHEL_lr;
      SHEL_hr = def_SHEL_hr;
    }
    int GetReadReflections() const {  return TotalReflections+OmittedReflections;  }
  };
protected:
  HklStat _HklStat;
  mutable TRefList _Reflections;  // ALL un-merged reflections
  mutable TRefPList _FriedelPairs;  // references form the _Reflections
  mutable TEFile::FileID HklStatFileID, HklFileID;  // if this is not the HKLSource, statistics is recalculated
  mutable TIntList _Redundancy;
  mutable int _FriedelPairCount;  // the numbe of pairs only
public:

  TAsymmUnit& aunit;

  RefinementModel(TAsymmUnit& au);
  virtual ~RefinementModel() {  Clear();  }
  ExperimentalDetails expl;
  XVarManager Vars;
  TSRestraintList rDFIX,  // restrained distances (DFIX)
                  rDANG,  // restrained angles (DANG)
                  rSADI,  // similar distances (SADI)
                  rCHIV,  // restrained atomic volume (CHIV)
                  rFLAT,  // planar groups (FLAT)
                  rDELU,  // rigid bond restraints (DELU)
                  rSIMU,  // similar Uij (SIMU)
                  rISOR,  // Uij components approximate to isotropic behavior (ISOR)
                  rEADP;  // equivalent adp, constraint
  TSameGroupList  rSAME;
  TAfixGroups AfixGroups;
  TExyzGroups ExyzGroups;

  // removes references to all deleted atoms
  void Validate();
  // creates a human readable description of the refinement
  void Describe(TStrList& lst, TPtrList<TCAtom>* a_res = NULL, TPtrList<TSimpleRestraint>* b_res = NULL);
#ifndef _NO_PYTHON
  PyObject* PyExport(bool export_connectivity);
#endif

  TDoubleList used_weight, proposed_weight;
  TIntList LS;      // up to four params
  TDoubleList PLAN;  // up to three params

  ConnInfo Conn;          // extra connectivity information
  
  const olxstr& GetHKLSource() const {  return HKLSource;  }
  //TODO: handle the change
  void SetHKLSource(const olxstr& src);
  olxstr GetHKLFStr() const {  
    olxstr rv(HKLF, 80);
    if( HKLF_m == def_HKLF_m )  {
      if( HKLF_wt == def_HKLF_wt )  {
        if( HKLF_mat.IsI() )  {
          if( HKLF_s != def_HKLF_s )
            rv << ' ' << HKLF_s;
        }
        else {
          rv << ' ' << HKLF_s;
          for( int i=0; i < 9; i++ )
            rv << ' ' << HKLF_mat[i/3][i%3];
        }
      }
      else  {
        rv << ' ' << HKLF_s;
        for( int i=0; i < 9; i++ )
          rv << ' ' << HKLF_mat[i/3][i%3];
        rv << ' ' << HKLF_wt;
      }
    }
    else  {
      rv << ' ' << HKLF_s;
      for( int i=0; i < 9; i++ )
        rv << ' ' << HKLF_mat[i/3][i%3];
      rv << ' ' << HKLF_wt << ' ' << HKLF_m;
    }
    return rv;  
  }
  template <class list> void SetHKLF(const list& hklf) {
    if( hklf.IsEmpty() )
      throw TInvalidArgumentException(__OlxSourceInfo, "empty HKLF");  
    HKLF = hklf[0].ToInt();
    if( hklf.Count() > 1 )
      HKLF_s = hklf[1].ToDouble();
    if( hklf.Count() > 10 )  {
      for( int i=0; i < 9; i++ )
        HKLF_mat[i/3][i%3] = hklf[2+i].ToDouble();
    }
    if( hklf.Count() > 11 )
      HKLF_wt = hklf[11].ToDouble();
    if( hklf.Count() > 12 )
      HKLF_wt = hklf[12].ToDouble();
    HKLF_set = true;
  }
  const int GetHKLF()        const {  return HKLF;  }
  void SetHKLF(int v)              {  HKLF = v;  HKLF_set = true;  }
  const mat3d& GetHKLF_mat() const {  return HKLF_mat;  }
  void SetHKLF_mat(const mat3d& v) {  HKLF_mat = v;  HKLF_set = true;  }
  double GetHKLF_s()         const {  return HKLF_s;  }
  void SetHKLF_s(double v)         {  HKLF_s = v;  HKLF_set = true;  }
  double GetHKLF_wt()        const {  return HKLF_wt;  }
  void SetHKLF_wt(double v)        {  HKLF_wt = v;  HKLF_set = true;  }
  double GetHKLF_m()         const {  return HKLF_m;  }
  void SetHKLF_m(double v)         {  HKLF_m = v;  HKLF_set = true;  }
  bool IsHKLFSet()           const {  return HKLF_set;  }

  int GetMERG()  const {  return MERG;  }
  // MERG 4 specifies not to use f''
  bool UseFdp()  const {  return MERG != 4;  }
  void SetMERG(int v)  {  MERG = v;  MERG_set = true;  }
  bool HasMERG() const {  return MERG_set;  }
  
  double GetOMIT_s()             const {  return OMIT_s;  }
  void SetOMIT_s(double v)             {  OMIT_s = v;  OMIT_set = true;  }
  double GetOMIT_2t()            const {  return OMIT_2t;  }
  void SetOMIT_2t(double v)            {  OMIT_2t = v;  OMIT_set = true;}
  bool HasOMIT()                 const {  return OMIT_set;  }
  int OmittedCount()             const {  return Omits.Count();  }
  const vec3i& GetOmitted(int i) const {  return Omits[i];  }
  void Omit(const vec3i& r)            {  Omits.AddCCopy(r);  OMITs_Modified = true;  }
  void ClearOmits()                    {  Omits.Clear();  OMITs_Modified = true;  }
  const vec3i_list& GetOmits()   const {  return Omits;  }
  template <class list> void AddOMIT(const list& omit)  {
    if( omit.Count() == 3 )  {  // reflection omit
      Omits.AddNew( omit[0].ToInt(), omit[1].ToInt(), omit[2].ToInt());
      OMITs_Modified = true;
    }
    else  {  // reflection transformation/filtering
      if( omit.Count() > 0 )
        OMIT_s = omit[0].ToDouble();
      if( omit.Count() > 1 )
        OMIT_2t = omit[1].ToDouble();
      OMIT_set = true;
    }
  }
  olxstr GetOMITStr() const {
    return olxstr(OMIT_s) << ' ' << OMIT_2t;
  }
  // processed user omits (hkl) and returns the number of removed reflections
  int ProcessOmits(TRefList& refs);

  // SHEL reflection resolution filter low/high
  double GetSHEL_lr()     const {  return SHEL_lr;  }
  void SetSHEL_lr(double v)     {  SHEL_lr = v;  SHEL_set = true;  }
  double GetSHEL_hr()     const {  return SHEL_hr;  }
  void SetSHEL_hr(double v)     {  SHEL_hr = v;  SHEL_set = true;  }
  bool HasSHEL()          const {  return SHEL_set;  }
  template <class list> void SetSHEL(const list& shel)  {
    if( shel.Count() > 0 )  {
      SHEL_lr = shel[0].ToDouble();
      if( shel.Count() > 1 )
        SHEL_hr = shel[1].ToDouble();
      SHEL_set = true;
    }
  }
  olxstr GetSHELStr() const {
    return olxstr(SHEL_lr) << ' ' << SHEL_hr;
  }

  const TDoubleList& GetBASF() const {  return BASF;  }
  olxstr GetBASFStr() const {
    olxstr rv;
    for( int i=0; i < BASF.Count(); i++ )  {
      rv << Vars.GetParam(*this, i);
      if( (i+1) < BASF.Count() )
        rv << ' ';
    }
    return rv;
  }
  
  template <class list> void SetTWIN(const list& twin) {
    if( twin.Count() > 8 )  {
      for( int i=0; i < 9; i++ )
        TWIN_mat[i/3][i%3] = twin[i].ToDouble();
    }
    if( twin.Count() > 9 )
      TWIN_n = twin[9].ToInt();
    TWIN_set = true;
  }
  olxstr GetTWINStr() const {
    olxstr rv;
    for( int i=0; i < 9; i++ )  {
      if( TWIN_mat[i/3][i%3] == 0 )
        rv << "0 ";
      else
        rv << TWIN_mat[i/3][i%3] << ' ';
    }
    return rv << TWIN_n;
  }
  const mat3d& GetTWIN_mat()  const {  return TWIN_mat;  }
  void SetTWIN_mat(const mat3d& m)  {  TWIN_mat = m;  TWIN_set = true;  }
  /* ShelXL manual:
If the racemic twinning is present at the same time as normal twinning, n should be doubled
(because there are twice as many components as before) and given a negative sign (to
indicate to the program that the inversion operator is to be applied multiplicatively with the
specified TWIN matrix). The number of BASF parameters, if any, should be increased from
m-1 to 2m-1 where m is the original number of components (equal to the new |n| divided by 2).
The TWIN matrix is applied m-1 times to generate components 2 ... m from the prime
reflection (component 1); components m+1 ... 2m are then generated as the Friedel opposites
of components 1 ... m  
*/
  const int GetTWIN_n()       const {  return TWIN_n;  }
  void SetTWIN_n(int v)             {  TWIN_n = v;  TWIN_set = true;  }
  bool HasTWIN()              const {  return TWIN_set;  }

  void AddBASF(double val)          {  
    BASF.Add(val);  
    BASF_Vars.Add(NULL);
  }
  template <class list> void SetBASF(const list& bs) {
    BASF.SetCount(bs.Count());
    BASF_Vars.SetCount(bs.Count());
    for( int i=0; i < bs.Count(); i++ )  {
      BASF_Vars[i] = NULL;
      BASF[i] = Vars.SetParam(*this, i, bs[i].ToDouble());
    }
  }
  const olxstr& GetRefinementMethod() const {  return RefinementMethod;  }
  void SetRefinementMethod(const olxstr& rm) {
    RefinementMethod = rm;
  }

  const olxstr& GetSolutionMethod() const {  return SolutionMethod;  }
  void SetSolutionMethod(const olxstr& sm)  {
    SolutionMethod = sm;
  }
  
  void SetIterations( int v ) {  
    if( LS.IsEmpty() ) 
      LS.Add(v);
    else
      LS[0] = v;  
  }
  void SetPlan(int v)        {  
    if( PLAN.IsEmpty() )  
      PLAN.Add(v);
    else
      PLAN[0] = v;  
  }

  // clears restraints, SFAC and used symm but not AfixGroups, Exyzroups and Vars
  void Clear();
  // to be called by the Vars
  void ClearVarRefs();
  void ClearAll()  {
    Clear();
    AfixGroups.Clear();
    ExyzGroups.Clear();
    Vars.ClearAll();
  }

  void AddInfoTab(const TStrList& l);
  int InfoTabCount()                const {  return InfoTables.Count();  }
  const InfoTab& GetInfoTab(int i)  const {  return InfoTables[i];  }
  InfoTab& GetInfoTab(int i)              {  return InfoTables[i];  }
  void DeleteInfoTab(int i)               {  InfoTables.Delete(i);  }
  InfoTab& AddHTAB();
  InfoTab& AddRTAB(const olxstr& codename, const olxstr& resi=EmptyString);
  bool ValidateInfoTab(const InfoTab& it);
  // adss new symmetry matrics, used in restraints/constraints 
  const smatd& AddUsedSymm(const smatd& matr, const olxstr& id=EmptyString);
  //removes the matrix or decriments the reference count
  void RemUsedSymm(const smatd& matr);
  // returns the number of the used symmetry matrices
  inline int UsedSymmCount()     const {  return UsedSymm.Count();  }
  // returns used symmetry matric at specified index
  inline const smatd& GetUsedSymm(size_t ind) const {  return UsedSymm.GetValue(ind);  }
  // return index of given symmetry matrix in the list or -1, if it is not in the list
  inline int UsedSymmIndex(const smatd& matr)  const {  return UsedSymm.IndexOfValue(matr);  }
  // deletes all used symmetry matrices
  inline void ClearUsedSymm()          {  UsedSymm.Clear();  }
  inline const smatd* FindUsedSymm(const olxstr& name)  {
    int i = UsedSymm.IndexOf(name);
    return i == -1 ? NULL : &UsedSymm.GetValue(i);
  }
  
  // adds new custom scatterer
  void AddNewSfac(const olxstr& label,
                  double a1, double a2, double a3, double a4,
                  double b1, double b2, double b3, double b4,
                  double c, double fp, double fdp, double mu, 
                  double r, double wt);
  // returns number of custom scatterers
  inline int SfacCount()  const  {  return SfacData.Count();  }
  // returns scatterer label at specified index
  inline const olxstr& GetSfacLabel(size_t index) const  {
    return SfacData.GetKey(index);
  }
  // returns scatterer at specified index
  inline XScatterer& GetSfacData(size_t index) const  {
    return *SfacData.GetValue(index);
  }
  // finds scatterer by label, returns NULL if nothing found
  inline XScatterer* FindSfacData(const olxstr& label) const  {
    int ind = SfacData.IndexOf(label);
    return ind == -1 ? NULL : SfacData.GetValue(ind);
  }
  // returns the restrained distance or -1
  double FindRestrainedDistance(const TCAtom& a1, const TCAtom& a2);

  template <class list> void AddEXYZ(const list& exyz) {
    if( exyz.Count() < 2 )
      throw TFunctionFailedException(__OlxSourceInfo, "incomplete EXYZ group");
    TExyzGroup& gr = ExyzGroups.New();
    for( int i=0; i < exyz.Count(); i++ )  {
      TCAtom* ca = aunit.FindCAtom(exyz[i]);
      if( ca == NULL )  {
        gr.Clear();
        throw TFunctionFailedException(__OlxSourceInfo, olxstr("unknown atom: ") << exyz[i] );
      }
      gr.Add(*ca);
    }
  }

  RefinementModel& Assign(const RefinementModel& rm, bool AssignAUnit);

  int FragCount()                const {  return Frags.Count();  }
  Fragment& GetFrag(int i)             {  return *Frags.GetValue(i);  }
  const Fragment& GetFrag(int i) const {  return *Frags.GetValue(i);  }
  Fragment* FindFragByCode(int code) {
    int ind = Frags.IndexOf(code);
    return ind == -1 ? NULL : Frags.GetValue(ind);
  }
  Fragment& AddFrag(int code, double a=1, double b=1, double c=1, double al=90, double be=90, double ga=90) {
    int ind = Frags.IndexOf(code);
    if( ind != -1 )
      throw TFunctionFailedException(__OlxSourceInfo, "dublicated FRAG instruction");
    return *Frags.Add(code, new Fragment(code, a, b, c, al, be, ga));
  }
  // the function does the atom fitting and clears the fragments
  void ProcessFrags();

  const HklStat& GetMergeStat();
  // merged according to MERG
  template <class Merger> HklStat GetRefinementRefList(const TSpaceGroup& sg, TRefList& out)  {
    HklStat stats;
    TRefList refs;
    FilterHkl(refs, stats);
    if( MERG != 0 )  {
      smatd_list ml;
      sg.GetMatrices(ml, mattAll^mattIdentity);
      bool mergeFP = (MERG == 4 || MERG == 3) && !sg.IsCentrosymmetric();
      stats = RefMerger::Merge<Merger>(ml, refs, out, Omits, mergeFP);
    }
    else
      stats = RefMerger::MergeInP1<Merger>(refs, out, Omits);
    return stats;
  }
  // Friedel pairs always merged
  template <class Merger> HklStat GetFourierRefList(const TSpaceGroup& sg, TRefList& out) {
    HklStat stats;
    TRefList refs;
    FilterHkl(refs, stats);
    smatd_list ml;
    sg.GetMatrices(ml, mattAll^mattIdentity);
    stats = RefMerger::Merge<Merger>(ml, refs, out, Omits, !sg.IsCentrosymmetric());
    return stats;
  }
  // P-1 merged, filtered
  template <class Merger> HklStat GetWilsonRefList(TRefList& out) {
    HklStat stats;
    TRefList refs;
    FilterHkl(refs, stats);
    smatd_list ml;
    ml.AddNew().I() *= -1;
    stats = RefMerger::Merge<Merger>(ml, refs, out, Omits, true);
    return stats;
  }
  // P1 merged, unfiltered
  template <class Merger> HklStat GetAllP1RefList(TRefList& out)  {
    HklStat stats;
    TRefList refs( GetReflections() );
    ApplyMatrix(refs, HKLF_mat);
    vec3i_list empty_omits;
    stats = RefMerger::MergeInP1<Merger>(refs, out, empty_omits);
    return stats;
  }
  // P1 merged, filtered
  template <class Merger> HklStat GetFilteredP1RefList(TRefList& out)  {
    HklStat stats;
    TRefList refs;
    FilterHkl(refs, stats);
    stats = RefMerger::MergeInP1<Merger>(refs, out, Omits);
    return stats;
  }
  // if none of the above functions help, try this ones
  // return complete list of unmerged reflections (HKLF matrix, if any is NOT applied)
  const TRefList& GetReflections() const;
  // filters the reflections according to the parameters
  HklStat& FilterHkl(TRefList& out, HklStat& stats);
  // returns un-filtered, un-merged list of the Friedel pairs
  const TRefPList& GetFriedelPairs() const  {
    GetReflections();
    return _FriedelPairs;
  }
  /* returns redundancy information, like list[0] is the number of reflections collected once
     list[1] = number of reflections collected wtice etc */
  const TIntList& GetRedundancyInfo() const {
    GetReflections();
    return _Redundancy;
  }
  // returns the number of pairs - has no relation to the lsi of
  int GetFriedelPairCount() const {
    GetReflections();
    return _FriedelPairCount;
  }
  // applies the HKLF matrix trnsformation
  void ApplyMatrix(TRefList& refs, const mat3d& m)  {
    if( m.IsI() )  return;
    const int rc = refs.Count();
    int hkl[3];
    for( int i=0; i < rc; i++ )  {
      TReflection& ref = refs[i];
      ref.MulHklR(hkl, HKLF_mat);  // indexe rounded
      ref.SetH(hkl[0]);
      ref.SetK(hkl[1]);
      ref.SetL(hkl[2]);
    }
  }
  IXVarReferencerContainer& GetRefContainer(const olxstr& id_name)  {
    try {  return *RefContainers[id_name];  }
    catch(...)  {
      throw TInvalidArgumentException(__OlxSourceInfo, "container id");
    }
  }
// IXVarReferencer implementation
  virtual short VarCount()                           const {  return BASF.Count();  }
  virtual const XVarReference* GetVarRef(short i)    const {  
    if( i < 0 || i >= BASF_Vars.Count() )
      throw TInvalidArgumentException(__OlxSourceInfo, "var index");
    return BASF_Vars[i];  
  }
  virtual olxstr GetVarName(short i)                 const {  
    if( i < 0 || i >= BASF_Vars.Count() )
      throw TInvalidArgumentException(__OlxSourceInfo, "var index");
    return olxstr("k") << (i+1);  
  }
  virtual XVarReference* GetVarRef(short i)                {  
    if( i < 0 || i >= BASF_Vars.Count() )
      throw TInvalidArgumentException(__OlxSourceInfo, "var index");
    return BASF_Vars[i];  
  }
  virtual void SetVarRef(short i, XVarReference* var_ref)  {  
    if( i < 0 || i >= BASF_Vars.Count() )
      throw TInvalidArgumentException(__OlxSourceInfo, "var index");
    BASF_Vars[i] = var_ref;  
  }
  virtual IXVarReferencerContainer& GetParentContainer() {  return *this;  }
  virtual double GetValue(short var_index) const {  
    if( var_index < 0 || var_index > BASF.Count() )
      throw TInvalidArgumentException(__OlxSourceInfo, "var_index");
    return BASF[var_index];  
  }
  virtual void SetValue(short var_index, const double& val) {  
    if( var_index < 0 || var_index > BASF.Count() )
      throw TInvalidArgumentException(__OlxSourceInfo, "var_index");
    BASF[var_index] = val;  
  }
  virtual bool IsValid() const {  return true;  }
//
// IXVarReferencerContainer implementation
  virtual olxstr GetIdName() const { 
    return VarRefrencerId;
  }
  virtual int GetReferencerId(const IXVarReferencer& vr) const {
    return 0;
  }
  virtual IXVarReferencer* GetReferencer(int id) {
    return this;
  }
  virtual int ReferencerCount() const {  return 1;  }
//
  void ToDataItem(TDataItem& item);
  void FromDataItem(TDataItem& item);
  
  struct ReleasedItems {
    TSimpleRestraintPList restraints;
    TPtrList<TSameGroup> sameList;
    //TPtrList<XLEQ> equations;
  };
};

EndXlibNamespace()

#endif

