#ifndef restraintsH
#define restraintsH

#include "xmodel.h"
#include "xscatlist.h"
#include "estrlist.h"
#include "estlist.h"

#include "indexlst.h"

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
const short
  shelxr_Distance = 1,
  shelxr_Angle    = 2,
  shelxr_Volume   = 3,
  shelxr_Complex  = 4;


class ARestraint  {
protected:
  XResidue* Resi;
  TPtrList<AScattererParamList> Allocated;
  inline AScattererParamList* AllocateList(const olxstr& scatterers) {  
    return Allocated.Add( XParamAtomListFactory::New(Parent, scatterers) );  
  }
public:
  ARestraint(IRefinementModel& parent) : Parent(parent), Resi(NULL) {  }
  virtual ~ARestraint() {
    for( int i=0; i < Allocated.Count(); i++ )
      delete Allocated[i];
  }
//  virtual bool Validate() const = 0;
  IRefinementModel& Parent;
};

class Restraint_Same : public ARestraint {
public:
  Restraint_Same(IRefinementModel& parent, 
                 const double defs[5], double s1=-1, double s2=-1) : 
                   ARestraint(parent), S1(s1), S2(s2), ReferenceAtoms(NULL)  {
    if( S1 == -1 )  S1 = defs[0];
    if( S2 == -1 )  S2 = S1*2;
  }
  void Init(const olxstr& scatterers)  {
    ReferenceAtoms = AllocateList(scatterers);
  }
  void AddDependent(const olxstr& scatterers)  {
    Dependent.Add( AllocateList(scatterers) );
  }

  AScattererParamList* ReferenceAtoms;
  TPtrList<AScattererParamList> Dependent;
  double S1, S2;
};

class Restraint_Dfix : public ARestraint {
public:
  Restraint_Dfix(IRefinementModel& parent, 
                 const double defs[5], double v, double s1=-1) : 
                   ARestraint(parent), S1(s1), Scatterers(NULL)  {
    if( S1 == -1 )  S1 = defs[0];
  }
  void Init(const olxstr& scatterers)  {
    Scatterers = AllocateList(scatterers);
  }
  double v, S1;
  AScattererParamList* Scatterers;
};

class Restraint_Dang : public ARestraint {
public:
  Restraint_Dang(IRefinementModel& parent, 
                 const double defs[5], double v, double s1=-1) : 
                   ARestraint(parent), S1(s1), Scatterers(NULL)  {
    if( S1 == -1 )  S1 = defs[0]*2;
  }
  void Init(const olxstr& scatterers)  {
    Scatterers = AllocateList(scatterers);
  }
  double v, S1;
  AScattererParamList* Scatterers;
};

class Restraint_Sadi : public ARestraint {
public:
  Restraint_Sadi(IRefinementModel& parent, 
                 const double defs[5], double s1=-1) : 
                   ARestraint(parent), S1(s1), Scatterers(NULL)  {
    if( S1 == -1 )  S1 = defs[0];
  }
  void Init(const olxstr& scatterers)  {
    Scatterers = AllocateList(scatterers);
  }
  double S1;
  AScattererParamList* Scatterers;
};

class Restraint_Chiv : public ARestraint {
public:
  Restraint_Chiv(IRefinementModel& parent, 
                 const double defs[5], double v = 0, double s1=-1) : 
                   ARestraint(parent), V(v), S1(s1), Scatterers(NULL)  {
    if( S1 == -1 )  S1 = defs[1];
  }
  void Init(const olxstr& scatterers)  {
    Scatterers = AllocateList(scatterers);
  }
  double V, S1;
  AScattererParamList* Scatterers;
};

class Restraint_Flat : public ARestraint {
public:
  Restraint_Flat(IRefinementModel& parent,
                 const double defs[5], double s1=-1) : 
                   ARestraint(parent), S1(s1), Scatterers(NULL)  {
    if( S1 == -1 )  S1 = defs[1];
  }
  void Init(const olxstr& scatterers)  {
    Scatterers = AllocateList(scatterers);
  }
  double S1;
  AScattererParamList* Scatterers;
};

class Restraint_Delu : public ARestraint {
public:
  Restraint_Delu(IRefinementModel& parent, 
                 const double defs[5], double s1=-1, double s2=-1) : 
                   ARestraint(parent), S1(s1), S2(s2), Scatterers(NULL)  {
    if( S1 == -1 )  S1 = defs[3];
    if( S2 == -1 )  S2 = S1;
  }
  void Init(const olxstr& scatterers)  {
    Scatterers = AllocateList(scatterers);
  }
  double S1, S2;
  AScattererParamList* Scatterers;
};

