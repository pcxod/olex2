/*
OLEX crystallographic model, (c) O Dolomanov, 2008
*/
// !!!! change to ifdef
#ifndef __OLX_XMODEL_H
#define __OLX_XMODEL_H

#include "xbase.h"
#include "symmat.h"
#include "scat_it.h"
#include "atominfo.h"
#include "bapp.h"
#include "log.h"

BeginXlibNamespace()
// hydrogen treatment and other rigid groups, m in shelx AFXI
const short
  rg_SP31          = 1, 
  rg_SP32          = 2, 
  rg_SP33          = 3, 
  rg_SP21          = 4, 
  rg_Pentagon      = 5,
  rg_Hexagon_135   = 6,
  rg_Hexagon_any   = 7,
  rg_O1_auto       = 8,
  rg_SP22          = 9,
  rg_Cp_star       = 10,
  rg_Naphthalene   = 11,
  rg_SP33_disorder = 12,
  rg_SP33_fourier  = 13,
  rg_O1_fourier    = 14,
  rg_BH            = 15,
  rg_SP11          = 16,
  rg_Refence       = 17; // any after 16


// refinemnt attributes
const short
  rt_Refine_none      = 1,  // all fixed
  rt_Refine_crd       = 2,  // all but coordinates fixed
  rt_Riding_d_fixed   = 3,  // riging ridgid group
  rt_Riding_d_free    = 4,  // riding 'breathing'
  rt_Ridgid_d_fixed   = 6,  // rotaiting, with bonds fixed
  rt_Rotating_d_fixed = 7,  // rotaiting, with bonds fixed
  rt_Rotating_d_free  = 8,  // rotating, with bond refined
  rt_Ridgid_d_free    = 9;  // rotating, with bond refined

struct XDependency;
struct XLinearEquation;
class XTDP;
class XScatterer;
class XSite;
class XResidue;

