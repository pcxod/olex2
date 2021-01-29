/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_math_plane_H
#define __olx_math_plane_H
#include "talist.h"

BeginEsdlNamespace()

namespace plane {
  // sorts points in plane given by center and normal
  template <class array_t, class accessor_t, class vec_t>
  void Sort(array_t &points, const accessor_t &accessor,
    const vec_t &center, const vec_t &normal)
  {
    if( points.IsEmpty() )  return;
    TArrayList<double> pvs(points.Count());
    double max_qd = 0;
    size_t m_i = 0;
    for (size_t i=0; i < points.Count(); i++) {
      double qd = (accessor(points[i])-center).QLength();
      if (qd > max_qd) {
        m_i = i;
        max_qd = qd;
      }
    }
    const vec_t origin = accessor(points[m_i])-center;
    pvs[m_i] = 0; // origin value
    for( size_t i=0; i < points.Count(); i++ )  {
      if (i == m_i) {
        continue;
      }
      vec_t vec = accessor(points[i]) - center;
      if (vec.QLength() < 1e-6) { // atom on the center!
        continue;
      }
      double ca = origin.CAngle(vec);
      vec = origin.XProdVec(vec);
      double vo;
      if (vec.IsNull(1e-6)) { // same direction as the origin
        vo = 0;
      }
      else {
        vo = (ca == -1 ? 0 : vec.CAngle(normal));
      }
      /* negative - vec is on the right, positive - on the left, if ca == 0,
      vec == (0,0,0)
      */
      if( ca >= 0 )  { // -90 to 90
        if (vo < 0) { // -90 to 0 3->4
          pvs[i] = 3.0 + ca;
        }
        else { // 0 to 90 0->1
          pvs[i] = 1.0 - ca;
        }
      }
      else if( ca > -1 ) {  // 90-270
        if (vo < 0) { // 180 to 270 2->3
          pvs[i] = 3.0 + ca;
        }
        else { // 90 to 180 1->2
          pvs[i] = 1.0 - ca;
        }
      }
      else  {  //-1, special case
        pvs[i] = 2;
      }
    }
    QuickSorter::Sort(pvs, TPrimitiveComparator(),
      SyncSortListener::MakeSingle(points));
  }
  enum {
    best = 0,
    worst = 2
  };

  template <typename FloatT = double>
  struct mean {

    struct unit_weight {
      template <typename item_t>
      FloatT operator() (const item_t&) const { return 1; }
    };

    struct out {
      TVector3<FloatT> center;
      // this represents RMSD only in the case of unit weights
      TVector3<FloatT> rms;
      TMatrix33<FloatT> normals;
      void sort() {
        bool swaps = true;
        while (swaps) {
          swaps = false;
          for (size_t i = 0; i < 2; i++) {
            if (rms[i] > rms[i + 1]) {
              olx_swap(normals[i], normals[i + 1]);
              olx_swap(rms[i], rms[i + 1]);
              swaps = true;
            }
          }
        }
      }
    };

    template <class list_t, class p_accessor_t, class w_accessor_t>
    static out calc(const list_t& list,
      const p_accessor_t& p_accessor,
      const w_accessor_t& w_accessor)
    {
      out rv;
      if (list.Count() < 2) {
        return rv;
      }
      FloatT w_sum = 0, w_sq_sum = 0;
      for (size_t i = 0; i < list.Count(); i++) {
        FloatT w = w_accessor(list[i]);
        rv.center += p_accessor(list[i]) * w;
        w_sum += w;
        w_sq_sum += w*w;
      }
      rv.center /= w_sum;
      TMatrix33<FloatT> m;
      for (size_t i = 0; i < list.Count(); i++) {
        TVector3<FloatT> t =
          (p_accessor(list[i]) - rv.center) * w_accessor(list[i]);
        m[0][0] += (t[0] * t[0]);
        m[0][1] += (t[0] * t[1]);
        m[0][2] += (t[0] * t[2]);
        m[1][1] += (t[1] * t[1]);
        m[1][2] += (t[1] * t[2]);
        m[2][2] += (t[2] * t[2]);
      }
      m[1][0] = m[0][1];
      m[2][0] = m[0][2];
      m[2][1] = m[1][2];
      TMatrix33<FloatT>::EigenValues(m /= w_sq_sum, rv.normals.I());
      for (int i = 0; i < 3; i++) {
        rv.rms[i] = m[i][i] < 0 ? 0 : sqrt(m[i][i]);
      }
      rv.sort();
      return rv;
    }
    // calculation with unit weights
    template <class list_t, class accessor_t>
    static out calc(const list_t& list, const accessor_t& accessor) {
      return calc(list, accessor, unit_weight());
    }
    // calculation with unit weights
    template <class list_t>
    static out calc(const list_t& list) {
      return calc(list, DummyAccessor(), unit_weight());
    }
    // calculate for list of olx_pair_t
    template <class list_t>
    static out calc_for_pairs(const list_t& pairs) {
      return calc(pairs,
        FunctionAccessor::MakeConst(&olx_pair_t<TVector3<FloatT>, FloatT>::GetA),
        FunctionAccessor::MakeConst(&olx_pair_t<TVector3<FloatT>, FloatT>::GetB)
      );
    }
  };

  template <typename FloatT=double>
  struct center {
    template <class list_t, class p_accessor_t, class w_accessor_t>
    static TVector3<FloatT> calc(const list_t& list,
      const p_accessor_t& p_accessor,
      const w_accessor_t& w_accessor)
    {
      TVector3<FloatT> c;
      if (list.Count() == 0) {
        return c;
      }
      FloatT w_sum = 0;
      for (size_t i = 0; i < list.Count(); i++) {
        FloatT w = w_accessor(list[i]);
        c += p_accessor(list[i]) * w;
        w_sum += w;
      }
      c /= w_sum;
      return c;
    }
    /* calculates a center of the object reflected in a plane defined by the
    given normal
    */
    template <class list_t, class p_accessor_t, class w_accessor_t>
    static TVector3<FloatT> calc_reflected(const list_t& list,
      const TVector3<FloatT> &center,
      const TVector3<FloatT> &normal,
      const p_accessor_t& p_accessor,
      const w_accessor_t& w_accessor)
    {
      TVector3<FloatT> c;
      if (list.Count() == 0) {
        return c;
      }
      FloatT w_sum = 0;
      for (size_t i = 0; i < list.Count(); i++) {
        FloatT w = w_accessor(list[i]);
        c += ((p_accessor(list[i])-center).Reflect(normal) + center) * w;
        w_sum += w;
      }
      c /= w_sum;
      return c;
    }

  };
};  //end namespace plane

EndEsdlNamespace()
#endif

