/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_symmspace_H
#define __olx_xl_symmspace_H
#include "edict.h"
#include "mat_id.h"
BeginXlibNamespace()

template <class MatList> class TSymmSpace  {
  MatList ml;
  const mat3d &cart2cell, &cell2cart, &hkl2cart;
  size_t start;
  bool centrosymmetric;
protected:
  TSymmSpace(const TSymmSpace& sp, size_t _start) :
    ml(sp.ml),
    cart2cell(sp.cart2cell),
    cell2cart(sp.cell2cart),
    hkl2cart(sp.hkl2cart),
    start(sp.start+_start),
    centrosymmetric(sp.centrosymmetric)  {}
public:
  TSymmSpace(const MatList& _ml,
    const mat3d& _cart2cell,
    const mat3d& _cell2cart,
    const mat3d& _hkl2cart,
    bool _centrosymmetric) :
      ml(_ml),
      cart2cell(_cart2cell),
      cell2cart(_cell2cart),
      hkl2cart(_hkl2cart),
      start(0),
      centrosymmetric(_centrosymmetric)  {}

  TSymmSpace(const TSymmSpace& sp) :
    ml(sp.ml),
    cart2cell(sp.cart2cell),
    cell2cart(sp.cell2cart),
    hkl2cart(sp.hkl2cart),
    start(sp.start),
    centrosymmetric(sp.centrosymmetric)  {}

  const smatd& operator [] (size_t i) const {  return ml[i+start];  }
  size_t Count() const {  return ml.Count()-start;  }
  bool IsEmpty() const {  return Count() == 0;  }
  bool IsCentrosymmetric() const {  return centrosymmetric;  }
  template <typename vec_type> vec3d Orthogonalise(const vec_type& v) const {
    return vec3d(
      v[0]*cell2cart[0][0] + v[1]*cell2cart[1][0] + v[2]*cell2cart[2][0],
      v[1]*cell2cart[1][1] + v[2]*cell2cart[2][1],
      v[2]*cell2cart[2][2]);
  }
  template <typename vec_type> vec_type& OrthogonaliseI(vec_type& v) const {
    v[0] = v[0]*cell2cart[0][0] + v[1]*cell2cart[1][0] + v[2]*cell2cart[2][0];
    v[1] = v[1]*cell2cart[1][1] + v[2]*cell2cart[2][1];
    v[2] = v[2]*cell2cart[2][2];
    return v;
  }
  template <typename vec_type> vec3d Fractionalise(const vec_type& v) const {
    return vec3d(
      v[0]*cart2cell[0][0] + v[1]*cart2cell[1][0] + v[2]*cart2cell[2][0],
      v[1]*cart2cell[1][1] + v[2]*cart2cell[2][1],
      v[2]*cart2cell[2][2]);
  }
  template <typename vec_type> vec_type& FractionaliseI(vec_type& v) const {
    v[0] = v[0]*cart2cell[0][0] + v[1]*cart2cell[1][0] + v[2]*cart2cell[2][0];
    v[1] = v[1]*cart2cell[1][1] + v[2]*cart2cell[2][1];
    v[2] = v[2]*cart2cell[2][2];
    return v;
  }
  template <typename vec_type> vec3d HklToCart(const vec_type& v) const {
    return vec3d(
      v[0]*hkl2cart[0][0],
      v[0]*hkl2cart[0][1] + v[1]*hkl2cart[1][1],
      v[0]*hkl2cart[0][2] + v[1]*hkl2cart[1][2] + v[2]*hkl2cart[2][2]
    );
  }
  TSymmSpace<MatList> SubListFrom(size_t _start) const {
    return TSymmSpace<MatList>(*this, _start);
  }
};

/* the adaptor for complex classes having MatrixCount()/GetMatrix(size_t)
methods
*/
template <class MatrixContainer> class MatrixListAdaptor {
  const MatrixContainer& mc;
public:
  MatrixListAdaptor(const MatrixContainer& _mc) : mc(_mc)  {}
  MatrixListAdaptor(const MatrixListAdaptor& mla) : mc(mla.mc)  {}
  size_t Count() const {  return mc.MatrixCount();  }
  bool IsEmpty() const {  return Count() == 0;  }
  const smatd& operator [](size_t i) const {  return mc.GetMatrix(i);  }
};

