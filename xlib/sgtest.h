/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef sgtestH
#define sgtestH
#include "typelist.h"
#include "hkl.h"
#include "symmlib.h"
#include "arrays.h"


BeginXlibNamespace()

  typedef olx_pair_t<double, double> TwoDoubles;
  typedef AnAssociation3<double, double, int> TwoDoublesInt;


class TSGStats  {
  TSpaceGroup* SpaceGroup;
  TwoDoublesInt Stats;
public:
  TSGStats(TSpaceGroup* sg, const TwoDoublesInt& stats) : Stats(stats)  {
    SpaceGroup = sg;
  }
  TSpaceGroup& GetSpaceGroup() const {  return *SpaceGroup;  }
  double GetSummI() const    {  return Stats.GetA();  }
  double GetSummSI() const   {  return Stats.GetB();  }
  int    GetCount() const    {  return Stats.GetC();  }
};

template <class OC>  class TElementStats  {
  OC LattOrSG;
  TwoDoublesInt SummStrong, SummWeak;
public:
  TElementStats(OC obj, const TwoDoublesInt& strong,
                                 const TwoDoublesInt& weak ) :
                                 SummStrong(strong), SummWeak(weak) {
    LattOrSG = obj;
  }
  OC& GetObject()                {  return LattOrSG;  }
  double GetSummStrongI()  const {  return SummStrong.GetA();  }
  double GetSummStrongSI() const {  return SummStrong.GetB();  }
  int GetStrongCount() const     {  return  SummStrong.GetC();  }
  double GetSummWeakI()    const {  return SummWeak.GetA();  }
  double GetSummWeakSI()   const {  return SummWeak.GetB();  }
  int GetWeakCount() const   {  return SummWeak.GetC();  }
};

class TSAStats  {
  TSymmElement* SymmElement;
  TwoDoublesInt Stats;
  bool Excluded, Present;
public:
  TSAStats(TSymmElement* se, const TwoDoublesInt& stats ): Stats( stats )  {
    Present = Excluded = false;
    SymmElement = se;
  }
  TSymmElement& GetSymmElement() const {  return *SymmElement;  }
  double GetSummI() const {  return Stats.GetA();  }
  double GetSummSI() const {  return Stats.GetB();  }
  int    GetCount() const {  return Stats.GetC();  }
  bool IsExcluded() const {  return Excluded;  }
  bool IsPresent() const {  return Present;  }
  void SetPresent()  {  Present = true;  }
  void Exclude()  {  Excluded = true;  }
};

class TSGTest  {
  TRefList Refs;  // P1 merged reflections
  double TotalI, TotalIS, 
         AverageI, AverageIS, 
         MaxI, MaxIS, 
         MinI, MinIS;
  vec3i minInd, maxInd;
  TArray3D< TReflection* >* Hkl3DArray;
public:
  TSGTest();
  virtual ~TSGTest();
  void MergeTest(const TPtrList<TSpaceGroup>& sgList, TTypeList<TSGStats>& res );
  void LatticeSATest(TTypeList<TElementStats<TCLattice*> >& latRes, TTypeList<TSAStats>& saRes );
  void WeakRefTest(const TPtrList<TSpaceGroup>& sgList, TTypeList<TElementStats<TSpaceGroup*> >& res);

  size_t GetP1RefCount() const {  return Refs.Count();  }

  double GetAverageI() const {  return AverageI;  }
  double GetAverageIS() const {  return AverageIS;  }
  double GetTotalI() const {  return TotalI;  }
  double GetTotalIS() const {  return TotalIS;  }
  double GetMaxI() const {  return MaxI;  }
  double GetMaxIS() const {  return MaxIS;  }
  double GetMinI() const {  return MinI;  }
  double GetMinIS() const {  return MinIS;  }
};

EndXlibNamespace()

#endif