typedef TPtrList<XScatterer> XScattererPList;
// for the rigid groups
class IRefinementModel {
public:
  virtual int GetReferenceSize(int FragId) = 0;
  virtual int ScattererCount() const = 0;
  virtual XScatterer& GetScatterer(int i) = 0;
  virtual XScatterer* FindScattererByName(const olxstr& name) = 0;
  virtual XResidue* FindResidueByNumber(int Number) = 0;
  virtual XResidue* NextResidue(const XResidue& xs) = 0;
  virtual XResidue* PrevResidue(const XResidue& xs) = 0;
  virtual void FindResiduesByClass(const olxstr& clazz, TPtrList<XResidue>& res) = 0;
  // finds residue by class name, if name is empty - adds the default residue
  virtual void FindResidues(const olxstr& name, TPtrList<XResidue>& res) = 0;
  virtual int UsedSymmCount() const = 0;
  virtual const smatd* GetUsedSymm(int i) const = 0;
  virtual int UsedSymmIndex(const smatd& symm) const = 0;
};
// for the refinables
class IRefinableOwner {
public:
  virtual ~IRefinableOwner() {  }
  virtual int Count() const = 0;
  virtual const olxstr& GetName(int i) const = 0;
};
//
struct XRefinable {
  double Value, Esd;
  bool Refinable;
  TPtrList<XLinearEquation> Equations;
  IRefinableOwner* Owner;
  olxstr Name;
  XRefinable(const olxstr& name = EmptyString) : 
    Name(name), Value(0), Esd(0), Refinable(false), Owner(NULL)  {  }
  inline bool IsDependent()      const {  return !Equations.IsEmpty();  }
};
//
struct XEquationMember {
  double Ratio;
  XRefinable* Refinable;
  XEquationMember() : Ratio(1), Refinable(NULL) {}
  XEquationMember(double ratio, XRefinable* refinable) :
    Ratio(ratio), Refinable(refinable) {  }
};
/* SUMP, +- free var, dependent Uiso 
  if there is only one member, that member Ratio has to be used
*/
struct XLinearEquation {
  XRefinable& Add(XRefinable& member, double ratio)  {
    Members.Add( *(new XEquationMember(ratio, &member)) );
    member.Equations.Add(this);
    return member;
  }
  XLinearEquation(double val, double sig) : Value(val), Sigma(sig) {}
  double Value, Sigma;
  TTypeList<XEquationMember> Members; 
};
//
class XScatterer : public IRefinableOwner {
public:
  XScatterer(const olxstr& label, TBasicAtomInfo* type) : 
      Label(label), Type(type), Owner(NULL), Occupancy("Occupancy") {
        Occupancy.Owner = this;
  }
  XRefinable Occupancy;
  XSite* Site; // might be shared by several scatterers
  XTDP* TDP;   // might be shared by several scatterers
  XResidue* Owner;  // managed by the group, when adding/removing
//  IRefinementModel* Parent;
  olxstr Label;
  int Id;      // must be synchronised with the position in the list
  TBasicAtomInfo* Type;
  // IRefinableOwner implementation
  virtual int Count()            const {  return 1;  }
  virtual const olxstr& GetName(int i) const {  return Label;  }
};
// thremal displacement parameter
class XTDP : public IRefinableOwner {
public:
  XRefinable Uani[6]; 
  enum Uind { U11=0, U22, U33, U23, U13, U12 };
  XTDP() {
    Uani[0].Name = "U11";  Uani[0].Owner = this;
    Uani[1].Name = "U22";  Uani[1].Owner = this;
    Uani[2].Name = "U33";  Uani[2].Owner = this;
    Uani[3].Name = "U23";  Uani[3].Owner = this;
    Uani[4].Name = "U13";  Uani[4].Owner = this;
    Uani[5].Name = "U12";  Uani[5].Owner = this;
  }
  void SetAniso(bool v)  {
    Uani[0].Name = v ? "U11" : "Uiso";
  }
  double GetUisoVal() const {
    if( !IsAniso() )  return Uani[0].Value;
    return (Uani[0].Value+Uani[1].Value+Uani[2].Value)/3;
  }
  bool IsAniso() const {  return Uani[0].Name.Length() == 2;  }
  TPtrList<XScatterer> Scatterers;  // list of scatterers sharing the TDP
  // IRefinableOwner implementation
  virtual int Count()            const {  return Scatterers.Count();  }
  virtual const olxstr& GetName(int i) const {  return Scatterers[i]->Label;  }
};
//
class XSite : public IRefinableOwner{
public:
  XSite() : SiteOccupancy(1) {
    Crd[0].Name = "X";  Crd[0].Owner = this;
    Crd[1].Name = "Y";  Crd[1].Owner = this;
    Crd[2].Name = "Z";  Crd[2].Owner = this;
  }
  inline double const& Esd(int i)       const {  return Crd[i].Esd;  }
  inline double& Esd(int i)                   {  return Crd[i].Esd;  }
  inline vec3d CrdAsVec()               const {  return vec3d(Crd[0].Value, Crd[1].Value, Crd[2].Value);  }
  inline void SetCrd(const vec3d& v)               {  Crd[0].Value = v[0];  Crd[1].Value = v[1];  Crd[2].Value = v[2];  }
  inline void SetCrd(double x, double y, double z) {  Crd[0].Value = x;  Crd[1].Value = y;  Crd[2].Value = z;  }
  inline bool IsShared()                const {  return Scatterers.Count() > 1;  }
  olxstr GetLabel()  {
    if( Scatterers.IsEmpty() )  return EmptyString;
    if( Scatterers.Count() == 1 )  return Scatterers[0]->Label;
    olxstr rv(Scatterers[0]->Label);
    for( int i=1; i < Scatterers.Count() - 1; i++ )
      rv << "," << Scatterers[i]->Label;
    return rv << " and " << Scatterers.Last()->Label;
  }

  XRefinable Crd[3]; // fractional crds
  TPtrList<XScatterer> Scatterers; // list of scatterers sharing the site
  double SiteOccupancy;  // crystallographic occupancy
  // IRefinableOwner implementation
  virtual int Count()            const {  return Scatterers.Count();  }
  virtual const olxstr& GetName(int i) const {  return Scatterers[i]->Label;  }
};
//
struct XScattererRef {
  XScatterer* scatterer;
  smatd const* symm; // this is borrowed from the Refine model
  XScattererRef() : scatterer(NULL), symm(NULL) {}
  XScattererRef(XScatterer* _xs, smatd const* _symm) : scatterer(_xs), symm(_symm) {}
  XScattererRef& operator = (const XScattererRef& sr)  {
    scatterer = sr.scatterer;
    symm = sr.symm;
    return *this;
  }
  olxstr GetLabel() const;
};
typedef TTypeList<XScattererRef> XScattererRefList;

class XResidue {
  TSStrPObjList<olxstr, AnAssociation2<XScatterer*,int>, true> Scatterers;
public:
  XResidue(IRefinementModel& parent, const olxstr& cl=EmptyString, int number = 0, 
    const olxstr& alias=EmptyString) : Parent(parent), ClassName(cl), Number(number), Alias(alias) {  }

