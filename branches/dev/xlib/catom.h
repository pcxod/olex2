#ifndef __olx_xl_catom_H
#define __olx_xl_catom_H
#include "xbase.h"
#include "chemdata.h"
#include "symmat.h"
#include "evpoint.h"
#include "typelist.h"
#include "tptrlist.h"
#include "dataitem.h"
#include "leq.h"

#ifndef _NO_PYTHON
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
  catom_flag_Detached   = 0x0020;

class TEllipsoid;
class TAfixGroup;
class TAfixGroups;
class TExyzGroup;
class TExyzGroups;
struct CXConnInfo;

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
  //TBasicAtomInfo* FAtomInfo;    // a pointer to TBasisAtomInfo object
  const cm_Element* Type;
  olxstr Label;    // atom's label
  size_t Id, EllpId;  // c_atoms id; this is also used to identify if TSAtoms are the same
  uint32_t FragmentId, // this is used in asymmetric unit sort and initialised in TLatice::InitBody()
    ResiId;
  uint16_t SameId, Flags;
  int8_t Part;
  double
    Occu,  // occupancy and its variable
    OccuEsd,
    Uiso,  // isotropic thermal parameter
    UisoEsd,
    QPeak,  // if atom is a peak atom - this is not 0
    UisoScale;  // for proxied Uiso (depending on the UisoOwner, this defines the scaled value
  TCAtom* UisoOwner;  // the Uiso owner, if any
  vec3d Center, Esd;  // fractional coordinates and esds
  TTypeList<TCAtom::Site>* AttachedSites, *AttachedSitesI;
  smatd_list* Equivs;
  /* Afix group is a fitted group, Hfix group the immediate dependent group */
  TAfixGroup* DependentAfixGroup, *ParentAfixGroup;
  TPtrList<TAfixGroup>* DependentHfixGroups;
  TExyzGroup* ExyzGroup;
  XVarReference* Vars[12]; //x,y,z,occu,uiso,U
  static olxstr VarNames[];
  CXConnInfo* ConnInfo;
  inline void SetId(size_t id)  {  Id = id;  }