class Restraint_Simu : public ARestraint {
public:
  Restraint_Simu(IRefinementModel& parent, 
                 const double defs[5], double s1=-1, double s2=-1, double dmax = 1.7) : 
                   ARestraint(parent), S1(s1), S2(s2), Dmax(dmax), Scatterers(NULL)  {
    if( S1 == -1 )  S1 = defs[0];
    if( S2 == -1 )  S2 = S1*2;
  }
  void Init(const olxstr& scatterers)  {
    Scatterers = AllocateList(scatterers);
  }
  double S1, S2, Dmax;
  AScattererParamList* Scatterers;
};

class Restraint_Isor : public ARestraint {
public:
  Restraint_Isor(IRefinementModel& parent, 
                 const double defs[5], double s1=-1, double s2=-1) : 
                   ARestraint(parent), S1(s1), S2(s2), Scatterers(NULL) {
    if( S1 == -1 )  S1 = defs[1];
    if( S2 == -1 )  S2 = S1*2;
  }
  void Init(const olxstr& scatterers)  {
    Scatterers = AllocateList(scatterers);
  }
  double S1, S2;
  AScattererParamList* Scatterers;
};

class Restraint_Ncsy : public ARestraint {
public:
  Restraint_Ncsy(IRefinementModel& parent, 
                 const double defs[5], double s1=-1, double s2=-1) : 
                   ARestraint(parent), S1(s1), S2(s2), Scatterers(NULL) {
    if( S1 == -1 )  S1 = defs[1];
    if( S2 == -1 )  S2 = S1*5;
  }
  void Init(const olxstr& scatterers)  {
    Scatterers = AllocateList(scatterers);
  }
  double S1, S2;
  AScattererParamList* Scatterers;
};


class XRigidGroup : public ARestraint {
  XScattererPList Scatterers;
  short RigidGroup_Code;
  short RefinementType_Code;
protected:
  int AtomCount;
  double D1, D2;
public:
  XRigidGroup(IRefinementModel& container, short code, 
              short rt, double d1 = -1, double d2 = -1) : ARestraint(container),
                RigidGroup_Code(code), RefinementType_Code(rt), D1(d1), D2(d2)  {
    if( code == rg_SP31)
      AtomCount = 1;
    else if( code == rg_SP32 )
      AtomCount = 2;
    else if( code == rg_SP33 )
      AtomCount = 3;
    else if( code == rg_SP21 )
      AtomCount = 1;
    else if( code == rg_Pentagon )  {
      AtomCount = 5;
      if( D1 == -1 )  D1 = 1.42;
    }
    else if( code == rg_Hexagon_135 )  {
      AtomCount = 6;
      if( D1 == -1 )  D1 = 1.39;
    }
    else if( code == rg_Hexagon_any )  {
      AtomCount = 6;
      if( D1 == -1 )  D1 = 1.39;
    }
    else if( code == rg_O1_auto )
      AtomCount = 1;
    else if( code == rg_SP22 )
      AtomCount = 2;
    else if( code == rg_Cp_star )  {
      AtomCount = 10;
      if( D1 == -1 )  D1 = 1.42;
      if( D2 == -1 )  D2 = 1.63;  // Me-Cp sidtance
    }
    else if ( code == rg_Naphthalene )  {
      AtomCount = 10;
      if( D1 == -1 )  D1 = 1.39;
    }
    else if( code == rg_SP33_disorder )
      AtomCount = 6;  
    else if( code == rg_SP33_fourier )
      AtomCount = 3;  
    else if( code == rg_O1_fourier )
      AtomCount = 1;  
    else if( code == rg_BH )
      AtomCount = 1;  
    else
      AtomCount = container.GetReferenceSize(code); 
  }
  inline short GetGroupType()          const {  return RigidGroup_Code;  }
  inline short GetRefinementType() const {  return RefinementType_Code;  }
  void AddScatterer(XScatterer* xs)  {
    if( Scatterers.Count() + 1 > AtomCount )
      throw TInvalidArgumentException(__OlxSourceInfo, "too many atoms in the rigid group");
    Scatterers.Add(xs);
  }
};
/* number of scatterers might be greater than the numbre of thermal displacement
parameters or sites, since the latter can be shared
*/
class XModel : public IRefinementModel {
  smatd_list UsedSymm;
public:
  XModel() {
    Residues.Add( new XResidue(*this) );  // default residue
    Scale = 1;
  }
  virtual ~XModel() {
  }
  virtual int GetReferenceSize(int FragId) {  return References[FragId]->Count();  }
  inline int referenceCount() const        {  return References.Count();  }

