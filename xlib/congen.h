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
  fgBH1 = 1,
  fgB_start = 1,
  fgB_end = 1,
  
  fgC_start = 11,
  fgCH1 = 11,
  fgCH2 = 12,
  fgCH3 = 13,
  fgCH3x2 = 14,
  fgC_end = 14,

  fgN_start = 21,
  fgNH1 = 21,
  fgNH1t = 22,
  fgNH2 = 23,
  fgNH3 = 24,
  fgNH4 = 25,
  fgN_end = 25,

  fgO_start = 31,
  fgOH3 = 31,
  fgOH2 = 32,
  fgOH1 = 33,
  fgO_end = 33,

  fgSi_start = 41,
  fgSiH1 = 41,
  fgSiH2 = 42,
  fgSi_end = 42,

  fgS_start = 51,
  fgSH1 = 51,
  fgS_end = 51
  ;

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
  fgPlanar      = 2,
  fgTetrahedral = 3;

typedef TTypeList< olx_pair_t<double, double> > TFixedValueList;

class AConstraintGenerator : public IOlxObject {
  bool UseRestrains;
protected:
  olx_pdict<uint32_t,double> Distances;
  void DoGenerateAtom(TResidue &r, TCAtomPList& created, TAsymmUnit& au,
    vec3d_list& Crds, const olxstr& StartingName);
  void GenerateAtom(TCAtomPList& created, TAtomEnvi& envi, const short Group,
    const cm_Element& atomType, TAtomEnvi* pivoting = 0);
  RefinementModel& RefMod;
  vec3d Generate_1(short group, const TAtomEnvi& envi) const;
public:
  AConstraintGenerator(RefinementModel& rm);
  virtual bool FixParam(const short paramMask, TStrList& res,
    const TCAtomPList& atoms, const TFixedValueList& values) = 0;
  virtual bool FixAtom(TAtomEnvi& envi, const short Group,
    const cm_Element& atomType, TAtomEnvi* pivoting = 0,
    TCAtomPList* generated = 0) = 0;
  virtual void AnalyseMultipart(const TAtomEnvi& envi,
    const TTypeList<TCAtomPList>& parts) = 0;
  
  DefPropBIsSet(UseRestrains);

  void ApplyCorrection(double v);
  //neutron.dist file format type
  void UpdateFromFile(const olxstr& fn, bool apply_temp_correction, double def=-1);

  double GetTempCorrection() const {
    return GetTempCorrection(RefMod);
  }

  static double GetTempCorrection(const RefinementModel& rm) {
    if (rm.expl.IsTemperatureSet()) {
      if (rm.expl.GetTempValue().GetV() < -70) {
        return 0.02;
      }
      else if (rm.expl.GetTempValue().GetV() < -20) {
        return 0.01;
      }
    }
    return 0;
  }

  static olx_pair_t<uint16_t, uint16_t> GetZGroupRange(unsigned Z);

  // this is used in TLattice::_AnalyseAtomHAdd
  TParamList Options;

  /* front 16 bits - number of bonds, following 8 bits - geometry, rear 8 bits
  - group */
  static uint32_t GenId(uint16_t groupId, uint16_t bonds,
    uint16_t geometry=fgDefault)
  {
    return ((uint32_t)(groupId))
      |(((uint32_t(geometry))) << 16)
      |(((uint32_t)bonds) << 24);
  }
  static uint32_t GroupFromId(uint32_t id)  {
    return (id&0x00ff);
  }
  static uint32_t GeometryFromId(uint32_t id)  {
    return (id&0x0f00) >> 16;
  }
  static uint32_t NumberOfBondsFromId(uint32_t id)  {
    return (id&0xf0000) >> 24;
  }
  static uint32_t AfixM2GroupId(int M, int Z);
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
