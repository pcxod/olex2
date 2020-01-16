/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_refmodel_H
#define __olx_xl_refmodel_H
#include "asymmunit.h"
#include "srestraint.h"
#include "xscatterer.h"
#include "experimental.h"
#include "leq.h"
#include "refmerge.h"
#include "afixgroup.h"
#include "exyzgroup.h"
#include "samegroup.h"
#include "reflection.h"
#include "fragment.h"
#include "symmlib.h"
#include "edict.h"
#include "constraints_ext.h"
#include "selected.h"
#include "calc_ext.h"

BeginXlibNamespace()

static const double
  def_HKLF_s  = 1,
  def_HKLF_wt = 1,
  def_OMIT_s  = -2,
  def_OMIT_2t = 180.0,
  def_SHEL_hr = 0,
  def_SHEL_lr = 100;  // ['infinity' in A]

enum {
  def_HKLF_m = 0,
  def_MERG = 2,
  def_TWIN_n = 2
};

// clear constants...
static uint32_t
  rm_clear_SAME = 0x00000001,
  rm_clear_AFIX = 0x00000002,
  rm_clear_VARS = 0x00000004,
  rm_clear_BadRefs = 0x00000008,
  rm_clear_ISAME = 0x00000010, // implicit SAME
  rm_clear_ALL = 0xFFFFFFFF,
  rm_clear_DEF = (rm_clear_ALL^(
  rm_clear_SAME|rm_clear_AFIX|rm_clear_VARS|rm_clear_BadRefs));

class RefinementModel : public IOlxObject {
  // in INS file is EQUV command
  struct Equiv  {
    int ref_cnt;
    smatd symop;
    Equiv() : ref_cnt(0)  {}
    Equiv(const Equiv& e) : ref_cnt(e.ref_cnt), symop(e.symop)  {}
    Equiv(const smatd& so) : ref_cnt(0), symop(so)  {}
    Equiv& operator = (const Equiv& e) {
      symop = e.symop;
      ref_cnt = e.ref_cnt;
      return *this;
    }
  };
  mutable olxstr_dict<Equiv, false> UsedSymm;
  olxstr_dict<XScatterer*, true> SfacData;
  olx_pdict<int, Fragment*> Frags;
  ContentList UserContent;
protected:
  olxstr HKLSource, ModelSource;
  olxstr RefinementMethod,  // L.S. or CGLS
         SolutionMethod;
  int MERG;
  mutable int HKLF; // can be modified by GetReflections!
  mat3d HKLF_mat;
  double HKLF_s, HKLF_wt;
  int HKLF_m;
  double OMIT_s, OMIT_2t;
  double SHEL_lr, SHEL_hr;
  mat3d TWIN_mat;
  int TWIN_n;
  bool TWIN_set, OMIT_set, MERG_set, HKLF_set, SHEL_set,
    DEFS_set;
  vec3i_list Omits;
  TDoubleList DEFS;
  olxstr_dict<IXVarReferencerContainer*, false> RefContainers;
  void SetDefaults();
  TTypeListExt<class InfoTab, IOlxObject> InfoTables;
  SelectedTableRows selectedTableRows;
  // adds a givin direction (if unique) and returns its name
  adirection *AddDirection(const TCAtomGroup &atoms, uint16_t type);
  // atoms omitted from the maps
  AtomRefList Omitted;
public:
  // needs to be extended for the us of the batch numbers...
  struct HklStat : public MergeStats {
    double MaxD, MinD, LimDmin, LimDmax,
      OMIT_s, OMIT_2t, SHEL_lr, SHEL_hr, MaxI, MinI, HKLF_m, HKLF_s;
    double Completeness;
    mat3d HKLF_mat;
    int MERG, HKLF;
    vec3i_list omits;
    size_t FilteredOff, // by LimD, OMIT_2t, SHEL_hr, SHEL_lr
      OmittedReflections, // refs after 0 0 0
      TotalReflections, // reflections read = OmittedRefs + TotalRefs
      IntensityTransformed,  // by OMIT_s
      DataCount; // number of reflections for the refinemenmt
    vec3i FileMinInd, FileMaxInd;  // hkl range in the file (all before 0 0 0)
    HklStat()  {  SetDefaults();  }
    HklStat(const HklStat& hs) {  this->operator = (hs);  }
    HklStat& operator = (const HklStat& hs);
    HklStat& operator = (const MergeStats& ms) {
      MergeStats::operator = (ms);
      return *this;
    }
    void SetDefaults();
    size_t GetReadReflections() const {
      return TotalReflections+OmittedReflections;
    }
    bool need_updating(const RefinementModel &r) const;
  };

