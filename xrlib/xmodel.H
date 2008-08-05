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
#include "indexlst.h"
#include "scat_it.h"
#include "xtdp.h"

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

const short  // relation of parameters to variables
  relation_None          = 0,
  relation_AsVar         = 1,
  relation_AsOneMinusVar = 2;

const short  // variable types
  var_type_None = 0,
  var_type_X    = 1,
  var_type_Y    = 2,
  var_type_Z    = 3,
  var_type_Sof  = 4,
  var_type_Uiso = 5;


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
class XScatterer;
class XSite;
class XResidue;
class XDisorderedGroup;

typedef TPtrList<XScatterer> XScattererPList;
typedef TPtrList<XSite> XSitePList;
//
struct XVar {
  double value;
  short type;   // type is required for calculation of derivatives
  XVar(double v=0, short t=var_type_None) : value(v), type(t)  {}
};
// 
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
  virtual XVar& NewVar(double v, short type) = 0;
  virtual int VarCount() const = 0;
  virtual XVar& GetVar(int index) = 0;
};
template <class Base> class XRefinableOptions {
  bool Refinable;
  short Relation; // one of the relation_ consts
  XVar* Var;
protected:
  Base* Owner;
  void (Base::*OnRefinableChange)(bool v);
  void (Base::*OnVarChange)(XVar* var);
  void (Base::*OnRelationChange)(short new_relation);
public:
  XRefinableOptions(Base* owner = NULL) : Owner(owner), Refinable(true), Relation(relation_None), 
    Var(NULL), OnRefinableChange(NULL), OnVarChange(NULL), OnRelationChange(NULL)  {  }
  XRefinableOptions(const XRefinableOptions& o) : Refinable(o.Refinable), Relation(o.Relation), 
    Var(o.Var), Owner(o.Owner), OnRefinableChange(o.OnRefinableChange), OnVarChange(o.OnVarChange), 
    OnRelationChange(o.OnRelationChange) {}
  // note that Owner will not be copied !!
  XRefinableOptions<Base>& operator = (const XRefinableOptions<Base>& o)  {
    Refinable = o.Refinable;
    Relation = o.Relation;
    Var = o.Var;
    OnRefinableChange = o.OnRefinableChange;
    OnVarChange = o.OnVarChange;
    OnRelationChange = o.OnRelationChange;
  }
  inline XRefinableOptions<Base>& SetRefinable(bool v)  {  
    Refinable = v;
    if( Owner != NULL && OnRefinableChange != NULL )  (Owner->*OnRefinableChange)(v);
    return *this;
  }
  inline bool IsRefinable() const {  return Refinable;  }
  inline XRefinableOptions<Base>& SetVar(XVar* var)  {
    Var = var;
    if( Owner != NULL && OnVarChange != NULL ) (Owner->*OnVarChange)(var);
    return *this;
  }
  inline XVar* GetVar() const {  return Var;  }
  inline XRefinableOptions<Base>& SetRelation(short relation)  {
    Relation = relation;
    if( Owner != NULL && OnRelationChange != NULL )  (Owner->*OnRelationChange)(relation);
    return *this;
  }
  inline short GetRelation() const {  return Relation;  }

  friend Base;
};
//
struct XEquationMember {
  double Ratio;
  XVar& Var;
  XEquationMember(double ratio, XVar& var) :
    Ratio(ratio), Var(var) {  }
};
/* SUMP */
struct XLinearEquation {
  XEquationMember& Add(double ratio, XVar& member)  {
    XEquationMember& rv = Members.Add( *(new XEquationMember(ratio, member)) );
    return rv;
  }
  XLinearEquation(double val, double sig) : value(val), sigma(sig) {}
  double value, sigma;
  TTypeList<XEquationMember> Members; 
};
//
/* scatterer is associated with a single TDp and a single site, however, several
scatterers might be involved. */
class XScatterer {
public:
  struct Scatterer  {
    struct _Ratio { // internal structure for refinable
    protected:
      void OnVarChange(XVar* var)  {
        if( var != NULL )  {
          if( var->type == var_type_None )  var->type = var_type_Sof;
          if( var->type != var_type_Sof )
            throw TInvalidArgumentException(__OlxSourceInfo, "variable type");
          if( options.GetRelation() == relation_AsVar )
            ratio = var->value;
          else if( options.GetRelation() == relation_AsOneMinusVar )
            ratio = 1.0 - var->value;
        }
      }
    public:
      double ratio;
      XRefinableOptions<_Ratio> options;
      _Ratio(double r=1) : ratio(r) {  
        options.Owner = this;
        options.OnVarChange = &_Ratio::OnVarChange;
      }
      _Ratio(const _Ratio& r) : ratio(r.ratio), options(r.options)  {
        options.Owner = this;
      }
      inline operator double& () {  return ratio;  }
      inline _Ratio& operator = (double r)  {  ratio = r;  return *this;  }
      inline _Ratio& operator = (const _Ratio& r)  {
        ratio = r.ratio;
        options = r.options;
      }
    };
    TBasicAtomInfo* type;
    olxstr label;
    _Ratio ratio;
    XScatterer& Owner; 
    double gaussians[9], fp, fdp;
    bool isFromLibrary, 
      isInitialised;  // will be false for Q-peaks or other elements not in the TScattererLib
    Scatterer(XScatterer& owner, const olxstr& _label, TBasicAtomInfo* _type, double _ratio) :
              label(_label), type(_type), ratio(_ratio), Owner(owner)  {  
      memset(&gaussians[0], 0, 9*sizeof(double));
      fp = fdp = 0;
      isInitialised = isFromLibrary = false;
    }
    Scatterer( const Scatterer& sc ) : label(sc.label), type(sc.type), ratio(sc.ratio), Owner(sc.Owner)  {
      memcpy(&gaussians[0], &sc.gaussians[0], 9*sizeof(double)); 
      fp = sc.fp;  fdp = sc.fdp;
      isFromLibrary = sc.isFromLibrary;
      isInitialised = sc.isInitialised;
    }
    Scatterer& operator = (const Scatterer& sc)  {
      label = sc.label;
      type = sc.type;
      ratio = sc.ratio;
      memcpy(&gaussians[0], &sc.gaussians[0], 9*sizeof(double)); 
      fp = sc.fp;  fdp = sc.fdp;
      isFromLibrary = sc.isFromLibrary;
      isInitialised = sc.isInitialised;
      return *this;
    }
  };
  struct _Crd {  // internal structure for refinable
    vec3d center, esd;
    XRefinableOptions<_Crd> options[3];
    _Crd()  {  }
    _Crd(const _Crd& c) : center(c.center), esd(c.esd)  {
      options[0] = c.options[0];
      options[1] = c.options[1];
      options[2] = c.options[2];
    }
    inline operator vec3d& () {  return center;  }
    inline _Crd& operator = (const vec3d& crd)  {  center = crd;  return *this;  }
    inline _Crd& operator = (const _Crd& c)  {
      center = c.center;
      esd = c.esd;
      options[0] = c.options[0];
      options[1] = c.options[1];
      options[2] = c.options[2];
    }
  };
  class _TDP {
  protected:
    XUani* uAni;
    XScatterer* uAniOwner;
    double scale, uIso;
    XRefinableOptions<_TDP> options;
    void OnVarChange(XVar* var)  {  // accsepts only relation_AsVar
      if( var == NULL )  {  return;  }
      else  {
        if( var->type == var_type_None )  var->type = var_type_Uiso;
        if( var->type != var_type_Uiso )
          throw TInvalidArgumentException(__OlxSourceInfo, "variable type");
        uAni = NULL;
        uAniOwner = NULL;
        uIso = var->value;
      }
    }
    void OnRefinableChange(bool v)  {
      if( v )  {
        if( uAniOwner != NULL )  {
          uAniOwner = NULL;
          options.Refinable = true;
        }
        else  {
          if( uAni != NULL )  uAni->refinable = true;
          else                options.Refinable = true;
        }
      }
      else  {
        if( uAniOwner == NULL )  {
          if( uAni != NULL )  uAni->refinable = false;
          else                options.Refinable = false;
        }
      }
    }
  public:
    _TDP() : uAni(NULL), uAniOwner(NULL), scale(1), uIso(0.05)  {
      options.Owner = this;
      options.OnVarChange = &_TDP::OnVarChange;
      options.OnRefinableChange = &_TDP::OnRefinableChange;
    }
    inline _TDP& SetUani(XUani* _uAni, XScatterer* _uAniOwner = NULL, double _scale=1.0)  {
      if( _uAniOwner != NULL &&  _uAniOwner->TDP.IsProxy() )
        throw TInvalidArgumentException(__OlxSourceInfo, "creating a proxy TDP to a proxy TDP");
      uAni = (_uAniOwner != NULL) ? NULL : _uAni;
      uAniOwner = _uAniOwner;
      scale = _scale;
      return *this;
    }
    inline XVar* GetVar() const {  return options.GetVar();  }
    inline bool IsRefinable()  const {  return uAniOwner != NULL ? false : (uAni != NULL ? uAni->refinable : options.IsRefinable());  }
    inline bool IsAnisotrpic() const {  return uAni != NULL;  }
    inline bool IsProxy()      const {  return uAniOwner != NULL;  }
    inline double GetUiso() const {  
      if( uAniOwner != NULL )
        return scale * uAniOwner->TDP.GetUiso();
      return ((uAni != NULL) ? uAni->GetUisoVal() : uIso);  
    }
  };
  struct _Occupancy {
  protected:
    void OnRelationChange(short v)  {
      if( options.Var == NULL )  return;
      if( v == relation_AsVar )
        occupancy = options.Var->value;
      else if( v == relation_AsOneMinusVar )
        occupancy = 1.0 - options.Var->value;
    }
    void OnVarChange(XVar* var)  {
      if( var != NULL )  {
        if( var->type == var_type_None )  var->type = var_type_Sof;
        if( var->type != var_type_Sof )
          throw TInvalidArgumentException(__OlxSourceInfo, "variable type");
        OnRelationChange( options.GetRelation() );
      }
    }
  public:
    double occupancy;
    XRefinableOptions<_Occupancy> options;
    _Occupancy(double o=1) : occupancy(o) {
      options.Owner = this;
      options.OnVarChange = &_Occupancy::OnVarChange;
    }
    _Occupancy(const _Occupancy& o) : occupancy(o.occupancy), options(o.options) {
      options.Owner = this;
    }
    inline _Occupancy& operator = (const _Occupancy& o)  {
      occupancy = o.occupancy;
      options = o.options;  // owners are not copied
      return *this;
    }
    inline _Occupancy& operator = (double v)  {  occupancy = v;  return *this;  }
    inline operator double& () {  return occupancy;  }
  };
private:
  TTypeList<XScatterer::Scatterer> Scatterers;
public:
  XScatterer(IRefinementModel& parent, double x, double y, double z) : 
      Parent(parent), Residue(NULL) {  
    Crd.center[0] = x;  Crd.center[1] = y;  Crd.center[2] = z;
    SiteOccupancy = 1;
  }
  _TDP TDP;  
  _Occupancy Occupancy;
  double SiteOccupancy;
  inline Scatterer& AddScatterer(const olxstr& label, TBasicAtomInfo* type, double ratio, double gaussians[9])  {
    Scatterer& sc = Scatterers.Add( new Scatterer(*this, label, type, ratio) );
    memcpy(&sc.gaussians[0], &gaussians[0], sizeof(double)*9);
    sc.isFromLibrary = false;
    sc.isInitialised = true;
    return sc;
  }
  inline Scatterer& AddScatterer(const olxstr& label, TBasicAtomInfo* type, double ratio, TScattererLib& scat_lib)  {
    Scatterer& sc = Scatterers.Add( new Scatterer(*this, label, type, ratio) );
    TLibScatterer* lib_sc = scat_lib.Find( type->GetSymbol() );
    if( lib_sc != NULL )  {
      if( lib_sc->Size() != 9 )
        throw TInvalidArgumentException(__OlxSourceInfo, "4 Gaussians data is expected");
      memcpy(&sc.gaussians[0], lib_sc->GetData(), sizeof(double)*9);
      sc.isInitialised = true;
    }
    return sc;
  }
  inline int Count()                          const {  return Scatterers.Count();  }
  inline Scatterer& operator [] (int i)             {  return Scatterers[i];  }
  inline const Scatterer& operator [] (int i) const {  return Scatterers[i];  }
  inline bool HasLabel(const olxstr& label)  const  {  return (IndexOfLabel(label) != -1);  }
  inline int IndexOfLabel(const olxstr& label)  const  {
    if( Scatterers.Count() == 1 )
      return (Scatterers[0].label.Comparei(label) == 0) ? 0 : -1;
    for( int i=0; i < Scatterers.Count(); i++ )
      if( Scatterers[i].label.Comparei(label) == 0 )
        return i;
    return -1;
  }
  // supports TBasicAtomInfo& and int
  template <class T> inline bool HasType(const T& type)  const  {  return IndexOfScatterer(type);  }
  // supports TBasicAtomInfo& and int
  template <class T> inline int IndexOfType(const T& type)  const  {
    if( Scatterers.Count() == 1 )
      return (*Scatterers[0].type == type) ? 0 : -1;
    for( int i=0; i < Scatterers.Count(); i++ )
      if( *Scatterers[i].type == type )
        return i;
    return false;
  }
  // supports TBasicAtomInfo& and int
  template <class T> inline bool IsType(const T& type) const {
    return (Scatterers.Count() == 1) ? *Scatterers[0].type == type : false;
  }
  inline const vec3d& GetCrd() const {  return Crd.center;  }
  inline double GetUiso()      const {  return TDP.GetUiso();  }
  XScatterer::_Crd Crd;

