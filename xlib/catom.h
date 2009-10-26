#ifndef catomH
#define catomH

#include "xbase.h"
#include "atominfo.h"
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
private:
  class TAsymmUnit *FParent;
  TBasicAtomInfo*  FAtomInfo;    // a pointer to TBasisAtomInfo object
  olxstr FLabel;    // atom's label
  size_t Id, EllpId;  // c_atoms id; this is also used to identify if TSAtoms are the same
  uint32_t FragmentId, // this is used in asymmetric unit sort and initialised in TLatice::InitBody()
    ResiId;
  uint16_t SameId, Flags;
  int8_t Part;
  double  Occu;     // occupancy and its variable
  double  Uiso, // isotropic thermal parameter; use it when Ellipsoid = NULL
    QPeak,      // if atom is a peak atom - this is not 0
    UisoEsd,    //
    UisoScale;  // for proxied Uiso (depending on the UisoOwner, this defines the scaled value
  TCAtom* UisoOwner;  // the Uiso owner, if any
  vec3d Center, Esd;  // fractional coordinates and esds
  uint8_t Degeneracy;
  TPtrList<TCAtom>* FAttachedAtoms, *FAttachedAtomsI;
  /* Afix group is a fitted group, Hfix group the immediate dependent group */
  TAfixGroup* DependentAfixGroup, *ParentAfixGroup;
  TPtrList<TAfixGroup>* DependentHfixGroups;
  TExyzGroup* ExyzGroup;
  XVarReference* Vars[12]; //x,y,z,occu,uiso,U
  static olxstr VarNames[];
  CXConnInfo* ConnInfo;
  inline void SetId(size_t id) {  Id = id;  }
public:
  TCAtom(TAsymmUnit *Parent);
  virtual ~TCAtom();
  inline TAsymmUnit* GetParent() const {  return FParent; }
  inline TBasicAtomInfo& GetAtomInfo() const {  return *FAtomInfo; }
  void SetAtomInfo(TBasicAtomInfo& A);

  inline olxstr& Label()  {  return FLabel; }
  // function validates and changes the atom type, use the syntax above just to set the label
  bool SetLabel(const olxstr &L);
  
  // returns just atom label
  inline const olxstr& GetLabel() const {  return FLabel;  }
  // if ResiId == -1 works the same as GetLabel(), otherwise appends '_' and Residue number
  olxstr GetResiLabel()  const;

  inline size_t AttachedAtomCount() const {
    return FAttachedAtoms == NULL ? 0 : FAttachedAtoms->Count();
  }
  inline TCAtom& GetAttachedAtom(size_t i) const {  return *FAttachedAtoms->Item(i);  }
  void AttachAtom(TCAtom *CA);
  inline bool IsAttachedTo(TCAtom& CA) const {
    return FAttachedAtoms == NULL ? false : FAttachedAtoms->IndexOf(&CA) != InvalidIndex;
  }
  inline void ClearAttachedAtoms()  {
    if( FAttachedAtoms != NULL )  {
      delete FAttachedAtoms;
      FAttachedAtoms = NULL; 
    }
    if( FAttachedAtomsI != NULL )  {
      delete FAttachedAtomsI;
      FAttachedAtomsI = NULL; 
    }
  }

  inline size_t AttachedAtomICount()      const {
    return FAttachedAtomsI == NULL ? 0 : FAttachedAtomsI->Count();
  }
  inline TCAtom& GetAttachedAtomI(size_t i)      const {  return *FAttachedAtomsI->Item(i);  }
  void AttachAtomI(TCAtom *CA);
  inline bool IsAttachedToI(TCAtom& CA)const {
    return FAttachedAtomsI == NULL ? false : FAttachedAtomsI->IndexOf(&CA) != InvalidIndex;
  }
  // beware - just the memory addresses compared!
  inline bool operator == (const TCAtom& ca)  const  {  return this == &ca;  }
  inline bool operator == (const TCAtom* ca)  const  {  return this == ca;  }
  inline bool operator != (const TCAtom& ca)  const  {  return this != &ca;  }
  inline bool operator != (const TCAtom* ca)  const  {  return this != ca;  }

