#include "refmodel.h"
#include "refutil.h"
#include "sfutil.h"
#include "xapp.h"
#include "refutil.h"

using namespace RefUtil;

Stats::Stats(bool update_scale)
: sum_wsqd(0),
  partial_R1_cnt(0),
  min_hkl(1000),
  max_hkl(-1000)
{
  partical_threshold = 2;
  TXApp& xapp = TXApp::GetInstance();
  RefinementModel& rm = xapp.XFile().GetRM();
  double scale_k = 1. / olx_sqr(rm.Vars.GetVar(0).GetValue());
  xapp.CalcFsq(refs, Fsq, false);
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
    const double Fc = sqrt(olx_abs(Fc2));
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


