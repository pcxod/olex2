#ifndef __olx_xlib_twinning_H
#define __olx_xlib_twinning_H
#include "reflection.h"
#include "refmodel.h"
#include "refutil.h"

BeginXlibNamespace()
namespace twinning  {

  struct twin_mate_calc {
    compd fc;
    double scale;
    twin_mate_calc(const compd& _fc, double _scale) : fc(_fc), scale(_scale)  {}
    twin_mate_calc() : scale(0)  {}
    double f_sq_calc() const {  return fc.qmod()*scale;  }
  };
  struct twin_mate_obs {
    double f_obs_sq, sig_obs, scale;
    twin_mate_obs(double _f_obs_sq, double _sig_obs, double _scale)
      : f_obs_sq(_f_obs_sq), sig_obs(_sig_obs), scale(_scale)  {}
    twin_mate_obs() : f_obs_sq(0), sig_obs(0), scale(0)  {}
  };
  struct detwin_result {
    double f_sq, sig;
    detwin_result() : f_sq(0), sig(0)  {} 
    detwin_result(double _f_sq, double _sig) : f_sq(_f_sq), sig(_sig) {}
  };
  struct twin_mate_full : public twin_mate_calc, public detwin_result {
    twin_mate_full(const compd& _fc, double _f_sq, double _sig, double _scale)
      : twin_mate_calc(_fc, _scale), detwin_result(_f_sq, _sig)  {}
    twin_mate_full() {}
  };

  struct detwinner_shelx  {
    template <typename twin_generator_t>
    static detwin_result detwin(const twin_generator_t& itr)  {
      twin_mate_full pr = itr.NextFull();
      double sum_f_sq=0;
      while( itr.HasNext() )
        sum_f_sq += itr.NextCalc().f_sq_calc();
      double s = pr.fc.qmod();
      s = s/(s*pr.scale+sum_f_sq);
      return detwin_result(pr.f_sq*s, pr.sig*s);
    }
  };

  struct detwinner_algebraic  {
    template <typename twin_generator_t>
    static detwin_result detwin(const twin_generator_t& itr)  {
      TTypeList<twin_mate_obs> all;
      while( itr.HasNext() )
        all.AddCCopy(itr.NextObs());

      if( all.Count() == 1 )
        return detwin_result(all[0].f_obs_sq*all[0].scale, all[0].sig_obs*all[0].scale);
      ematd m(all.Count(), all.Count()), r(all.Count(), 2);
      for( size_t i=0; i < all.Count(); i++ )  {
        size_t s = i;
        for( size_t j=0; j < all.Count(); j++, s++ )
          m[i][s >= all.Count() ? s-all.Count(): s] = all[i].scale;
        r[i][0] = all[i].f_obs_sq;
        r[i][1] = olx_sqr(all[i].sig_obs);
      }
      ematd::GaussSolve(m, r);
      return detwin_result(r[0][0], sqrt(r[0][1]));
    }
  };

  template <typename twin_iterator> struct twin_mate_generator {
    const twin_iterator& itr;
    const TDoubleList& scales;
    const TArrayList<compd>& Fc;
    twin_mate_generator(const twin_iterator& _itr, const TDoubleList& _scales,
      const TArrayList<compd>& _Fc)
      : itr(_itr), scales(_scales), Fc(_Fc)  {}
    bool HasNext() const {  return itr.HasNext();  }
    twin_mate_full NextFull() const {
      TReflection r = itr.Next();
      if( r.GetTag() < 0 )
        return twin_mate_full();
      if( (size_t)r.GetTag() > Fc.Count() )
        throw TIndexOutOfRangeException(__OlxSourceInfo, r.GetTag(), 0, Fc.Count());
      const size_t bi = olx_abs(r.GetBatch())-1;
      return twin_mate_full(
        Fc[r.GetTag()], r.GetI(), r.GetS(), bi < scales.Count() ? scales[bi] : 0);
    }
    twin_mate_calc NextCalc() const {
      TReflection r = itr.Next();
      if( r.GetTag() < 0 )
        return twin_mate_calc();
      if( (size_t)r.GetTag() > Fc.Count() )
        throw TIndexOutOfRangeException(__OlxSourceInfo, r.GetTag(), 0, Fc.Count());
      const size_t bi = olx_abs(r.GetBatch())-1;
      return twin_mate_calc(Fc[r.GetTag()], bi < scales.Count() ? scales[bi] : 0);
    }
  };

