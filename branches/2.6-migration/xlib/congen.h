#ifndef congenH
#define congenH

#include "envilist.h"
#include "refmodel.h"

BeginXlibNamespace()

// fixable groups
const short fgCH3     = 1,  
            fgCH2     = 2,  
            fgCH1     = 3,  
            fgOH3     = 4,  
            fgOH2     = 5,  
            fgOH1     = 6,  
            fgNH4     = 7,  
            fgNH3     = 8,  
            fgNH2     = 9,
            fgNH1     = 10,
            fgBH1     = 11,
            fgSiH1    = 12,
            fgSH1     = 13;

// fixable parameters
const short fpDistance = 0x0001,
            fpAngle    = 0x0002,
            fpTAngle   = 0x0004,
            fpVolume   = 0x0008;

typedef TTypeList< AnAssociation2<double, double> > TFixedValueList;


class AConstraintGenerator : public IEObject{
protected:
  void DoGenerateAtom( TCAtomPList& created, TAsymmUnit& au, vec3d_list& Crds, const olxstr& StartingName);
  void GenerateAtom( TCAtomPList& created, TAtomEnvi& envi, const short Group, const TBasicAtomInfo& atomType, TAtomEnvi* pivoting = NULL);
  RefinementModel& RefMod;
public:
  AConstraintGenerator(RefinementModel& rm) : RefMod(rm) {}
  virtual bool FixParam(const short paramMask, TStrList& res, const TCAtomPList& atoms, const TFixedValueList& values) = 0;
  virtual bool FixAtom(TAtomEnvi& envi, const short Group, const TBasicAtomInfo& atomType, 
    TAtomEnvi* pivoting = NULL, TCAtomPList* generated = NULL) = 0;
};

EndXlibNamespace()

#endif
