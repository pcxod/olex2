#ifndef restraintsH
#define restraintsH

#include "ematrix.h"
//#include "evector.h"
#include "catom.h"

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

class TRestraintAtom {
  TCAtom* Atom;
  smatd* Symm;
public:
  TRestraintAtom(TCAtom* A)  {  Atom = A; Symm = NULL;  }
  TRestraintAtom(TCAtom* A, smatd* M)  {
    Atom = A;
    if( M != NULL )
      Symm = new smatd( *M );
    else
      Symm = NULL;
  }
  TRestraintAtom(const TRestraintAtom& ra)  {
    Atom = ra.GetAtom();
    if( ra.GetSymm() != NULL )
      Symm = new smatd( *ra.GetSymm() );
    else
      Symm = NULL;
  }
  virtual ~TRestraintAtom()  {
    if( Symm )  delete Symm;
  }

  inline TCAtom*   GetAtom() const {  return Atom;  }
  inline smatd* GetSymm()    const {  return Symm;  }
  inline void SetAtom(TCAtom* a)   {  Atom = a;  }
  void SetSymm(smatd* m)   {
    if( Symm != NULL )  delete Symm;
    if( m != NULL)  Symm = new smatd( *m );
    else
      Symm = NULL;
  }
};

class TRAtomGroup  {
  TTypeList <TRestraintAtom> Atoms;
public:
  TRAtomGroup()  {  }
  virtual ~TRAtomGroup()  {  }

  TRAtomGroup&  operator = (const TRAtomGroup& ag)  {
    for( int i=0; i < ag.Count(); i++ )
      Atoms.AddNew<TRestraintAtom>( ag[i] );
    return *this;
  }
  TRestraintAtom& AddAtom(TCAtom* CA, smatd* Symm = NULL )  {
    return Atoms.AddNew<TCAtom*,smatd*>(CA, Symm);
  }
  inline int Count() const {  return Atoms.Count();  }
  TRestraintAtom& operator[] (int i)  const {  return Atoms[i];  }

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
