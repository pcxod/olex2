#ifndef sgtestH
#define sgtestH
#include "typelist.h"
#include "hkl.h"
#include "symmlib.h"
#include "arrays.h"


BeginXlibNamespace()

  typedef AnAssociation2<double, double> TwoDoubles;
  typedef AnAssociation3<double, double, int> TwoDoublesInt;


class TSGStats  {
  TSpaceGroup* SpaceGroup;
  TwoDoublesInt Stats;
public:
  TSGStats(TSpaceGroup* sg, const TwoDoublesInt& stats) : Stats(stats)  {
    SpaceGroup = sg;
  }
  inline TSpaceGroup& GetSpaceGroup()  const  {  return *SpaceGroup;  }
  inline double GetSummI()  const     {  return Stats.GetA();  }
  inline double GetSummSI()  const    {  return Stats.GetB();  }
  inline int    GetCount()  const     {  return Stats.GetC();  }
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
  inline OC& GetObject()                {  return LattOrSG;  }
  inline double GetSummStrongI()  const {  return SummStrong.GetA();  }
  inline double GetSummStrongSI() const {  return SummStrong.GetB();  }
  inline int GetStrongCount() const     {  return  SummStrong.GetC();  }
  inline double GetSummWeakI()    const {  return SummWeak.GetA();  }
  inline double GetSummWeakSI()   const {  return SummWeak.GetB();  }
  inline int GetWeakCount()   const     {  return SummWeak.GetC();  }
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
  inline TSymmElement& GetSymmElement()  const  {  return *SymmElement;  }
  inline double GetSummI()  const     {  return Stats.GetA();  }
  inline double GetSummSI()  const    {  return Stats.GetB();  }
  inline int    GetCount()  const     {  return Stats.GetC();  }
  inline bool IsExcluded()  const          {  return Excluded;  }
  inline bool IsPresent()  const           {  return Present;  }
  inline void SetPresent()                 {  Present = true;  }
  inline void Exclude()                    {  Excluded = true;  }
};

class TSGTest  {
  TRefList Refs;  // P1 merged reflections
  double TotalI, TotalIS, 
         AverageI, AverageIS, 
         MaxI, MaxIS, 
         MinI, MinIS;
  int HklRefCount; // number of read reflections
  long minH, maxH, minK, maxK, minL, maxL;
  TArray3D< TReflection* >* Hkl3DArray;
public:
  TSGTest( const olxstr& hklFileName, const mat3d& hkl_tranform);
  virtual ~TSGTest();
  void MergeTest(const TPtrList<TSpaceGroup>& sgList, TTypeList<TSGStats>& res );
  void LatticeSATest(TTypeList<TElementStats<TCLattice*> >& latRes, TTypeList<TSAStats>& saRes );
  void WeakRefTest(const TPtrList<TSpaceGroup>& sgList, TTypeList<TElementStats<TSpaceGroup*> >& res);

  inline int GetHklRefCount()   const {  return HklRefCount;  }
  inline int GetP1RefCount()    const {  return Refs.Count();  }

  inline double GetAverageI()   const  {  return AverageI;  }
  inline double GetAverageIS()  const  {  return AverageIS;  }
  inline double GetTotalI()     const  {  return TotalI;  }
  inline double GetTotalIS()    const  {  return TotalIS;  }
  inline double GetMaxI()       const  {  return MaxI;  }
  inline double GetMaxIS()      const  {  return MaxIS;  }
  inline double GetMinI()       const  {  return MinI;  }
  inline double GetMinIS()      const  {  return MinIS;  }
};

EndXlibNamespace()

#endif
