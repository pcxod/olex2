/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_catom_H
#define __olx_xl_catom_H
#include "xbase.h"
#include "chemdata.h"
#include "symmat.h"
#include "typelist.h"
#include "tptrlist.h"
#include "dataitem.h"
#include "leq.h"

#ifdef _PYTHON
  #include "pyext.h"
#endif

BeginXlibNamespace()

const short
  catom_var_name_Scale = 0,
  catom_var_name_X     = 1, // order matters var_name_X+i
  catom_var_name_Y     = 2,
  catom_var_name_Z     = 3,
  catom_var_name_Sof   = 4,
  catom_var_name_Uiso  = 5,
  catom_var_name_U11   = 6, // order maters var_name_U11+i
  catom_var_name_U22   = 7,
  catom_var_name_U33   = 8,
  catom_var_name_U23   = 9,
  catom_var_name_U13   = 10,
  catom_var_name_U12   = 11;

const short
  catom_flag_Deleted    = 0x0001,
  catom_flag_Growable   = 0x0002,
  catom_flag_HAttached  = 0x0004,
  catom_flag_Saved      = 0x0008,
  catom_flag_Masked     = 0x0010,
  catom_flag_Detached   = 0x0020,
  catom_flag_Processed  = 0x0040, // generic flag
  catom_flag_FixedType  = 0x0080,
  catom_flag_RingAtom   = 0x0100;

class TEllipsoid;
class TAfixGroup;
class TAfixGroups;
class TExyzGroup;
class TExyzGroups;
struct CXConnInfo;
class RefinementModel;

class TCAtom: public ACollectionItem, public IXVarReferencer  {
public:
  struct Site  {  // identifies an atomic site, matrix*atom.ccrd()
    TCAtom* atom;
    smatd matrix;
    Site(TCAtom* a, const smatd& m) : atom(a), matrix(m)  {}
    Site(const Site& s) : atom(s.atom), matrix(s.matrix)  {}
    Site& operator = (const Site& s)  {
      atom = s.atom;
      matrix = s.matrix;
      return *this;
    }
  };
private:
  class TAsymmUnit* Parent;
  const cm_Element* Type;
  olxstr Label;    // atom's label
  // Id is also used to identify if TSAtoms are the same
  size_t Id, EllpId;
  /* this is used in asymmetric unit sort and initialised in
  TLatice::InitBody()
  */
  uint32_t FragmentId,
    ResiId;
  uint16_t SameId, Flags;
  int16_t PartAndCharge;
  double
    Occu,  // occupancy and its variable
    OccuEsd,
    Uiso,  // isotropic thermal parameter
    UisoEsd,
    QPeak,  // if atom is a peak atom - this is not 0
    /* for proxied Uiso (depending on the UisoOwner, this defines the scaled
    value
    */
    UisoScale;
  TCAtom* UisoOwner;  // the Uiso owner, if any
  vec3d Center, Esd;  // fractional coordinates and esds
  TTypeList<TCAtom::Site> AttachedSites, AttachedSitesI;
  smatd_list* Equivs;
  /* Afix group is a fitted group, Hfix group the immediate dependent group */
  TAfixGroup* DependentAfixGroup, *ParentAfixGroup;
  TPtrList<TAfixGroup>* DependentHfixGroups;
  TExyzGroup* ExyzGroup;
  XVarReference* Vars[12]; //x,y,z,occu,uiso,U
  static olxstr VarNames[];
  CXConnInfo* ConnInfo;
  void SetId(size_t id)  {  Id = id;  }
  int SortSitesByDistanceAsc(const Site &s1, const Site &s2) const;
  int SortSitesByDistanceDsc(const Site &s1, const Site &s2) const {
    return -SortSitesByDistanceAsc(s1, s2);
  }
  typedef TDirectAccessor<TCAtom> DirectAccessor;
public:
  TCAtom(TAsymmUnit* Parent);
  virtual ~TCAtom();
  TAsymmUnit* GetParent() const {  return Parent; }
  //TBasicAtomInfo& GetAtomInfo() const {  return *FAtomInfo; }
  //void SetAtomInfo(TBasicAtomInfo& A);
  const cm_Element& GetType() const {  return *Type; }
  void SetType(const cm_Element& A);

  /* function validates and changes the atom type. If validate is true, the
  atom type is guessed from the label and changed. If validate is false, the
  label is set without changing the atom type
  */
  void SetLabel(const olxstr& L, bool validate=true);

  // returns atom label
  const olxstr& GetLabel() const {  return Label;  }
  /* if ResiId == -1 works the same as GetLabel(), otherwise appends '_' and
  the Residue number
  */
  olxstr GetResiLabel() const;