public:
  TCAtom(TAsymmUnit* Parent);
  virtual ~TCAtom();
  inline TAsymmUnit* GetParent() const {  return Parent; }
  //inline TBasicAtomInfo& GetAtomInfo() const {  return *FAtomInfo; }
  //void SetAtomInfo(TBasicAtomInfo& A);
  const cm_Element& GetType() const {  return *Type; }
  void SetType(const cm_Element& A);

  /* function validates and changes the atom type. If validate is true, the atom type is guessed
  from the label and changed. If validate is false, the label is set without changing the atom type */
  void SetLabel(const olxstr& L, bool validate=true);
  
  // returns atom label
  const olxstr& GetLabel() const {  return Label;  }
  // if ResiId == -1 works the same as GetLabel(), otherwise appends '_' and the Residue number
  olxstr GetResiLabel() const;

  inline size_t AttachedSiteCount() const {
    return AttachedSites == NULL ? 0 : AttachedSites->Count();
  }
  bool IsAttachedTo(const TCAtom& ca) const {
    for( size_t i=0; i < AttachedSiteCount(); i++ )
      if( GetAttachedSite(i).atom == &ca )
        return true;
    return false;
  }
  inline TCAtom::Site& GetAttachedSite(size_t i) const {  return AttachedSites->GetItem(i);  }
  inline TCAtom& GetAttachedAtom(size_t i) const {  return *AttachedSites->GetItem(i).atom;  }
  // will add only a unique set {atom, matrix}, returns true if the set is unique
  bool AttachSite(TCAtom* atom, const smatd& matrix);
  void ClearAttachedSites()  {
    if( AttachedSites != NULL )  {
      delete AttachedSites;
      AttachedSites = NULL; 
    }
    if( AttachedSitesI != NULL )  {
      delete AttachedSitesI;
      AttachedSitesI = NULL; 
    }
  }

  inline size_t AttachedSiteICount() const {
    return AttachedSitesI == NULL ? 0 : AttachedSitesI->Count();
  }
  inline TCAtom::Site& GetAttachedSiteI(size_t i) const {  return AttachedSitesI->GetItem(i);  }
  inline TCAtom& GetAttachedAtomI(size_t i) const {  return *AttachedSitesI->GetItem(i).atom;  }
  // will only add a unique set of {atom, matrix}, returns true if the set is unique
  bool AttachSiteI(TCAtom* atom, const smatd& matrix);
  // pointers only compared!
  inline bool operator == (const TCAtom& ca) const {  return this == &ca;  }
  inline bool operator == (const TCAtom* ca) const {  return this == ca;  }
  inline bool operator != (const TCAtom& ca) const {  return this != &ca;  }
  inline bool operator != (const TCAtom* ca) const {  return this != ca;  }

  void  Assign(const TCAtom& S);

  inline size_t GetId() const {  return Id;  }
  DefPropP(uint32_t, ResiId)
  DefPropP(uint32_t, FragmentId)
  DefPropP(uint16_t, SameId)
  DefPropP(size_t, EllpId)
  DefPropP(int8_t, Part)
  DefPropP(TExyzGroup*, ExyzGroup)

  // returns multiplicity of the position
  size_t GetDegeneracy() const {  return EquivCount()+1;  }
  // used by TUnitCell to initialise position symmetry
  void AddEquiv(const smatd& m)  {
    if( Equivs == NULL )  Equivs = new smatd_list;
    Equivs->AddCCopy(m);
  }
  // number of non identity symmops under which the position is invariant
  size_t EquivCount() const {  return Equivs == NULL ? 0 : Equivs->Count();  }
  const smatd& GetEquiv(size_t i) const {  return Equivs->GetItem(i);  }
  void AssignEquivs(const TCAtom& a);
  // to be used externally by the UnitCell!
  void ClearEquivs();

  CXConnInfo& GetConnInfo() const {  return *ConnInfo;  }
  void SetConnInfo(CXConnInfo& ci);

  int GetAfix() const;
  DefPropP(TAfixGroup*, ParentAfixGroup)
  DefPropP(TAfixGroup*, DependentAfixGroup)
  size_t DependentHfixGroupCount() const {  return DependentHfixGroups == NULL ? 0 : DependentHfixGroups->Count();  }
  TAfixGroup& GetDependentHfixGroup(size_t i) {  return *DependentHfixGroups->GetItem(i);  }
  const TAfixGroup& GetDependentHfixGroup(size_t i) const {  return *DependentHfixGroups->GetItem(i);  }
  void RemoveDependentHfixGroup(TAfixGroup& hg) {  DependentHfixGroups->Remove(&hg);  }
  void ClearDependentHfixGroups() {  
    if( DependentHfixGroups != NULL ) DependentHfixGroups->Clear();
  }
  void AddDependentHfixGroup(TAfixGroup& hg) {
    if( DependentHfixGroups == NULL )  DependentHfixGroups = new TPtrList<TAfixGroup>;
    DependentHfixGroups->Add(&hg);
  }
  DefPropP(double, Occu)
  DefPropP(double, OccuEsd)
  DefPropP(double, Uiso)
  DefPropP(double, UisoEsd)
  DefPropP(double, UisoScale)
  DefPropP(TCAtom*, UisoOwner)
  DefPropP(double, QPeak)
  DefPropBFIsSet(Deleted,   Flags, catom_flag_Deleted)
  DefPropBFIsSet(Saved,     Flags, catom_flag_Saved)
  DefPropBFIsSet(HAttached, Flags, catom_flag_HAttached)
  DefPropBFIsSet(Growable,  Flags, catom_flag_Growable)
  DefPropBFIsSet(Masked,    Flags, catom_flag_Masked)
  DefPropBFIsSet(Detached,  Flags, catom_flag_Detached)
  bool IsAvailable() const {  return (Flags&(catom_flag_Detached|catom_flag_Masked|catom_flag_Deleted)) == 0;  }

  TEllipsoid* GetEllipsoid() const;
  void UpdateEllp(const TEllipsoid& NV);
  void AssignEllp(TEllipsoid *NV);

  inline vec3d& ccrd()  {  return Center;  }
  inline vec3d const& ccrd() const {  return Center;  }
  inline vec3d& ccrdEsd()  {  return Esd;  }
  inline vec3d const& ccrdEsd() const {  return Esd;  }
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
//
  void ToDataItem(TDataItem& item) const;
  void FromDataItem(TDataItem& item);