  inline int Count() const {  return Scatterers.Count();  }
  inline XScatterer& operator [] (int i)             {  return *Scatterers.Object(i).A(); }
  inline XScatterer const& operator [] (int i) const {  return *Scatterers.GetObject(i).GetA(); }
  inline XScatterer* FindScattererByName(const olxstr& name) {
    int i = Scatterers.IndexOf(name);
    return i == -1 ? NULL : Scatterers.Object(i).A();
  }
  // returns the scatterers in the original order
  void GetScatterers( TPtrList<XScatterer>& res )  {
    const int si = res.Count();
    res.SetCount( res.Count() + Scatterers.Count() );
    for( int i=0; i < Scatterers.Count(); i++ )
      res[si+Scatterers.Object(i).GetB()] = Scatterers.Object(i).A();
  }
  XResidue& operator = (const XResidue& res)  {
    Scatterers.Clear();
    Scatterers.SetCapacity(res.Count() );
    for( int i=0; i < res.Count(); i++ )
      AddScatterer( &Parent.GetScatterer(res[i].Id) );
    ClassName = res.ClassName;
    Number = res.Number;
    Alias = res.Alias;
    return *this;
  }
  inline void SetCapacity(int c)  {  Scatterers.SetCapacity(c);  }
  inline void AddScatterer(XScatterer* xs)  {
    Scatterers.Add(xs->Label, AnAssociation2<XScatterer*,int>(xs, Scatterers.Count()) );
    if( xs->Owner != NULL )
      xs->Owner->RemoveScatterer(xs);
    xs->Owner = this;
  }
  inline void RemoveScatterer(XScatterer* xs)  {
    int ind = -1;
    for( int i=0; i < Scatterers.Count(); i++ )
      if( Scatterers.GetObject(i).GetA() == xs )  {
        ind = i;
        break;
      }
    if( ind = -1 )  {
      Scatterers.Object(ind).A()->Owner = NULL;
      Scatterers.Delete(ind);
    }
  }
  IRefinementModel& Parent;
  olxstr ClassName, Alias;
  int Number, Id;
};
//
// FRAG reference data implementation
class XFrag {
  struct XFragAtom {
    vec3d Crd;
    TBasicAtomInfo* Type; // though ignored in shelx, can be used for validation
    olxstr Label;
    XFragAtom(const olxstr& label, TBasicAtomInfo* bai, const vec3d& crd) :
      Label(label), Type(bai), Crd(crd)  {  } 
  };
  // members
  mat3d Cell2Cartesian;
  TTypeList<XFragAtom> Atoms;
  bool Cartesian;
public:
  XFrag(double cell[6])  {
    if( cell[0] == cell[1] && cell[1] == cell[2] && cell[2] == 1 &&
      cell[3] == cell[4] && cell[4] == cell[5] && cell[5] == 90 ) {
      Cartesian = true;
    }
    else  {
      Cartesian = false;
      double cG = cos(cell[5]/180*M_PI),
        cB = cos(cell[4]/180*M_PI),
        cA = cos(cell[3]/180*M_PI),
        sG = sin(cell[5]/180*M_PI),
        sB = sin(cell[4]/180*M_PI),
        sA = sin(cell[3]/180*M_PI);
      double cs = sG/(cell[2]*sqrt( (1-cA*cA-cB*cB-cG*cG) + 2*(cA*cB*cG)));

      Cell2Cartesian[0][0] = cell[0];
      Cell2Cartesian[1][0] = cell[1]*cG;
      Cell2Cartesian[2][0] = cell[2]*cB;
      Cell2Cartesian[1][1] = cell[1]*sG;
      Cell2Cartesian[2][1] = -cell[2]*(cB*cG-cA)/sG;
      Cell2Cartesian[2][2] = 1./cs;
    }
  }
  inline int Count()                    const {  return Atoms.Count();  }
  inline vec3d GetCrd(int i)            const {  return Cartesian ? Atoms[i].Crd : Atoms[i].Crd*Cell2Cartesian;  }
  inline TBasicAtomInfo* GetType(int i) const { return Atoms[i].Type;  }
  inline const olxstr& GetLabel(int i)  const {  return Atoms[i].Label;  }
  inline void AddAtom(const olxstr& label, TBasicAtomInfo* bai, const vec3d& crd)  {
    Atoms.AddNew(label, bai, crd);
  }
};

class AScattererParamList  {
protected:
  virtual bool GetExplicit() const = 0; 
  virtual void DoExpand(IRefinementModel& parent, XResidue* resi, XScattererRefList& ag) = 0;
public:
  AScattererParamList()  {  }
  virtual ~AScattererParamList() {}
  inline bool IsExplicit()   const { return GetExplicit();  }
  inline bool IsExpandable() const {  return !GetExplicit();  }
  inline void Expand(IRefinementModel& parent, XResidue* cres, XScattererRefList& ag)  {  
    DoExpand(parent, cres, ag);  
  }  
};

EndXlibNamespace()
#endif