  struct BadReflection {
    vec3i index;
    double Fo, Fc, esd, factor;
    BadReflection(const vec3i &_index, double _Fo, double _Fc, double _esd,
      double _factor=0)
    : index(_index), Fo(_Fo), Fc(_Fc), esd(_esd), factor(_factor)
    {
      if (factor == 0 && esd != 0)
        factor = (Fo-Fc)/esd;
    }
    BadReflection() : Fo(0), Fc(0), esd(0), factor(0) {}
    void UpdateFactor() {
      if (esd != 0)
        factor = (Fo-Fc)/esd;
    }
    static int CompareDirect(const BadReflection &r1, const BadReflection &r2) {
      return olx_cmp(r1.factor, r2.factor);
    }
    static int CompareAbs(const BadReflection &r1, const BadReflection &r2) {
      return olx_cmp(olx_abs(r1.factor), olx_abs(r2.factor));
    }
  };

protected:
  mutable HklStat _HklStat;
  mutable TRefList _Reflections;  // ALL un-merged reflections
  // if this is not the HKLSource, statistics is recalculated
  mutable TEFile::FileID HklStatFileID, HklFileID;
  mutable mat3d HklFileMat;
  mutable TIntList _Redundancy;
  mutable int _FriedelPairCount;  // the numbe of pairs only
  TTypeList<BadReflection> BadReflections;
  TActionQList Actions;
  olxstr AtomListToStr(const TTypeList<ExplicitCAtomRef> &al,
    size_t group_size, const olxstr &sep) const;
  olxdict<ExplicitCAtomRef *, vec3d, TPointerComparator> atom_refs;
  /* this is used to initialise the old_atom_ids */
  olxdict<TCAtom *, size_t, TPointerComparator> atom_ids;
  TSizeList old_atom_ids;
  olx_pdict<long, double> completeness_cache;
  long d_to_key(double d, bool centro) const {
    return olx_round(d*1e5) * (centro ? -1 : 1);
  }
public:
  RefinementModel(TAsymmUnit& au);
  virtual ~RefinementModel() {  Clear(rm_clear_DEF);  }
  ExperimentalDetails expl;
  XVarManager Vars;
  TSRestraintList
    rDFIX,  // restrained distances (DFIX)
    rDANG,  // restrained angles (DANG)
    rSADI,  // similar distances (SADI)
    rCHIV,  // restrained atomic volume (CHIV)
    rFLAT,  // planar groups (FLAT)
    rDELU,  // rigid bond restraints (DELU)
    rSIMU,  // similar Uij (SIMU)
    rISOR,  // Uij components approximate to isotropic behavior (ISOR)
    rEADP,  // equivalent adp, constraint
    rAngle,
    rDihedralAngle,
    rFixedUeq,
    rSimilarUeq,
    rSimilarAdpVolume,
    rRIGU;
  TExyzGroups ExyzGroups;
  TAfixGroups AfixGroups;
  TSameGroupList rSAME;
  TActionQueue &OnSetBadReflections,
    &OnCellDifference;
  TAsymmUnit& aunit;
  ConstraintContainer<rotated_adp_constraint> SharedRotatedADPs;
  ConstraintContainer<same_group_constraint> SameGroups;
  ConstraintContainer<adirection> Directions;
  ConstraintContainer<tls_group_constraint> cTLS;
  // restraints and constraints register
  olxstr_dict<IConstraintContainer*> rcRegister;
  TPtrList<IConstraintContainer> rcList;  // when order matters
  TPtrList<TSRestraintList> rcList1;
  // removes references to all deleted atoms
  void Validate();
  // creates a human readable description of the refinement
  const_strlist Describe();
#ifdef _PYTHON
  PyObject* PyExport(bool export_connectivity);
#endif

