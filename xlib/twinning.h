/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xlib_twinning_H
#define __olx_xlib_twinning_H
#include "reflection.h"
#include "refmodel.h"
#include "refutil.h"
#include "arrays.h"

BeginXlibNamespace()
namespace twinning {

  struct twin_mate_calc {
    compd fc;
    double scale;
    twin_mate_calc(const compd& _fc, double _scale)
      : fc(_fc), scale(_scale)
    {}
    twin_mate_calc()
      : scale(0)
    {}
    double f_sq_calc() const { return fc.qmod() * scale; }
  };
  struct twin_mate_obs {
    double f_obs_sq, sig_obs, scale;
    twin_mate_obs(double _f_obs_sq, double _sig_obs, double _scale)
      : f_obs_sq(_f_obs_sq), sig_obs(_sig_obs), scale(_scale)
    {}
    twin_mate_obs()
      : f_obs_sq(0), sig_obs(0), scale(0)
    {}
  };
  struct detwin_result {
    double f_sq, sig;
    detwin_result() : f_sq(0), sig(0)
    {}
    detwin_result(double _f_sq, double _sig)
      : f_sq(_f_sq), sig(_sig)
    {}
  };
  struct twin_mate_full : public twin_mate_calc, public detwin_result {
    twin_mate_full(const compd& _fc, double _f_sq, double _sig, double _scale)
      : twin_mate_calc(_fc, _scale), detwin_result(_f_sq, _sig)
    {}
    twin_mate_full()
    {}
  };

  class handler {
  public:
    struct iterator {
      const handler& obs;
      size_t h_idx;
      mutable size_t current;
      mutable double scale;
      iterator(const handler& parent, size_t h_idx)
        : obs(parent), h_idx(h_idx),
        current(InvalidIndex), scale(0)
      {}
      bool has_next() const {
        return current == InvalidIndex || obs.components[h_idx].Count() > current;
      }
      const vec3i& next_index() const {
        if (++current == 0) {
          scale = obs.scales[olx_abs(obs.measured[h_idx].GetBatch()) - 1].value;
          return obs.measured[h_idx].GetHkl();
        }
        else {
          scale = obs.components[h_idx][current - 1].scale.value;
          return obs.components[h_idx][current - 1].index;
        }
      }
      const TReflection& next_obs() const {
        vec3i hkl = next_index();
        return obs.measured[obs.find_obs(hkl)];
      }
    };
    struct twin_mate_generator {
      const iterator& itr;
      const TArrayList<compd>& Fc;
      twin_mate_generator(const iterator& _itr, const TArrayList<compd>& _Fc)
        : itr(_itr), Fc(_Fc)
      {}
      bool has_next() const { return itr.has_next(); }
      olx_object_ptr<twin_mate_full> next_full() const {
        vec3i hkl = itr.next_index();
        const map_et& r = itr.obs.hkl_to_ref_map->Value(hkl);
        return new twin_mate_full(
          Fc[r.a],
          itr.obs.measured[itr.h_idx].GetI(),
          itr.obs.measured[itr.h_idx].GetS(),
          itr.scale);
      }
      twin_mate_calc next_calc() const {
        vec3i hkl = itr.next_index();
        return twin_mate_calc(Fc[itr.obs.find_calc(hkl)], itr.scale);
      }
    };
    // constructors
    handler(const SymmSpace::InfoEx& _sym_info, const TRefList& refs,
      const TDoubleList& _scales,
      const mat3d& tm, int n);

    handler(const SymmSpace::InfoEx& _sym_info, const TRefList& refs,
      const RefUtil::ResolutionAndSigmaFilter& filter,
      const TDoubleList& _scales);

    iterator iterate(size_t i) const {
      return iterator(*this, i);
    }

    size_t find_calc(const vec3i& h) const {
      return hkl_to_ref_map->Value(h).a;
    }

    size_t find_obs(const vec3i& h) const {
      return hkl_to_ref_map->Value(h).b;
    }

    template <typename detwinner_t>
    void detwin(const detwinner_t& dt, TRefList& out,
      const TArrayList<compd>& Fc)
    {
      out = measured;
      for (size_t i = 0; i < out.Count(); i++) {
        TReflection& r = out[i];
        detwin_result res = dt.detwin(twin_mate_generator(iterate(i), Fc));
        r.SetI(res.f_sq);
        r.SetS(res.sig);
        r.SetBatch(TReflection::NoBatchSet);
      }
    }

    template <typename detwinner_t, typename merger_t>
    void detwin_and_merge(const detwinner_t& dt, const merger_t& merger,
      TRefList& out, const TArrayList<compd>& Fc, TArrayList<compd>* pF)
    {
      detwin(dt, out, Fc);
      TRefPList to_merge(out);
      out.ReleaseAll();
      SymmSpace::InfoEx si = sym_info;
      si.centrosymmetric = true;
      RefMerger::Merge<merger_t>(sym_info, to_merge, out, vec3i_list());
      to_merge.DeleteItems(false);
      if (pF != 0) {
        TArrayList<compd>& F = *pF;
        olx_array::TArray3D<map_et>& map = *hkl_to_ref_map;
        F.SetCount(out.Count());
        for (size_t i = 0; i < out.Count(); i++) {
          size_t f_i;
          if (!map.IsInRange(out[i].GetHkl()) ||
            (f_i = map(out[i].GetHkl()).a) == InvalidIndex)
          {
            throw TFunctionFailedException(__OlxSourceInfo,
              "merging does not match");
          }
          F[i] = Fc[f_i];
        }
      }
    }
    void calc_fsq(const TArrayList<compd>& Fc, evecd& Fsq) {
      Fsq.Resize(measured.Count());
      for (size_t i = 0; i < measured.Count(); i++) {
        Fsq[i] = calc_f_sq_1(twin_mate_generator(iterate(i), Fc));
      }
    }

  protected:
    double calc_f_sq_1(const twin_mate_generator& itr) {
      double res = itr.next_calc().f_sq_calc();
      while (itr.has_next()) {
        res += itr.next_calc().f_sq_calc();
      }
      return res;
    }
    struct BASF_p {
      double value;
      BASF_p(double v)
        : value(v)
      {}
    };
    struct twin_component {
      vec3i index;
      const BASF_p& scale;
      twin_component(const vec3i& index, const BASF_p& scale)
        : index(index), scale(scale)
      {}
    };
  private:
    void calc_range();
    const SymmSpace::InfoEx& sym_info;
    TTypeList<TTypeList<twin_component> > components;
    TTypeList< BASF_p> scales;
    typedef olx_pair_t<size_t, size_t> map_et;
    olx_object_ptr<olx_array::TArray3D<map_et> > hkl_to_ref_map;
  public:
    TRefList measured;
    vec3i_list unique_indices;
    RefinementModel::HklStat ms;
  };


  // uses only scales and Fobs to deconvolute the intensities into components
  struct detwinner_algebraic {
    ematd _m;
    detwinner_algebraic(const TDoubleList& scales);
    void detwin(const handler::iterator& itr, TTypeList<TReflection>& res) const;
  };

  // uses only Fc
  struct detwinner_shelx {
    detwin_result detwin(const handler::twin_mate_generator& itr) const;
  };

  // uses both Fc and F_obs
  struct detwinner_mixed {
    detwin_result detwin(const handler::twin_mate_generator& itr) const;
  };
}; //end of the twinning namespace
EndXlibNamespace()
#endif