  XResidue* Residue;  // managed by the XResidue, when adding/removing
  IRefinementModel& Parent;
  // IRefinableOwner implementation
  virtual olxstr GetName() const {  
    if( Scatterers.Count() == 1 )
      return Scatterers[0].label;
    olxstr rv;
    for( int i=0; i < Scatterers.Count(); i++ )  {
      rv << Scatterers[i].label;
      if( (i+1) < Scatterers.Count() )
        rv << '&';
    }
    return rv;  
  }
};
//
struct XScattererRef {
  XScatterer::Scatterer* scatterer;
  smatd const* symm; // this is borrowed from the Refine model
  XScattererRef() : scatterer(NULL), symm(NULL) {}
  XScattererRef(XScatterer::Scatterer* _xs, smatd const* _symm) : scatterer(_xs), symm(_symm) {}
  XScattererRef& operator = (const XScattererRef& sr)  {
    scatterer = sr.scatterer;
    symm = sr.symm;
    return *this;
  }
  olxstr GetLabel() const;
};
typedef TTypeList<XScattererRef> XScattererRefList;
/* We cannot use indexes here, as scatterer labels might change at runtime */
class XResidue {
  TPtrList<XScatterer> Scatterers;
public:
  XResidue(IRefinementModel& parent, const olxstr& cl=EmptyString, int number = 0, 
    const olxstr& alias=EmptyString) : Parent(parent), ClassName(cl), Number(number), Alias(alias) {  }

