#ifndef congenH
#define congenH

#include "envilist.h"
#include "refmodel.h"

BeginXlibNamespace()

// fixable groups
const uint16_t
  fgCH3  = 1,  
  fgCH2  = 2,  
  fgCH1  = 3,  
  fgOH3  = 4,  
  fgOH2  = 5,  
  fgOH1  = 6,  
  fgNH4  = 7,  
  fgNH3  = 8,  
  fgNH2  = 9,
  fgNH1  = 10,
  fgBH1  = 11,
  fgSiH1 = 12,
  fgSiH2 = 13,
  fgSH1  = 14;

// fixable parameters
const uint16_t
  fpDistance = 0x0001,
  fpAngle    = 0x0002,
  fpTAngle   = 0x0004,
  fpVolume   = 0x0008;

typedef TTypeList< AnAssociation2<double, double> > TFixedValueList;


class AConstraintGenerator : public IEObject{
protected:
  olxdict<uint32_t,double, TPrimitiveComparator> Distances;
  static uint32_t GenId(uint16_t groupId, uint16_t bonds)  {
    return ((uint32_t)(groupId))|(((uint32_t)bonds) << 16);
  }
  void DoGenerateAtom( TCAtomPList& created, TAsymmUnit& au, vec3d_list& Crds, const olxstr& StartingName);
  void GenerateAtom(TCAtomPList& created, TAtomEnvi& envi, const short Group, const cm_Element& atomType, TAtomEnvi* pivoting = NULL);
  RefinementModel& RefMod;
public:
  AConstraintGenerator(RefinementModel& rm);
  virtual bool FixParam(const short paramMask, TStrList& res, const TCAtomPList& atoms, const TFixedValueList& values) = 0;
  virtual bool FixAtom(TAtomEnvi& envi, const short Group, const cm_Element& atomType, 
    TAtomEnvi* pivoting = NULL, TCAtomPList* generated = NULL) = 0;
  virtual void AnalyseMultipart(const TTypeList<TCAtomPList>& parts) = 0;
};

EndXlibNamespace()

#endif
