#ifndef __olx_ref_model_H
#define __olx_ref_model_H
#include "asymmunit.h"
#include "xscatterer.h"
#include "experimental.h"
#include "leq.h"
#include "refutil.h"
#include "afixgroup.h"
#include "exyzgroup.h"

BeginXlibNamespace()

static const double 
  def_HKLF_s  = 1,
  def_HKLF_wt = 1,
  def_HKLF_m  = 0,
  def_OMIT_s  = -2,
  def_OMIT_2t = 180.0;
static const short 
 def_MERG   = 2,
 def_TWIN_n = 2;

class RefinementModel  {
  // in INS file is EQUV command
  smatd_list UsedSymm;
  TCSTypeList<olxstr, XScatterer*> SfacData;  // label + params
protected:
  olxstr HKLSource;
  olxstr RefinementMethod,  // L.S. or CGLS
         SolutionMethod;
  int MERG, HKLF;
  mat3d HKLF_mat;
  double HKLF_s, HKLF_wt, HKLF_m;
  double OMIT_s, OMIT_2t;
  mat3d TWIN_mat;
  int TWIN_n;
  bool TWIN_set, OMIT_set, MERG_set, HKLF_set;
  vec3i_list Omits;
  TDoubleList BASF;
  void SetDefaults();
  TEFile::FileID HklStatFileID;  // if this is not the HKLSource, statistics is recalculated
public:
  // needs to be extended for the us of the batch numbers...
  struct HklStat : public MergeStats {
    double MaxD, MinD, LimD;
    int FilteredOff, // by LimD, OMIT_2t
      OmittedByUser, // OMIT h k l 
      IntensityTransformed;  // by OMIT_s
    HklStat()  {
      SetDefaults();
    }
    HklStat( const HklStat& hs ) {
      this->operator = (hs);
    }
    HklStat& operator = (const HklStat& hs)  {
      MergeStats::operator = (hs);
      MaxD = hs.MaxD;  MinD = hs.MinD;  LimD = hs.LimD;
      FilteredOff = hs.FilteredOff;
      IntensityTransformed = hs.IntensityTransformed;
      OmittedByUser = hs.OmittedByUser;
      return *this;
    }
    HklStat& operator = (const MergeStats& ms)  {
      MergeStats::operator = (ms);
      return *this;
    }
    void SetDefaults()  {
      MergeStats::SetDefaults();
      MaxD = MinD = LimD = 0;
      FilteredOff = IntensityTransformed = OmittedByUser = 0;
    }
  };
protected:
 HklStat _HklStat;
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

  void Validate() {
    rDFIX.ValidateAll();
    rDANG.ValidateAll();
    rSADI.ValidateAll();
    rCHIV.ValidateAll();
    rFLAT.ValidateAll();
    rDELU.ValidateAll();
    rSIMU.ValidateAll();
    rISOR.ValidateAll();
    rEADP.ValidateAll();
    ExyzGroups.ValidateAll();
    AfixGroups.ValidateAll();
    Vars.Validate();
  }
  // creates a human readable description of the refinement
  void Describe(TStrList& lst);

  TDoubleList used_weight, proposed_weight;
  TIntList LS;      // up to four params
  TDoubleList PLAN;  // up to three params
  
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
  const mat3d GetHKLF_mat()  const {  return HKLF_mat;  }
  void SetHKLF_mat(const mat3d& v) {  HKLF_mat = v;  HKLF_set = true;  }
  double GetHKLF_s()         const {  return HKLF_s;  }
  void SetHKLF_s(double v)         {  HKLF_s = v;  HKLF_set = true;  }
  double GetHKLF_wt()        const {  return HKLF_wt;  }
  void SetHKLF_wt(double v)        {  HKLF_wt = v;  HKLF_set = true;  }
  double GetHKLF_m()         const {  return HKLF_m;  }
  void SetHKLF_m(double v)         {  HKLF_m = v;  HKLF_set = true;  }
  bool IsHKLFSet()           const {  return HKLF_set;  }

