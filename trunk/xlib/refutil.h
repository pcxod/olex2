#ifndef __olx_ref_util_H
#define __olx_ref_util_H
#include "xbase.h"
#include "symmat.h"
#include "symspace.h"
#include "arrays.h"

BeginXlibNamespace()

namespace RefUtil {
  template <class MatList> size_t GetBijovetPairs(const TRefList& refs, const vec3i& min_indices,
    const vec3i& max_indices, TRefPList& pos, TRefPList& neg, const MatList& ml)
  {
    SymSpace::InfoEx sp = SymSpace::Compact(ml);
    if( sp.centrosymmetric )
      return 0;
    pos.SetCapacity(refs.Count()/2);
    neg.SetCapacity(refs.Count()/2);
    TArray3D<TReflection*> hkl3d(min_indices, max_indices);
    for( size_t i=0; i < refs.Count(); i++ )  {
      hkl3d(refs[i].GetHkl()) = &refs[i];
      refs[i].SetTag(i);
    }
    size_t cnt = 0;
    for( size_t i=0; i < refs.Count(); i++ )  {
      if( refs[i].GetTag() < 0 )  continue;
      refs[i].SetTag(-1);
      for( size_t mi=0; mi < sp.matrices.Count(); mi++ )  {
        const vec3i& pi = refs[i].GetHkl();
        vec3i ni;
        refs[i].MulHkl(ni, sp.matrices[mi]);
        ni *= -1;
        if( hkl3d.IsInRange(ni) && hkl3d(ni) != NULL ) {
          TReflection& n = *hkl3d(ni);
          if( n.GetTag() < 0 )  continue;
          pos.Add(refs[i]);
          neg.Add(n)->SetTag(-1);
          cnt++;
        }
      }
    }
    return cnt;
  }

  class ResolutionAndSigmaFilter  {
    const RefinementModel& rm;
    RefinementModel::HklStat& stats;
    double h_o_s, min_d, max_d;
  public:
    ResolutionAndSigmaFilter(const RefinementModel& _rm,
      RefinementModel::HklStat& _stats)
    : rm(_rm), stats(_stats)
    {
      double SHEL_hr = rm.GetSHEL_hr();
      double SHEL_lr = rm.GetSHEL_lr();
      if( SHEL_hr > SHEL_lr ) olx_swap(SHEL_hr, SHEL_lr);
      h_o_s = 0.5*rm.GetOMIT_s();
      const double two_sin_2t = 2*sin(rm.GetOMIT_2t()*M_PI/360.0);
      min_d = rm.expl.GetRadiation()/(two_sin_2t == 0 ? 1e-6 : two_sin_2t);
      if( rm.HasSHEL() && SHEL_hr > min_d )
        min_d = SHEL_hr;
      stats.LimDmax = max_d = SHEL_lr;
      stats.LimDmin = min_d;
      stats.MinD = 100;
      stats.MaxD = -100;
      stats.MinI = 100;
      stats.MaxI = -100;
      stats.MERG = rm.GetMERG();
      stats.OMIT_s = rm.GetOMIT_s();
      stats.OMIT_2t = rm.GetOMIT_2t();
      stats.SHEL_lr = SHEL_lr;
      stats.SHEL_hr = SHEL_hr;
      stats.MinIndexes = vec3i(100, 100, 100);
      stats.MaxIndexes = -stats.MinIndexes;
    }
    bool IsOutside(const TReflection& r) const {
      const double d = 1/r.ToCart(rm.aunit.GetHklToCartesian()).Length();
      if( (h_o_s > 0 && r.GetI() < h_o_s*r.GetS()) || d >= max_d || d <= min_d )  {
        stats.FilteredOff++;
        return true;
      }
      olx_update_min_max(r.GetI(), stats.MinI, stats.MaxI);
      olx_update_min_max(d, stats.MinD, stats.MaxD);
      vec3i::UpdateMinMax(r.GetHkl(), stats.MinIndexes, stats.MaxIndexes);
      return false;
    }
  };
};  // end of namespace RefUtil

EndXlibNamespace()
#endif
