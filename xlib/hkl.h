/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xlib_hkl_H
#define __olx_xlib_hkl_H

#include "xbase.h"
#include "arrays.h"
#include "ins.h"

#include "refmerge.h"
#include "symmlib.h"

BeginXlibNamespace()

class THklFile: public IOlxObject  {
  static int HklCmp(const TReflection &I1, const TReflection &I2);
  TRefList Refs;
  TArray3D< TRefPList* > *Hkl3D;
  mat3d Basis;
protected:
  vec3i MaxHkl, MinHkl;
  double MaxI, MaxIS, MinI, MinIS;
  /* the function must be caled before the reflection is added to the list, as
  it needs to initialise the starting values of min and max
  */
  void UpdateMinMax(const TReflection& r);
  void InitHkl3D();
  void Clear3D();
  void Init();
public:
  THklFile();
  THklFile(const mat3d& hkl_transformation);
  virtual ~THklFile()  {  Clear();  }

  void Append(const THklFile& hkls);
  void Append(const TRefPList& hkls);
  // the reflection will be deleted by this object
  void Append(TReflection& hkl);
  // function has to be called to sort the list of reflections
  void EndAppend();

  TReflection& Reflection(size_t i) const {  return Refs[i];  }
  TReflection& operator [](size_t i) const {  return Refs[i];  }
  size_t RefCount() const {  return Refs.Count();  }
  const TRefList &RefList() const { return Refs;  }

  const vec3i& GetMaxHkl() const {  return MaxHkl;  }
  const vec3i& GetMinHkl() const {  return MinHkl;  }
  double GetMaxI() const { return MaxI;  }
  double GetMaxIS() const { return MaxIS;  }
  double GetMinI() const { return MinI;  }
  double GetMinIS() const { return MinIS;  }

  void Clear();
  void Sort()  {  QuickSorter::SortSF(Refs, HklCmp);  }
  /* reads the HKL file, marking reflections after 000 or new line as omitted.
  Returns all remaining information information as an Ins file or NULL
  */
  olx_object_ptr<TIns> LoadFromFile(const olxstr& FN, bool get_ins);
  olx_object_ptr<TIns> LoadFromStrings(const TCStrList& lines, bool get_ins);
  bool SaveToFile(const olxstr& FN);
  void UpdateRef(const TReflection& R);
  // returns reflections owned by this object
  ConstPtrList<TReflection> AllRefs(const TReflection& R, const smatd_list& sg) {
    return AllRefs(R.GetHkl(), sg);
  }
  ConstPtrList<TReflection> AllRefs(const vec3i& idx, const smatd_list& sg);
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
