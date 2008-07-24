#ifndef restraintsH
#define restraintsH

#include "xmodel.h"
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



class ARestraint  {
  olxstr ResiName;
  virtual bool GetExplicit() const = 0; 
  virtual void DoExpand(IRefinementModel& parent, TArrayList<ARestraint>& rl) = 0;
public:
  ARestraint() {  }
  bool IsExplicit() const { return GetExplicit();  }
  bool IsExpandable() const {  return !GetExplicit();  }
  void Expand(IRefinementModel& parent, TArrayList<ARestraint>& rl)  {  DoExpand(parent, rl);  }  
};

class XSameDistanceRestraint : public ARestraint {
  
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
              short rt, double d1 = -1, double d2 = -1) : 
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
      AtomCount = container.GetFragmentSize(code); 
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
public:
  virtual int ScattererCount() const {  return Scatterers.Count();  }
  virtual XScatterer& GetScatterer(int i)  {  return Scatterers[i];  }
  virtual XScatterer* FindScattererByName(const olxstr& name) {
    return SortedScatterers[name];
  }
  virtual XResidue* FindResidueByNumber(int Number) {
    return Residues[Number];
  }
  virtual void FindResiduesByClass(const olxstr& clazz, TPtrList<XResidue>& res) {
    for( int i=0; i < Residues.Count(); i++ )
      if( Residues.Object(i)->ClassName.Comparei(clazz) == 0 )
        res.Add( Residues.Object(i) );
  }

  double Scale;  // global Fo/Fc scale
  TTypeList<XScatterer> Scatterers;
  TTypeList<XSite> Sites;
  TTypeList<XTDP> TDPs;
  // a list of all residues with key - number
  TPSTypeList<int, XResidue*> Residues;
  // a list of all scattererers for quick access by label
  TSStrPObjList<olxstr, XScatterer*, false> SortedScatterers;
  TPSTypeList<int, XFrag*> References;
};

class TRefinementModel {
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