  TDoubleList used_weight, proposed_weight;
  TIntList LS;       // up to four params
  TDoubleList PLAN;  // up to three params

  ConnInfo Conn;     // extra connectivity information

  CalculatedVars CVars;
  const olxstr &GetModelSource() const { return ModelSource; }
  void SetModelSource(const olxstr &src);
  const olxstr &GetHKLSource() const { return HKLSource; }
  void SetHKLSource(const olxstr &src);
  
  olxstr GetHKLFStr() const;

  template <class list> void SetHKLF(const list& hklf) {
    if (hklf.IsEmpty()) {
      throw TInvalidArgumentException(__OlxSourceInfo, "empty HKLF");
    }
    HKLF = hklf[0].ToInt();
    if (HKLF > 4) {
      MERG = 0;
    }
    if (hklf.Count() > 1) {
      HKLF_s = hklf[1].ToDouble();
    }
    if( hklf.Count() > 10 )  {
      for (int i = 0; i < 9; i++) {
        HKLF_mat[i / 3][i % 3] = hklf[2 + i].ToDouble();
      }
    }
    else if (hklf.Count() > 2) {
      TBasicApp::NewLogEntry(logError) <<
        (olxstr("Invalid HKLF matrix ignored: ").quote() << hklf.Text(' ', 2));
    }
    if (hklf.Count() > 11) {
      HKLF_wt = hklf[11].ToDouble();
    }
    if (hklf.Count() > 12) {
      HKLF_m = hklf[12].ToInt();
    }
    HKLF_set = true;
  }
  // special handling of possibly merged numerical values...
  void SetHKLFString(const olxstr &str);
  int GetHKLF() const {  return HKLF;  }
  void SetHKLF(int v)  {  HKLF = v;  HKLF_set = true;  }
  const mat3d& GetHKLF_mat() const {  return HKLF_mat;  }
  void SetHKLF_mat(const mat3d& v) {
    if (HKLF_mat != v) { // make sure it gets applied to the reflections
      _Reflections.Clear();
    }
    HKLF_mat = v;
    HKLF_set = true;
  }
  double GetHKLF_s() const {  return HKLF_s;  }
  void SetHKLF_s(double v)  {  HKLF_s = v;  HKLF_set = true;  }
  double GetHKLF_wt() const {  return HKLF_wt;  }
  void SetHKLF_wt(double v)  {  HKLF_wt = v;  HKLF_set = true;  }
  double GetHKLF_m() const {  return HKLF_m;  }
  void SetHKLF_m(int v)  {  HKLF_m = v;  HKLF_set = true;  }
  bool IsHKLFSet() const {  return HKLF_set;  }

  int GetMERG() const {  return MERG;  }
  // MERG 4 specifies not to use f''
  bool UseFdp() const {  return MERG != 4;  }
  void SetMERG(int v)  {  MERG = v;  MERG_set = true;  }
  bool HasMERG() const {  return MERG_set;  }

  double GetOMIT_s() const {  return OMIT_s;  }
  void SetOMIT_s(double v)  {  OMIT_s = v;  OMIT_set = true;  }
  double GetOMIT_2t() const {  return OMIT_2t;  }
  void SetOMIT_2t(double v) {  OMIT_2t = v;  OMIT_set = true;}
  bool HasOMIT() const {  return OMIT_set;  }
  size_t OmittedCount() const {  return Omits.Count();  }
  const vec3i& GetOmitted(size_t i) const {  return Omits[i];  }
  void Omit(const vec3i& r);
  void ClearOmits() { Omits.Clear(); }
  void ClearOmit() { OMIT_set = false; }
  const vec3i_list& GetOmits() const {  return Omits;  }
  void AddOMIT(const TStrList& omit);
  void DelOMIT(const TStrList& omit);
  const AtomRefList &OmittedAtoms() const { return Omitted; }
  olxstr GetOMITStr() const {
    return olxstr(OMIT_s) << ' ' << OMIT_2t;
  }
  // processed user omits (hkl) and returns the number of removed reflections
  size_t ProcessOmits(TRefList& refs);