namespace SymmSpace {

  extern int sort_group(const smatd& m1, const smatd& m2);

  struct Info {
  protected:
    static int sort_matrix_list(const smatd& m1, const smatd& m2) {
      return olx_cmp(rotation_id::get(m1.r), rotation_id::get(m2.r));
    }
  public:
    bool centrosymmetric;
    /* list of the compact matrices including the identity
    */
    smatd_list matrices;
    /* if not zero, all inversion of the symmetry operators also translates by
    this vector
    */
    vec3d inv_trans;
    // always positive
    short latt;
    bool operator == (const Info& info) const;
    bool operator != (const Info& info) const {
      return !this->operator==(info);
    }
    // makes sure that matrix list is sorted and translations are normalised
    void normalise(const vec3d_list& translations);
    // just sorts matrices
    void normalise() {
      QuickSorter::SortSF(matrices, &sort_matrix_list);
    }
    // expands this compact form
    smatd_list::const_list_type expand() const;
    // normalises the translation - puts in the [0,1) range
    static vec3d normalise_t(const vec3d& t);
  };

  struct InfoEx {  // has a list of translation vectors vs latt number
    bool centrosymmetric;
    smatd_list matrices;
    vec3d_list vertices;
  };

  template <class SP> static Info GetInfo(const SP& sp,
    vec3d_list* translations_ = NULL)
  {
    if (sp.Count() == 0) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        "at least one matrix is expected");
    }
    vec3d_list translations;
    Info rv;
    rv.latt = -1;
    rv.centrosymmetric = false;
    if (sp.Count() == 1) {
      if (!sp[0].r.IsI() || !sp[0].t.IsNull()) {
        throw TInvalidArgumentException(__OlxSourceInfo,
          "identity matrix expected");
      }
      rv.latt = 1; //'P';
      rv.matrices.AddCopy(sp[0]);
      return rv;
    }
    olx_pdict<int, smatd_list> groups;
    bool identity_found = false;
    for (size_t i = 0; i < sp.Count(); i++) {
      smatd_list& l = groups.Add(rotation_id::get(sp[i].r));
      l.AddCopy(sp[i]);
      if (!identity_found && sp[i].IsI()) {
        identity_found = true;
      }
    }
    if (!identity_found) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        "missing identity matrix");
    }
    const size_t min_a_group = groups.GetValue(0).Count();
    for (size_t i = 0; i < groups.Count(); i++) {
      if (groups.GetValue(i).Count() != min_a_group)
        throw TInvalidArgumentException(__OlxSourceInfo, "matrix list");
    }
    if (min_a_group == 1)
      rv.latt = 1; //'P';
    else if (min_a_group == 2) {
      vec3d t = groups.GetValue(0)[1].t - groups.GetValue(0)[0].t;
      t -= t.Floor<int>();
      if (olx_abs(t[0] - 0.5) < 1e-6) {
        if (olx_abs(t[1] - 0.5) < 1e-6) {
          if (olx_abs(t[2] - 0.5) < 1e-6) {
            rv.latt = 2; //'I';
          }
          else if (olx_abs(t[2]) < 1e-6) {
            rv.latt = 7; //'C';
          }
        }
        else if (olx_abs(t[1]) < 1e-6 && olx_abs(t[2] - 0.5) < 1e-6) {
          rv.latt = 6; //'B';
        }
      }
      else if (olx_abs(t[0]) < 1e-6 && olx_abs(t[1] - 0.5) < 1e-6 &&
        olx_abs(t[2] - 0.5) < 1e-6)
      {
        rv.latt = 5; //'A';
      }
      translations.AddCopy(t);
    }
    else if (min_a_group == 3) {
      vec3d t1 = groups.GetValue(0)[1].t - groups.GetValue(0)[0].t;
      vec3d t2 = groups.GetValue(0)[2].t - groups.GetValue(0)[0].t;
      vec3d t1r(2. / 3, 1. / 3, 1. / 3), t2r(1. / 3, 2. / 3, 2. / 3);
      vec3d t1s(1. / 3, 1. / 3, 2. / 3), t2s(2. / 3, 2. / 3, 1. / 3);
      vec3d t1t(1. / 3, 2. / 3, 1. / 3), t2t(2. / 3, 1. / 3, 2. / 3);
      t1 -= t1.Floor<int>();
      t2 -= t2.Floor<int>();
      if ((t1.Equals(t1r, 1e-3) && t2.Equals(t2r, 1e-3)) ||
        (t1.Equals(t2r, 1e-3) && t2.Equals(t1r, 1e-3)))
      {
        rv.latt = 3; //'R';
      }
      else if ((t1.Equals(t1s, 1e-3) && t2.Equals(t2s, 1e-3)) ||
        (t1.Equals(t2s, 1e-3) && t2.Equals(t1s, 1e-3)))
      {
        rv.latt = 8; //'S';
      }
      else if ((t1.Equals(t1t, 1e-3) && t2.Equals(t2t, 1e-3)) ||
        (t1.Equals(t2t, 1e-3) && t2.Equals(t1t, 1e-3)))
      {
        rv.latt = 9; //'T';
      }
      translations.AddCopy(t1);
      translations.AddCopy(t2);
    }
    else if (min_a_group) {
      translations.AddNew(0.0, 0.5, 0.5);
      translations.AddNew(0.5, 0.0, 0.5);
      translations.AddNew(0.5, 0.5, 0.0);
      rv.latt = 4; //'F';
    }
    if (rv.latt < 0) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "could not deduce lattice centering");
    }
    for (size_t i = 0; i < groups.Count(); i++) {
      for (size_t j = 0; j < groups.GetValue(i).Count(); j++) {
        groups.GetValue(i)[j].t =
          Info::normalise_t(groups.GetValue(i)[j].t);
      }
      QuickSorter::SortSF(groups.GetValue(i), &sort_group);
      groups.GetValue(i).SetCount(1);
    }
    // find out if centrosymmetric...
    if (groups.Count() > 1) {
      const size_t gc = groups.Count();
      bool inv_t_initialised = false;
      for (size_t i = 0; i < groups.Count(); i++) {
        const size_t ii = groups.IndexOf(rotation_id::negate(groups.GetKey(i)));
        if (ii == InvalidIndex)
          continue;
        /* check that the thing folds back onto itself */
        size_t i1 = i, i2 = ii;
        if (groups.GetValue(i)[0].r.Determinant() < 0)
          olx_swap(i1, i2);
        vec3d t = Info::normalise_t(
          (groups.GetValue(i1)[0].t - groups.GetValue(i2)[0].t).Abs());
        if (!inv_t_initialised) {
          rv.inv_trans = t;
          inv_t_initialised = true;
        }
        else {
          if (!t.Equals(rv.inv_trans, 1e-3)) {
            throw TFunctionFailedException(__OlxSourceInfo,
              "inversion translation varies");
          }
        }
        if (i2 == i)
          groups.Delete(i--);
        else
          groups.Delete(i2);
      }
      rv.centrosymmetric = (groups.Count() * 2 == gc);
      if (!rv.centrosymmetric && groups.Count() != gc) {
        throw TFunctionFailedException(__OlxSourceInfo,
          "incomplete inversion");
      }
    }
    for (size_t i = 0; i < groups.Count(); i++) {
      rv.matrices.AddCopy(groups.GetValue(i)[0]);
    }
    rv.normalise(translations);
    if (translations_ != 0) {
      *translations_ = translations;
    }
    return rv;
  }
  // returns a compact form of SymmSpace
  template <class SP> static InfoEx Compact(const SP& sp) {
    InfoEx rv;
    Info info = GetInfo(sp, &rv.vertices);
    rv.centrosymmetric = info.centrosymmetric;
    for (size_t i = 0; i < info.matrices.Count(); i++) {
      if (!info.matrices[i].IsI()) {
        rv.matrices.AddCopy(info.matrices[i]);
      }
    }
    // expand -1 if not at (0,0,0)
    if (!info.inv_trans.IsNull(1e-3)) {
      rv.centrosymmetric = false;
      // use info - it has the I
      size_t mc = info.matrices.Count();
      rv.matrices.SetCapacity(mc * 2);
      smatd invt(-mat3d::Idenity(), -info.inv_trans * 2);
      for (size_t i = 0; i < mc; i++) {
        smatd& m = rv.matrices.AddCopy(info.matrices[i] * invt);
        m.t -= m.t.Floor<int>();
      }
    }
    return rv;
  }
};  // end namespace SymmSpace

EndXlibNamespace()
#endif
