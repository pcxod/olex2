/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef hklH
#define hklH

#include "xbase.h"
#include "evector.h"
#include "arrays.h"
#include "asymmunit.h"

#include "refmerge.h"
#include "symmlib.h"

BeginXlibNamespace()

class THklFile: public IEObject  {
  static int HklCmp(const TReflection &I1, const TReflection &I2);
  TRefPList Refs;
  TArray3D< TRefPList* > *Hkl3D;
  mat3d Basis;
protected:
  vec3i MaxHkl, MinHkl;
  double MaxI, MaxIS, MinI, MinIS;
  // the function must be caled before the reflection is added to the list, as
  // it needs to initialise the starting values of min and max
  inline void UpdateMinMax(const TReflection& r)  {
    if( Refs.IsEmpty() )  {
      // set starting values
      MinHkl[0] = MaxHkl[0] = r.GetH();
      MinHkl[1] = MaxHkl[1] = r.GetK();
      MinHkl[2] = MaxHkl[2] = r.GetL();
      MinI = MaxI = r.GetI();
      MinIS = MaxIS = r.GetS();
    }
    else  {
      vec3i::UpdateMinMax(r.GetHkl(), MinHkl, MaxHkl);
      if( r.GetI() < MinI )  {  MinI = r.GetI();  MinIS = r.GetS();  }
      if( r.GetI() > MaxI )  {  MaxI = r.GetI();  MaxIS = r.GetS();  }
    }
  }
  void InitHkl3D();
  void Clear3D();
public:
  THklFile() : Hkl3D(NULL)  {  Basis.I();  }
  THklFile(const mat3d& hkl_transformation) :
    Hkl3D(NULL), Basis(hkl_transformation)  {}
    virtual ~THklFile()  {  Clear();  }

  void Append(const THklFile& hkls);
  void Append(const TRefPList& hkls);
  // the reflection will be deleted by this object
  void Append(TReflection& hkl);
  // function has to be called to sort the list of reflections
  void EndAppend();

  inline TReflection& Reflection(size_t i)  const {  return *Refs[i];  }
  inline TReflection& operator [](size_t i) const {  return *Refs[i];  }
  inline size_t RefCount() const {  return Refs.Count();  }

  inline const vec3i& GetMaxHkl() const {  return MaxHkl;  }
  inline const vec3i& GetMinHkl() const {  return MinHkl;  }
  inline double GetMaxI() const { return MaxI;  }
  inline double GetMaxIS() const { return MaxIS;  }
  inline double GetMinI() const { return MinI;  }
  inline double GetMinIS() const { return MinIS;  }

  void Clear();
  void Sort()  {  QuickSorter::SortSF(Refs, HklCmp);  }
  /* if ins loader is passed and the hkl file has CELL and SFAC in it, 
  it will be initalised and if the ins_initialised is provided - it will be set
  True if the CELL and SFAC are found
  */
  bool LoadFromFile(const olxstr& FN, class TIns* ins = NULL,
    bool* ins_initialised=NULL);
  bool SaveToFile(const olxstr& FN);
  void UpdateRef(const TReflection& R);
  // returns reflections owned by this object
  void AllRefs(const TReflection& R, const smatd_list& sg, TRefPList& Res);
//..............................................................................
  template <class Merger> void MergeInP1(TRefList& output) const {
    vec3i_list omits;
    RefMerger::MergeInP1<Merger>(Refs, output, omits);
  }
//..............................................................................
  /* a primitive check if a line is an HKL file line - this is used on the
  first line to check wether the file is a proper HKL
  */
  static bool IsHKLFileLine(const olxstr& l)  {
    if( l.Length() >= 28 )  {
      if( !l.SubString(0,4).IsNumber() || !l.SubString(4,4).IsNumber() ||
        !l.SubString(8,4).IsNumber() || !l.SubString(12,8).IsNumber() ||
        !l.SubString(20,8).IsNumber() )
        return false;
      return true;
    }
    return false;
  }

  // saves to file a list of reflections
  static bool SaveToFile(const olxstr& FN, const TRefPList& Reflections,
    bool Append = true);
  // saves to file a list of reflections
  static bool SaveToFile(const olxstr& FN, const TRefList& Reflections);
};
//---------------------------------------------------------------------------

EndXlibNamespace()
#endif