//  TAtomsInfo *AtomsInfo() const;
  void  Assign(const TCAtom& S);

  inline size_t GetId() const {  return Id;  }
  DefPropP(uint32_t, ResiId)
  DefPropP(uint32_t, FragmentId)
  DefPropP(uint16_t, SameId)
  DefPropP(size_t, EllpId)
  DefPropP(uint8_t, Degeneracy)
  DefPropP(int8_t, Part)
  DefPropP(TExyzGroup*, ExyzGroup)
  
  CXConnInfo& GetConnInfo() const {  return *ConnInfo;  }
  void SetConnInfo(CXConnInfo& ci);

  int GetAfix() const;
  DefPropP(TAfixGroup*, ParentAfixGroup)
  DefPropP(TAfixGroup*, DependentAfixGroup)
  size_t DependentHfixGroupCount() const {  return DependentHfixGroups == NULL ? 0 : DependentHfixGroups->Count();  }
  TAfixGroup& GetDependentHfixGroup(size_t i) {  return *DependentHfixGroups->Item(i);  }
  const TAfixGroup& GetDependentHfixGroup(size_t i) const {  return *DependentHfixGroups->Item(i);  }
  void RemoveDependentHfixGroup(TAfixGroup& hg) {  DependentHfixGroups->Remove(&hg);  }
  void ClearDependentHfixGroups() {  
    if( DependentHfixGroups != NULL ) DependentHfixGroups->Clear();
  }
  void AddDependentHfixGroup(TAfixGroup& hg) {
    if( DependentHfixGroups == NULL )  DependentHfixGroups = new TPtrList<TAfixGroup>;
    DependentHfixGroups->Add(&hg);
  }
  DefPropP(double, Occu)
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

  inline vec3d& ccrd()                {  return Center;  }
  inline vec3d const& ccrd()    const {  return Center;  }
  inline vec3d& ccrdEsd()             {  return Esd;  }
  inline vec3d const& ccrdEsd() const {  return Esd;  }
// IXVarReferencer implementation
  virtual size_t VarCount() const {  return 12;  }
  virtual const XVarReference* GetVarRef(size_t i) const {  
    if( i >= VarCount() )
      throw TInvalidArgumentException(__OlxSourceInfo, "var index");
    return Vars[i];  
  }
  virtual XVarReference* GetVarRef(size_t i)  {  
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
  virtual IXVarReferencerContainer& GetParentContainer();
  virtual double GetValue(size_t var_index) const;
  virtual void SetValue(size_t var_index, const double& val);
  virtual bool IsValid() const {  return !IsDeleted();  }
  virtual olxstr GetIdName() const {  return FLabel;  }
//
  void ToDataItem(TDataItem& item) const;
  void FromDataItem(TDataItem& item);
#ifndef _NO_PYTHON
  PyObject* PyExport();
#endif
  static int CompareAtomLabels(const olxstr& S, const olxstr& S1);
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
    if( a1->GetResiId() != a2->GetResiId() )          return olx_cmp_size_t(a1->GetResiId(),a2->GetResiId());
    // asc sort by label
    if( a1->GetAtomInfo().GetIndex() == a2->GetAtomInfo().GetIndex() )
      return TCAtom::CompareAtomLabels( a1->GetLabel(), a2->GetLabel());
    // desc sort my mr
    if( (a1->GetAtomInfo().GetIndex() - a2->GetAtomInfo().GetIndex() ) < 0 )  return 1;
    return -1;
  }
};
//..............................................................................
class TCAtomPCenterComparator  {
public:
  static int Compare(TCAtom* a1, TCAtom* a2)  {
    double p = a1->ccrd().QLength() - a2->ccrd().QLength();
    if( p < 0 )  return -1;
    return (p > 0) ? 1 : 0;
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

