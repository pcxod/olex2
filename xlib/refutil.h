/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_ref_util_H
#define __olx_ref_util_H
#include "xbase.h"
#include "symmat.h"
#include "reflection.h"
#include "arrays.h"

BeginXlibNamespace()

namespace RefUtil {
  template <class MatList>
  size_t GetBijovetPairs(const TRefList& refs, const vec3i& min_indices,
    const vec3i& max_indices, TRefPList& pos, TRefPList& neg,
    const MatList& ml)
  {
    SymmSpace::InfoEx sp = SymmSpace::Compact(ml);
    if (sp.centrosymmetric) {
      return 0;
    }
    pos.SetCapacity(refs.Count() / 2);
    neg.SetCapacity(refs.Count() / 2);
    TArray3D<TReflection*> hkl3d(min_indices, max_indices);
    for (size_t i = 0; i < refs.Count(); i++) {
      hkl3d(refs[i].GetHkl()) = &refs[i];
      refs[i].SetTag(i);
    }
    size_t cnt = 0;
    for (size_t i = 0; i < refs.Count(); i++) {
      if (refs[i].GetTag() < 0) {
        continue;
      }
      refs[i].SetTag(-1);
      for (size_t mi = 0; mi < sp.matrices.Count(); mi++) {
        const vec3i& pi = refs[i].GetHkl();
        vec3i ni;
        refs[i].MulHkl(ni, sp.matrices[mi]);
        ni *= -1;
        if (hkl3d.IsInRange(ni) && hkl3d(ni) != 0) {
          TReflection& n = *hkl3d(ni);
          if (n.GetTag() < 0) {
            continue;
          }
          pos.Add(refs[i]);
          neg.Add(n)->SetTag(-1);
          cnt++;
        }
      }
    }
    return cnt;
  }

  /* Requires and instance of TXApp */
  struct Stats {
    TRefList refs;
    evecd Fsq;
    evecd weights,
      wsqd; // weighted square differences 
    double R1, R1_partial, wR2;
    double sum_wsqd;
    double partical_threshold; // = 2;
    size_t partial_R1_cnt;
    vec3i min_hkl, max_hkl;
    Stats(bool update_scale);

    double UpdatePartialR1(double threshold);

    TRefPList::const_list_type GetNBadRefs(size_t N, double *wR2 = 0) const {
      return GetBadRefs_(N, 0.0, true, wR2);
    }
    TRefPList::const_list_type GetBadRefs(double threshold, double *wR2 = 0) const {
      return GetBadRefs_(0, threshold, false, wR2);
    }
  protected:
    TRefPList::const_list_type GetBadRefs_(size_t N, double th,
      bool use_n,
      double *wR2) const;
  };
};  // end of namespace RefUtil

EndXlibNamespace()
#endif
