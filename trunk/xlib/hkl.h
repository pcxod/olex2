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
#include "cif.h"
#include "refmerge.h"
#include "symmlib.h"

BeginXlibNamespace()

class THklFile: public IOlxObject {
  static int HklCmp(const TReflection &r1, const TReflection &r2) {
    int d = r1.CompareTo(r2);
    return (d == 0 ? olx_cmp(r1.GetI(), r2.GetI()) : d);
  }
  TRefList Refs;
  olx_array::TArray3D< TRefPList* > *Hkl3D;
  mat3d Basis;
  int HKLF;
protected:
  vec3i MaxHkl, MinHkl;
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
  template <class list_t>
  void Append(const list_t& hkls) {
    if (hkls.IsEmpty()) return;
    for (size_t i = 0; i < hkls.Count(); i++) {
      TReflection &r = olx_ref::get(hkls[i]);
      UpdateMinMax(r);
      index_t t = Refs.Count();
      Refs.AddCopy(r).SetTag(t);
    }
    EndAppend();
  }
  // the reflection will be deleted by this object
  void Append(TReflection& hkl);
  // function has to be called to sort the list of reflections
  void EndAppend();

  TReflection& operator [](size_t i) const {  return Refs[i];  }
  size_t RefCount() const {  return Refs.Count();  }
  const TRefList &RefList() const { return Refs;  }

  const vec3i& GetMaxHkl() const {  return MaxHkl;  }
  const vec3i& GetMinHkl() const {  return MinHkl;  }
  /* -1 - unknown 3 - amplitudes, 4 - intensities;
  gets initialised when reading from a CIF or can be set externally
  */
  int GetHKLF() const { return HKLF; }
  void SetHKLF(int v) { HKLF = v; }
  void Clear();
  void Sort()  {  QuickSorter::SortSF(Refs, HklCmp);  }
  /* reads the HKL file, marking reflections after 000 or new line as omitted.
  Returns all remaining information information as an Ins file or NULL
  */
  olx_object_ptr<TIns> LoadFromFile(const olxstr& FN, bool get_ins);
  olx_object_ptr<TIns> LoadFromStrings(const TStrList& lines, bool get_ins);
  void SaveToFile(const olxstr& FN) {
    THklFile::SaveToFile(FN, Refs);
  }
  void UpdateRef(const TReflection& R);
  // returns reflections owned by this object
  TRefPList::const_list_type AllRefs(const TReflection& R,
    const smatd_list& sg)
  {
    return AllRefs(R.GetHkl(), sg);
  }
  TRefPList::const_list_type AllRefs(const vec3i& idx, const smatd_list& sg);
//..............................................................................
  template <class Merger> void MergeInP1(TRefList& output) const {
    RefMerger::MergeInP1<Merger>(Refs, output, vec3i_list());
  }
//..............................................................................
  /* a primitive check if a line is an HKL file line - this is used on the
  first line to check wether the file is a proper HKL
  */
  static bool IsHKLFileLine(const olxstr& l) {
    if (l.Length() >= 28) {
      return (l.SubString(0, 4).IsNumber() && l.SubString(4, 4).IsNumber() &&
        l.SubString(8, 4).IsNumber() && l.SubString(12, 8).IsNumber() &&
        l.SubString(20, 8).IsNumber());
    }
    return false;
  }

