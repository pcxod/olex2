/******************************************************************************
* Copyright (c) 2004-2024 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/
#pragma once
#include "threex3.h"
#include "estrlist.h"

namespace hkl_util {
  struct Ref {
    vec3i hkl;
    double I, S;
    Ref(int h, int k, int l, double I, double S)
      : hkl(h, k, l),
      I(I), S(S)
    {}

    Ref(double I)
      : I(I), S(0)
    {}

    const vec3i& GetHKL() const { return hkl; }
    double GetI() const { return I; }
    double GetS() const { return S; }
  };
  
  olx_object_ptr<Ref> str2ref(const olxcstr& s);

  TTypeList<Ref>::const_list_type lines2refs(const TCStrList& lines,
    size_t l_len);
  
  typedef TTypeList<olx_pair_t<double, size_t> > fingerprint_t;
  
  template <class ref_list_t>
  typename fingerprint_t::const_list_type calc_fingerprint(const ref_list_t& refs,
    size_t fp_len = 10)
  {
    TTypeList<olx_pair_t<double, size_t> > rv(olx_reserve(fp_len));

    double minI = 1e3, maxI = -1e-3;
    for (size_t i = 0; i < refs.Count(); i++) {
      olx_update_min_max(refs[i].GetI(), minI, maxI);
    }
    // max(I/N) = N (-1)
    const int max_v = (int)(fp_len * fp_len) - 1;
    double range = maxI - minI,
      scale = max_v / range;
    for (size_t i = 0; i < fp_len; i++) {
      rv.AddNew(0, 0);
    }
    for (size_t i = 0; i < refs.Count(); i++) {
      int I = (int)((refs[i].GetI() - minI) * scale);
      if (I < 0) {
        I = 0;
      }
      else if (I > max_v) {
        I = max_v;
      }
      int idx = I / (int)fp_len;
      rv[idx].a += I;
      rv[idx].b++;
    }
    double cnt_scale = 10000.0 / refs.Count();
    for (size_t i = 0; i < rv.Count(); i++) {
      rv[i].a /= rv[i].b;
      rv[i].b = olx_round(rv[i].b * cnt_scale);
      rv[i].a *= rv[i].b;
    }
    return rv;

  }

  
  static typename fingerprint_t::const_list_type calc_fingerprint(const TCStrList& lines,
    size_t l_len, size_t fp_len = 10)
  {
    return calc_fingerprint(lines2refs(lines, l_len), fp_len);
  }

  // computes sum(abs(f1.a/f1.b-f2.a/f2.b))/N, should be ~0 for matching fps
  double corelate(const fingerprint_t& f1, const fingerprint_t& f2);

  // use_N - if to report the bin sizes
  olxcstr fingerprint2str(const fingerprint_t &fp, bool use_N = true);
}