  size_t AttachedSiteCount() const {  return AttachedSites.Count();  }
  bool IsAttachedTo(const TCAtom& ca) const {
    for( size_t i=0; i < AttachedSites.Count(); i++ )
      if( AttachedSites[i].atom == &ca )
        return true;
    return false;
  }
  TCAtom::Site& GetAttachedSite(size_t i) const {
    return AttachedSites.GetItem(i);
  }
  TCAtom& GetAttachedAtom(size_t i) const {
    return *AttachedSites[i].atom;
  }
  /* will add only a unique set {atom, matrix}, returns true if the set is
  unique
  */
  bool AttachSite(TCAtom* atom, const smatd& matrix);
  void ClearAttachedSites()  {
    AttachedSites.Clear();
    AttachedSitesI.Clear();
  }
  // aplies conninfo to the list of attached sites
  void UpdateAttachedSites();

  size_t AttachedSiteICount() const {  return AttachedSitesI.Count();  }
  TCAtom::Site& GetAttachedSiteI(size_t i) const {
    return AttachedSitesI[i];
  }
  TCAtom& GetAttachedAtomI(size_t i) const {
    return *AttachedSitesI[i].atom;
  }
  /* will only add a unique set of {atom, matrix}, returns true if the set is
  unique
  */
  bool AttachSiteI(TCAtom* atom, const smatd& matrix);
  // pointers only compared!
  bool operator == (const TCAtom& ca) const {  return this == &ca;  }
  bool operator == (const TCAtom* ca) const {  return this == ca;  }
  bool operator != (const TCAtom& ca) const {  return this != &ca;  }
  bool operator != (const TCAtom* ca) const {  return this != ca;  }

  void  Assign(const TCAtom& S);

  size_t GetId() const {  return Id;  }
  DefPropP(uint32_t, ResiId)
  DefPropP(uint32_t, FragmentId)
  DefPropP(uint16_t, SameId)
  DefPropP(size_t, EllpId)
  DefPropP(TExyzGroup*, ExyzGroup)

  int GetPart() const { return (int)(int8_t)(PartAndCharge&0x00ff); }
  void SetPart(int v) {
    PartAndCharge = (PartAndCharge & 0xff00) | v;
  }
  int GetCharge() const { return (int)(int8_t)((PartAndCharge & 0xff00) >> 8); }
  void SetCharge(int v) {
    PartAndCharge = ((int16_t)v << 8) | (PartAndCharge & 0x00ff);
  }

  // returns multiplicity of the position
  size_t GetDegeneracy() const {  return EquivCount()+1;  }
  // used by TUnitCell to initialise position symmetry
  void AddEquiv(const smatd& m) {
    if (Equivs == NULL) Equivs = new smatd_list;
    Equivs->AddCopy(m);
  }
  // number of non identity symmops under which the position is invariant
  size_t EquivCount() const {  return Equivs == NULL ? 0 : Equivs->Count();  }
  const smatd& GetEquiv(size_t i) const {  return Equivs->GetItem(i);  }
  struct SiteSymmCon GetSiteConstraints() const;
  void AssignEquivs(const TCAtom& a);
  // to be used externally by the UnitCell!
  void ClearEquivs();

  CXConnInfo& GetConnInfo() const {  return *ConnInfo;  }
  void SetConnInfo(CXConnInfo& ci);

