/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_align_H
#define __olx_sdl_align_H
#include "../threex3.h"
#include "../ematrix.h"
BeginEsdlNamespace()

namespace align {

  struct out {
    // a 4x4 matrix of the quaternions
    ematd quaternions;
    /* root mean square distances, note that if non-unit weights are used, these values
    will differe from the RMSD calculated directly:
    (sum(distance^2*w^2)/sum(w^2)^0.5 vs. (sum(distance^2)/n)^0.5
    */
    evecd rmsd;
    vec3d center_a, center_b;
    out() : quaternions(4, 4), rmsd(4) {}
  };
  struct point {
    vec3d value;
    double weight;
    point() : weight(1.0) {}
    point(const vec3d& p, double w = 1.0) : value(p), weight(w) {}
    point(const point& ap) : value(ap.value), weight(ap.weight) {}
    point& operator = (const point& ap) {
      value = ap.value;
      weight = ap.weight;
      return *this;
    }
  };
  struct pair {
    point a, b;
    pair() {}
    pair(const pair& p) : a(p.a), b(p.b) {}
    pair(const point& _a, const point& _b) : a(_a), b(_b) {}
    pair& operator = (const pair& p) {
      a = p.a;
      b = p.b;
      return *this;
    }
    vec3d GetValueA() const { return a.value; }
    double GetWeightA() const { return a.weight; }
    vec3d GetValueB() const { return b.value; }
    double GetWeightB() const { return b.weight; }
  };
  /* finds allignment quaternions for given coordinates and their weights
  the rmsds (and the quaternions) are sorted ascending
  Acta A45 (1989), 208.
  The resulting matrices map {b} set to {a} */
  template <class List> out FindAlignmentQuaternions(const List& pairs,
    bool update_centers=true)
  {
    out ao;
    double swa = 0, swb = 0, sws = 0;
    for (size_t i = 0; i < pairs.Count(); i++) {
      if (update_centers) {
        ao.center_a += pairs[i].GetValueA()*pairs[i].GetWeightA();
        ao.center_b += pairs[i].GetValueB()*pairs[i].GetWeightB();
        swa += pairs[i].GetWeightA();
        swb += pairs[i].GetWeightB();
      }
      sws += pairs[i].GetWeightA()*pairs[i].GetWeightB();
    }
    if (update_centers) {
      ao.center_a /= swa;
      ao.center_b /= swb;
    }
    ematd evm(4, 4);
    for (size_t i = 0; i < pairs.Count(); i++) {
      const vec3d v1 = (pairs[i].GetValueA() - ao.center_a)*pairs[i].GetWeightA();
      const vec3d v2 = (pairs[i].GetValueB() - ao.center_b)*pairs[i].GetWeightB();
      const vec3d p = v1 + v2;
      const vec3d m = v1 - v2;
      evm[0][0] += (m[0] * m[0] + m[1] * m[1] + m[2] * m[2]);
      evm[0][1] += (p[1] * m[2] - m[1] * p[2]);
      evm[0][2] += (m[0] * p[2] - p[0] * m[2]);
      evm[0][3] += (p[0] * m[1] - m[0] * p[1]);
      evm[1][1] += (p[1] * p[1] + p[2] * p[2] + m[0] * m[0]);
      evm[1][2] += (m[0] * m[1] - p[0] * p[1]);
      evm[1][3] += (m[0] * m[2] - p[0] * p[2]);
      evm[2][2] += (p[0] * p[0] + p[2] * p[2] + m[1] * m[1]);
      evm[2][3] += (m[1] * m[2] - p[1] * p[2]);
      evm[3][3] += (p[0] * p[0] + p[1] * p[1] + m[2] * m[2]);
    }
    evm[1][0] = evm[0][1];
    evm[2][0] = evm[0][2];
    evm[2][1] = evm[1][2];
    evm[3][0] = evm[0][3];
    evm[3][1] = evm[1][3];
    evm[3][2] = evm[2][3];
    ematd::EigenValues(evm /= sws, ao.quaternions.I());
    for (int i = 0; i < 4; i++) {
      ao.rmsd[i] = (evm[i][i] <= 0 ? 0 : sqrt(evm[i][i]));
    }
    bool changes = true;
    while (changes) {
      changes = false;
      for (int i = 0; i < 3; i++) {
        if (ao.rmsd[i + 1] < ao.rmsd[i]) {
          ao.quaternions.SwapRows(i, i + 1);
          olx_swap(ao.rmsd[i], ao.rmsd[i + 1]);
          changes = true;
        }
      }
    }
    return ao;
  }
  // returns unweighted RMSD
  template <class List> double CalcRMSD(const List& pairs,
    const align::out& ao)
  {
    double rmsd = 0;
    mat3d m;
    QuaternionToMatrix(ao.quaternions[0], m);
    for (size_t i = 0; i < pairs.Count(); i++) {
      rmsd += (pairs[i].GetValueA() - ao.center_a)
        .QDistanceTo((pairs[i].GetValueB() - ao.center_b)*m);
    }
    return sqrt(rmsd / pairs.Count());
  }
  // two lists to 'pair' adaptor
  template <class ValueList>
  struct ListsToPairAdaptorV {
    const ValueList& vlist;
    const size_t count;
    struct pair {
      const ListsToPairAdaptorV& parent;
      size_t index;
      pair(const ListsToPairAdaptorV& _parent, size_t i)
        : parent(_parent), index(i) {}
      vec3d GetValueA() const { return parent.vlist[index]; }
      double GetWeightA() const { return 1; }
      vec3d GetValueB() const { return parent.vlist[parent.count + index]; }
      double GetWeightB() const { return 1; }
    };
    ListsToPairAdaptorV(const ValueList& _vlist) :
      vlist(_vlist), count(vlist.Count() / 2)
    {
      if ((vlist.Count() % 2) != 0) {
        throw TInvalidArgumentException(__OlxSourceInfo, "list size");
      }
    }
    pair operator [] (size_t i) const {
      return pair(*this, i);
    }
    size_t Count() const { return count; }
  };
  template <class ValueList, class WeightList>
  struct ListsToPairAdaptorVW {
    const ValueList& vlist;
    const WeightList& wlist;
    const size_t count;
    struct pair {
      const ListsToPairAdaptorVW& parent;
      size_t index;
      pair(const ListsToPairAdaptorVW& _parent, size_t i)
        : parent(_parent), index(i) {}
      vec3d GetValueA() const { return parent.vlist[index]; }
      double GetWeightA() const { return parent.wlist[index]; }
      vec3d GetValueB() const { return parent.vlist[parent.count + index]; }
      double GetWeightB() const { return parent.wlist[parent.count + index]; }
    };
    ListsToPairAdaptorVW(const ValueList& _vlist, const WeightList& _wlist) :
      vlist(_vlist), wlist(_wlist), count(vlist.Count() / 2)
    {
      if (wlist.Count() < vlist.Count()) {
        throw TInvalidArgumentException(__OlxSourceInfo, "weights list");
      }
      if ((vlist.Count() % 2) != 0) {
        throw TInvalidArgumentException(__OlxSourceInfo, "list size");
      }
    }
    pair operator [] (size_t i) const {
      return pair(*this, i);
    }
    size_t Count() const { return count; }
  };
  struct ListToPairAdaptor {
    template <class ValueList, class WeightList> static
      ListsToPairAdaptorVW<ValueList, WeightList> Make(
        const ValueList& _vlist, const WeightList& _wlist)
    {
      return ListsToPairAdaptorVW<ValueList, WeightList>(_vlist, _wlist);
    }
    template <class ValueList> static
      ListsToPairAdaptorV<ValueList> Make(const ValueList& _vlist) {
      return ListsToPairAdaptorV<ValueList>(_vlist);
    }
  };

};  // end namespace align

