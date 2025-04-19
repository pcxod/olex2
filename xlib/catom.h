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
#include "olxvptr.h"

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
  catom_flag_RingAtom   = 0x0100,
  catom_flag_Centroid   = 0x0200,
  catom_flag_Polymer    = 0x0400 // atom belongs to a polymeric entity
  ;

class TEllipsoid;
class TAfixGroup;
class TAfixGroups;
class TExyzGroup;
class TExyzGroups;
struct CXConnInfo;
class RefinementModel;
struct Atom3DId;

struct Disp {
  compd value;
  double fp_su, fdp_su;
  Disp() : fp_su(0), fdp_su(0) {}
  Disp(const compd &v) : value(v), fp_su(0), fdp_su(0) {}
};

class TCAtom : public ACollectionItem, public IXVarReferencer {
public:
  struct Site {  // identifies an atomic site, matrix*atom.ccrd()
    TCAtom* atom;
    smatd matrix;
    Site(TCAtom* a, const smatd& m) : atom(a), matrix(m) {}
    Site(const Site& s) : atom(s.atom), matrix(s.matrix) {}
    Site& operator = (const Site& s) {
      atom = s.atom;
      matrix = s.matrix;
      return *this;
    }
    int Compare(const Site& s) const {
      int r = olx_cmp(atom->GetId(), s.atom->GetId());
      if (r == 0) {
        return olx_cmp(matrix.GetId(), s.matrix.GetId());
      }
      return r;
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
  // lower 8 bits - part, 9-10 - chirality, 11-15 - charge, 16 - charge sign
  int16_t PartAndChargeAndChirality;
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
  TAfixGroup* DependentAfixGroup, * ParentAfixGroup;
  TPtrList<TAfixGroup>* DependentHfixGroups;
  TExyzGroup* ExyzGroup;
  XVarReference* Vars[12]; //x,y,z,occu,uiso,U
  static olxstr VarNames[];
  CXConnInfo* ConnInfo;
  void SetId(size_t id) { Id = id; }
  int SortSitesByDistanceAsc(const Site& s1, const Site& s2) const;
  int SortSitesByDistanceDsc(const Site& s1, const Site& s2) const {
    return -SortSitesByDistanceAsc(s1, s2);
  }
  // Shelxl SPEC
  double SpecialPositionDeviation;
  olx_object_ptr<Disp> disp;
public:
  TCAtom(TAsymmUnit* Parent);
  virtual ~TCAtom();
  TAsymmUnit* GetParent() const { return Parent; }
  const cm_Element& GetType() const { return *Type; }
  void SetType(const cm_Element& A);

  /* function validates and changes the atom type. If validate is true, the
  atom type is guessed from the label and changed. If validate is false, the
  label is set without changing the atom type
  */
  void SetLabel(const olxstr& L, bool validate = true);

  // returns atom label
  const olxstr& GetLabel() const { return Label; }
  /* if ResiId == -1 works the same as GetLabel(), otherwise appends '_' and
  the Residue number. If add part is true also adds '^' + parts as a Latin
  letter
  */
  olxstr GetResiLabel(bool add_part) const;
  olxstr GetResiLabel() const { return GetResiLabel(false); }

  size_t AttachedSiteCount() const { return AttachedSites.Count(); }
  bool IsAttachedTo(const TCAtom& ca) const;
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
  void ClearAttachedSites() {
    AttachedSites.Clear();
    AttachedSitesI.Clear();
  }
  // aplies conninfo to the list of attached sites
  void UpdateAttachedSites();

  size_t AttachedSiteICount() const { return AttachedSitesI.Count(); }
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
  bool operator == (const TCAtom& ca) const { return this == &ca; }
  bool operator == (const TCAtom* ca) const { return this == ca; }
  bool operator != (const TCAtom& ca) const { return this != &ca; }
  bool operator != (const TCAtom* ca) const { return this != ca; }

  void  Assign(const TCAtom& S);

  size_t GetId() const { return Id; }
  DefPropP(uint32_t, ResiId);
  DefPropP(uint32_t, FragmentId);
  DefPropP(uint16_t, SameId);
  DefPropP(size_t, EllpId);
  DefPropP(TExyzGroup*, ExyzGroup);
  DefPropP(double, SpecialPositionDeviation);

  int GetPart() const { return (int)(int8_t)(PartAndChargeAndChirality & 0x00ff); }
  void SetPart(int v) {
    PartAndChargeAndChirality = (PartAndChargeAndChirality & 0xff00) | (uint8_t)v;
  }
  int GetCharge() const {
    int c = ((PartAndChargeAndChirality & 0x7c00) >> 10);
    return PartAndChargeAndChirality & 0x8000 ? -c : c;
  }
  void SetCharge(int v) {
    if (v < 0) {
      PartAndChargeAndChirality = ((int16_t)(-v) << 10) | (PartAndChargeAndChirality & 0x03ff)
        | 0x8000;
    }
    else {
      PartAndChargeAndChirality = ((int16_t)v << 10) | (PartAndChargeAndChirality & 0x03ff);
    }
  }

  bool IsChiral() const {
    return (PartAndChargeAndChirality & 0x0300) != 0;
  }

  void ClearChiralFlag() {
    PartAndChargeAndChirality &= ~0x0300;
  }

  bool IsChiralR() const {
    return (PartAndChargeAndChirality & 0x0100) != 0;
  }

  void SetChiralR(bool v) {
    PartAndChargeAndChirality = (PartAndChargeAndChirality & ~0x0300) | 0x0100;
  }

  bool IsChiralS() const {
    return (PartAndChargeAndChirality & 0x0200) != 0;
  }

  void SetChiralS(bool v) {
    PartAndChargeAndChirality = (PartAndChargeAndChirality & ~0x0300) | 0x0200;
  }
  // returns multiplicity of the position
  size_t GetDegeneracy() const { return EquivCount() + 1; }
  // used by TUnitCell to initialise position symmetry
  void AddEquiv(const smatd& m) {
    if (Equivs == 0) {
      Equivs = new smatd_list;
    }
    Equivs->AddCopy(m);
  }
  // number of non identity symmops under which the position is invariant
  size_t EquivCount() const { return Equivs == 0 ? 0 : Equivs->Count(); }
  const smatd& GetEquiv(size_t i) const { return Equivs->GetItem(i); }
  struct SiteSymmCon GetSiteConstraints() const;
  void AssignEquivs(const TCAtom& a);
  // to be used externally by the UnitCell!
  void ClearEquivs();

  CXConnInfo& GetConnInfo() const { return *ConnInfo; }
  void SetConnInfo(CXConnInfo& ci);

  int GetAfix() const;
  DefPropP(TAfixGroup*, ParentAfixGroup);
  DefPropP(TAfixGroup*, DependentAfixGroup);
  size_t DependentHfixGroupCount() const {
    return DependentHfixGroups == 0 ? 0 : DependentHfixGroups->Count();
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
    if (DependentHfixGroups != 0) {
      DependentHfixGroups->Clear();
    }
  }
  void AddDependentHfixGroup(TAfixGroup& hg) {
    if (DependentHfixGroups == 0) {
      DependentHfixGroups = new TPtrList<TAfixGroup>;
    }
    DependentHfixGroups->Add(&hg);
  }
  DefPropP(double, Occu);
  // return chemical coccupancy, i.e. CrystOccu*site_multiplicity
  double GetChemOccu() const { return GetOccu() * GetDegeneracy(); }
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
  DefPropBFIsSet(Centroid, Flags, catom_flag_Centroid);
  DefPropBFIsSet(Polymer, Flags, catom_flag_Polymer);
  bool CheckFlags(short v) const { return (Flags & v) == v; }
  bool IsAvailable() const {
    return
      (Flags & (catom_flag_Detached | catom_flag_Masked | catom_flag_Deleted)) == 0;
  }
  bool IsGrowable() const { return GetDegeneracy() > 1; }
  TEllipsoid* GetEllipsoid() const;
  void UpdateEllp(const TEllipsoid& NV);
  void AssignEllp(TEllipsoid* NV);

  vec3d& ccrd() { return Center; }
  vec3d const& ccrd() const { return Center; }
  vec3d& ccrdEsd() { return Esd; }
  vec3d const& ccrdEsd() const { return Esd; }
  //
  bool IsPositionFixed(bool any=false) const;
  // for either Uiso or U
  bool IsADPFixed(bool any = false) const;
  // IXVarReferencer implementation
  virtual size_t VarCount() const { return 12; }
  virtual XVarReference* GetVarRef(size_t i) const {
    if (i >= VarCount()) {
      throw TInvalidArgumentException(__OlxSourceInfo, "var index");
    }
    return Vars[i];
  }
  virtual olxstr GetVarName(size_t i) const {
    if (i >= VarCount()) {
      throw TInvalidArgumentException(__OlxSourceInfo, "var index");
    }
    return VarNames[i];
  }
  virtual void SetVarRef(size_t i, XVarReference* var_ref) {
    if (i >= VarCount()) {
      throw TInvalidArgumentException(__OlxSourceInfo, "var index");
    }
    Vars[i] = var_ref;
  }
  virtual IXVarReferencerContainer& GetParentContainer() const;
  virtual double GetValue(size_t var_index) const;
  virtual void SetValue(size_t var_index, const double& val);
  virtual bool IsValid() const { return !IsDeleted(); }
  virtual olxstr GetIdName() const { return Label; }
  virtual TIString ToString() const;
#ifdef _DEBUG
  void SetTag(index_t v) const { ACollectionItem::SetTag(v); }
#endif
  const olx_object_ptr<Disp>& GetDisp() const { return disp; }
  olx_object_ptr<Disp>& GetDisp() { return disp; }
  //
  void ToDataItem(TDataItem& item) const;
  void FromDataItem(TDataItem& item);
#ifdef _PYTHON
  PyObject* PyExport(bool export_attached_sites);
#endif
  static int CompareAtomLabels(const olxstr& S, const olxstr& S1);
  Atom3DId Get3DId() const;

  template <class Accessor>
  struct FlagsAnalyser_ {
    const Accessor& accessor;
    const short ref_flags;
    FlagsAnalyser_(const Accessor& accessor_, short _ref_flags)
      : accessor(accessor_), ref_flags(_ref_flags)
    {}
    template <class Item>
    bool OnItem(const Item& o, size_t) const {
      return (olx_ref::get(accessor(o)).Flags & ref_flags) != 0;
    }
  };
  template <class acc_t>
  static FlagsAnalyser_<acc_t>
    FlagsAnalyser(const acc_t& acc, short flag) {
    return FlagsAnalyser_<acc_t>(acc, flag);
  }
  static FlagsAnalyser_<DummyAccessor>
    FlagsAnalyser(short flags) {
    return FlagsAnalyser_<DummyAccessor>(DummyAccessor(), flags);
  }

  template <class Accessor>
  struct FlagSetter_ {
    const Accessor& accessor;
    const short ref_flags;
    bool set;
    FlagSetter_(const Accessor& accessor_, short ref_flags_, bool set_)
      : accessor(accessor_), ref_flags(ref_flags_), set(set_)
    {}
    template <class Item>
    void OnItem(Item& o, size_t) const {
      return olx_set_bit(set, olx_ref::get(accessor(o)).Flags, ref_flags);
    }
  };
  template <class acc_t>
  static FlagSetter_<acc_t>
    FlagSetter(const acc_t& acc, short ref_flags, bool set) {
    return FlagSetter_<acc_t>(acc, ref_flags, set);
  }
  static FlagSetter_<DummyAccessor>
    FlagSetter(short ref_flags, bool set) {
    return FlagSetter_<DummyAccessor>(DummyAccessor(), ref_flags, set);
  }

  template <class Accessor>
  struct TypeAnalyser_ {
    const Accessor& accessor;
    const short ref_type;
    TypeAnalyser_(const Accessor& accessor_, short _ref_type)
      : accessor(accessor_), ref_type(_ref_type)
    {}
    template <class Item> bool OnItem(const Item& o, size_t) const {
      return olx_ref::get(accessor(o)).GetType() == ref_type;
    }
  };
  template <class acc_t>
  static TypeAnalyser_<acc_t>
    TypeAnalyser(const acc_t& acc, const cm_Element& e) {
    return TypeAnalyser_<acc_t>(acc, e.z);
  }
  template <class acc_t>
  static TypeAnalyser_<acc_t>
    TypeAnalyser(const acc_t& acc, short z) {
    return TypeAnalyser_<acc_t>(acc, z);
  }
  static TypeAnalyser_<DummyAccessor>
    TypeAnalyser(short z) {
    return TypeAnalyser_<DummyAccessor>(DummyAccessor(), z);
  }
  static TypeAnalyser_<DummyAccessor>
    TypeAnalyser(const cm_Element& e) {
    return TypeAnalyser_<DummyAccessor>(DummyAccessor(), e.z);
  }

  /* recursive tag setter, uses Processed property to define the ermination
  procedure
  */
  static void SetTagRecursively(TCAtom& a, index_t v);
  friend class TAsymmUnit;
};
//..............................................................................
  typedef TPtrList<TCAtom> TCAtomPList;
  typedef TTypeList<TCAtom> TCAtomList;
//..............................................................................
class TCAtomComparator  {
public:
  static int Compare(const TCAtom& a1, const TCAtom& a2);
  static int Compare(const TCAtom* a1, const TCAtom* a2) {
    return Compare(*a1, *a2);
  }
};
//..............................................................................
class TCAtomCenterComparator {
public:
  static int Compare(const TCAtom& a1, const TCAtom& a2) {
    return olx_cmp(a1.ccrd().QLength(), a2.ccrd().QLength());
  }
};
//..............................................................................
class TGroupCAtom {
  TCAtom* Atom;
  const smatd* Matrix;
  olxstr Name;
public:
  TGroupCAtom() : Atom(0), Matrix(0) {}
  TGroupCAtom(TCAtom* a, const smatd* m = 0) : Atom(a), Matrix(m) {}
  TGroupCAtom(TCAtom& a, const smatd& m)
    : Atom(&a), Matrix(m.IsFirst() ? 0 : &m) {}
  TGroupCAtom(const olxstr& name, TCAtom* a, const smatd* m = 0)
    : Atom(a), Matrix(m), Name(name) {}
  TGroupCAtom(const TGroupCAtom& ga)
    : Atom(ga.Atom), Matrix(ga.Matrix), Name(ga.Name) {}
  TGroupCAtom(const TDataItem& di, const RefinementModel& rm) {
    FromDataItem(di, rm);
  }
  DefPropP(TCAtom*, Atom)
    const smatd* GetMatrix() const { return Matrix; }
  vec3d ccrd() const {
    return Matrix == 0 ? Atom->ccrd() : *Matrix * Atom->ccrd();
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
// with stores coordinates with float type precision
struct Atom3DId {
  const static uint64_t
    z_mask = 0x00000000000000FF,
    a_mask = 0x0000000001FFFF00,
    b_mask = 0x000003FFFE000000,
    c_mask = 0x07FFFC0000000000,
    a_sig  = 0x0800000000000000,
    b_sig  = 0x1000000000000000,
    c_sig  = 0x2000000000000000,
    mask_m = 0x000000000001FFFF, // max crd value
    cell_m = 16 // max unit cells in each direction
    ;
  /*  0-8 - z
  9-25, 26-42, 43-59 - a, b c, 60-62 - signs
  */
  uint64_t id;
  vec3d get_crd() const;
  int8_t get_z() const {
    return (int8_t)(id & z_mask);
  }
  Atom3DId(uint64_t id=~0) : id(id) {}
  Atom3DId(int8_t z, const vec3d& crd, double multiplier = 1);
  Atom3DId& operator = (const Atom3DId& i) {
    this->id = i.id;
    return *this;
  }
  Atom3DId& operator = (const uint64_t& i) {
    this->id = i;
    return *this;
  }
  Atom3DId& operator = (const olxstr& s);
  bool operator == (const Atom3DId& i) const {
    return id == i.id;
  }
  int Compare(const Atom3DId& i) const {
    return olx_cmp(id, i.id);
  }

  olxstr ToString() const;
};
EndXlibNamespace()
#endif
