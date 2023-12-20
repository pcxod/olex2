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
    bool standardise_for_omit;
    smatd_list standardisation_matrices;
  public:
    ResolutionAndSigmaFilter(const RefinementModel& _rm);
    void SetStats(RefinementModel::HklStat& stats) const;
    bool IsOutside(const TReflection& r) const;
    bool IsOmitted(const vec3i& hkl) const;
    void AdjustIntensity(TReflection& r) const;
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

  struct F_evaluator {
    double get(double v) const { return v; }
    double get(const compd& v) const { return v.mod(); }
    template <class ref_t>
    double get(const ref_t& r) const {
      return TReflection::GetF(r);
    }
    double getw(double w) const { return sqrt(w); }
  };
  struct Fsq_evaluator {
    double get(double v) const { return v; }
    double get(const compd& v) const { return v.qmod(); }
    template <class ref_t>
    double get(const ref_t& r) const {
      return TReflection::GetFsq(r);
    }
    double getw(double w) const { return w; }
  };

  /* calculates the scale weighted L.S. scale to minimise
  * sum[w(Fc/scale - Fo)^2], as weight can depend on Fo - Fc is
  * brought ont the Fo scale
  */
  template <class eval_t, class FList, class RefList,
    class weight_t, class filter_t>
  static double CalcScale(const eval_t &eval,
    const FList& F, const RefList& refs,
    const weight_t& weights, const filter_t filter)
  {
    double swXX = 0, swXY = 0;
    for (size_t i = 0; i < F.Count(); i++) {
      const TReflection& r = olx_ref::get(refs[i]);
      if (!filter.DoesPass(r)) {
        continue;
      }
      double Fo = eval.get(r);
      double Fc = eval.get(F[i]);
      double w = eval.getw(weights.Calculate(r));
      swXX += w * Fc * Fc;
      swXY += w * Fc * Fo;
    }
    return swXX / swXY;
  }

  template <int ext>
  struct SigmaWeightCalculator {
    double Calculate(const TReflection& r) const {
      if (ext == 1) {
        return 1. / olx_max(r.GetS(), 1e-6);
      }
      else if (ext == 2) {
        return 1. / olx_max(r.GetS() * r.GetS(), 1e-6);
      }
      else {
        return 1. / olx_max(pow(r.GetS(), (double)ext), 1e-6);
      }
    }
  };

  /* Reflection tag must match index in the weights! */
  struct CustomWeightCalculator {
    template <class list_t>
    struct CustomWeightCalculator_ {
      const list_t& weights;
      CustomWeightCalculator_(const list_t& weights)
        : weights(weights)
      {}
      double Calculate(const TReflection& r) const {
        return weights[r.GetTag()];
      }
    };
    template <class list_t>
    static CustomWeightCalculator_<list_t> make(const list_t& weights) {
      return CustomWeightCalculator_<list_t>(weights);
    }
  };

  struct ShelxWeightCalculator {
    TDoubleList wght;
    mat3d h2c;
    double scale;
    ShelxWeightCalculator(const TDoubleList& wght_, const mat3d& h2c,
      double scale);
    double Calculate(const TReflection& r, double Fc2) const;

    static double get_Fc2(double v) { return v; }
    static double get_Fc2(const compd &v) { return v.qmod(); }

    template<class ref_list_t, typename fc_list_t>
    TDoubleList::const_list_type CalcuateAll(ref_list_t &refs,
      const fc_list_t &Fcs)
    {
      TDoubleList rv(refs.Count());
      for (size_t i = 0; i < refs.Count(); i++) {
        TReflection& r = olx_ref::get(refs[i]);
        r.SetTag(i);
        rv[i] = Calculate(r, get_Fc2(Fcs[i]));
      }
      return rv;
    }
  };


  template <class eval_t, typename fc_list_t, class ref_list_t>
  static double CalcScaleShelx(const RefinementModel& rm,
    const eval_t& eval, const fc_list_t& fcs, ref_list_t& refs, double scale=-1)
  {
    if (scale == -1) {
      scale = 1. / olx_sqr(rm.Vars.GetVar(0).GetValue());
    }
    return CalcScale(eval, fcs, refs,
      CustomWeightCalculator::make(
        ShelxWeightCalculator(rm.used_weight,
          rm.aunit.GetHklToCartesian(),
          scale).CalcuateAll(refs, fcs).obj()),
      TReflection::DummyFilter());
  }
  template <typename fc_list_t, class ref_list_t>
  static double CalcFScaleShelx(const RefinementModel& rm,
    const fc_list_t& fcs, ref_list_t& refs)
  {
    return CalcScaleShelx(rm, F_evaluator(), fcs, refs);
  }
  template <typename fc_list_t, class ref_list_t>
  static double CalcFsqScaleShelx(const RefinementModel& rm,
    const fc_list_t& fcs, ref_list_t& refs)
  {
    return CalcScaleShelx(rm, Fsq_evaluator(), fcs, refs);
  }

  template <class eval_t, typename fc_list_t, class ref_list_t>
  static double CalcScaleSigma(const RefinementModel& rm,
    const eval_t& eval, const fc_list_t& fcs, ref_list_t& refs)
  {
    return CalcScale(eval, fcs, refs, SigmaWeightCalculator<2>(),
      TReflection::DummyFilter());
  }
  template <typename fc_list_t, class ref_list_t>
  static double CalcFScaleSigma(const RefinementModel& rm,
      const fc_list_t& fcs, ref_list_t& refs)
  {
    return CalcScaleSigma(rm, RefUtil::F_evaluator(), fcs, refs);
  }
  template <typename fc_list_t, class ref_list_t>
  static double CalcFsqScaleSigma(const RefinementModel& rm,
      const fc_list_t& fcs, ref_list_t& refs)
  {
    return CalcScaleSigma(rm, RefUtil::Fsq_evaluator(), fcs, refs);
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
    Stats(bool update_scale, bool fcf);

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