  int GetAfix() const;
  DefPropP(TAfixGroup*, ParentAfixGroup)
  DefPropP(TAfixGroup*, DependentAfixGroup)
  size_t DependentHfixGroupCount() const {
    return DependentHfixGroups == NULL ? 0 : DependentHfixGroups->Count();
  }
  TAfixGroup& GetDependentHfixGroup(size_t i) {
    return *DependentHfixGroups->GetItem(i);
  }
  const TAfixGroup& GetDependentHfixGroup(size_t i) const {
    return *DependentHfixGroups->GetItem(i);
  }
  void RemoveDependentHfixGroup(TAfixGroup& hg) {
    DependentHfixGroups->Remove(&hg);
  }
  void ClearDependentHfixGroups() {
    if( DependentHfixGroups != NULL ) DependentHfixGroups->Clear();
  }
  void AddDependentHfixGroup(TAfixGroup& hg) {
    if( DependentHfixGroups == NULL )
      DependentHfixGroups = new TPtrList<TAfixGroup>;
    DependentHfixGroups->Add(&hg);
  }
  DefPropP(double, Occu)
  // return chemical coccupancy, i.e. CrystOccu*site_multiplicity
  double GetChemOccu() const {  return GetOccu()*GetDegeneracy();  }
  DefPropP(double, OccuEsd);
  DefPropP(double, Uiso);
  DefPropP(double, UisoEsd);
  DefPropP(double, UisoScale);
  DefPropP(TCAtom*, UisoOwner);
  DefPropP(double, QPeak);
  DefPropBFIsSet(Deleted, Flags, catom_flag_Deleted);
  DefPropBFIsSet(Saved, Flags, catom_flag_Saved);
  DefPropBFIsSet(HAttached, Flags, catom_flag_HAttached);
  DefPropBFIsSet(Masked, Flags, catom_flag_Masked);
  DefPropBFIsSet(Detached, Flags, catom_flag_Detached);
  DefPropBFIsSet(Processed, Flags, catom_flag_Processed);
  DefPropBFIsSet(FixedType, Flags, catom_flag_FixedType);
  DefPropBFIsSet(RingAtom, Flags, catom_flag_RingAtom);
  bool IsAvailable() const {
    return
      (Flags&(catom_flag_Detached|catom_flag_Masked|catom_flag_Deleted)) == 0;
  }
  bool IsGrowable() const {  return GetDegeneracy() > 1;  }
  TEllipsoid* GetEllipsoid() const;
  void UpdateEllp(const TEllipsoid& NV);
  void AssignEllp(TEllipsoid *NV);

  vec3d& ccrd()  {  return Center;  }
  vec3d const& ccrd() const {  return Center;  }
  vec3d& ccrdEsd()  {  return Esd;  }
  vec3d const& ccrdEsd() const {  return Esd;  }
// IXVarReferencer implementation
  virtual size_t VarCount() const {  return 12;  }
  virtual XVarReference* GetVarRef(size_t i) const {
    if( i >= VarCount() )
      throw TInvalidArgumentException(__OlxSourceInfo, "var index");
    return Vars[i];
  }
  virtual olxstr GetVarName(size_t i) const {
    if( i >= VarCount() )
      throw TInvalidArgumentException(__OlxSourceInfo, "var index");
    return VarNames[i];
  }
  virtual void SetVarRef(size_t i, XVarReference* var_ref) {
    if( i >= VarCount() )
      throw TInvalidArgumentException(__OlxSourceInfo, "var index");
    Vars[i] = var_ref;
  }
  virtual IXVarReferencerContainer& GetParentContainer() const;
  virtual double GetValue(size_t var_index) const;
  virtual void SetValue(size_t var_index, const double& val);
  virtual bool IsValid() const {  return !IsDeleted();  }
  virtual olxstr GetIdName() const {  return Label;  }
#ifdef _DEBUG
  void SetTag(index_t v) { ACollectionItem::SetTag(v); }
#endif
//
  void ToDataItem(TDataItem& item) const;
  void FromDataItem(TDataItem& item);
#ifdef _PYTHON
  PyObject* PyExport(bool export_attached_sites);
#endif
  static int CompareAtomLabels(const olxstr& S, const olxstr& S1);

  template <class Accessor> struct FlagsAnalyser_ {
    const Accessor &accessor;
    const short ref_flags;
    FlagsAnalyser_(const Accessor &accessor_, short _ref_flags)
      : accessor(accessor_), ref_flags(_ref_flags)  {}
    template <class Item>
    bool OnItem(const Item& o, size_t) const {
      return (accessor(o).Flags&ref_flags) != 0;
    }
  };
  template <class acc_t> static FlagsAnalyser_<acc_t>
  FlagsAnalyser(const acc_t &acc, short flag) {
    return FlagsAnalyser_<acc_t>(acc, flag);
  }
  static FlagsAnalyser_<DirectAccessor>
  FlagsAnalyser(short flags) {
    return FlagsAnalyser_<DirectAccessor>(DirectAccessor(), flags);
  }

  template <class Accessor> struct FlagSetter_ {
    const Accessor &accessor;
    const short ref_flags;
    bool set;
    FlagSetter_(const Accessor &accessor_, short ref_flags_, bool set_)
      : accessor(accessor_), ref_flags(ref_flags_), set(set_)  {}
    template <class Item>
    void OnItem(Item& o, size_t) const {
      return olx_set_bit(set, accessor(o).Flags, ref_flags);
    }
  };
  template <class acc_t> static FlagSetter_<acc_t>
  FlagSetter(const acc_t &acc, short ref_flags, bool set) {
    return FlagSetter_<acc_t>(acc, ref_flags, set);
  }
  static FlagSetter_<DirectAccessor>
  FlagSetter(short ref_flags, bool set) {
    return FlagSetter_<DirectAccessor>(DirectAccessor(), ref_flags, set);
  }