  template <typename twin_iterator> struct obs_twin_mate_generator {
  };

  template <typename twin_calc_generator_t>
  double calc_f_sq(const twin_calc_generator_t& tw)  {
    double res = tw.NextCalc().f_sq_calc();
    while( tw.HasNext() )
      res += tw.NextCalc().f_sq_calc();
    return res;
  }
  
  struct merohedral  {
    struct iterator  {
      const merohedral& parent;
      const size_t src_index;
      mutable int current;
      mutable vec3i index;
      iterator(const merohedral& _parent, size_t _src_index)
        : parent(_parent),
          src_index(_src_index),
          index(parent.all_refs[src_index].GetHkl()),
          current(0) {}
      bool HasNext() const {  return current < olx_abs(parent.n);  }
      TReflection Next() const {
        int i = current++;
        if( parent.n < 0 && i == olx_abs(parent.n)/2 )
          index = -index;
        TReflection rv = (i == 0 ? TReflection(parent.all_refs[src_index], index, 1)
          : TReflection(parent.all_refs[src_index], (index = parent.matrix*index), -(i+1)));
        index_t tag = -1;
        vec3i ni = (i == 0 ? index : TReflection::Standardise(index, parent.sym_info));
        if( parent.hkl_to_ref_map.IsInRange(ni) )
          tag = parent.hkl_to_ref_map(ni);
        rv.SetTag(tag);
        return rv;
      }
    };
    merohedral(const SymSpace::InfoEx& _sym_info, const TRefList& _all_refs,
      RefinementModel::HklStat& _ms, const TDoubleList& _scales, const mat3i& tm, int _n)
      : sym_info(_sym_info), all_refs(_all_refs), ms(_ms),
        scales(_scales),
        hkl_to_ref_map(_ms.MinIndexes, _ms.MaxIndexes),
        matrix(tm), n(_n)
    {
      hkl_to_ref_map.FastInitWith(-1);
      for( size_t i=0; i < all_refs.Count(); i++ )  {
        hkl_to_ref_map(all_refs[i].GetHkl()) = i;
        all_refs[i].SetTag(i);
      }
    }
    template <typename detwinner_t>
    void detwin(const detwinner_t& dt, TRefList& out, const TArrayList<compd>& Fc)  {
      out = all_refs;
      for( size_t i=0; i < out.Count(); i++ )  {
        TReflection& r = out[i];
        iterator itr(*this, i);
        detwin_result res = dt.detwin(twin_mate_generator<iterator>(itr, scales, Fc));
        r.SetI(res.f_sq);
        r.SetS(res.sig);
        r.SetBatch(TReflection::NoBatchSet);
      }

    }
    void calc_fsq(const TArrayList<compd>& Fc, evecd& Fsq)  {
      Fsq.Resize(all_refs.Count());
      for( size_t i=0; i < all_refs.Count(); i++ )  {
        Fsq[i] = calc_f_sq(
          twin_mate_generator<iterator>(iterator(*this, i), scales, Fc));
      }
    }
    const SymSpace::InfoEx& sym_info;
    const TRefList& all_refs;
    RefinementModel::HklStat ms;
    const TDoubleList& scales;
    TArray3D<size_t> hkl_to_ref_map;
    mat3i matrix;
    int n;
  };
  /**/
  struct general  {
    struct iterator  {
      const general& parent;
      mutable size_t current;
      const size_t off;
      iterator(const general& _parent, size_t start) : parent(_parent), off(start), current(0)  {}
      bool HasNext() const {
        return ( current == 0 ||
          ((off-current) > 0 && parent.all_refs[off-current].GetBatch() < 0));
      }
      const TReflection& Next() const {
        return parent.all_refs[off-current++];
      }
    };
    general(const SymSpace::InfoEx& _sym_info, const TRefList& _all_refs,
      const RefUtil::ResolutionAndSigmaFilter& filter, const TDoubleList& _scales)
      : sym_info(_sym_info), all_refs(_all_refs),
        scales(_scales),
        F_indices(NULL)
    {
      vec3i mi(100,100,100), mx = -mi;
      for( size_t i=0; i < all_refs.Count(); i++ )
        vec3i::UpdateMinMax(all_refs[i].GetHkl(), mi, mx);
      vec3i::UpdateMinMax(TReflection::Standardise(mi, sym_info), mi, mx);
      vec3i::UpdateMinMax(TReflection::Standardise(mx, sym_info), mi, mx);
      TArray3D<size_t>& hkl3d = *(F_indices = new TArray3D<size_t>(mi, mx));
      F_indices->FastInitWith(-1);
      reflections.Clear().SetCapacity(all_refs.Count());
      for( size_t i=all_refs.Count()-1; i != InvalidIndex; i-- )  {
        if( all_refs[i].IsOmitted() )  {
          ms.OmittedByUser++;
          continue;
        }
        if( filter.IsOutside(all_refs[i]) )  {
          all_refs[i].SetTag(-1);
          continue;
        }
        vec3i hkl = TReflection::Standardise(all_refs[i].GetHkl(), sym_info);
        if( TReflection::IsAbsent(hkl, sym_info) || filter.IsOmitted(hkl) )  {
          if( all_refs[i].GetBatch() > 0 )  {
            size_t j=i;
            bool all_absent = true;
            while( --j != InvalidIndex && all_refs[j].GetBatch() < 0 )  {
              if( !TReflection::IsAbsent(all_refs[j].GetHkl(), sym_info) &&
                !filter.IsOmitted(TReflection::Standardise(all_refs[j].GetHkl(), sym_info)))
              {
                all_absent = false;
                break;
              }
            }
            if( all_absent )  {
              all_refs[i].SetTag(-1);
              ms.SystematicAbsentcesRemoved++;
              i = j+1;
              continue;
            }
          }
        }
        if( hkl3d(hkl) == InvalidIndex )  {
          all_refs[i].SetTag(hkl3d(hkl) = unique_indices.Count());
          unique_indices.AddCCopy(hkl);
        }
        else
          all_refs[i].SetTag(hkl3d(hkl));
        if( all_refs[i].GetBatch() >= 0 )
          reflections.AddCCopy(all_refs[i]).SetTag(i);
      }
      reflections.ForEach(RefUtil::ResolutionAndSigmaFilter::IntensityModifier(filter));
    }
    ~general()  {
      if( F_indices != NULL )
        delete F_indices;
    }
    void calc_fsq(const TArrayList<compd>& Fc, evecd& Fsq)  {
      Fsq.Resize(reflections.Count());
      for( size_t i=0; i < reflections.Count(); i++ )  {
        iterator itr(*this, reflections[i].GetTag());
        Fsq[i] = calc_f_sq(twin_mate_generator<iterator>(itr, scales, Fc));
      }
    }

