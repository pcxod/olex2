/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "hkl.h"
#include "lst.h"
#include "ins.h"
#include "cif.h"
#include "emath.h"
#include "efile.h"
#include "estrlist.h"
#include "exception.h"
#include "ematrix.h"
#include "symmlib.h"
#include "math/composite.h"

using namespace olx_array;
//..............................................................................
THklFile::THklFile() {
  Basis.I();
  Init();
}
//..............................................................................
THklFile::THklFile(const mat3d& hkl_transformation)
  : Basis(hkl_transformation)
{
  Init();
}
//..............................................................................
void THklFile::Init() {
  Hkl3D = NULL;
  HKLF = -1;
}
//..............................................................................
void THklFile::Clear() {
  HKLF = -1;
  Refs.Clear();
  Clear3D();
}
//..............................................................................
void THklFile::UpdateMinMax(const TReflection& r) {
  if (Refs.IsEmpty()) {
    MinHkl = MaxHkl = r.GetHkl();
  }
  else {
    vec3i::UpdateMinMax(r.GetHkl(), MinHkl, MaxHkl);
  }
}
//..............................................................................
void THklFile::Clear3D() {
  if (Hkl3D == NULL)  return;
  for (int i=MinHkl[0]; i <= MaxHkl[0]; i++) {
    for (int j=MinHkl[1]; j <= MaxHkl[1]; j++)
      for (int k=MinHkl[2]; k <= MaxHkl[2]; k++)
        if (Hkl3D->Value(i,j,k) != NULL)
          delete Hkl3D->Value(i,j,k);
  }
  delete Hkl3D;
  Hkl3D = NULL;
}
//..............................................................................
olx_object_ptr<TIns> THklFile::LoadFromFile(const olxstr& FN, bool get_ins,
  const olxstr &format)
{
  try {
    TEFile::CheckFileExists(__OlxSourceInfo, FN);
    TStrList SL = TEFile::ReadLines(FN);
    if (SL.IsEmpty()) {
      throw TEmptyFileException(__OlxSrcInfo, FN);
    }
    return LoadFromStrings(SL, get_ins, format);
  }
  catch (const TExceptionBase& e) {
    throw TFunctionFailedException(__OlxSourceInfo, e);
  }
}
//..............................................................................
olx_object_ptr<TIns> THklFile::LoadFromStrings(const TStrList& SL,
  bool get_ins, const olxstr &format)
{
  olx_object_ptr<TIns> rv;
  if (SL.IsEmpty()) {
    return rv;
  }
  try {
    Clear();
    if (format.Equals("free")) {
      bool HasBatch = false;
      {
        TStrList toks(SL[0], ' ');
        if (toks.Count() == 6 && toks[5].IsInt()) {
          HasBatch = true;
        }
      }
      for (size_t i = 0; i < SL.Count(); i++) {
        TStrList toks(SL[i], ' ');
        if (toks.Count() < 5 || (HasBatch && toks.Count() < 6)) {
          break;
        }
        TReflection* ref;
        try {
          ref = new TReflection(
            toks[0].ToInt(), toks[1].ToInt(), toks[2].ToInt(),
            toks[3].ToDouble(),
            toks[4].ToDouble());
        }
        catch (const TExceptionBase &) {
          break;
        }
        if (HasBatch) {
          ref->SetBatch(toks[5].ToInt());
        }
        if (ref->GetHkl().IsNull()) {
          delete ref;
          break;
        }
        UpdateMinMax(*ref);
        Refs.Add(ref);
        ref->SetTag(Refs.Count());
      }
      return 0;
    }
    TSizeList fl = TSizeList::FromList(TStrList(format, ","),
      FunctionAccessor::MakeConst(&olxstr::ToSizeT));
    if (fl.IsEmpty()) {
      fl << 4 << 4 << 4 << 8 << 8;
    }
    else {
      if (fl.Count() < 5) {
        throw TInvalidArgumentException(__OlxSourceInfo, "line format");
      }
    }
    const size_t fmt_len = olx_sum(fl);
    const size_t fidx2 = fl[0] + fl[1],
      fidx3 = fidx2 + fl[2],
      fidx4 = fidx3 + fl[3],
      fidx5 = fidx4 + fl[4]
      ;
    {  // validate if 'real' HKL, not fcf
      if (!IsHKLFileLine(SL[0], fl)) {
        TCif cif;
        try {
          cif.LoadFromStrings(SL);
          // find first data block with reflections...
          cif_dp::cetTable* hklLoop = cif.FindLoopGlobal("_refln", true);
          if (hklLoop == NULL) {
            // tonto mess?
            hklLoop = cif.FindLoopGlobal("_diffrn_refln", true);
            if (hklLoop == 0) {
              throw TInvalidArgumentException(__OlxSourceInfo,
                "no reflection loop found");
            }
          }
          olx_object_ptr<ref_list> refs = FromCifTable(*hklLoop);
          if (refs.is_valid()) {
            Refs = refs().a;
            HKLF = (refs().b ? 4 : 3);
          }
          if (get_ins && cif.FindEntry("_cell_length_a") != NULL) {
            rv = new TIns;
            rv().GetRM().Assign(cif.GetRM(), true);
          }
        }
        catch(TExceptionBase& e) {
          olx_object_ptr<ref_list> refs = FromTonto(SL);
          if (!refs.is_valid()) {
            throw TFunctionFailedException(__OlxSrcInfo, e,
              "unsupported file format");
          }
          Refs = refs().a;
          HKLF = (refs().b ? 4 : 3);
        }
        MaxHkl = vec3i(-100);
        MinHkl = vec3i(100);
        Refs.ForEach(olx_func::make(this, &THklFile::UpdateMinMax));
        return rv;
      }
    }
    bool ZeroRead = false,
      HasBatch = false;
    size_t line_length = 0, i=0;
    const bool apply_basis = !Basis.IsI();
    size_t removed_cnt = 0;
    const size_t line_cnt = SL.Count();
    Refs.SetCapacity(line_cnt);
    for (; i < line_cnt; i++) {
      const olxstr& line = SL[i];
      if (i == 0) {
        if (line.Length() >= fmt_len +4) {
          HasBatch = true;
          line_length = line.Length();
        }
        else if (line.Length() >= fmt_len) {
          line_length = line.Length();
        }
        else {
          throw TInvalidArgumentException(__OlxSourceInfo, "file content");
        }
      }
      if (line.Length() != line_length) {
        if (line_cnt - i > 50 && !ZeroRead) {
          TBasicApp::NewLogEntry(logWarning) << (line_cnt - i) <<
            " Lines of the HKL file have been ignored after line #" <<
            (i+1);
        }
        break;
      }
      try {
        int h = line.SubString(0, fl[0]).ToInt(),
          k = line.SubString(fl[0],fl[1]).ToInt(),
          l = line.SubString(fidx2,fl[2]).ToInt();
        if (h == 0 && k == 0 && l == 0) {
          ZeroRead = true;
          continue;
        }
        TReflection* ref = HasBatch ?
          new TReflection(h, k, l,
            // trim  fix for field overrun for -0.0 by CrystalClear
            line.SubString(fidx3,fl[3]).TrimL('0').ToDouble(),
            line.SubString(fidx4,fl[4]).TrimL('0').ToDouble(),
            line.SubString(fidx5,4).IsNumber() ? line.SubString(fidx5,4).ToInt()
            : 1)
          :
          new TReflection(h, k, l, line.SubString(fidx3,fl[3]).ToDouble(),
            line.SubString(fidx4,fl[4]).ToDouble());
        ref->SetOmitted(ZeroRead);
        if (apply_basis) {
          vec3d nh = Basis*vec3d(ref->GetHkl());
          vec3i nih = nh.Round<int>();
          if (!nh.Equals(nih, 0.004) || nih.IsNull()) {
            delete ref;
            removed_cnt++;
            continue;
          }
          ref->SetHkl(nih);
        }
        UpdateMinMax(*ref);
        Refs.Add(ref);
        ref->SetTag(Refs.Count());
      }
      catch(const TExceptionBase& e) {
        TBasicApp::NewLogEntry(logInfo) <<
          olxstr("Not an HKL line ") << (i+1) << ", breaking";
        if (line_cnt - i > 50 && !ZeroRead) {
          TBasicApp::NewLogEntry(logWarning) << (line_cnt - i) <<
            " Lines of the HKL file have been ignored after line #" <<
            (i + 1);
        }
        break;
      }
    }
    if (removed_cnt != 0) {
      TBasicApp::NewLogEntry(logError) <<
        "HKL transformation leads to non-integral/invalid Miller indices";
      TBasicApp::NewLogEntry(logError) << "Removed: " << removed_cnt <<
        " invalid reflections";
    }
    if (get_ins && (SL.Count() - i) > 2) {
      TStrList toks = SL.SubListFrom(i).GetObject();
      olx_object_ptr<TIns> ins(new TIns);
      try {
        ins().LoadFromStrings(toks);
        rv = ins;
      }
      catch (const TExceptionBase &e) {
        TBasicApp::NewLogEntry(logInfo) << "Failed on reading INS from HKL: "
          << e.GetException()->GetFullMessage();
      }
    }
  }
  catch (const TExceptionBase& e)  {
    throw TFunctionFailedException(__OlxSourceInfo, e);
  }
  if (Refs.IsEmpty())
    throw TFunctionFailedException(__OlxSourceInfo, "empty reflections file");
  return rv;
}
//..............................................................................
void THklFile::UpdateRef(const TReflection& R) {
  size_t ind = olx_abs(R.GetTag()) - 1;
  if (ind >= Refs.Count()) {
    throw TInvalidArgumentException(__OlxSourceInfo, "reflection tag");
  }
  Refs[ind].SetI(R.GetI());
  Refs[ind].SetS(R.GetS());
  Refs[ind].SetOmitted(R.IsOmitted());
}
//..............................................................................
void THklFile::InitHkl3D() {
  if (Hkl3D != NULL)  return;
  volatile TStopWatch sw(__FUNC__);
  TArray3D<TRefPList*> &hkl3D = *(new TArray3D<TRefPList*>(MinHkl, MaxHkl));
  for (size_t i=0; i < Refs.Count(); i++) {
    TReflection &r1 = Refs[i];
    TRefPList *&rl = hkl3D(r1.GetHkl());
    if (rl == NULL)
      rl = new TRefPList();
    rl->Add(r1);
  }
  Hkl3D = &hkl3D;
}
//..............................................................................
ConstPtrList<TReflection> THklFile::AllRefs(const vec3i& idx,
  const smatd_list& ml)
{
  TRefPList rv;
  SortedObjectList<vec3i, TComparableComparator> ri;
  for (size_t i=0; i < ml.Count(); i++) {
    ri.AddUnique(TReflection::MulHkl(idx, ml[i]));
  }
  InitHkl3D();
  for (size_t j=0; j < ri.Count(); j++) {
    if (!Hkl3D->IsInRange(ri[j])) continue;
    TRefPList* r = Hkl3D->Value(ri[j]);
    if (r != NULL)
      rv.AddAll(*r);
  }
  return rv;
}
//..............................................................................
void THklFile::Append(TReflection& hkl) {
  UpdateMinMax(hkl);
  hkl.SetTag(Refs.Count());
  Refs.Add(hkl);
}
//..............................................................................
void THklFile::EndAppend()  {
  //Refs.QuickSorter.SortSF(Refs, HklCmp);
}
//..............................................................................
void THklFile::Append(const THklFile& hkls)  {
  if( !hkls.RefCount() )  return;
  Append(hkls.Refs);
}
//..............................................................................
void THklFile::SaveToFile(const olxstr& FN, const TRefPList& refs) {
  if (refs.IsEmpty()) return;
  //if (Append && TEFile::Exists(FN)) {
  //  THklFile F;
  //  F.LoadFromFile(FN, false);
  //  F.Append(refs);
  //  F.SaveToFile(FN);
  //}
  TEFile out(FN, "w+b");
  SaveToStream(refs, out);
}
//..............................................................................
void THklFile::SaveToFile(const olxstr& FN, const TRefList& refs) {
  TEFile out(FN, "w+b");
  SaveToStream(refs, out);
}
//..............................................................................
olx_object_ptr<THklFile::ref_list> THklFile::FromCifTable(
  const cif_dp::cetTable &t)
{
  bool intensity;
  TRefList refs;
  olxstr prefix = t.GetName();
  const size_t hInd = t.ColIndex(prefix + "_index_h");
  const size_t kInd = t.ColIndex(prefix + "_index_k");
  const size_t lInd = t.ColIndex(prefix + "_index_l");
  size_t mInd = t.ColIndex(prefix + "_F_squared_meas");
  size_t sInd = t.ColIndex(prefix + "_F_squared_sigma");
  size_t batch = t.ColIndex(prefix + "_scale_group_code");
  if (mInd == InvalidIndex) {
    mInd = t.ColIndex(prefix + "_F_meas");
    sInd = t.ColIndex(prefix + "_F_sigma");
    intensity = false;
  }
  else {
    intensity = true;
  }
  if ((hInd | kInd | lInd | mInd | sInd) == InvalidIndex) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      "could not locate <h k l meas sigma> data");
  }
  bool zero_found = false;
  for (size_t i = 0; i < t.RowCount(); i++) {
    const cif_dp::CifRow& r = t[i];
    TReflection &ref = refs.Add(
      new TReflection(
      r[hInd]->GetStringValue().ToInt(),
      r[kInd]->GetStringValue().ToInt(),
      r[lInd]->GetStringValue().ToInt(),
      r[mInd]->GetStringValue().ToDouble(),
      r[sInd]->GetStringValue().ToDouble()));
    if (batch != InvalidIndex) {
      ref.SetBatch(r[batch]->GetStringValue().ToInt());
    }
    if (!zero_found && ref.GetHkl().IsNull()) {
      zero_found = true;
      continue;
    }
    ref.SetOmitted(zero_found);
  }
  olx_object_ptr<ref_list> rv(new ref_list);
  rv().a.TakeOver(refs);
  rv().b = intensity;
  return rv;
}
//..............................................................................
olx_object_ptr<THklFile::ref_list> THklFile::FromTonto(const TStrList &l_) {
  /* despite the fact the format is structured, here we do a simplistic parsing
  */
  // make a single line
  TStrList l = l_;
  for (size_t i = 0; i < l.Count(); i++) {
    size_t ci = l[i].FirstIndexOf('!');
    if (ci != InvalidIndex) {
      l[i].SetLength(ci);
    }
    l[i].Replace('\t', ' ')
      .Trim(' ')
      .DeleteSequencesOf(' ');
  }
  olx_object_ptr<ref_list> rv;
  TRefList refs;
  using namespace exparse;
  olxstr data = l.Pack().Text(' ');
  olxstr key_rdata("reflection_data="),
    key_keys("keys="),
    key_data("data=");
  size_t idx = data.IndexOf(key_rdata);
  if (idx == InvalidIndex) return rv;
  idx = data.FirstIndexOf(key_keys, idx+key_rdata.Length());
  if (idx == InvalidIndex) return rv;
  // extract the key legend
  idx = data.FirstIndexOf('{', idx+key_keys.Length());
  if (idx == InvalidIndex) return rv;
  olxstr keys;
  if (!parser_util::parse_brackets(data, keys, idx))
    return rv;
  TStrList ktoks(keys, ' ');
  const size_t hi = ktoks.IndexOfi("h=");
  const size_t ki = ktoks.IndexOfi("k=");
  const size_t li = ktoks.IndexOfi("l=");
  size_t fi = ktoks.IndexOfi("f_exp=");
  size_t si = ktoks.IndexOfi("f_sigma=");
  bool intensity;
  if ((fi | si) == InvalidIndex) {
    fi = ktoks.IndexOfi("i_exp=");
    si = ktoks.IndexOfi("i_sigma=");
    intensity = true;
  }
  else {
    intensity = false;
  }
  if ((hi|ki|li|fi|si) == InvalidIndex) {
    return rv;
  }
  idx = data.FirstIndexOfi(key_data, idx);
  if (idx == InvalidIndex) return rv;
  idx = data.FirstIndexOf('{', idx + key_data.Length());
  if (idx == InvalidIndex) return rv;
  olxstr data_data;
  if (!parser_util::parse_brackets(data, data_data, idx))
    return rv;
  TStrList dtoks(data_data, ' ');
  if ((dtoks.Count() % ktoks.Count()) != 0) {
    return rv;
  }
  const size_t rc = dtoks.Count() / ktoks.Count();
  for (size_t i = 0; i < rc; i++) {
    size_t off = i*ktoks.Count();
    TReflection &r = refs.AddNew(
      dtoks[off + hi].ToInt(),
      dtoks[off + ki].ToInt(),
      dtoks[off + li].ToInt(),
      dtoks[off + fi].ToDouble(),
      dtoks[off + si].ToDouble()
      );
  }
  rv = new ref_list;
  rv().a.TakeOver(refs);
  rv().b = intensity;
  return rv;
}
//..............................................................................
bool THklFile::IsHKLFileLine(const olxstr& l, const olxstr &format) {
  if (format.IsEmpty()) {
    if (l.Length() >= 28) {
      return (l.SubString(0, 4).IsNumber() && l.SubString(4, 4).IsNumber() &&
        l.SubString(8, 4).IsNumber() && l.SubString(12, 8).IsNumber() &&
        l.SubString(20, 8).IsNumber());
    }
    else {
      return false;
    }
  }
  TSizeList fl = TSizeList::FromList(TStrList(format, ","),
    FunctionAccessor::MakeConst(&olxstr::ToSizeT));
  size_t ll = olx_sum(fl);
  if (l.Length() >= ll) {
    size_t idx = 0;
    for (size_t i = 0; i < fl.Count(); i++) {
      if (!l.SubString(idx, fl[i]).IsNumber()) {
        return false;
      }
      idx += fl[i];
    }
    return true;
  }
  return false;
}
//..............................................................................
bool THklFile::IsHKLFileLine(const olxstr& l, const TSizeList &format) {
  size_t idx = 0;
  for (size_t i = 0; i < format.Count(); i++) {
    if (idx + format[i] > l.Length()) {
      return false;
    }
    if (!l.SubString(idx, format[i]).IsNumber()) {
      return false;
    }
    idx += format[i];
  }
  return true;
}
//..............................................................................
