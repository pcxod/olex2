/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_congen_H
#define __olx_xl_congen_H
#include "envilist.h"
#include "refmodel.h"
BeginXlibNamespace()

// groups
const uint16_t
  fgCH3  = 1,
  fgCH2  = 2,
  fgCH1  = 3,
  fgOH3  = 4,
  fgOH2  = 5,
  fgOH1 = 6,
  fgNH4  = 7,
  fgNH3  = 8,
  fgNH2  = 9,
  fgNH1  = 10,
  fgNH1t  = 11,
  fgBH1  = 12,
  fgSiH1 = 13,
  fgSiH2 = 14,
  fgSH1  = 15;

// parameters
const uint16_t
  fpDistance = 0x0001,
  fpAngle    = 0x0002,
  fpTAngle   = 0x0004,
  fpVolume   = 0x0008;

// geometry
const uint16_t
  fgDefault     = 0,
  fgLinear      = 1,
  fgPlaner      = 2,
  fgTetrahedral = 3;

typedef TTypeList< olx_pair_t<double, double> > TFixedValueList;

class AConstraintGenerator : public IOlxObject {
  bool UseRestrains;
protected:
  olx_pdict<uint32_t,double> Distances;
  void DoGenerateAtom(TCAtomPList& created, TAsymmUnit& au, vec3d_list& Crds,
    const olxstr& StartingName);
  void GenerateAtom(TCAtomPList& created, TAtomEnvi& envi, const short Group,
    const cm_Element& atomType, TAtomEnvi* pivoting = NULL);
  RefinementModel& RefMod;
public:
  AConstraintGenerator(RefinementModel& rm);
  virtual bool FixParam(const short paramMask, TStrList& res,
    const TCAtomPList& atoms, const TFixedValueList& values) = 0;
  virtual bool FixAtom(TAtomEnvi& envi, const short Group,
    const cm_Element& atomType, TAtomEnvi* pivoting = NULL,
    TCAtomPList* generated = NULL) = 0;
  virtual void AnalyseMultipart(const TAtomEnvi& envi,
    const TTypeList<TCAtomPList>& parts) = 0;
  
  DefPropBIsSet(UseRestrains)

  /* front 16 bits - number of bonds, following 8 bits - geometry, rear 8 bits
  - group */
  static uint32_t GenId(uint16_t groupId, uint16_t bonds,
    uint16_t geometry=fgDefault)
  {
    return ((uint32_t)(groupId))
      |(((uint32_t(geometry))) << 8)
      |(((uint32_t)bonds) << 16);
  }
  static uint32_t GroupFromId(uint32_t id)  {
    return (id&0x000f);
  }
  static uint32_t GeometryFromId(uint32_t id)  {
    return (id&0x00f0) >> 8;
  }
  static uint32_t NumberOfBondsFromId(uint32_t id)  {
    return (id&0xff00) >> 16;
  }
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