    template <typename detwinner_t>
    void detwin(const detwinner_t& dt, TRefList& out, const TArrayList<compd>& Fc)  {
      out = reflections;
      for( size_t i=0; i < out.Count(); i++ )  {
        TReflection& r = out[i];
        twinning::general::iterator itr(*this, r.GetTag());
        detwin_result res = dt.detwin(twin_mate_generator<iterator>(itr, scales, Fc));
        r.SetI(res.f_sq);
        r.SetS(res.sig);
        r.SetBatch(TReflection::NoBatchSet);
      }
    }
    template <typename detwinner_t, typename merger_t>
    void detwin_and_merge(const detwinner_t& dt, const merger_t& merger, TRefList& out,
      const TArrayList<compd>& Fc, TArrayList<compd>* pF)
    {
      detwin(dt, out, Fc);
      TRefPList to_merge(out, DirectAccessor());
      out.ReleaseAll();
      SymSpace::InfoEx si = sym_info;
      si.centrosymmetric = true;
      RefMerger::Merge<merger_t>(sym_info, to_merge, out, vec3i_list());
      to_merge.DeleteItems(false);
      if( pF != NULL )  {
        TArrayList<compd>& F = *pF;
        TArray3D<size_t>& hkl3d = *F_indices;
        F.SetCount(out.Count());
        for( size_t i=0; i < out.Count(); i++ )  {
          size_t f_i;
          if( !hkl3d.IsInRange(out[i].GetHkl()) ||
            (f_i = hkl3d(out[i].GetHkl())) == InvalidIndex )
            throw TFunctionFailedException(__OlxSourceInfo, "merging does not match");
          F[i] = Fc[f_i];
        }
      }
    }
    const SymSpace::InfoEx& sym_info;
    const TRefList& all_refs;
    TRefList reflections;
    vec3i_list unique_indices;
    RefinementModel::HklStat ms;
    const TDoubleList& scales;
    TArray3D<size_t>* F_indices;
  };
}; //end of the twinning namespace
EndXlibNamespace()
#endif
