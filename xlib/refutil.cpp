/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "refmodel.h"
#include "refutil.h"
#include "sfutil.h"
#include "xapp.h"
#include "refutil.h"
#include "cif.h"
#include "unitcell.h"

using namespace RefUtil;

ResolutionAndSigmaFilter::ResolutionAndSigmaFilter(const RefinementModel& _rm) : rm(_rm),
_stats(0)
{
  double SHEL_hr = rm.GetSHEL_hr();
  double SHEL_lr = rm.GetSHEL_lr();
  if (SHEL_hr > SHEL_lr) {
    olx_swap(SHEL_hr, SHEL_lr);
  }
  h_o_s = 0.5 * rm.GetOMIT_s();
  const double two_sin_2t = 2 * sin(rm.GetOMIT_2t() * M_PI / 360.0);
  min_d = rm.expl.GetRadiation() / (two_sin_2t == 0 ? 1e-6 : two_sin_2t);
  if (rm.HasSHEL() && SHEL_hr > min_d) {
    min_d = SHEL_hr;
  }
  max_d = SHEL_lr;
  standardise_for_omit = rm.GetHKLF() < 5 && rm.GetMERG() != 0;
  if (standardise_for_omit) {
    standardisation_matrices.SetCapacity(rm.aunit.MatrixCount() * (rm.aunit.GetLatt() > 0 ? 2 : 1) + 1);
    standardisation_matrices.AddCopyAll(rm.aunit.GetMatices());
    if (rm.aunit.GetLatt() > 0) {
      standardisation_matrices.AddNew().I() *= -1;
      for (size_t i = 0; i < rm.aunit.MatrixCount(); i++) {
        standardisation_matrices.AddCopy(rm.aunit.GetMatrix(i)) *= -1;
      }
    }
  }
}
//.............................................................................
void ResolutionAndSigmaFilter::SetStats(RefinementModel::HklStat& stats) const {
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
//.............................................................................
bool ResolutionAndSigmaFilter::IsOutside(const TReflection& r) const {
  const double d = 1 / r.ToCart(rm.aunit.GetHklToCartesian()).Length();
  if ((h_o_s > 0 && r.GetI() < h_o_s * r.GetS()) || d >= max_d || d <= min_d) {
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
//.............................................................................
bool ResolutionAndSigmaFilter::IsOmitted(const vec3i& hkl) const {
  if (standardise_for_omit) {
    return rm.GetOmits().Contains(
      TReflection::Standardise(hkl, standardisation_matrices));
  }
  else {
    return rm.GetOmits().Contains(hkl);
  }
}
//.............................................................................
void ResolutionAndSigmaFilter::AdjustIntensity(TReflection& r) const {
  if (r.GetI() < h_o_s * r.GetS()) {
    r.SetI(h_o_s * r.GetS());
    if (_stats != 0) {
      _stats->IntensityTransformed++;
    }
  }
}
//.............................................................................
//.............................................................................
//.............................................................................
ShelxWeightCalculator::ShelxWeightCalculator(const TDoubleList& wght_,
  const mat3d& h2c, double scale)
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
//.............................................................................
double ShelxWeightCalculator::Calculate(const TReflection& r, double Fc2) const {
  const double Fo2 = scale * r.GetI();
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
  double t = olx_sqr(scale * r.GetS()) + olx_sqr(wght[0] * P) + wght[1] * P + wght[3];
  if (wght[4] != 0) {
    t += wght[4] * stl;
  }
  return q / t;
}
//.............................................................................
//.............................................................................
//.............................................................................
Stats::Stats(bool update_scale, bool fcf)
: sum_wsqd(0),
  partial_R1_cnt(0),
  min_hkl(1000),
  max_hkl(-1000)
{
  partical_threshold = 2;
  TXApp& xapp = TXApp::GetInstance();
  RefinementModel& rm = xapp.XFile().GetRM();
  double scale_k = 1. / olx_sqr(rm.Vars.GetVar(0).GetValue());
  if (fcf) {
    scale_k = 1;
    using namespace cif_dp;
    TCif cif;
    cif.LoadFromFile(TEFile::ChangeFileExt(xapp.XFile().GetFileName(), "fcf"));
    cetTable *rl = cif.FindLoop("_refln");
    if (rl == 0) {
      throw TInvalidArgumentException(__OlxSourceInfo, "FCF file, LIST 4 is expected");
    }
    size_t h_idx = rl->ColIndex("_refln_index_h");
    size_t k_idx = rl->ColIndex("_refln_index_k");
    size_t l_idx = rl->ColIndex("_refln_index_l");
    size_t Fc_sq_idx = rl->ColIndex("_refln_F_squared_calc");
    size_t Fo_sq_idx = rl->ColIndex("_refln_F_squared_meas");
    size_t sig_idx = rl->ColIndex("_refln_F_squared_sigma");
    size_t weight_idx = rl->ColIndex("_refln_F_squared_weight");
    if ((h_idx | k_idx | l_idx | Fc_sq_idx | sig_idx | Fo_sq_idx) == InvalidIndex) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        "FCF file, LIST 4 with weights is expected");
    }
    refs.SetCapacity(rl->RowCount());
    Fsq.SetCount(rl->RowCount());
    if (weight_idx != InvalidIndex) {
      weights.SetCount(rl->RowCount());
    }
    for (size_t i = 0; i < rl->RowCount(); i++) {
      TReflection* r = new TReflection(
        (*rl)[i][h_idx]->GetStringValue().ToInt(),
        (*rl)[i][k_idx]->GetStringValue().ToInt(),
        (*rl)[i][l_idx]->GetStringValue().ToInt(),
        (*rl)[i][Fo_sq_idx]->GetStringValue().ToDouble(),
        (*rl)[i][sig_idx]->GetStringValue().ToDouble());
      refs.Add(r);
      Fsq[i] = (*rl)[i][Fc_sq_idx]->GetStringValue().ToDouble();
      if (weight_idx != InvalidIndex) {
        weights[i] = (*rl)[i][weight_idx]->GetStringValue().ToDouble();
      }
      refs[i].SetTag(i);
    }
  }
  else {
    xapp.CalcFsq(refs, Fsq, false, SFUtil::EXTIDest::Fo);
  }
  if (weights.IsEmpty()) {
    weights.Resize(refs.Count());
    ShelxWeightCalculator weight_c(rm.used_weight,
      rm.aunit.GetHklToCartesian(), scale_k);
    if (update_scale) {
      weight_c.scale = CalcScale(Fsq_evaluator(), Fsq, refs,
        SigmaWeightCalculator<2>(),
        TReflection::DummyFilter());
    }
    for (size_t i = 0; i < refs.Count(); i++) {
      weights[i] = weight_c.Calculate(refs[i], Fsq[i]);
      refs[i].SetTag(i);
    }
  }
  if (update_scale) {
    scale_k = CalcScale(Fsq_evaluator(), Fsq, refs,
      CustomWeightCalculator::make(weights),
      TReflection::DummyFilter());
  }
  
  double wR2d = 0, R1u = 0, R1d = 0, R1up = 0, R1dp = 0;
  wsqd.Resize(refs.Count());
  for (size_t i = 0; i < refs.Count(); i++) {
    TReflection& r = refs[i];
    r *= scale_k;
    vec3i::UpdateMinMax(r.GetHkl(), min_hkl, max_hkl);
    const double Fc2 = Fsq[i];
    const double Fc = sqrt(Fc2);
    const double Fo2 = r.GetI();
    const double Fo = sqrt(Fo2 < 0 ? 0 : Fo2);
    wsqd[i] = weights[i] * olx_sqr(Fo2 - Fc2);
    sum_wsqd += wsqd[i];
    wR2d += weights[i] * olx_sqr(Fo2);
    double Fd = olx_abs(Fo - Fc);
    R1u += Fd;
    R1d += Fo;
    if (Fo2 >= partical_threshold * r.GetS()) {
      R1up += Fd;
      R1dp += Fo;
      partial_R1_cnt++;
    }
  }
  wR2 = sqrt(sum_wsqd / wR2d);
  R1 = R1u / R1d;
  R1_partial = R1up / R1dp;
}
//.............................................................................
double Stats::UpdatePartialR1(double threshold) {
  partical_threshold = threshold;
  double R1up = 0, R1dp = 0;
  partial_R1_cnt = 0;
  for (size_t i = 0; i < refs.Count(); i++) {
    TReflection& r = refs[i];
    if (r.GetI() >= partical_threshold * r.GetS()) {
      double Fc2 = Fsq[i];
      const double Fc = sqrt(olx_abs(Fc2));
      const double Fo = sqrt(r.GetI() < 0 ? 0 : r.GetI());
      R1up += olx_abs(Fo - Fc);
      R1dp += Fo;
      partial_R1_cnt++;
    }
  }
  return (R1_partial = R1up / olx_max(1, R1dp));
}
//.............................................................................
TRefPList::const_list_type Stats::GetBadRefs_(size_t N, double th,
  bool use_n, double *wR2) const
{
  typedef olx_pair_t<double, size_t> pair_t;
  TTypeList<pair_t> dr;
  for (size_t i = 0; i < wsqd.Count(); i++) {
    dr.Add(new pair_t(wsqd[i], i));
  }
  QuickSorter::Sort(dr, ReverseComparator::Make(
    ComplexComparator::Make(
      FunctionAccessor::MakeConst(&pair_t::GetA),
      TPrimitiveComparator()))
  );
  TRefPList testr;
  double rup = 0, rdn = 0;
  for (size_t i = 0; i < dr.Count(); i++) {
    if (use_n) {
      if (i == N) {
        break;
      }
    }
    else if (dr[i].a/sum_wsqd < th) {
      break;
    }
    TReflection& r = refs[dr[i].b];
    testr.Add(&r);
    if (wR2 != 0) {
      double w = weights[r.GetTag()];
      rup += w * olx_sqr(r.GetI() - Fsq[r.GetTag()]);
      rdn += w * olx_sqr(r.GetI());
    }
  }
  if (wR2 != 0) {
    *wR2 = sqrt(rup / rdn);
  }
  return testr;
}