  virtual int ScattererCount() const       {  return Scatterers.Count();  }
  virtual XScatterer& GetScatterer(int i)  {  return Scatterers[i];  }
  virtual XScatterer* FindScattererByName(const olxstr& name) {
    for( int i=0; i < Scatterers.Count(); i++ )
      if( Scatterers[i].Label.Comparei(name) == 0 )
        return &Scatterers[i];
    return NULL;
  }

  virtual inline XResidue* FindResidueByNumber(int Number) {
    for( int i=0; i < Residues.Count(); i++ )
      if( Residues[i].Number == Number )  
        return &Residues[i];
    return NULL;
  }
  virtual void FindResiduesByClass(const olxstr& clazz, TPtrList<XResidue>& res) {
    for( int i=0; i < Residues.Count(); i++ )
      if( Residues[i].ClassName.Comparei(clazz) == 0 )
        res.Add( &Residues[i] );
  }
  virtual XResidue* NextResidue(const XResidue& xs) {
    return (xs.Id+1) == Residues.Count() ? NULL : &Residues[xs.Id+1];
  }
  virtual XResidue* PrevResidue(const XResidue& xs) {
    return (xs.Id == 0) ? NULL : &Residues[xs.Id-1];
  }
  XResidue& NewResidue(const olxstr& clazzName, int Number, const olxstr& alias=EmptyString) {
    XResidue* xr = new XResidue(*this, clazzName, Number, alias);
    xr->Id = Residues.Count();
    return Residues.Add(*xr);
  }
  virtual void FindResidues(const olxstr& name, TPtrList<XResidue>& res) {
    if( name.IsEmpty() ) {
      res.Add( &Residues[0] );
      return;
    }
    FindResiduesByClass(name, res);
  }
  const smatd& AddUsedSymm(const smatd& matr) {
    int ind = UsedSymm.IndexOf(matr);
    smatd* rv = NULL;
    if( ind == -1 )  {
      rv = &UsedSymm.Add( *(new smatd(matr)) );
      rv->SetTag(1);
    }
    else  {
      UsedSymm[ind].IncTag();
      rv = &UsedSymm[ind];
    }
    return *rv;
  }
  void RemUsedSymm(const smatd& matr) {
    int ind = UsedSymm.IndexOf(matr);
    if( ind == -1 )
      throw TInvalidArgumentException(__OlxSourceInfo, "matrix is not in the list");
    UsedSymm[ind].DecTag();
    if( UsedSymm[ind].GetTag() == 0 )
      UsedSymm.Delete(ind);
  }
  virtual inline int UsedSymmCount() const {  
    return UsedSymm.Count();  
  }
  virtual inline const smatd* GetUsedSymm(int ind) const {  
    return &UsedSymm[ind];  
  }
  virtual inline int UsedSymmIndex(const smatd& matr)  const {  
    return UsedSymm.IndexOf(matr);  
  }
  inline void ClearUsedSymm()          {  UsedSymm.Clear();  }
  
  double Scale;  // global Fo/Fc scale
  TTypeList<XSite> Sites;
  TTypeList<XTDP> TDPs;
  // a list of all residues with key - number
  TTypeList<XResidue> Residues;
  // a list of all scattererers for quick access by label
  TTypeList<XScatterer> Scatterers;
  TPSTypeList<int, XFrag*> References;
  TTypeList<XRigidGroup> RigidGroups;
  TTypeList<XLinearEquation> LinearEquations;

  TTypeList<Restraint_Ncsy> NCSY;
  TTypeList<Restraint_Isor> ISOR;
  TTypeList<Restraint_Simu> SIMU;
  TTypeList<Restraint_Delu> DELU;
  TTypeList<Restraint_Flat> FLAT;
  TTypeList<Restraint_Chiv> CHIV;
  TTypeList<Restraint_Sadi> SADI;
  TTypeList<Restraint_Dfix> DFIX;
  TTypeList<Restraint_Dang> DANG;
  TTypeList<Restraint_Same> SAME;
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