  template <class Accessor> struct TypeAnalyser_ {
    const Accessor &accessor;
    const short ref_type;
    TypeAnalyser_(const Accessor &accessor_, short _ref_type)
      : accessor(accessor_), ref_type(_ref_type)  {}
    template <class Item> bool OnItem(const Item& o, size_t) const {
      return accessor(o).GetType() == ref_type;
    }
  };
  template <class acc_t> static TypeAnalyser_<acc_t>
  TypeAnalyser(const acc_t &acc, const cm_Element &e) {
    return TypeAnalyser_<acc_t>(acc, e.z);
  }
  template <class acc_t> static TypeAnalyser_<acc_t>
  TypeAnalyser(const acc_t &acc, short z) {
    return TypeAnalyser_<acc_t>(acc, z);
  }
  static TypeAnalyser_<DirectAccessor>
  TypeAnalyser(short z) {
    return TypeAnalyser_<DirectAccessor>(DirectAccessor(), z);
  }
  static TypeAnalyser_<DirectAccessor>
  TypeAnalyser(const cm_Element &e) {
    return TypeAnalyser_<DirectAccessor>(DirectAccessor(), e.z);
  }

  /* recursive tag setter, uses Processed property to define the ermination
  procedure
  */
  static void SetTagRecursively(TCAtom &a, index_t v);

  friend class TAsymmUnit;
};
//..............................................................................
  typedef TPtrList<TCAtom> TCAtomPList;
  typedef TTypeList<TCAtom> TCAtomList;
//..............................................................................
class TCAtomComparator  {
public:
  template <class item_a_t, class item_b_t>
  static int Compare(const item_a_t &a1_, const item_b_t &a2_) {
    const TCAtom &a1 = olx_ref::get(a1_),
      &a2 = olx_ref::get(a2_);
    if (a1.GetFragmentId() != a2.GetFragmentId())
      return a1.GetFragmentId() - a2.GetFragmentId();
    if (a1.GetResiId() != a2.GetResiId())
      return olx_cmp(a1.GetResiId(), a2.GetResiId());
    // by label
    if (a1.GetType() == a2.GetType())
      return TCAtom::CompareAtomLabels(a1.GetLabel(), a2.GetLabel());
    // by mass
    return olx_cmp(a1.GetType().GetMr(), a2.GetType().GetMr());
  }
};
//..............................................................................
class TCAtomCenterComparator  {
public:
  static int Compare(const TCAtom &a1, const TCAtom &a2)  {
    return olx_cmp(a1.ccrd().QLength(), a2.ccrd().QLength());
  }
};
//..............................................................................
class TGroupCAtom  {
  TCAtom* Atom;
  const smatd* Matrix;
  olxstr Name;
public:
  TGroupCAtom() : Atom(NULL), Matrix(NULL)  {}
  TGroupCAtom(TCAtom* a, const smatd* m=NULL) : Atom(a), Matrix(m)  {}
  TGroupCAtom(TCAtom& a, const smatd& m)
    : Atom(&a), Matrix(m.IsFirst() ? NULL : &m)  {}
  TGroupCAtom(const olxstr& name, TCAtom* a, const smatd* m=NULL)
    : Atom(a), Matrix(m), Name(name)  {}
  TGroupCAtom(const TGroupCAtom& ga)
    : Atom(ga.Atom), Matrix(ga.Matrix), Name(ga.Name)  {}
  TGroupCAtom(const TDataItem& di, const RefinementModel& rm)  {
    FromDataItem(di, rm);
  }
  DefPropP(TCAtom*, Atom)
  const smatd* GetMatrix() const {  return Matrix;  }
  vec3d ccrd() const {
    return Matrix == NULL ? Atom->ccrd() : *Matrix*Atom->ccrd();
  }
  DefPropC(olxstr, Name)
  /* if atom's Residue is default or residue number equals to ResiNumber - it
  it dropped
  */
  olxstr GetFullLabel(const RefinementModel& rm, int ResiNumber) const;
  /* if Resiname is number - calls the function above with ResiNumber.ToInt(),
  else if it is empty - calls the single argument function, otherwise compares
  the atoms' residue name with ResiName and if the names match - drops it
  */
  olxstr GetFullLabel(const RefinementModel& rm, const olxstr& ResiName) const;
  olxstr GetFullLabel(const RefinementModel& rm) const;
  void ToDataItem(TDataItem& di) const;
  void FromDataItem(const TDataItem& di, const RefinementModel& rm);
};
//..............................................................................
typedef TTypeList<TGroupCAtom> TCAtomGroup;
//..............................................................................
EndXlibNamespace()
#endif