  int GetMERG()  const {  return MERG;  }
  void SetMERG(int v)  {  MERG = v;  MERG_set = true;  }
  bool HasMERG() const {  return MERG_set;  }
  
  double GetOMIT_s()             const {  return OMIT_s;  }
  void SetOMIT_s(double v)             {  OMIT_s = v;  OMIT_set = true;  }
  double GetOMIT_2t()            const {  return OMIT_2t;  }
  void SetOMIT_2t(double v)            {  OMIT_2t = v;  OMIT_set = true;}
  bool HasOMIT()                 const {  return OMIT_set;  }
  int OmittedCount()             const {  return Omits.Count();  }
  const vec3i& GetOmitted(int i) const {  return Omits[i];  }
  void Omit(const vec3i& r)            {  Omits.AddCCopy(r);  }
  void ClearOmits()                    {  Omits.Clear();  }
  template <class list> void AddOMIT(const list& omit)  {
    if( omit.Count() == 3 )  {  // reflection omit
      Omits.AddNew( omit[0].ToInt(), omit[1].ToInt(), omit[2].ToInt());
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

  const TDoubleList& GetBASF() const {  return BASF;  }
  olxstr GetBASFStr() const {
    olxstr rv;
    for( int i=0; i < BASF.Count(); i++ )  {
      rv << BASF[i];
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

  void AddBASF(double val)          {  BASF.Add(val);  }
  template <class list> void SetBASF(const list& bs) {
    BASF.SetCount(bs.Count());
    for( int i=0; i < bs.Count(); i++ )
      BASF[i] = bs[i].ToDouble();
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
    if( LS.IsEmpty() == 0 ) 
      LS.Add(v);
    else
      LS[0] = v;  
  }
  void SetPlan(int v)        {  
    if( PLAN.IsEmpty() == 0 )  
      PLAN.Add(v);
    else
      PLAN[0] = v;  
  }

  // clears restraints, SFAC and used symm but not AfixGroups, Exyzroups and Vars
  void Clear();
  void ClearAll()  {
    Clear();
    AfixGroups.Clear();
    ExyzGroups.Clear();
    Vars.ClearAll();
  }
  // adss new symmetry matrics, used in restraints/constraints 
  const smatd& AddUsedSymm(const smatd& matr);
  //removes the matrix or decriments the reference count
  void RemUsedSymm(const smatd& matr);
  // returns the number of the used symmetry matrices
  inline int UsedSymmCount()     const {  return UsedSymm.Count();  }
  // returns used symmetry matric at specified index
  inline const smatd& GetUsedSymm(size_t ind)  {  return UsedSymm[ind];  }
  // return index of given symmetry matrix in the list or -1, if it is not in the list
  inline int UsedSymmIndex(const smatd& matr)  const {  return UsedSymm.IndexOf(matr);  }
  // deletes all used symmetry matrices
  inline void ClearUsedSymm()          {  UsedSymm.Clear();  }
  
  // adds new custom scatterer
  void AddNewSfac(const olxstr& label,
                  double a1, double a2, double a3, double a4,
                  double b1, double b2, double b3, double b4,
                  double c, double mu, double r, double wt);
  // returns number of custom scatterers
  inline int SfacCount()  const  {  return SfacData.Count();  }
  // returns scatterer label at specified index
  inline const olxstr& GetSfacLabel(size_t index) const  {
    return SfacData.GetComparable(index);
  }
  // returns scatterer at specified index
  inline XScatterer& GetSfacData(size_t index) const  {
    return *SfacData.GetObject(index);
  }
  // finds scatterer by label, returns NULL if nothing found
  inline XScatterer* FindSfacData(const olxstr& label) const  {
    return SfacData[label];
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

  const HklStat& GetMergeStat();

  void ToDataItem(TDataItem& item);
  void FromDataItem(TDataItem& item);
  
  struct ReleasedItems {
    TSimpleRestraintPList restraints;
    //TPtrList<XLEQ> equations;
  };
};

EndXlibNamespace()

#endif

