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
//
struct XRefinable {
  double Value, Esd;
  bool Refinable;
  XLinearEquation* Equation;
  XSite* Scatterer;
  olxstr Name;
  XRefinable(const olxstr& name = EmptyString) : 
    Name(name), Value(0), Esd(0), Refinable(false), Dependency(NULL)  {  }
  inline bool IsDependent()      const {  return Equation != NULL;  }
};
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
struct XTDP  {
  XRefinable Uani[6]; 
  enum Uind { U11=0, U22, U33, U23, U13, U12 };
  XTDP() {
    Uani[0].Name = "U11";
    Uani[1].Name = "U22";
    Uani[2].Name = "U33";
    Uani[3].Name = "U23";
    Uani[4].Name = "U13";
    Uani[5].Name = "U12";
  }
  void SetAniso(bool v)  {
    Uani[0].Name = v ? "U11" : "Uiso";
  }
  double GetUisoVal() const {
    if( !IsAniso() )  return Uani[0].Value;
    return (Uani[0].Value+Uani[1].Value+Uani[2].Value)/3;
  }
  bool IsAniso() const {  return Uani[0].Length() == 2;  }
};
class XScatterer  {
public:
  XScatterer(const olxstr& label, TBasicInfo* type) : 
      Label(label), Type(type), Ocupancy("Occupansy") {
  }
  XRefinable Occupancy;
  XTDP* TDP;  // might beshared by several scatterers
  olxstr Label;
  TBasicAtomInfo* Type;
};

class XSite {
public:
  XSite() {
    Crd[0].Name = "X";
    Crd[1].Name = "Y";
    Crd[2].Name = "Z";
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
  TPtrList<XScatterer> Scatterers;
  double SiteOccupancy;  // crystallographic occupancy
};
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
  virtual void DoExpand(TAsymmUnit& parent, TAymmUnit::TResidue* resi, TCAtomGroup& ag) = 0;
public:
  AAtomList(TAsymmUnit& p)  {  }
  virtual ~AAtomList() {}
  bool IsExplicit() const { return GetExplicit();  }
  bool IsExpandable() const {  return !GetExplicit();  }
  void Expand(TAsymmUnit& parent, TCAtomGroup& ag)  {  DoExpand(parent, ag);  }  
};
/* explicit atoms list represent a set of atoms, referenced explicetely by name, residue name
or symmetry equivalent, the list is can be evaluated at the creation time */
class TExplicitAtomList : public AAtomList {
  TCAtomGroup atoms;
protected:
  virtual void DoExpand(TAsymmUnit& parent, TAymmUnit::TResidue* resi, TCAtomGroup& ag)  {  ag = atoms;  }
  virtual boll GetExplicit() const {  return true;  }
public:
  TExplicitAtomList(TAsymmUnit& parent, const olxstr& exp) : AAtomList()  { 
    TAtomReference ar(exp);
    ar._Expand(Parent, atoms, NULL);
  }

};
/* expandable atoms list maight contain indirection operators < or > and residue scrolling
 +,- operators. The list can be evalueated at any time, but might change at runtime */
class TExpandableAtomList : public AAtomList {
  TStrPObjList<olxstr, TGroupCAtom*> atoms;
protected:
  virtual void DoExpand(TAsymmUnit& parent, TAymmUnit::TResidue* resi, TCAtomGroup& ag)  {
    for( int i=0; i < atoms.Count(); i++ )  {
      if( atoms.Object(i) != NULL ) 
        atoms[i] = atoms.Object(i)->GetFullLabel();
    }
    TAtomReference ar( atoms.Text(' ') );
    ar._Expand(Parent, ag, resi);
  }
  virtual bool GetExplicit() const {  return false;  }
public:
  TExpandableAtomList(TAsymmUnit& parent, const olxstr& expression) : atoms(expression, ' ')  {
    for( int i=0; i < atoms.Count(); i++ )  {
      if( atoms[i].CharAt(0) == '$' )  //SFAC
        continue;
      if( atoms[i].IndexOf('+') != -1 || atoms[i].IndexOf('-') != -1 || // residue scrolling
          atoms[i].IndexOf('*') !+ -1 ) //reference to all residues
        continue;
      // here should be only proper atoms
      int eq_ind = atoms[i].IndexOf('$');
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
        olxstr aname = ( (resi_ind == -1) ? Expression : Expression.SubStringTo(resi_ind) );
        TPtrList<TAsymmUnit::TResidue> residues;
        parent.FindResidues(res_name, residues);
        if( residues.Count() == 1 )  {
          for( int i=0; i < residues[0]->Count(); i++ )  {
            TCAtom& ca = residues[0]->GetAtom();
            if( ca.IsDeleted() )  continue;
            if( ca.GetLabel().Comparei(aname) == 0 )  {
              atoms.Object(i) = new TGroupCAtom(&ca, eqiv);
              break;
            }
          }
        }
      }
    }
  }
  virtual TExpandableAtomList()  {
    for( int i=0; i < atoms.Count(); i++ )
      if( atoms.Object(i) != NULL )  delete atoms.Object(i);
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

class IRefinementModelContainer {
public:
  virtual int GetFragmentSize(int FragId) = 0;
};

class ARigidGroup : public ARestraint {
  TCAtomPList Atoms;
  RigidGroup RigidGroup_Code;
  RefinementType RefinementType_Code;
protected:
  int AtomCount;
  double DefD;
public:
  ARigidGroup(IRefinementModelContainer& container, RigidGroup code, 
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
      throw TInvalidArgumentException(__OlxSourceInfo, "too many atoms in the ridgid group");
  }
};

class XModel  {
  double Scale;  // global Fo/Fc scale
  TArrayList<XScatterer> Scatterers;
  TArrayList<Sites> Sites;
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