  const TTypeList<BadReflection> &GetBadReflectionList() const {
    return BadReflections;
  }

  void SetBadReflectionList(
    const TTypeList<BadReflection>::const_list_type &bad_refs)
  {
    OnSetBadReflections.Enter(this);
    BadReflections = bad_refs;
    OnSetBadReflections.Exit(this);
  }

  // SHEL reflection resolution filter low/high
  double GetSHEL_lr() const {  return SHEL_lr;  }
  void SetSHEL_lr(double v)  {  SHEL_lr = v;  SHEL_set = true;  }
  double GetSHEL_hr() const {  return SHEL_hr;  }
  void SetSHEL_hr(double v)  {  SHEL_hr = v;  SHEL_set = true;  }
  bool HasSHEL() const {  return SHEL_set;  }
  void SetSHEL(const TStrList& shel);
  void ClearShell() { SHEL_set = false; }
  olxstr GetSHELStr() const {  return olxstr(SHEL_lr) << ' ' << SHEL_hr;  }

  TDoubleList::const_list_type GetBASFAsDoubleList() const;
  // returns a list of [1-sum(basf), basf[0], basf[1],...] - complete scales
  TDoubleList::const_list_type GetScales() const;
  olxstr GetBASFStr() const;

  template <class list> void SetTWIN(const list& twin) {
    if (twin.Count() > 8) {
      for (size_t i = 0; i < 9; i++) {
        TWIN_mat[i / 3][i % 3] = twin[i].ToDouble();
      }
    }
    if (twin.Count() > 9) {
      TWIN_n = twin[9].ToInt();
    }
    TWIN_set = true;
  }
  olxstr GetTWINStr() const;
  const mat3d& GetTWIN_mat()  const {  return TWIN_mat;  }
  void SetTWIN_mat(const mat3d& m)  {  TWIN_mat = m;  TWIN_set = true;  }
  /* ShelXL manual:
If the racemic twinning is present at the same time as normal twinning, n
should be doubled (because there are twice as many components as before) and
given a negative sign (to indicate to the program that the inversion operator
is to be applied multiplicatively with the specified TWIN matrix). The number
of BASF parameters, if any, should be increased from m-1 to 2m-1 where m is the
original number of components (equal to the new |n| divided by 2). The TWIN
matrix is applied m-1 times to generate components 2 ... m from the prime
reflection (component 1); components m+1 ... 2m are then generated as the
Friedel opposites of components 1 ... m
*/
  int GetTWIN_n() const {  return TWIN_n;  }
  void SetTWIN_n(int v)  {  TWIN_n = v;  TWIN_set = true;  }
  bool HasTWIN() const {  return TWIN_set;  }
  void RemoveTWIN()  {  TWIN_set = false;  }

  // sets default esd values for restraints
  void SetDEFS(const TStrList &df);
  olxstr GetDEFSStr() const;
  bool IsDEFSSet() const {  return DEFS_set;  }

  DefPropC(olxstr, RefinementMethod)
  DefPropC(olxstr, SolutionMethod)

  void SetIterations(int v);
  void SetPlan(int v);

  /* clears restraints, SFAC and used symm but not AfixGroups, Exyzroups and
  Vars
  */
  void Clear(uint32_t clear_mask);
  // to be called by the Vars
  void ClearVarRefs();