  // saves to file a list of reflections
  static void SaveToFile(const olxstr& FN, const TRefPList& Reflections);
  // saves to file a list of reflections
  static void SaveToFile(const olxstr& FN, const TRefList& Reflections);
  template <class ref_list_t>
  static void SaveToStream(const ref_list_t& refs, IDataOutputStream &out) {
    if (refs.IsEmpty()) return;
    double scale = RefListUtil::CalcScale(refs);
    TReflection NullRef;
    if (olx_ref::get(refs[0]).IsBatchSet())
      NullRef.SetBatch(0);
    const size_t ref_str_len = NullRef.ToString().Length();
    const size_t bf_sz = ref_str_len + 1;
    olx_array_ptr<char> ref_bf(new char[bf_sz]);
    for (size_t i = 0; i < refs.Count(); i++) {
      const TReflection &r = olx_ref::get(refs[i]);
      if (!r.IsOmitted())
        out.Writecln(r.ToCBuffer(ref_bf, bf_sz, scale), ref_str_len);
    }
    out.Writecln(NullRef.ToCBuffer(ref_bf, bf_sz, 1), ref_str_len);
    for (size_t i = 0; i < refs.Count(); i++) {
      const TReflection &r = olx_ref::get(refs[i]);
      if (r.IsOmitted())
        out.Writecln(r.ToCBuffer(ref_bf, bf_sz, scale), ref_str_len);
    }
  }
  protected:
  // helper functions
  static void RefToRow_(const TReflection &r, cif_dp::CifRow &row, bool batch) {
    row[0] = new cif_dp::cetString(r.GetH());
    row[1] = new cif_dp::cetString(r.GetK());
    row[2] = new cif_dp::cetString(r.GetL());
    row[3] = new cif_dp::cetString(r.GetI());
    row[4] = new cif_dp::cetString(r.GetS());
    if (batch)
      row[5] = new cif_dp::cetString(r.GetBatch());
  }
  public:
  /* saves given reflections to standard CIF table. If intensity is true,
  reflections I is marked as Fsq, otherwise - as F. In the case of empty
  reflection list return NULL.
  */
  template <class ref_list_t>
  static cif_dp::cetTable *ToCIF(const ref_list_t& refs, TCif &out,
    bool intensity=true)
  {
    if (refs.IsEmpty()) return 0;
    cif_dp::cetTable &t = out.AddLoopDef(
      "_refln_index_h,_refln_index_k,_refln_index_l");
    if (intensity) {
      t.AddCol("_refln_F_squared_meas");
      t.AddCol("_refln_F_squared_sigma");
    }
    else {
      t.AddCol("_refln_F_meas");
      t.AddCol("_refln_F_sigma");
    }
    bool batch_set;
    if ((batch_set = olx_ref::get(refs[0]).IsBatchSet())) {
      t.AddCol("_refln_scale_group_code");
    }
    t.SetRowCapacity(refs.Count());
    size_t omitted_cnt = 0;
    for (size_t i = 0; i < refs.Count(); i++) {
      TReflection &r = olx_ref::get(refs[i]);
      if (r.IsOmitted()) {
        omitted_cnt++;
        continue;
      }
      RefToRow_(r, t.AddRow(), batch_set);
    }
    if (omitted_cnt != 0) {
      TReflection nr;
      if (batch_set) {
        nr.SetBatch(0);
      }
      RefToRow_(nr, t.AddRow(), batch_set);
      for (size_t i = 0; i < refs.Count(); i++) {
        TReflection &r = olx_ref::get(refs[i]);
        if (r.IsOmitted()) {
          RefToRow_(r, t.AddRow(), batch_set);
        }
      }
    }
    return &t;
  }

  // reflection list + data type, false - F, true - Fsq
  typedef olx_pair_t<TRefList, bool> ref_list;

  /* reads data from a CIF table and returns read reflections and a boolean
  flag indicateing if the reflections I is amplitudes (F, false) or intensity
  (Fsq, true). Throws TInvalidArgumentException if required columns are not
  found.
  */
  static olx_object_ptr<ref_list> FromCifTable(const cif_dp::cetTable &);
  /* tries to read Tonto like reflcetions file. The function fails if the
  returned pointer is NOT valid. if the reflections I is amplitudes (F, false)
  or intensity (Fsq, true).
 */
  static olx_object_ptr<ref_list> FromTonto(const TStrList &);
};
//---------------------------------------------------------------------------

EndXlibNamespace()
#endif
