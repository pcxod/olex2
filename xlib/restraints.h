#ifndef restraintsH
#define restraintsH

#include "ematrix.h"
#include "atomref.h"
#include "asymmunit.h"
#include "catom.h"
#include "estrlist.h"
#include "estlist.h"

BeginXlibNamespace()

const short  rpCrdX = 0x0001,
             rpCrdY = 0x0002,
             rpCrdZ = 0x0004,
             rpCrds = rpCrdX|rpCrdY|rpCrdZ,
             rpSof  = 0x0008,
             rpUiso = 0x0010,
             rpUani = 0x0020;

const short  rtChiv = 0x0001,
             rtFlat = 0x0002,
             rtSadi = 0x0004,
             rtSame = 0x0006,
             rtIsor = 0x0010,
             rtDelu = 0x0020,
             rtSemu = 0x0040,
             rtDang = 0x0080,
             rtDfix = 0x0100,
             rtExyz = 0x0200,
             rtEadp = 0x0400;

// hydrogen treatment and other rigid groups, m in shelx AFXI
enum RigidGroup {
  SP31          = 1, 
  SP32          = 2, 
  SP33          = 3, 
  SP21          = 4, 
  Pentagon      = 5,
  Hexagon_135   = 6,
  Hexagon_any   = 7,
  O1_auto       = 8,
  SP22          = 9,
  Cp_star       = 10,
  Naphthalene   = 11
  SP33_disorder = 12,
  SP33_fourier  = 13,
  O1_fourier    = 14,
  BH            = 15,
  SP11          = 16,
  Refence // any after 16
};

// refinemnt attributes
enum RefinementType {
  Refine_none      = 1,  // all fixed
  Refine_crd       = 2,  // all but coordinates fixed
  Riding_d_fixed   = 3,  // riging ridgid group
  Riding_d_free    = 4,  // riding 'breathing'
  Ridgid_d_fixed   = 6,  // rotaiting, with bonds fixed
  Rotating_d_fixed = 7,  // rotaiting, with bonds fixed
  Rotating_d_free  = 8,  // rotating, with bond refined
  Ridgid_d_free    = 9   // rotating, with bond refined
};
struct XDependency;
struct XLinearEquation;
class XScatterer;
class XSite;
class XScattererGroup;
// for the rigid groups
class IRefinementModel {
public:
  virtual int GetFragmentSize(int FragId) = 0;
  virtual int ScattererCount() const = 0;
  virtual XScatterer& GetScatterer(int i) = 0;
  virtual XScatterer* FindScattererByName(const olxstr& name) = 0;
  virtual XScattererGroup* FindResidueByNumber(int Number) = 0;
  virtual XScatterer* NextResidue(XScattererGroup* xs) = 0;
  virtual XScatterer* PrevResidue(XScattererGroup* xs) = ;
  virtual void FindResiduesByClass(const olxstr& clazz, TPtrList<XScattererGroup>& res) = 0;
};
// for the refinables
class IRefinableOwner {
public:
  virtual ~IRefinableOwner() {  }
  virtual int Count() const = 0;
  virtual olxstr& GetName(int i) const = 0;
};
//
struct XRefinable {
  double Value, Esd;
  bool Refinable;
  XLinearEquation* Equation;
  IRefinableOwner* Owner;
  olxstr Name;
  XRefinable(const olxstr& name = EmptyString) : 
    Name(name), Value(0), Esd(0), Refinable(false), Dependency(NULL), Owner(NULL)  {  }
  inline bool IsDependent()      const {  return Equation != NULL;  }
};
//
struct XEquationMameber {
  double Ratio;
  XRefinable* Refinable;
  XEquationMameber() : Ratio(1), Refinable(NULL) {}
  XEquationMameber(double ratio, XRefinable* refinable) :
    Ratio(ratio), Refinable(refinable) {  }
};
/* SUMP, +- free var, dependent Uiso 
  if there is only one member, that member Ratio has to be used
*/
struct XLinearEquation {
  double Value, Sigma;
  TArrayList<XRefinable> Members; 
  XLinearEquation(double val, double sig) : value(val), Sigma(sig) {}
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
  bool IsAniso() const {  return Uani[0].Length() == 2;  }
  TPtrList<XScatterer> Scatterers;  // list of scatterers sharing the TDP
  // IRefinableOwner implementation
  virtual int Count()            const {  return Scatterers.Count();  }
  virtual olxstr& GetName(int i) const {  return Scatterers[i]->Label;  }
};
//
class XSite : public IRefinableOwner{
public:
  XSite() : SiteOccupancy(1) {
    Crd[0].Name = "X";  Crd[0].Owner = this;
    Crd[1].Name = "Y";  Crd[1].Owner = this;
    Crd[2].Name = "Z";  Crd[2].Owner = this;
  }
  inline double const& Crd(int i)       const {  return Crd[i].Value;  }
  inline double& Crd(int i)                   {  return Crd[i].Value;  }
  inline double const& Esd(int i)       const {  return Crd[i].Esd;  }
  inline double& Esd(int i)                   {  return Crd[i].Esd;  }
  inline vec3d CrdAsVec()               const {  return vec3d(Crd[0].Value, Crd[1].Value, Crd[2].Value);  }
  inline SetCrd(const vec3d& v)               {  Crd[0].Value = v[0];  Crd[1].Value = v[1];  Crd[2].Value = v[2];  }
  inline SetCrd(double x, double y, double z) {  Crd[0].Value = x;  Crd[1].Value = y;  Crd[2].Value = z;  }
  inline bool IsShared()                const {  return Scatterers.Count() > 1;  }
  olxstr GetLabel()  {
    if( Scattrerers.IsEmpty() )  return EmptyString;
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
  virtual olxstr& GetName(int i) const {  return Scatterers[i]->Label;  }
};

class XScatterer : public IRefinableOwner {
public:
  XScatterer(const olxstr& label, TBasicInfo* type) : 
      Label(label), Type(type), Owner(NULL), Ocupancy("Occupancy") {
        Occupancy.Owner = this;
  }
  XRefinable Occupancy;
  XSite* Site; // might be shared by several scatterers
  XTDP* TDP;   // might be shared by several scatterers
  XScattererGroup* Owner;  // managed by the group, when adding/removing
  olxstr Label;
  int Id;      // must be synchronised with the position in the list
  TBasicAtomInfo* Type;
  // IRefinableOwner implementation
  virtual int Count()            const {  return 1;  }
  virtual olxstr& GetName(int i) const {  return Label;  }
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
};
typedef TTypeList<XScattererRef> XScattererRefList;

class XScattererGroup {
  IRefinementModel& Parent;
  TSStrPObjList<olxstr, AnAssociation2<XScatterer*,int>, true> Scatterers;
public:
  XScattererGroup(IRefinementModel& parent, const olxstr& cl=EmptyString, int number = 0, 
    const olxstr& alias=EmptyString) : Parent(parent), ClassName(cl), Number(number), Alias(alias) {  }