  void AddInfoTab(const TStrList& l);
  size_t InfoTabCount() const {  return InfoTables.Count();  }
  const InfoTab& GetInfoTab(size_t i) const {  return InfoTables[i];  }
  InfoTab& GetInfoTab(size_t i)  {  return InfoTables[i];  }
  void DeleteInfoTab(size_t i)  {  InfoTables.Delete(i);  }
  InfoTab& AddHTAB();
  InfoTab& AddRTAB(const olxstr& codename);
  InfoTab& AddCONF();
  // internal: this should be called before the AU atom coordinates are changed
  void BeforeAUUpdate_();
  // internal: this should be called after the AU atom coordinates are changed
  void AfterAUUpdate_();
  /* For internal use - this returns sensible results only in between a call to
  BenAUUpdate_ and EndAUUpdate_
  */
  const olxdict<ExplicitCAtomRef *, vec3d, TPointerComparator> & GetAtomRefs_()
  {
    return atom_refs;
  }
  // internal: this should be called before the AU atom coordinates are changed
  void BeforeAUSort_();
  // internal: this should be called after the AU atom coordinates are changed
  void AfterAUSort_();
  // internal: sorts all possible atom containers
  void Sort_();
  /* For internal use - this returns sensible results only in between a call to
  BenAUSort_ and EndAUSorte_ each [TCAtom::GetId()] returns previous value
  */
  const TSizeList & GetOldAtomIds_() { return old_atom_ids; }