#ifndef _NO_PYTHON
  PyObject* PyExport();
#endif
  static int CompareAtomLabels(const olxstr& S, const olxstr& S1);
  template <class Accessor=DirectAccessor> struct FlagsAnalyser  {
    const short ref_flags;
    FlagsAnalyser(short _ref_flags) : ref_flags(_ref_flags)  {}
    template <class Item> inline bool OnItem(const Item& o, size_t) const {
      return (Accessor::Access(o).Flags&ref_flags) != 0;
    }
  };
  template <class Accessor=DirectAccessor> struct TypeAnalyser  {
    const short ref_type;
    TypeAnalyser(const cm_Element _ref_type) : ref_type(_ref_type.z)  {}
    TypeAnalyser(short _ref_type) : ref_type(_ref_type)  {}
    template <class Item> inline bool OnItem(const Item& o, size_t) const {
      return Accessor::Access(o).GetType() == ref_type;
    }
  };
  friend class TAsymmUnit;
};
//..............................................................................
  typedef TPtrList<TCAtom> TCAtomPList;
  typedef TTypeList<TCAtom> TCAtomList;
//..............................................................................
class TCAtomPComparator  {
public:
  static int Compare(const TCAtom* a1, const TCAtom* a2)  {
    if( a1->GetFragmentId() != a2->GetFragmentId() )  return a1->GetFragmentId() - a2->GetFragmentId();
    if( a1->GetResiId() != a2->GetResiId() )  return olx_cmp(a1->GetResiId(),a2->GetResiId());
    // asc sort by label
    if( a1->GetType() == a2->GetType() )
      return TCAtom::CompareAtomLabels(a1->GetLabel(), a2->GetLabel());
    // desc sort my weight
    if( (a1->GetType().GetMr() - a2->GetType().GetMr()) < 0 )  return 1;
    return -1;
  }
};
//..............................................................................
class TCAtomPCenterComparator  {
public:
  static int Compare(const TCAtom* a1, const TCAtom* a2)  {
    const double p = a1->ccrd().QLength() - a2->ccrd().QLength();
    return (p < 0 ? -1 :(p > 0 ? 1 : 0));
  }
};
//..............................................................................
class TCAtomTagComparator  {
public:
  static int Compare(const TCAtom* a1, const TCAtom* a2)  {
    return (a1->GetTag() < a2->GetTag() ? -1 : (a1->GetTag() > a2->GetTag() ? 1 : 0));
  }
};
//..............................................................................
class TGroupCAtom  {
  TCAtom* Atom;
  const smatd* Matrix;
  olxstr Name;
public:
  TGroupCAtom() : Atom(NULL), Matrix(NULL)  {  }
  TGroupCAtom(TCAtom* a, const smatd* m=NULL) : Atom(a), Matrix(m)  { }
  TGroupCAtom(const olxstr& name, TCAtom* a, const smatd* m=NULL) : Name(name), Atom(a), Matrix(m)  { }
  TGroupCAtom(const TGroupCAtom& ga) : Atom(ga.Atom), Matrix(ga.Matrix), Name(ga.Name)  { }
  DefPropP(TCAtom*, Atom)
  inline const smatd* GetMatrix() const {  return Matrix;  }
  DefPropC(olxstr, Name)
  // if atom's Residue is default or residue number equals to ResiNumber - it it dropped
  olxstr GetFullLabel(class RefinementModel& rm, const int ResiNumber) const;
  /* if Resiname is number - calls the function above with ResiNumber.ToInt(), else if it is empty - calls
    the single argument function, otherwise compares the atoms' residue name with ResiName and
    if the names match - drops it
  */
  olxstr GetFullLabel(class RefinementModel& rm, const olxstr& ResiName) const;
  olxstr GetFullLabel(class RefinementModel& rm) const;
};
//....................................................................................
typedef TTypeList<TGroupCAtom> TCAtomGroup;
//..............................................................................


EndXlibNamespace()
#endif