  inline int Count() const {  return SortedScatterers.Count();  }
  inline XScatterer& operator [] (int i)             {  return *Scatterers.Object(i).A(); }
  inline XScatterer const& operator [] (int i) const {  return *Scatterers.Object(i).GetA(); }
  inline XScatterer* FindScattererByName(const olxstr& name) {
    int i = Scatterers.IndexOf(name);
    return i == -1 ? NULL : Scatterers.Object(i).B();
  }
  // returns the scatterers in the original order
  void GetScatterers( TPtrList<XScatterer>& res )  {
    const int si = res.Count();
    res.SetCount( res.Count() + Scatterers.Count() );
    for( int i=0; i < Scatterers.Count(); i++ )
      res[si+Scatterers.Object(i).GetB()] = Scatterers.Object(i).A();
  }
  XScattererGroup& operator = (const XScattererGroup& res)  {
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
    Scatterers.Add(xs->Label, Sxs(xs, Scatterers.Count(0));
    if( xs->Owner != NULL )
      xs->Owner->RemoveScatterer(xs);
    xs->Owner = this;
  }
  inline void RemoveScatterer(XScatterer* xs)  {
    int i = Scatterers.IndexOfObject(xs);
    if( ind = -1 )  {
      Scatterers.Object(i).A()->Owner = NULL;
      Scatterers.Delete(i);
    }
  }
  olxstr ClassName, Alias;
  int Number;
};
//
// FRAG reference data implementation
class XFrag {
  mat3d Cell2Cartesian;
  TTypeList<XFragAtom> Atoms;
  struct XFragAtom {
    vec3d Crd;
    TBasicAtomInfo* Type; // though ignored in shelx, can be used for validation
    olxstr Label;
    XFragAtom(const olxstr& label, TBasicAtomInfo* bai, const vec3d& crd) :
      Label(label), Type(bai), Crd(crd)  {  } 
  };
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
  inline vec3d GetCrd(int i)            const {  return Cartesian ? Atoms[i].Crd : Atoms[i].Crd*FragToCartesian;  }
  inline TBasicAtomInfo* GetType(int i) const { return Atoms[i].Type;  }
  inline const olxstr& GetLabel(int i)  const {  return Atoms[i].Label;  }
  inline void AddAtom(const olxstr& label, TBasicAtomInfo* bai, vec3d crd)  {
    Atoms.AddNew(labels, bai, crd);
  }
};

class AAtomList  {
protected:
  virtual bool GetExplicit() const = 0; 
  virtual void DoExpand(IRefinementModel& parent, XScattererGroup* resi, XScattererRefList& ag) = 0;
public:
  AAtomList()  {  }
  virtual ~AAtomList() {}
  inline bool IsExplicit()   const { return GetExplicit();  }
  inline bool IsExpandable() const {  return !GetExplicit();  }
  inline void Expand(IRefinementModel& parent, XScattererRefList& ag)  {  DoExpand(parent, ag);  }  
};
/* explicit atoms list represent a set of atoms, referenced explicetely by name, residue name
or symmetry equivalent, the list is can be evaluated at the creation time */
class TExplicitAtomList : public AAtomList {
  XScattererRefList atoms;
protected:
  virtual void DoExpand(IRefinementModel& parent, XScattererGroup* resi, TCAtomGroup& ag)  { 
    ag = atoms;  
  }
  virtual boll GetExplicit() const {  return true;  }
public:
  TExplicitAtomList(IRefinementModel& parent, const olxstr& exp) : AAtomList()  { 
    TAtomReference ar(exp);
    ar._Expand(Parent, atoms, NULL);
  }

};
/* expandable atoms list maight contain indirection operators < or > and residue scrolling
 +,- operators. The list can be evalueated at any time, but might change at runtime */
class TExpandableAtomList : public AAtomList {
  TStrPObjList<olxstr, XScattererRef*> atoms;
protected:
  virtual void DoExpand(IRefinementModel& parent, XScattererGroup* resi, XScattererRefList& ag)  {
    for( int i=0; i < atoms.Count(); i++ )  {
      if( atoms.Object(i) != NULL ) 
        atoms[i] = atoms.Object(i)->GetFullLabel();
    }
    TAtomReference ar( atoms.Text(' ') );
    ar._Expand(Parent, ag, resi);
  }
  virtual bool GetExplicit() const {  return false;  }
  inline bool IsValidScatterer(XScatterer* xs)  {
    return !(*xs->Type == iHydrogenIndex ||
             *xs->Type == iDeuteriumIndex ||
             *xs->Type == iQPeakIndex );
  }
public:
  TExpandableAtomList(IRefinementModel& parent, const olxstr& expression) : atoms(expression, ' ')  {
    for( int i=0; i < atoms.Count(); i++ )  {
      if( atoms[i].CharAt(0) == '$' )  //SFAC
        continue;
      if( atoms[i].IndexOf('+') != -1 || atoms[i].IndexOf('-') != -1 || // residue scrolling
          atoms[i].IndexOf('*') !+ -1 || //reference to all residues
          atoms[i].CharAt(0) == '>' || atoms[i].CharAt(0) == '<' ) //atom indirection
        continue;
      // here should be only proper atoms
      int eq_ind = atoms[i].IndexOf('$');
      int resi_ind = Expression.IndexOf('_');
      olxstr resi_name = (resi_ind == -1 ? EmptyString : Expression.SubStringFrom(resi_ind+1));
      // check if it is just an equivalent position
      const smatd* eqiv = NULL;
      int eqiv_ind = resi_name.IndexOf('$');
      if( eqiv_ind != -1 )  {  // 0 is for SFAC type, skipped above
        olxstr str_eqiv( resi_name.SubStringFrom(eqiv_ind+1) );
        if( !str_eqiv.IsNumber() )  throw TInvalidArgumentException(__OlxSourceInfo, olxstr("equivalent id: ") << str_eqiv);
        int eqi = str_eqiv.ToInt()-1;
        if( eqi < 0 || eqi >= au.UsedSymmCount() )  throw TInvalidArgumentException(__OlxSourceInfo, olxstr("equivalent index: ") << str_eqiv);
        eqiv = &au.GetUsedSymm(eqi);
        resi_name = resi_name.SubStringTo(eqiv_ind);
      }
      if( !resi_name.IsEmpty() && !resi_name.IsNumber() )  // must be a number
        throw TInvalidArgumentException(__OlxSourceInfo, "invalid residue number");
      olxstr aname = ( (resi_ind == -1) ? Expression : Expression.SubStringTo(resi_ind) );
      XScattererGroup* resi = parent.FindResidueByNumber(resi_name.ToInt());
      if( resi == NULL )  {
        TBasicApp::GetLog() << (olxstr("invalid residue '") << resi_name << "' in [" << expression << ']' );
        continue;
      }
      XScatterer* xs = resi->FindScattererByName(aname);
      if( xs == NULL )  {
        TBasicApp::GetLog() << (olxstr("invalid atom '") << aname << "' in [" << expression << ']' );
        continue;
      }
      atoms.Object(i) = new XScattererRef(&ca, eqiv);
    }
  }
  virtual ~TExpandableAtomList()  {
    for( int i=0; i < atoms.Count(); i++ )
      if( atoms.Object(i) != NULL )  delete atoms.Object(i);
  }
  int _Expand(IRefinementModel& parent, const olxstr& expr, 
              XScattererRefList& scatterers, XScattererGroup& resi, TPtrList<XScatterer>& resi_cont)  {
    if( resi == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "invalid residue provided");
    if( resi->Count() == 0 )  return 0;
    int xsc = scatterers.Count();
    if( expr.IsEmpty() )  {  // all atoms of residue
        scatterers.SetCapacity( scatterers.Count() + resi->Count() );
        for( int i=0; i < resi->Count(); i++ )  {
          XScatterer* xs = &(*resi)[i];
          if( IsValidScatterer(xs) )
            scatterers.AddNew(xs, NULL);
        }
        return atoms.Count() - xsc;
    }
    else if( expr.Comparei("first") == 0 )  { 
      int i=0;
      XScatterer* xs = resi_cont[i];
      while( (i+1) < resi_cont.Count() && !IsValidScatterer(xs) )  {
        i++;
        xs = resi_cont[i];
      }
      if( !IsValidAtom(xs) )  return 0;
      scatterers.AddNew(xs, NULL);
      return 1;
    }
    else if( expr.Comparei("last") == 0 )  { 
      int i=resi_cont.Count()-1;
      XScatterer* xs = resi_cont[i];
      while( (i-1) >= 0 && !IsValidScatterer(xs) )  {
        i--;
        xs = resi_cont[i];
      }
      if( !IsValidAtom(xs) )  return 0;
      scatterers.AddNew(xs, NULL);
      return 1;
    }
    // validate complex expressions with >< chars
    int gs_ind = expr.IndexOf('>'),
        ls_ind = expr.IndexOf('<');
    if( gs_ind != -1 || ls_ind != -1 )  {
      TCAtomGroup from, to;
      if( gs_ind != -1 )  {  // it is inverted in shelx ...
        TAtomReference(Expression.SubStringTo(gs_ind).Trim(' '))._Expand(au, from, CurrResi);
        TAtomReference(Expression.SubStringFrom(gs_ind+1).Trim(' '))._Expand(au, to, CurrResi);
      }
      else  {
        TAtomReference(Expression.SubStringTo(ls_ind).Trim(' '))._Expand(au, to, CurrResi);
        TAtomReference(Expression.SubStringFrom(ls_ind+1).Trim(' '))._Expand(au, from, CurrResi);
      }
      if( from.Count() != 1 )
        throw TFunctionFailedException(__OlxSourceInfo, "failed to expand >/< expression: invalid 'from'");
      if( to.Count() != 1)
        throw TFunctionFailedException(__OlxSourceInfo, "failed to expand >/< expression: invalid 'to'");
      if( from[0].GetAtom()->GetId() >= to[0].GetAtom()->GetId() )
        throw TFunctionFailedException(__OlxSourceInfo, "invalid direction");
      if( from[0].GetMatrix() != to[0].GetMatrix() )
        throw TFunctionFailedException(__OlxSourceInfo, "EQIV must be the same in >/< expresion");
      if( gs_ind != -1 )  {
        for( int i=from[0].GetAtom()->GetId(); i <= to[0].GetAtom()->GetId(); i++ )  {
          TCAtom* ca = &au.GetAtom(i);
          if( !IsValidAtom(ca) )  continue;
          atoms.AddNew( ca, from[0].GetMatrix() );
        }
      }
      else  {
        for( int i=to[0].GetAtom()->GetId(); i >= from[0].GetAtom()->GetId(); i-- )  {
          TCAtom* ca = &au.GetAtom(i);
          if( !IsValidAtom(ca) )  continue;
          atoms.AddNew( ca, from[0].GetMatrix() );
        }
      }
      return atoms.Count() - ac;
    }
    //
    int resi_ind = Expression.IndexOf('_');
    olxstr resi_name = (resi_ind == -1 ? EmptyString : Expression.SubStringFrom(resi_ind+1));
    // check if it is just an equivalent position
    const smatd* eqiv = NULL;
    int eqiv_ind = resi_name.IndexOf('$');
    if( eqiv_ind > 0 )  {  // 0 is for SFAC type
      olxstr str_eqiv( resi_name.SubStringFrom(eqiv_ind+1) );
      if( !str_eqiv.IsNumber() )  throw TInvalidArgumentException(__OlxSourceInfo, olxstr("equivalent id: ") << str_eqiv);
      int eqi = str_eqiv.ToInt()-1;
      if( eqi < 0 || eqi >= au.UsedSymmCount() )  throw TInvalidArgumentException(__OlxSourceInfo, olxstr("equivalent index: ") << str_eqiv);
      eqiv = &au.GetUsedSymm(eqi);
      resi_name = resi_name.SubStringTo(eqiv_ind);
    }
    // validate syntax
    TPtrList<TAsymmUnit::TResidue> residues;
    if( !resi_name.IsEmpty() && (resi_name.CharAt(0) == '+' || resi_name.CharAt(0) == '-') )  {
      if( CurrResi == NULL )  throw TInvalidArgumentException(__OlxSourceInfo, "current residue");
      if( resi_name.CharAt(0) == '+' )  residues.Add(au.NextResidue(*CurrResi));
      else                              residues.Add(au.PrevResidue(*CurrResi));
    }
    else  {
      if( CurrResi != NULL )  residues.Add(CurrResi);
      if( !resi_name.IsEmpty() )  // empty resi name refers to all atom outside RESI
        au.FindResidues(resi_name, residues);  
      if( residues.IsEmpty() )  throw TInvalidArgumentException(__OlxSourceInfo, olxstr("invalid residue class/number: ") << resi_name);
    }
    if( Expression.CharAt(0) == '$' )  {  // sfac type
      olxstr sfac = ((resi_ind == -1) ? Expression.SubStringFrom(1) : Expression.SubString(1, resi_ind-1));
      TBasicAtomInfo* bai = au.GetAtomsInfo()->FindAtomInfoBySymbol(sfac);
      if( bai == NULL )  throw TInvalidArgumentException(__OlxSourceInfo, olxstr("sfac=") << sfac);
      for( int i=0; i < residues.Count(); i++ )  {
        for( int j=0; j < residues[i]->Count(); j++ )  {
          TCAtom* ca = &residues[i]->GetAtom(j);
          if( !ca->IsDeleted() && ca->GetAtomInfo() == *bai )  // cannot use IsValid here, $H woill not work
            atoms.AddNew( ca, eqiv );
        }
      }
    }
    else  {  // just an atom
      olxstr aname = ( (resi_ind == -1) ? Expression : Expression.SubStringTo(resi_ind) );
      for( int i=0; i < residues.Count(); i++ )  {
        if( residues[i] == NULL )  continue;
        for( int j=0; j < residues[i]->Count(); j++ )  {
          TCAtom* ca = &residues[i]->GetAtom(j);
          if( !ca->IsDeleted() && ca->GetLabel().Comparei(aname) == 0 )  {  // must be unique!
            atoms.AddNew( ca, eqiv );
            break;
          }
        }
      }
    }
    return atoms.Count() - ac;
  }
};

class TAtomListFactory {
public:
  static AAtomList* New(TAsymmUnit& parent, const olxstr& exp)  {
    if( exp.IndexOf('$') == 0 ||  // sfac 
        exp.IndexOf('*') == 0 ||  // all residues
        exp.IndexOf('>') != -1 || exp.indexOf('<') != -1 || // residue indirections
        exp.indexOf('+') != -1 || exp.indexOf('-') != -1 )  // resifue scrolling
      return new TExpandableAtomList(exp);
    else
      return new TExplicitAtomList(parent, exp);
  }
};

class ARestraint  {
  olxstr ResiName;
  virtual bool GetExplicit() const = 0; 
  virtual void DoExpand(TAsymmUnit& parent, TArrayList<ARestraint>& rl) = 0;
public:
  ARestraint() {  }
  bool IsExplicit() const { return GetExplicit();  }
  bool IsExpandable() const {  return !GetExplicit();  }
  void Expand(TAsymmUnit& parent, TArrayList<ARestraint>& rl)  {  DoExpand(parent, rl);  }  
};

class ARigidGroup : public ARestraint {
  TCAtomPList Atoms;
  RigidGroup RigidGroup_Code;
  RefinementType RefinementType_Code;
protected:
  int AtomCount;
  double DefD;
public:
  ARigidGroup(IRefinementModel& container, RigidGroup code, 
    RefinementType rt, double d = -1) : RigidGrope_Code(code), RefinementType_Code(rt), DefD(d)  {
    switch( code )  {
      case RigidGroup::SP31:
        AtomCount = 1;
        break;
      case RigidGroup::SP32:
        AtomCount = 2;
        break;
      case RigidGroup::SP33:
        AtomCount = 3;
        break;
      case RigidGroup::SP21:
        AtomCount = 1;
        break;
      case RigidGroup::Pentagon:
        AtomCount = 5;
        if( DefD == -1 )  DefD = 1.42;
        break;
      case RigidGroup::Hexagon_135:
        AtomCount = 6;
        if( DefD == -1 )  DefD = 1.39;
        break;
      case RigidGroup::Hexagon_any:
        AtomCount = 6;
        if( DefD == -1 )  DefD = 1.39;
        break;
      case RigidGroup::O1_auto:
        AtomCount = 1;
        break;
      case RigidGroup::SP22:
        AtomCount = 2;
        break;
      case RigidGroup::Cp_star:
        AtomCount = 10;
        if( DefD == -1 )  DefD = 1.42;
        break;
      case RigidGroup::Naphthalene:
        AtomCount = 10;
        if( DefD == -1 )  DefD = 1.39;
        break;
      case RigidGroup::SP33_disorder:
        AtomCount = 6;  
        break;
      case RigidGroup::SP33_fourier:
        AtomCount = 3;  
        break;
      case RigidGroup::O1_fourier:
        AtomCount = 1;  
        break;
      case RigidGroup::BH:
        AtomCount = 1;  
        break;
      default:
        AtomCount = container.GetFragmentSize(RigidGroupeCode); 
    }
  }
  inline RigidGroup GetGroupType()          const {  return RigidGroup_Code;  }
  inline RefinementType GetRefinementType() const {  return RefinementType_Code;  }
  void AddAtom(TCAtom& ca)  {
    if( Atoms.Count() + 1 > AtomCount )
      throw TInvalidArgumentException(__OlxSourceInfo, "too many atoms in the rigid group");
  }
};
/* number of scatterers might be greater than the numbre of thermal displacement
parameters or sites, since the latter can be shared
*/
class XModel : public IRefinementModel {
public:
  virtual int ScattererCount() const {  return Scatterers.Count();  }
  virtual XScatterer& GetScatterer(int i)  {  return Scatterers[i];  }
  virtual XScatterer* FindScattererByName(const olxstr& name) {
    return SortedScatterers[name];
  }
  virtual XScattererGroup* FindResidueByNumber(int Number) {
    return Rsidues[Number];
  }
  virtual void FindResiduesByClass(const olxstr& clazz, TPtrList<XScattererGroup>& res) {
    for( int i=0; i < Residues.Count(); i++ )
      if( Residues.Object(i)->ClassName.Comparei(clazz) == 0 )
        res.Add( Residues.Object(i);
  }

  double Scale;  // global Fo/Fc scale
  TTypeList<XScatterer> Scatterers;
  TTypeList<Sites> Sites;
  TTypeList<XTDP> TDPs;
  // a list of all residues with key - number
  TPSTypeList<int, XScattererGroup*> Residues;
  // a list of all scattererers for quick access by label
  TSStrPObjList<olxstr, XScatterer, false> SortedScatterers;
  TPSTypeList<int, XFrag*> References;
};

class TRefinementModel {
};

// could be a list of distances, angles tortion angles etc
// vovers: SADI or SAME
class TSimRestraintList : public IEObject  {
  TTypeList<TRAtomGroup> Groups;
  double Esd;
public:
  TSimRestraintList()  {  }
  virtual ~TSimRestraintList()  {  }