  // if the name is empty - all tabs a removed
  void ClearInfoTab(const olxstr &name);
  SelectedTableRows &GetSelectedTableRows() { return selectedTableRows;  }
  const SelectedTableRows &GetSelectedTableRows() const {
    return selectedTableRows;
  }
  bool ValidateInfoTab(const InfoTab& it);
  // adss new symmetry matrics, used in restraints/constraints
  const smatd& AddUsedSymm(const smatd& matr, const olxstr& id=EmptyString());
  //removes the matrix or decriments the reference count
  void RemUsedSymm(const smatd& matr) const;
  // returns the number of the used symmetry matrices
  size_t UsedSymmCount() const {  return UsedSymm.Count();  }
  // returns used symmetry matric at specified index
  const smatd& GetUsedSymm(size_t ind) const {
    return UsedSymm.GetValue(ind).symop;
  }
  /* return index of given symmetry matrix in the list or -1, if it is not in
  the list
  */
  size_t UsedSymmIndex(const smatd& matr) const;
  // deletes all used symmetry matrices
  void ClearUsedSymm()  {  UsedSymm.Clear();  }
  const smatd* FindUsedSymm(const olxstr& name) const {
    const size_t i = UsedSymm.IndexOf(name);
    return (i == InvalidIndex ? NULL : &UsedSymm.GetValue(i).symop);
  }
  /* initialises ID's of the matrices to conform to the unit cell, this called
  by TLattice
  */
  void UpdateUsedSymm(const class TUnitCell& uc);
  // throws an exception if not found
  adirection& DirectionById(const olxstr &id) const;
  // adds new custom scatterer (created with new, will be deleted)
  void AddSfac(XScatterer& sc);
  // returns number of custom scatterers
  size_t SfacCount() const {  return SfacData.Count();  }
  // returns scatterer at specified index
  XScatterer& GetSfacData(size_t i) const {
    return *SfacData.GetValue(i);
  }
  // finds scatterer by label, returns NULL if nothing found
  XScatterer* FindSfacData(const olxstr& label) const {
    return SfacData.Find(label, 0);
  }
  // user content management
  const ContentList& GetUserContent() const {  return UserContent;  }
  const olxstr GetUserContentStr() const {
    olxstr rv;
    for (size_t i = 0; i < UserContent.Count(); i++) {
      rv << ' ' << UserContent[i].ToString();
    }
    return rv.IsEmpty() ? rv : rv.SubStringFrom(1);
  }
  template <class StrLst> void SetUserContentType(const StrLst& sfac) {
    UserContent.Clear();
    for (size_t i = 0; i < sfac.Count(); i++) {
      AddUserContent(sfac[i], 0);
    }
  }
  template <class StrLst> void SetUserContent(const StrLst& sfac,
    const StrLst& unit)
  {
    if (sfac.Count() != unit.Count()) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        "UNIT/SFAC lists mismatch");
    }
    UserContent.Clear();
    for (size_t i = 0; i < sfac.Count(); i++) {
      AddUserContent(sfac[i], unit[i].ToDouble(),
        XScatterer::ChargeFromLabel(sfac[i]));
    }
  }
  void SetUserContent(const olxstr& sfac, const olxstr& unit)  {
    SetUserContent(TStrList(sfac, ' '), TStrList(unit, ' '));
  }
  void SetUserContent(const ContentList& cnt)  {
    UserContent = cnt;
  }
  template <class StrLst> void SetUserContentSize(const StrLst& unit)  {
    if (UserContent.Count() != unit.Count()) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        "UNIT/SFAC lists mismatch");
    }
    for (size_t i = 0; i < UserContent.Count(); i++) {
      UserContent[i].count = unit[i].ToDouble();
    }
  }
  void AddUserContent(const olxstr& type, double amount=0, int charge=0) {
    const cm_Element* elm = XElementLib::FindBySymbolEx(type);
    if (elm == NULL) {
      throw TInvalidArgumentException(__OlxSourceInfo, "element");
    }
    UserContent.AddNew(*elm, amount, XScatterer::ChargeFromLabel(type));
  }
  void AddUserContent(const cm_Element& elm, double amount = 0, int charge = 0) {
    UserContent.AddNew(elm, amount, charge);
  }
  void SetUserFormula(const olxstr& frm, bool mult_z=true)  {
    UserContent.Clear();
    XElementLib::ParseElementString(frm, UserContent);
    for (size_t i = 0; i < UserContent.Count(); i++) {
      UserContent[i].count *= (mult_z ? aunit.GetZ() : 1.0);
    }
  }
  // returns the restrained distance or -1
  double FindRestrainedDistance(const TCAtom& a1, const TCAtom& a2);

  template <class list> void AddEXYZ(const list& exyz) {
    if (exyz.Count() < 2) {
      throw TFunctionFailedException(__OlxSourceInfo, "incomplete EXYZ group");
    }
    TExyzGroup& gr = ExyzGroups.New();
    for (size_t i = 0; i < exyz.Count(); i++) {
      TCAtom* ca = aunit.FindCAtom(exyz[i]);
      if (ca == NULL) {
        gr.Clear();
        throw TFunctionFailedException(__OlxSourceInfo,
          olxstr("unknown atom: ") << exyz[i]);
      }
      gr.Add(*ca);
    }
  }

  RefinementModel& Assign(const RefinementModel& rm, bool AssignAUnit);
  /* updates refinable params - BASF, FVAR, WGHT, returns false if objects
  mismatch */
  bool Update(const RefinementModel& rm);

  size_t FragCount() const {  return Frags.Count();  }
  Fragment& GetFrag(size_t i)  {  return *Frags.GetValue(i);  }
  const Fragment& GetFrag(size_t i) const {  return *Frags.GetValue(i);  }
  Fragment* FindFragByCode(int code) const { return Frags.Find(code, NULL); }
  Fragment& AddFrag(int code, double a=1, double b=1, double c=1, double al=90,
    double be=90, double ga=90)
  {
    size_t ind = Frags.IndexOf(code);
    if( ind != InvalidIndex )  {
      throw TFunctionFailedException(__OlxSourceInfo,
        "duplicated FRAG instruction");
    }
    return *Frags.Add(code, new Fragment(code, a, b, c, al, be, ga));
  }
  // the function does the atom fitting and clears the fragments
  void ProcessFrags();

  const HklStat& GetMergeStat();
  // merged according to MERG
  template <class SymSpace, class Merger>
  HklStat GetRefinementRefList(const SymSpace& sp, TRefList& out)  {
    HklStat stats;
    TRefList refs;
    FilterHkl(refs, stats);
    if( MERG != 0 && HKLF != 5 )  {
      bool mergeFP = (MERG == 4 || MERG == 3) && !sp.IsCentrosymmetric();
      stats = RefMerger::Merge<Merger>(sp, refs, out,
        Omits, mergeFP);
      stats.MERG = MERG;
    }
    else
      stats = RefMerger::MergeInP1<Merger>(refs, out, Omits);
    return AdjustIntensity(out, stats);
  }
  // Friedel pairs always merged
  template <class Symm, class Merger>
  HklStat GetFourierRefList(const Symm& sp, TRefList& out) {
    HklStat stats;
    TRefList refs;
    FilterHkl(refs, stats);
    stats = RefMerger::Merge<Merger>(sp, refs, out, Omits, true);
    return AdjustIntensity(out, stats);
  }
  // P-1 merged, filtered
  template <class Merger> HklStat GetWilsonRefList(TRefList& out) {
    HklStat stats;
    TRefList refs;
    FilterHkl(refs, stats);
    smatd_list ml;
    ml.AddNew().I();
    stats = RefMerger::Merge<Merger>(ml, refs, out, Omits, true);
    return AdjustIntensity(out, stats);
  }
  // P1 merged, unfiltered
  template <class Merger> HklStat GetAllP1RefList(TRefList& out)  {
    HklStat stats;
    TRefList refs(GetReflections());
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
    return AdjustIntensity(out, stats);
  }
  /* if none of the above functions help, try this ones
    return complete list of unmerged reflections (HKLF matrix, if any, is
    applied)
  */
  const TRefList& GetReflections() const;
  /* Sets the reflection list for this object - operates only on the mutable
  members*/
  void SetReflections(const TRefList &refs) const;
  // this will be only valid if any list of the reflections was called
  const HklStat& GetReflectionStat() const {  return _HklStat;  }
  void ResetHklStats() { _HklStat.SetDefaults();  }
  // filters the reflections according to the parameters
  HklStat& FilterHkl(TRefList& out, HklStat& stats) const;
  TRefPList::const_list_type GetNonoverlappingRefs(const TRefList& refs);
  // adjust intensity of reflections according to OMIT
  HklStat& AdjustIntensity(TRefList& out, HklStat& stats) const;
  /* returns redundancy information, like list[0] is the number of reflections
     collected once list[1] = number of reflections collected wtice etc
  */
  const TIntList& GetRedundancyInfo() const {
    GetReflections();
    return _Redundancy;
  }
  // uses algebraic relation
  void DetwinAlgebraic(TRefList& refs, const HklStat& st,
    const SymmSpace::InfoEx& info_ex) const;
  // convinience method for mrehedral::detwin<> with  detwin_mixed
  void DetwinMixed(TRefList& refs, const TArrayList<compd>& F,
    const HklStat& st, const SymmSpace::InfoEx& info_ex) const;
  // convinience method for mrehedral::detwin<> with  detwin_shelx
  void DetwinShelx(TRefList& refs, const TArrayList<compd>& F,
    const HklStat& st, const SymmSpace::InfoEx& info_ex) const;
  // returns the number of pairs
  int GetFriedelPairCount() const {
    GetReflections();
    return _FriedelPairCount;
  }
  olx_pair_t<vec3i, vec3i> CalcIndicesToD(double d,
    const SymmSpace::InfoEx *si=0) const;
  vec3i CalcMaxHklIndexFor2Theta(double two_theta=60, double tol=1e-3) const {
    return CalcMaxHklIndexForD(expl.GetRadiation()/(2*sin(two_theta*M_PI/360)),
      tol);
  }
  vec3i CalcMaxHklIndexForD(double d, double tol=1e-3) const {
    return vec3i(aunit.GetAxes()/d + tol);
  }
  double CalcCompletenessTo2Theta(double tt, bool Laue);
  IXVarReferencerContainer& GetRefContainer(const olxstr& id_name)  {
    try {  return *RefContainers.Get(id_name);  }
    catch(...)  {
      throw TInvalidArgumentException(__OlxSourceInfo, "container id");
    }
  }
  TPtrList<const TSRestraintList>::const_list_type GetRestraints() const;
  TPtrList<TSRestraintList>::const_list_type GetRestraints();
  void ToDataItem(TDataItem& item);
  void FromDataItem(TDataItem& item);
  olxstr WriteInsExtras(const TCAtomPList* atoms,
    bool write_internals) const;
  void ReadInsExtras(const TStrList &items);
  // initialises default values for esd and if needs, value (SIMU)
  TSimpleRestraint &SetRestraintDefaults(TSimpleRestraint &restraint) const;
  // returns true if restraint parameters are default
  bool IsDefaultRestraint(const TSimpleRestraint &restraint) const;
  bool IsDefaultRestraint(const TSameGroup &restraint) const;
  // feeds on .options - instance static
  bool DoShowRestraintDefaults() const;
  void LibHasOccu(const TStrObjList& Params, TMacroData& E);
  void LibOSF(const TStrObjList& Params, TMacroData& E);
  void LibBASF(const TStrObjList& Params, TMacroData& E);
  void LibFVar(const TStrObjList& Params, TMacroData& E);
  void LibEXTI(const TStrObjList& Params, TMacroData& E);
  void LibUpdateCRParams(const TStrObjList& Params, TMacroData& E);
  void LibCalcCompleteness(const TStrObjList& Params, TMacroData& E);
  void LibMaxIndex(const TStrObjList& Params, TMacroData& E);
  // restraints & constraints related functions
  void LibShareADP(TStrObjList &Cmds, const TParamList &Opts, TMacroData &E);
  void LibNewAfixGroup(TStrObjList &Cmds, const TParamList &Options,
    TMacroData &E);
  void LibNewRestraint(TStrObjList &Cmds, const TParamList &Options,
    TMacroData &E);
  void LibModelSrc(const TStrObjList &Params, TMacroData &E);

  TLibrary* ExportLibrary(const olxstr& name=EmptyString());
  struct VPtr : public olx_virtual_ptr<RefinementModel> {
    virtual IOlxObject *get_ptr() const;
  };

  struct ReleasedItems {
    TSimpleRestraintPList restraints;
    TPtrList<TSameGroup> sameList;
    //TPtrList<XLEQ> equations;
  };
  
  // EXTI stuff
  struct EXTI {
    static vec3d HklToCart(const vec3i& mi, const mat3d &hkl2cart) {
      return vec3d(
        mi[0] * hkl2cart[0][0],
        mi[0] * hkl2cart[0][1] + mi[1] * hkl2cart[1][1],
        mi[0] * hkl2cart[0][2] + mi[1] * hkl2cart[1][2] + mi[2] * hkl2cart[2][2]
      );
    }
    struct Shelxl {
      const double l_sq_o_4, k;
      const mat3d hkl2cart;
      Shelxl(double l, double x, const mat3d &hkl2cart)
        : l_sq_o_4(l*l*0.25), k(l*l*l*x*0.001 / 2), hkl2cart(hkl2cart)
      {}
      double CalcForF2(const vec3i &mi, double Fc_sq) const {
        const double x = EXTI::HklToCart(mi, hkl2cart).QLength()*l_sq_o_4;
        return sqrt(1 + Fc_sq * k / sqrt(x*(1 - x)));
      }
      double CalcForF(const vec3i &mi, double Fc_sq) const {
        const double x = EXTI::HklToCart(mi, hkl2cart).QLength()*l_sq_o_4;
        return pow(1 + Fc_sq * k / sqrt(x*(1 - x)), -0.25);
      }
      bool IsValid() const { return k != 0; }
    };

    template <class RefList, class FList, class Corrector>
    static void CorrectF(const RefList& refs, FList& F, const Corrector& cr) {
      if (!cr.IsValid()) {
        return;
      }
      if (refs.Count() != F.Count()) {
        throw TInvalidArgumentException(__OlxSrcInfo, "arrays size");
      }
      for (size_t i = 0; i < refs.Count(); i++) {
        F[i] *= cr.CalcForF(TReflection::GetHkl(refs[i]), F[i].qmod());
      }
    }
  };

  EXTI::Shelxl GetShelxEXTICorrector() const;
  
};

EndXlibNamespace()

#endif
