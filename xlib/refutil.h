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
#include "bitarray.h"

BeginXlibNamespace()

namespace RefUtil {
  class ResolutionAndSigmaFilter {
    const RefinementModel& rm;
    mutable RefinementModel::HklStat *_stats;
    double h_o_s, min_d, max_d;
  public:
    ResolutionAndSigmaFilter(const RefinementModel& _rm) : rm(_rm),
      _stats(0)
    {
      double SHEL_hr = rm.GetSHEL_hr();
      double SHEL_lr = rm.GetSHEL_lr();
      if (SHEL_hr > SHEL_lr) {
        olx_swap(SHEL_hr, SHEL_lr);
      }
      h_o_s = 0.5*rm.GetOMIT_s();
      const double two_sin_2t = 2 * sin(rm.GetOMIT_2t()*M_PI / 360.0);
      min_d = rm.expl.GetRadiation() / (two_sin_2t == 0 ? 1e-6 : two_sin_2t);
      if (rm.HasSHEL() && SHEL_hr > min_d) {
        min_d = SHEL_hr;
      }
      max_d = SHEL_lr;
    }
    void SetStats(RefinementModel::HklStat &stats) const {
      _stats = &stats;
      stats.LimDmax = max_d;
      stats.LimDmin = min_d;
      stats.MinD = 100;
      stats.MaxD = -100;
      stats.MinI = 100;
      stats.MaxI = -100;
      stats.MERG = rm.GetMERG();
      stats.OMIT_s = rm.GetOMIT_s();
      stats.OMIT_2t = rm.GetOMIT_2t();
      stats.SHEL_lr = rm.GetSHEL_lr();
      stats.SHEL_hr = rm.GetSHEL_hr();
      stats.MinIndexes = vec3i(100, 100, 100);
      stats.MaxIndexes = -stats.MinIndexes;
    }
    bool IsOutside(const TReflection& r) const {
      const double d = 1 / r.ToCart(rm.aunit.GetHklToCartesian()).Length();
      if ((h_o_s > 0 && r.GetI() < h_o_s*r.GetS()) || d >= max_d || d <= min_d) {
        if (_stats != 0) {
          _stats->FilteredOff++;
        }
        return true;
      }
      if (_stats != 0) {
        olx_update_min_max(r.GetI(), _stats->MinI, _stats->MaxI);
        olx_update_min_max(d, _stats->MinD, _stats->MaxD);
        vec3i::UpdateMinMax(r.GetHkl(), _stats->MinIndexes, _stats->MaxIndexes);
      }
      return false;
    }
    bool IsOmitted(const vec3i& hkl) const {
      return (rm.GetOmits().IndexOf(hkl) != InvalidIndex);
    }
    void AdjustIntensity(TReflection& r) const {
      if (r.GetI() < h_o_s*r.GetS()) {
        r.SetI(h_o_s*r.GetS());
        if (_stats != 0) {
          _stats->IntensityTransformed++;
        }
      }
    }
    struct IntensityModifier {
      const ResolutionAndSigmaFilter& parent;
      IntensityModifier(const ResolutionAndSigmaFilter& _parent) : parent(_parent) {}
      void OnItem(TReflection& r, size_t) const { parent.AdjustIntensity(r); }
    };

  };

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
    TEBitArray used(refs.Count());
    olx_array::TArray3D<TReflection*> hkl3d(min_indices, max_indices);
    for (size_t i = 0; i < refs.Count(); i++) {
      hkl3d(refs[i].GetHkl()) = &refs[i];
      refs[i].SetTag(i);
    }
    for (size_t i = 0; i < refs.Count(); i++) {
      if (used[i]) {
        continue;
      }
      used.SetTrue(i);
      for (size_t mi = 0; mi < sp.matrices.Count(); mi++) {
        const vec3i& pi = refs[i].GetHkl();
        vec3i ni = TReflection::MulHkl(refs[i].GetHkl(), sp.matrices[mi]);
        ni *= -1;
        bool found = false;
        if (hkl3d.IsInRange(ni) && hkl3d(ni) != 0) {
          TReflection& n = *hkl3d(ni);
          if (used[n.GetTag()]) {
            continue;
          }
          pos.Add(refs[i]);
          neg.Add(n);
          used.SetTrue(n.GetTag());
          found = true;
        }
        if (found) {
          break;
        }
      }
    }
    return pos.Count();
  }

  /* Requires an instance of TXApp */
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
