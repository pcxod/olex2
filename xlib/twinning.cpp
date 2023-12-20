/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "twinning.h"
using namespace twinning;
using namespace olx_array;

handler::handler(const SymmSpace::InfoEx& _sym_info, const TRefList& refs,
  const TDoubleList& _scales,
  const mat3d& tm, int n)
  : sym_info(_sym_info),
  measured(refs)
{
  if (_scales.Count() + 1 != olx_abs(n)) {
    throw TInvalidArgumentException(__OlxSourceInfo, "BASF or N");
  }
  scales.AddNew(1.0 - olx_sum(_scales));
  for (size_t i = 0; i < _scales.Count(); i++) {
    scales.AddNew(_scales[i]);
  }
  components.SetCount(refs.Count());
  TArrayList<mat3d> matrices;
  size_t t = olx_abs(n);
  if (n < 0) {
    t /= 2;
  }
  for (size_t j = 0; j < t - 1; j++) {
    matrices.Add(tm);
  }
  if (n < 0) {
    matrices.Add().I() *= -1;
    mat3i m = tm * -1;
    for (size_t j = 0; j < t - 1; j++) {
      matrices.Add(m);
    }
  }
  for (size_t i = 0; i < measured.Count(); i++) {
    measured[i].SetBatch(1);
    for (size_t j = 0; j < matrices.Count(); j++) {
      vec3i hkl = TReflection::Standardise(
        (matrices[j] * measured[i].GetHkl()).Round<int>(), sym_info);
      if (!TReflection::IsAbsent(hkl, sym_info)) {
        components[i].AddNew(hkl, scales[j + 1]);
      }
    }
  }
  calc_range();
}
//..............................................................................
handler::handler(const SymmSpace::InfoEx& _sym_info, const TRefList& refs,
  const RefUtil::ResolutionAndSigmaFilter& filter,
  const TDoubleList& _scales)
  : sym_info(_sym_info)
{
  filter.SetStats(ms);
  scales.AddNew(1.0 - olx_sum(_scales));
  for (size_t i = 0; i < _scales.Count(); i++) {
    scales.AddNew(_scales[i]);
  }
  components.SetCapacity(refs.Count());
  measured.SetCapacity(refs.Count());
  for (size_t i = 0; i < refs.Count(); i++) {
    olx_object_ptr<TTypeList<twin_component> > comp =
      new TTypeList<twin_component>();
    bool omit = false;
    while (i < refs.Count() && refs[i].GetBatch() < 0) {
      size_t bi = olx_abs(refs[i].GetBatch()) - 1;
      if (bi >= scales.Count()) {
        throw TInvalidArgumentException(__OlxSourceInfo,
          olxstr("batch number in: ").quote() << refs[i].ToString());
      }
      vec3i hkl = TReflection::Standardise(refs[i].GetHkl(), sym_info);
      if (refs[i].IsOmitted()) {
        ms.OmittedByUser++;
        omit = true;
      }
      else if (filter.IsOutside(refs[i])) {
        omit = true;
      }
      else if (filter.IsOmitted(hkl)) {
        omit = true;
      }
      if (!TReflection::IsAbsent(hkl, sym_info)) {
        comp->AddNew(hkl, scales[bi]);
      }
      else {
        ms.SystematicAbsencesRemoved++;
      }
      i++;
    }
    if (i >= refs.Count()) {
      break;
    }
    // becomes incomplete set for detwinning
    if (omit) {
      continue;
    }
    if (olx_abs(refs[i].GetBatch()) >= scales.Count() + 1) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        olxstr("batch number in: ").quote() << refs[i].ToString());
    }
    if (refs[i].IsOmitted() || filter.IsOmitted(refs[i].GetHkl())) {
      ms.OmittedByUser++;
      continue;
    }
    if (filter.IsOutside(refs[i].GetHkl())) {
      ms.FilteredOff++;
      continue;
    }
    components.Add(comp.release());
    measured.AddNew(TReflection::Standardise(refs[i].GetHkl(), sym_info),
      refs[i].GetI(), refs[i].GetS(), refs[i].GetBatch());
  }
  calc_range();
}
//..............................................................................
void handler::calc_range() {
  unique_indices.SetCapacity(measured.Count());
  vec3i min_idx(100), max_idx(-100);
  for (size_t i = 0; i < measured.Count(); i++) {
    iterator itr = iterate(i);
    while (itr.has_next()) {
      vec3i h = itr.next_index();
      vec3i::UpdateMinMax(h, min_idx, max_idx);
    }
  }
  hkl_to_ref_map = new TArray3D<map_et>(min_idx, max_idx);
  TArray3D<map_et>& map = *hkl_to_ref_map;
  map.FastInitWith(-1);
  // make sure unique_inices match the measured at the beginning!
  for (size_t i = 0; i < measured.Count(); i++) {
    map(measured[i].GetHkl()) = map_et(unique_indices.Count(), i);
    unique_indices << measured[i].GetHkl();
  }
  for (size_t i = 0; i < measured.Count(); i++) {
    iterator itr = iterate(i);
    itr.next_index();
    while (itr.has_next()) {
      const vec3i& h = itr.next_index();
      map_et& found = map(h);
      if (found.a == InvalidIndex) {
        found.a = unique_indices.Count();
        found.b = i;
        unique_indices << h;
      }
    }
  }
}
//..............................................................................
//..............................................................................
//..............................................................................
detwinner_algebraic::detwinner_algebraic(const TDoubleList& scales)
  : _m(scales.Count(), scales.Count())
{
  for (size_t i = 0; i < scales.Count(); i++) {
    size_t s = i;
    for (size_t j = 0; j < scales.Count(); j++, s++) {
      _m[i][s >= scales.Count() ? s - scales.Count() : s] = scales[j];
    }
  }
  if (!math::LU::Invert(_m)) {
    throw TFunctionFailedException(__OlxSourceInfo, "cannot invert the matrix");
  }
}
//..............................................................................
void detwinner_algebraic::detwin(const handler::iterator& itr,
  TTypeList<TReflection>& res) const
{
  TTypeList<TReflection> all;
  evecd I(_m.ColCount()), S(_m.ColCount());
  while (itr.has_next()) {
    TReflection& r = all.AddCopy(itr.next_obs());
    const size_t si = olx_abs(r.GetBatch()) - 1;
    if (si >= _m.ColCount()) {
      throw TInvalidArgumentException(__OlxSourceInfo, "batch number");
    }
    I[si] = r.GetI();
    S[si] = olx_sqr(r.GetS());
  }
  I = _m * I;
  S = _m * S;
  for (size_t i = 0; i < all.Count(); i++) {
    if (i > 0 && all[i].GetHkl() == all[0].GetHkl()) {
      continue;
    }
    TReflection& r = res.AddCopy(all[i]);
    r.SetI(I[i]);
    r.SetS(sqrt(S[i]));
  }
}
//..............................................................................
//..............................................................................
//..............................................................................
detwin_result detwinner_mixed::detwin(const handler::twin_mate_generator& itr) const {
  TTypeList<twin_mate_full> all;
  while (itr.has_next()) {
    all.Add(itr.next_full().release());
  }
  double f_sq = 0, s_sq = 0;
  for (size_t i = 0; i < all.Count(); i++) {
    double dn = 0;
    size_t s = i;
    for (size_t j = 0; j < all.Count(); j++, s++) {
      size_t ind = (s >= all.Count() ? s - all.Count() : s);
      dn += all[j].fc.qmod() * all[ind].scale;
    }
    double coeff = all[i].scale * all[0].fc.qmod() / dn;
    f_sq += coeff * all[i].f_sq;
    s_sq += coeff * olx_sqr(all[i].sig);
  }
  return detwin_result(f_sq, sqrt(s_sq));
}
//..............................................................................
//..............................................................................
//..............................................................................
detwin_result detwinner_shelx::detwin(const handler::twin_mate_generator& itr) const {
  olx_object_ptr<twin_mate_full> pr = itr.next_full();
  double sum_f_sq = pr->f_sq_calc();
  while (itr.has_next()) {
    sum_f_sq += itr.next_calc().f_sq_calc();
  }
  double s = pr->fc.qmod() / sum_f_sq;
  return detwin_result(pr->f_sq * s, pr->sig * s);
}
//..............................................................................