template <typename FloatT=double>
struct inertia {

  struct unit_weight {
    template <typename item_t>
    FloatT operator() (const item_t &) const { return 1; }
  };

  struct out {
    TVector3<FloatT> moments, center;
    TMatrix33<FloatT> axis;
    void sort() {
      bool swaps = true;
      while (swaps) {
        swaps = false;
        for (int i = 0; i < 2; i++) {
          if (moments[i] > moments[i + 1]) {
            olx_swap(axis[i], axis[i + 1]);
            olx_swap(moments[i], moments[i + 1]);
            swaps = true;
          }
        }
      }
      // keep the orientation system
      axis[2] = axis[0].XProdVec(axis[1]).Normalise();
    }
  };

  template <class list_t, class p_accessor_t, class w_accessor_t>
  static out calc(const list_t &list,
    const p_accessor_t &p_accessor,
    const w_accessor_t &w_accessor)
  {
    out rv;
    if (list.Count() < 2)
      return rv;
    FloatT w_sum = 0;
    for (size_t i = 0; i < list.Count(); i++) {
      FloatT w = w_accessor(list[i]);
      rv.center += p_accessor(list[i])*w;
      w_sum += w;
    }
    rv.center /= w_sum;
    TMatrix33<FloatT> I;
    for (size_t i = 0; i < list.Count(); i++) {
      TVector3<FloatT> v =
        (p_accessor(list[i]) - rv.center)*w_accessor(list[i]);
      I[0][0] += (olx_sqr(v[1]) + olx_sqr(v[2]));
      I[1][1] += (olx_sqr(v[0]) + olx_sqr(v[2]));
      I[2][2] += (olx_sqr(v[0]) + olx_sqr(v[1]));
      I[0][1] -= v[0] * v[1];
      I[0][2] -= v[0] * v[2];
      I[1][2] -= v[1] * v[2];
    }
    I[1][0] = I[0][1];
    I[2][0] = I[0][2];
    I[2][1] = I[1][2];
    TMatrix33<FloatT>::EigenValues(I, rv.axis.I());
    for (int i = 0; i < 3; i++) {
      rv.moments[i] = I[i][i] < 0 ? 0 : sqrt(I[i][i]);
    }
    return rv;
  }
  // calcualtion with unit weights
  template <class list_t, class accessor_t>
  static out calc(const list_t &list, const accessor_t &accessor) {
    return calc(list, accessor, unit_weight());
  }
};  // end struct inertia

EndEsdlNamespace()
#endif
