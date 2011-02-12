/* (c) O. Dolomanov 2009 */
#ifndef __olx_xl_congen_H
#define __olx_xl_congen_H
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
  virtual void AnalyseMultipart(const TAtomEnvi& envi, const TTypeList<TCAtomPList>& parts) = 0;
};

// This can replace hardcoded values at some point...
//struct ParamAnalyser {
//  struct Range  {
//    double up, down;
//    int type;
//    Range(double _down, double _up, int _type=-1) : up(_up), down(_down), type(_type)  {}
//  };
//  typedef TTypeList<Range> RangeList;
//  static olxdict<uint32_t,RangeList, TPrimitiveComparator> ranges;
//  static uint32_t GenId(uint16_t groupId, uint16_t bonds)  {
//    return ((uint32_t)(groupId))|(((uint32_t)bonds) << 16);
//  }
//  static inline bool inrange(const double& v, const Range& r, int type=-1)  {
//    if( r.up < 0 )  return v > r.down;
//    if( r.down < 0 )  return v < r.up;
//    return v > r.down && v < r.up;
//  }
//  static inline bool inrange(const double& v1, const Range& r1, const double& v2, const Range& r2)  {
//    return (inrange(v1, r1) && inrange(v2,r2)) || (inrange(v1,r2) && inrange(v2,r1));
//  }
//  static const RangeList& GetRange(uint32_t id)  {
//    if( ranges.IsEmpty() )  {
//      RangeList& rlOH1_1 = ranges.Add(GenId(fgOH1,1));
//      rlOH1_1.AddNew(1.3, -1);
//      RangeList& rlOH1_2 = ranges.Add(GenId(fgOH1,2));
//      rlOH1_1.AddNew(1.8, -1);
//      rlOH1_1.AddNew(1.3, 1.8);
//    }
//  }
//  static bool IsOH1(const TAtomEnvi& envi)  {
//    if( envi.GetBase().GetType() != iOxygenZ )  return false;
//    if( envi.Count() == 1 )
//      return inrange(envi.GetBase().crd().DistanceTo(envi.GetCrd(0)), GetRange(GenId(fgOH1,1))[0]);
//    if( envi.Count() == 2 )  {
//      const RangeList d = GetRange(GenId(fgOH1,2));  
//      return inrange(
//        AE.GetCrd(0).DistanceTo(envi.GetBase().crd()), d[0],
//        AE.GetCrd(1).DistanceTo(envi.GetBase().crd()), d[1]
//      );
//    }
//    return false;
//  }
//};

EndXlibNamespace()

#endif
