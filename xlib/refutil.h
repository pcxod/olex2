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
      stats.MinIndices = vec3i(100, 100, 100);
      stats.MaxIndices = -stats.MinIndices;
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
        vec3i::UpdateMinMax(r.GetHkl(), _stats->MinIndices, _stats->MaxIndices);
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

  /* calculates the scale sum(w*Fc*Fc)/sum(w*Fo^2) Fc = k*Fo.
  F - a list of doubles (F) or complex values
  refs - a list of reflections (Fo^2), sqrt of I will be taken
  */
  template <class eval_t, class FList, class RefList,
    class weight_t, class filter_t>
  static double CalcScale(const eval_t &eval,
    const FList& F, const RefList& refs,
    const weight_t& weights, const filter_t filter)
  {
    double sFo = 0, sFc = 0;
    for (size_t i = 0; i < F.Count(); i++) {
      const TReflection& r = olx_ref::get(refs[i]);
      if (!filter.DoesPass(r)) {
        continue;
      }
      double Fo = eval.get(r);
      double w = eval.getw(weights.Calculate(r));
      sFo += w * Fo * Fo;
      sFc += w * Fo * eval.get(F[i]);
    }
    return sFc / sFo;
  }
  /* calculates a best line scale : Fc = k*Fo + a.
  F - a list of doubles (F) or complex values (F)
  refs - a list of doubles (Fo) or reflections (Fo^2)
  */
  template <class eval_t, class FList, class RefList,
    class weight_t, class filter_t>
  void CalcScale(const eval_t &eval,
    const FList& F, const RefList& refs,
    const weight_t& weights, const filter_t filter,
    double& k, double& a)
  {
    double sx = 0, sy = 0, sxs = 0, sxy = 0, sw = 0;
    const size_t f_cnt = F.Count();
    for (size_t i = 0; i < f_cnt; i++) {
      const TReflection& r = olx_ref::get(refs[i]);
      if (!filter.DoesPass(r)) {
        continue;
      }
      double w = eval.getw(weights.Calculate(r));
      const double I = eval.get(r);
      const double qm = eval.get(F[i]);
      sx += w * I;
      sy += w * qm;
      sxy += w * I * qm;
      sxs += w * I * I;
      sw += w;
    }
    k = (sxy - sx * sy / sw) / (sxs - sx * sx / sw);
    a = (sy - k * sx) / sw;
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
      double scale)
      :wght(wght_),
      h2c(h2c),
      scale(scale)
    {
      bool fix_f = wght.Count() < 6;
      while (wght.Count() < 6) {
        wght.Add(0.0);
      }
      if (fix_f) {
        wght[5] = 1. / 3;
      }
    }
    double Calculate(const TReflection& r, double Fc2) const {
      const double Fo2 = scale*r.GetI();
      const double P = wght[5] * olx_max(0, Fo2) + (1.0 - wght[5]) * Fc2;
      double stl = 0;
      if (wght[2] != 0 || wght[4] != 0) {
        stl = 1. / sqrt(4 * r.ToCart(h2c).QLength());
      }
      double q = 1.0;
      if (wght[2] > 0) {
        q = olx_exp(wght[2] * olx_sqr(stl));
      }
      else if (wght[2] < 0) {
        q = 1 - olx_exp(wght[2] * olx_sqr(stl));
      }
      double t = olx_sqr(scale*r.GetS()) + olx_sqr(wght[0] * P) + wght[1] * P + wght[3];
      if (wght[4] != 0) {
        t += wght[4] * stl;
      }
      return q / t;
    }

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
  static double CalcScale(const RefinementModel& rm,
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
  static double CalcFScale(const RefinementModel& rm,
    const fc_list_t& fcs, ref_list_t& refs)
  {
    return CalcScale(rm, F_evaluator(), fcs, refs);
  }
  template <typename fc_list_t, class ref_list_t>
  static double CalcFsqScale(const RefinementModel& rm,
    const fc_list_t& fcs, ref_list_t& refs)
  {
    return CalcScale(rm, Fsq_evaluator(), fcs, refs);
  }

  template <class eval_t, typename fc_list_t, class ref_list_t>
  static olx_pair_t<double, double>
    CalcScaleR(const RefinementModel& rm,
      const eval_t& eval, const fc_list_t& fcs, ref_list_t& refs, double scale=-1)
  {
    if (scale == -1) {
      scale = 1. / olx_sqr(rm.Vars.GetVar(0).GetValue());
    }
    olx_pair_t<double, double> rv(0, 0);
    CalcScale(eval, fcs, refs,
      CustomWeightCalculator::make(
        ShelxWeightCalculator(rm.used_weight,
          rm.aunit.GetHklToCartesian(),
          scale).CalcuateAll(refs, fcs).obj()),
      TReflection::DummyFilter(),
      rv.a, rv.b);
    return rv;
  }
  template <typename fc_list_t, class ref_list_t>
  static olx_pair_t<double, double>
    CalcFScaleR(const RefinementModel& rm,
      const fc_list_t& fcs, ref_list_t& refs)
  {
    return CalcScaleR(rm, RefUtil::F_evaluator(), fcs, refs);
  }
  template <typename fc_list_t, class ref_list_t>
  static olx_pair_t<double, double>
    CalcFsqScaleR(const RefinementModel& rm,
      const fc_list_t& fcs, ref_list_t& refs)
  {
    return CalcScaleR(rm, RefUtil::Fsq_evaluator(), fcs, refs);
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