  inline int Count() const {  return Groups.Count();  }
  TRAtomGroup& operator[] (int i)  const {  return Groups[i];  }
};
// covers DFIX, DANG
class TDisConstraintList : public IEObject  {
  TTypeList<TRAtomGroup> Groups;
  double Esd, Value;
public:
  TDisConstraintList()  {  }
  virtual ~TDisConstraintList()  {  }

  inline int Count() const {  return Groups.Count();  }
  TRAtomGroup& operator[] (int i)  const {  return Groups[i];  }
};
// covers: SUMP and free varibles for sof
class TSumpConstraintList : public IEObject  {
  TTypeList<TRAtomGroup> Groups;
  double Esd, Value;
public:
  TSumpConstraintList()  {  }
  virtual ~TSumpConstraintList()  {  }

  inline int Count() const {  return Groups.Count();  }
  TRAtomGroup& operator[] (int i)  const {  return Groups[i];  }
};

/* this would cover:  AFIX  */
class TRidingGroup  {
  TRAtomGroup Group;
  TRestraintAtom BaseAtoms;
  int FixedParams; // crd, sof, U(Uij)
public:

};

// covers: EXYZ, EADP
class TEquivAtomParams : public  TRAtomGroup {
  short Param; // XYZ or Uiso or Uani
};



/* this would cover:  PART
  Overridden params can be only sof for SHELX
*/
class TDisorderPart : public  TRAtomGroup {
  int OverriddenParams;
  evecd ParamValues;
public:

};

/*

class ASiteRestraint {
  TTypeList <TRestraintAtom> Atoms;
  double Value, Esd;
  virtual bool Apply(basiccfile, testringlist& list) = 0;
};

class ARestraintParam  {
  TTypeList <TRestraintAtom> Atoms;
public:
  virtual double GetValue() const = 0;
  
  inline int GetAtomCount()  const  {  return Atoms.Count();  }
  inline TRestraintAtom& GetAtom(int i)  const {  return Atoms[i];  }
  inline TRestraintAtom& operator [](int i)  const {  return Atoms[i];  }
  
};    

class TRestraintDistance : public ARestraintParam  {
public:
  virtual double GetValue() const  {
    vec3d v1, v2;
    v1 = GetAtom(0).GetAtom()->ccrd();  
    if( GetAtom(0).GetSymm() )  {
       v1 *= *GetAtom(0).GetSymm();
       v1[0] += GetAtom(0).GetSymm()->Data(0)[3];
       v1[1] += GetAtom(0).GetSymm()->Data(1)[3];
       v1[2] += GetAtom(0).GetSymm()->Data(2)[3];
    }  
    
    v2 = GetAtom(1).GetAtom()->ccrd();  
    if( GetAtom(1).GetSymm() )  {
       v2 *= *GetAtom(1).GetSymm();
       v2[0] += GetAtom(1).GetSymm()->Data(0)[3];
       v2[1] += GetAtom(1).GetSymm()->Data(1)[3];
       v2[2] += GetAtom(1).GetSymm()->Data(2)[3];
    }  
    GetAtom(0).GetAtom()->Parent()->CellToCartesian( v1 );
    GetAtom(1).GetAtom()->Parent()->CellToCartesian( v2 );
    return v1.DistanceTo( v2 );
  }    
};    

class TRestraintAngle : public ARestraintParam  {
public:
  virtual double GetValue() const  {
    vec3d v1, v2, v0;
    v1 = GetAtom(0).GetAtom()->ccrd();  
    if( GetAtom(0).GetSymm() )  {
       v1 *= *GetAtom(0).GetSymm();
       v1[0] += GetAtom(0).GetSymm()->Data(0)[3];
       v1[1] += GetAtom(0).GetSymm()->Data(1)[3];
       v1[2] += GetAtom(0).GetSymm()->Data(2)[3];
    }  
    
    v0 = GetAtom(1).GetAtom()->ccrd();  
    if( GetAtom(1).GetSymm() )  {
       v0 *= *GetAtom(1).GetSymm();
       v0[0] += GetAtom(1).GetSymm()->Data(0)[3];
       v0[1] += GetAtom(1).GetSymm()->Data(1)[3];
       v0[2] += GetAtom(1).GetSymm()->Data(2)[3];
    }  

    v2 = GetAtom(2).GetAtom()->ccrd();  
    if( GetAtom(2).GetSymm() )  {
       v2 *= *GetAtom(2).GetSymm();
       v2[0] += GetAtom(2).GetSymm()->Data(0)[3];
       v2[1] += GetAtom(2).GetSymm()->Data(1)[3];
       v2[2] += GetAtom(2).GetSymm()->Data(2)[3];
    }  
    GetAtom(0).GetAtom()->Parent()->CellToCartesian( v1 );
    GetAtom(1).GetAtom()->Parent()->CellToCartesian( v0 );
    GetAtom(2).GetAtom()->Parent()->CellToCartesian( v2 );
    v1 -= v0;
    v2 -= v0;
    double cang = v1.CAngle(v2);
    return acos(cang)*180/M_PI;
  }    
};    

class TRestraintTAngle : public ARestraintParam  {
public:
  virtual double GetValue() const  {
    vec3d v1, v01, v2, v02;
    vec3d B, D;
    v1 = GetAtom(0).GetAtom()->ccrd();  
    if( GetAtom(0).GetSymm() )  {
       v1 *= *GetAtom(0).GetSymm();
       v1[0] += GetAtom(0).GetSymm()->Data(0)[3];
       v1[1] += GetAtom(0).GetSymm()->Data(1)[3];
       v1[2] += GetAtom(0).GetSymm()->Data(2)[3];
    }  
    
    v01 = GetAtom(1).GetAtom()->ccrd();  
    if( GetAtom(1).GetSymm() )  {
       v01 *= *GetAtom(1).GetSymm();
       v01[0] += GetAtom(1).GetSymm()->Data(0)[3];
       v01[1] += GetAtom(1).GetSymm()->Data(1)[3];
       v01[2] += GetAtom(1).GetSymm()->Data(2)[3];
    }  

    v02 = GetAtom(2).GetAtom()->ccrd();  
    if( GetAtom(2).GetSymm() )  {
       v02 *= *GetAtom(2).GetSymm();
       v02[0] += GetAtom(2).GetSymm()->Data(0)[3];
       v02[1] += GetAtom(2).GetSymm()->Data(1)[3];
       v02[2] += GetAtom(2).GetSymm()->Data(2)[3];
    }  

    v2 = GetAtom(3).GetAtom()->ccrd();  
    if( GetAtom(3).GetSymm() )  {
       v2 *= *GetAtom(3).GetSymm();
       v2[0] += GetAtom(3).GetSymm()->Data(0)[3];
       v2[1] += GetAtom(3).GetSymm()->Data(1)[3];
       v2[2] += GetAtom(3).GetSymm()->Data(2)[3];
    }  

    GetAtom(0).GetAtom()->Parent()->CellToCartesian( v1 );
    GetAtom(1).GetAtom()->Parent()->CellToCartesian( v01 );
    GetAtom(2).GetAtom()->Parent()->CellToCartesian( v02 );
    GetAtom(3).GetAtom()->Parent()->CellToCartesian( v2 );
    
    v1 -= v01;
    v2 -= v02;
    
    B = v02 - v01;
    D = v01 - v02;
    
    v01 = v1.XProdVec(B);
    v02 = v2.XProdVec(D);
    double cang = v01.CAngle(v02);
    return acos(cang)*180/M_PI;
  }    
};    

class TREquation  {

  ARestraint* A, *B;
  relation ==, summ == 
};

// sadi 0.02 a1 a2 a1 a3
// create two TRestraintDistance (a1,a2) and (a1,a3)
// add equation 

class AFileFormatRestraintDictionary  {
  "distance", "angle", "tangle", "rigid group"

}; */
EndXlibNamespace()

#endif
