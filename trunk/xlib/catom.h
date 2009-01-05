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

BeginXlibNamespace()

//struct Parameter  {
//  double Value;
//  bool Refinable;
//};

class TEllipsoid;
class TAfixGroup;
class TAfixGroups;
class TExyzGroup;
class TExyzGroups;

class TCAtom: public ACollectionItem  {
private:
  class TAsymmUnit *FParent;
  TBasicAtomInfo*  FAtomInfo;    // a pointer to TBasisAtomInfo object
  olxstr FLabel;    // atom's label
  int     Id, Tag;       // c_atoms id; this is also used to identify if TSAtoms are the same
  int     LoaderId; // id of the atom in the asymmertic unit of the loader
  int     ResiId, SameId, EllpId;   // residue and SADI ID
  double  Occu;     // occupancy and its variable
  double  Uiso, // isotropic thermal parameter; use it when Ellipsoid = NULL
    QPeak,      // if atom is a peak atom - this is not 0
    UisoEsd,    //
    UisoScale;  // for proxied Uiso (depending on the UisoOwner, this defines the scaled value
  TCAtom* UisoOwner;  // the Uiso owner, if any
  int     FragmentId;   // this is used in asymmetric unit sort and initialised in TLatice::InitBody()
  vec3d Center, Esd;  // fractional coordinates and esds
  short Part, Degeneracy;
  bool Deleted, 
    Saved;  // is true the atoms already saved (to work aroung SAME, AFIX)
  bool CanBeGrown,
       HAttached;  // used for the hadd command
  TPtrList<TCAtom>* FAttachedAtoms, *FAttachedAtomsI;
  /* Afix group is a fitted group, Hfix group the immediate dependent group */
  TAfixGroup* DependentAfixGroup, *ParentAfixGroup;
  TPtrList<TAfixGroup>* DependentHfixGroups;
  TExyzGroup* ExyzGroup;
  TPtrList<XVarReference> Vars;  // 12 variable pointers
public:
  TCAtom(TAsymmUnit *Parent);
  virtual ~TCAtom();
  inline TAsymmUnit* GetParent()       const {  return FParent; }
  inline TBasicAtomInfo& GetAtomInfo() const {  return *FAtomInfo; }
  void SetAtomInfo(TBasicAtomInfo* A);

  inline olxstr& Label()                   {  return FLabel; }
  // function validates and changes the atom type, use the syntax above just to set the label
  bool SetLabel(const olxstr &L);

  inline const olxstr& GetLabel()  const   {  return FLabel;  }

  inline int AttachedAtomCount()       const {
    return FAttachedAtoms == NULL ? 0 : FAttachedAtoms->Count();
  }
  inline TCAtom& GetAttachedAtom(int i)       const {  return *FAttachedAtoms->Item(i);  }
  void AttachAtom(TCAtom *CA);
  inline bool IsAttachedTo(TCAtom& CA) const {
    return FAttachedAtoms == NULL ? false : FAttachedAtoms->IndexOf(&CA) != -1;
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

  inline int AttachedAtomICount()      const {
    return FAttachedAtomsI == NULL ? 0 : FAttachedAtomsI->Count();
  }
  inline TCAtom& GetAttachedAtomI(int i)      const {  return *FAttachedAtomsI->Item(i);  }
  void AttachAtomI(TCAtom *CA);
  inline bool IsAttachedToI(TCAtom& CA)const {
    return FAttachedAtomsI == NULL ? false : FAttachedAtomsI->IndexOf(&CA) != -1;
  }
  // beware - just the memory addresses compared!
  inline bool operator == (const TCAtom& ca)  const  {  return this == &ca;  }
  inline bool operator == (const TCAtom* ca)  const  {  return this == ca;  }
  inline bool operator != (const TCAtom& ca)  const  {  return this != &ca;  }
  inline bool operator != (const TCAtom* ca)  const  {  return this != ca;  }

//  TAtomsInfo *AtomsInfo() const;
  void  Assign(const TCAtom& S);

  DefPropP(int, Id)
  DefPropP(int, Tag)
  DefPropP(int, LoaderId)
  DefPropP(int, ResiId)
  DefPropP(int, SameId)
  DefPropP(int, EllpId)
  DefPropP(int, FragmentId)
  DefPropP(TExyzGroup*, ExyzGroup)
//  short   Frag;    // fragment index
  DefPropP(short, Degeneracy)
  DefPropP(short, Part)
  int GetAfix() const;
  DefPropP(TAfixGroup*, ParentAfixGroup)
  DefPropP(TAfixGroup*, DependentAfixGroup)
  int DependentHfixGroupCount() const {  return DependentHfixGroups == NULL ? 0 : DependentHfixGroups->Count();  }
  TAfixGroup& GetDependentHfixGroup(int i) {  return *DependentHfixGroups->Item(i);  }
  const TAfixGroup& GetDependentHfixGroup(int i) const {  return *DependentHfixGroups->Item(i);  }
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
  DefPropB(Deleted)
  DefPropB(HAttached)
  DefPropB(Saved)
  // can be grown is set by UnitCell::Init
  DefPropP(bool, CanBeGrown)

  void SetVarRef(XVarReference& vr)  {
    Vars[vr.var_name] = &vr;
  }
  void NullVarRef(short param_name)  {
    Vars[param_name] = NULL;
  }
  XVarReference* GetVarRef(short i) {
    return Vars[i];
  }
  TEllipsoid* GetEllipsoid() const;
  void UpdateEllp( const TEllipsoid& NV);
  void AssignEllp(TEllipsoid *NV);

  inline vec3d& ccrd()                {  return Center;  }
  inline vec3d const& ccrd()    const {  return Center;  }
  inline vec3d& ccrdEsd()             {  return Esd;  }
  inline vec3d const& ccrdEsd() const {  return Esd;  }

  void ToDataItem(TDataItem& item) const;
  void FromDataItem(TDataItem& item);

  static int CompareAtomLabels(const olxstr& S, const olxstr& S1);
};
//..............................................................................
  typedef TPtrList<TCAtom> TCAtomPList;
  typedef TTypeList<TCAtom> TCAtomList;
//..............................................................................
class TCAtomPComparator  {
public:
  static int Compare(TCAtom* a1, TCAtom* a2)  {
    if( a1->GetFragmentId() != a2->GetFragmentId() )  return a1->GetFragmentId() - a2->GetFragmentId();
    if( a1->GetResiId() != a2->GetResiId() )          return a1->GetResiId() - a2->GetResiId();
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
  DefPropP(TCAtom*, Atom)
  inline const smatd* GetMatrix() const {  return Matrix;  }
  DefPropC(olxstr, Name)
  olxstr GetFullLabel(class RefinementModel& rm) const;
};
//....................................................................................
typedef TTypeList<TGroupCAtom> TCAtomGroup;
//..............................................................................


EndXlibNamespace()
#endif