  inline int Count() const {  return Scatterers.Count();  }
  inline XScatterer& operator [] (int i)             {  return *Scatterers[i]; }
  inline XScatterer const& operator [] (int i) const {  return *Scatterers[i]; }
  inline XScatterer::Scatterer* FindScattererByName(const olxstr& name) {
    for( int i=0; i < Scatterers.Count(); i++ )  {
      int ind = Scatterers[i]->IndexOfLabel(name);
      if( ind != -1 )  return &(*Scatterers[i])[ind];
    }
    return NULL;
  }
  inline int IndexOf(XScatterer* xs) const {  return Scatterers.IndexOf(xs);  }
  XResidue& operator = (const XResidue& res)  {
    Scatterers = res.Scatterers;
    ClassName = res.ClassName;
    Number = res.Number;
    Alias = res.Alias;
    return *this;
  }
  inline XResidue& SetCapacity(int c)  {  Scatterers.SetCapacity(c);  return *this;  }
  inline void AddScatterer(XScatterer* xs)  {
    if( xs->Residue != NULL )
      xs->Residue->RemoveScatterer(xs);
    xs->Residue = this;
    Scatterers.Add(xs);
  }
  inline void RemoveScatterer(XScatterer* xs)  {
    int ind = Scatterers.IndexOf(xs);
    if( ind != -1 )  {
      Scatterers[ind]->Residue = NULL;
      Scatterers.Delete(ind);
    }
  }
  IRefinementModel& Parent;
  olxstr ClassName, Alias;
  int Number, Id;
};
//
//class XDisorderedGroup : public XRefinable, public IRefinableOwner {
//public:
//  XDisorderedGroup() : XRefinable("Occupancy")  {
//    XRefinable::Owner = this;
//  }
//  TPtrList<XScatterer::Scatterer> Scatterers; // list of scatterers in the group
//  void AddScatterer(XScatterer* xs)  {
//    if( xs->Occupancy != NULL ) 
//      xs->Occupancy->SetOwner(xs, NULL);
//    xs->Occupancy = this;
//  }
//  XScatterer::Scatterer* RemoveScatterer(XScatterer* xs)  {
//    Scatterers.Remove(xs);
//    xs->Occupancy = NULL;
//    return xs;
//  }
//  virtual void SetOwner(IRefinableOwner* sender, IRefinableOwner* owner) {  
//    if( owner == NULL )  
//      Scatterers.Remove((XScatterer*)sender);
//    else
//      Scatterers.Add( (XScatterer*)owner);
//  }
//  virtual IRefinableOwner* GetOwner() const {  return XRefinable::Owner;  }
//  // IRefinableOwner implementation
//  virtual olxstr GetName() const {  
//    olxstr rv;
//    for( int i=0; i < Scatterers.Count(); i++ )  {
//      rv << Scatterers[i]->GetName();  
//      if( (i+1) < Scatterers.Count() )
//        rv << ',';
//    }
//    return rv;
//  }
//};
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
  mutable olxstr Expression;
  virtual bool GetExplicit() const = 0; 
  virtual void DoExpand(IRefinementModel& parent, XResidue* resi, XScattererRefList& ag) = 0;
public:
  AScattererParamList(const olxstr& expr) : Expression(expr)  {  }
  virtual ~AScattererParamList() {}
  inline bool IsExplicit()   const { return GetExplicit();  }
  inline bool IsExpandable() const {  return !GetExplicit();  }
  inline void Expand(IRefinementModel& parent, XResidue* cres, XScattererRefList& ag)  {  
    DoExpand(parent, cres, ag);  
  }  
  // returns original string representation
  virtual const olxstr& StrRepr() const {  return Expression;  }
};

EndXlibNamespace()
#endif

