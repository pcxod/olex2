#ifndef hklH
#define hklH

#include "xbase.h"
#include "evector.h"
#include "arrays.h"
#include "asymmunit.h"

#include "refutil.h"

BeginXlibNamespace()

class THklFile: public IEObject  {
  static int HklCmp(const TReflection* I1, const TReflection* I2);
  TRefPList Refs;
  TArray3D< TRefPList* > *Hkl3D;
protected:
  vec3i MaxHkl, MinHkl;
  double MaxI, MaxIS, MinI, MinIS;
  // the function must be caled before the reflection is added to the list, as
  // it needs to initialise the starting values of min and max
  void UpdateMinMax(const TReflection& r);
  void InitHkl3D();
  void Clear3D();
public:
  THklFile();
  virtual ~THklFile();

  void Append(const THklFile& hkls);
  void Append(const TRefPList& hkls);
  // the reflection will be deleted by this object
  void Append(TReflection& hkl);
  // function has to be called to sort the list of reflections
  void EndAppend();

  inline TReflection& Reflection(int i)  const {  return *Refs[i];  }
  inline TReflection& operator [](int i) const {  return *Refs[i];  }
  inline int RefCount() const                  {  return Refs.Count();  }

  inline const vec3i& GetMaxHkl()  const {  return MaxHkl;  }
  inline const vec3i& GetMinHkl()  const {  return MinHkl;  }
  inline double GetMaxI()          const { return MaxI;  }
  inline double GetMaxIS()         const { return MaxIS;  }
  inline double GetMinI()          const { return MinI;  }
  inline double GetMinIS()         const { return MinIS;  }

  void Clear();
  inline void Sort()  {  Refs.QuickSorter.SortSF(Refs, HklCmp);  }
  /* if ins loader is passed and the hkl file has CELL and SFAC in it, 
  it will be initalised
  */
  bool LoadFromFile(const olxstr& FN, class TIns* ins = NULL);
  bool SaveToFile(const olxstr& FN);
  void UpdateRef(const TReflection& R);
  // returns reflections owned by this object
  void AllRefs(int h, int k, int l, const TAsymmUnit& AU, TRefPList& Res)  {
    TReflection r(h, k, l);
    AllRefs(r, AU, Res);
  }
  template <class VC>
    void AllRefs(const TVector<VC> &hkl, const TAsymmUnit& AU, TRefPList& Res)  {
      AllRefs(hkl[0], hkl[1], hkl[2],AU, Res);
    }
  void AllRefs(const TReflection& R, const TAsymmUnit& AU, TRefPList& Res);
//..............................................................................
  MergeStats Merge(const class TSpaceGroup& AU, bool MergeInverse, TRefList& output) const;
//..............................................................................
  MergeStats SimpleMerge(smatd_list& ml, TRefList& output) const {
    return  RefMerger::Merge<TSimpleMerger>(ml, Refs, output);
  }

  // saves to file a list of reflections
  static bool SaveToFile(const olxstr& FN, const TRefPList& Reflections, bool Append = true);
  // saves to file a list of reflections
  static bool SaveToFile(const olxstr& FN, const TRefList& Reflections);
};
//---------------------------------------------------------------------------

EndXlibNamespace()
#endif

