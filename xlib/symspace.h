/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_symspace_H
#define __olx_xl_symspace_H
#include "edict.h"
#include "mat_id.h"
BeginXlibNamespace()

template <class MatList> class TSymSpace  {
  MatList ml;
  const mat3d &cart2cell, &cell2cart, &hkl2cart;
  bool centrosymmetric;
  size_t start;
protected:
  TSymSpace(const TSymSpace& sp, size_t _start) :
    ml(sp.ml),
    cart2cell(sp.cart2cell),
    cell2cart(sp.cell2cart),
    hkl2cart(sp.hkl2cart),
    start(sp.start+_start),
    centrosymmetric(sp.centrosymmetric)  {}
public:
  TSymSpace(const MatList& _ml,
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

  TSymSpace(const TSymSpace& sp) :
    ml(sp.ml),
    cart2cell(sp.cart2cell),
    cell2cart(sp.cell2cart),
    hkl2cart(sp.hkl2cart),
    start(sp.start),
    centrosymmetric(sp.centrosymmetric)  {}

  inline const smatd& operator [] (size_t i) const {  return ml[i+start];  }
  inline size_t Count() const {  return ml.Count()-start;  }
  inline bool IsEmpty() const {  return Count() == 0;  }
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
  TSymSpace<MatList> SubListFrom(size_t _start) const {  return TSymSpace<MatList>(*this, _start);  }
};

// the adaptor for complex classes having MatrixCount()/GetMatrix(size_t) methods
template <class MatrixContainer> class MatrixListAdaptor {
  const MatrixContainer& mc;
public:
  MatrixListAdaptor(const MatrixContainer& _mc) : mc(_mc)  {}
  MatrixListAdaptor(const MatrixListAdaptor& mla) : mc(mla.mc)  {}
  inline size_t Count() const {  return mc.MatrixCount();  }
  inline bool IsEmpty() const {  return Count() == 0;  }
  inline const smatd& operator [](size_t i) const {  return mc.GetMatrix(i);  }
};

namespace SymSpace  {
  static int sort_group(const smatd *m1, const smatd *m2) {
    int r = olx_cmp(m1->t.QLength(), m2->t.QLength());
    if (r == 0) {
      if (m1->t[0] == 0 || m2->t[0] == 0) {
        if (m1->t[0] == 0 && m2->t[0] == 0)
          return olx_cmp(m1->t[1], m2->t[1]);
        return olx_cmp(m1->t[0], m2->t[0]);
      }
    }
    return r;
  }
  struct Info  {
    bool centrosymmetric;
    /* holds references of the original object, be careful with const&
    containers!
    */
    smatd_list matrices;
    /* if not zero, all inversion of the symmetry operators also translates by
    this vector
    */
    vec3d inv_trans;
    // always positive
    short latt;
    // a list of lattice translations
    vec3d_list translations;
  };
  struct InfoEx  {  // has a list of translation vectors vs latt number
    bool centrosymmetric;
    smatd_list matrices;
    vec3d_list vertices;
  };
  template <class SP> static Info GetInfo(const SP& sp)  {
    if( sp.Count() == 0 ) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        "at least one matrix is expected");
    }
    Info rv;
    rv.latt = -1;
    rv.centrosymmetric = false;
    if( sp.Count() == 1 )  {
      if( !sp[0].r.IsI() || !sp[0].t.IsNull() ) {
        throw TInvalidArgumentException(__OlxSourceInfo,
          "identity matrix expected");
      }
      rv.latt = 1; //'P';
      rv.matrices.AddCopy(sp[0]);
      return rv;
    }
    olxdict<int, smatd_list, TPrimitiveComparator> groups;
    bool identity_found = false;
    for( size_t i=0; i < sp.Count(); i++ )  {
      smatd_list& l = groups.Add(rotation_id::get(sp[i].r));
      l.AddCopy(sp[i]);
      if (!identity_found && sp[i].IsI())
        identity_found = true;
    }
    if (!identity_found) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        "missing identity matrix");
    }
    const size_t min_a_group=groups.GetValue(0).Count();
    for( size_t i=0; i < groups.Count(); i++ )  {
      if( groups.GetValue(i).Count() != min_a_group )
        throw TInvalidArgumentException(__OlxSourceInfo, "matrix list");
    }
    if( min_a_group ==  1 )
      rv.latt = 1; //'P';
    else if( min_a_group == 2 )  {
      vec3d t = groups.GetValue(0)[1].t - groups.GetValue(0)[0].t;
      t -= t.Floor<int>();
      if( olx_abs(t[0]-0.5) < 1e-6 )  {
        if( olx_abs(t[1]-0.5) < 1e-6 )  {
          if( olx_abs(t[2]-0.5) < 1e-6 )
            rv.latt = 2; //'I';
          else if( olx_abs(t[2]) < 1e-6 )
            rv.latt = 7; //'C';
        }
        else if( olx_abs(t[1]) < 1e-6 && olx_abs(t[2]-0.5) < 1e-6 )
          rv.latt = 6; //'B';
      }
      else if( olx_abs(t[0]) < 1e-6 && olx_abs(t[1]-0.5) < 1e-6 &&
               olx_abs(t[2]-0.5) < 1e-6 )
      {
        rv.latt = 5; //'A';
      }
      rv.translations.AddCopy(t);
    }
    else if( min_a_group == 3 )  {
      vec3d t1 = groups.GetValue(0)[1].t - groups.GetValue(0)[0].t;
      vec3d t2 = groups.GetValue(0)[2].t - groups.GetValue(0)[0].t;
      vec3d t1r(2./3, 1./3, 1./3), t2r(1./3, 2./3, 2./3);
      vec3d t1s(1./3, 1./3, 2./3), t2s(2./3, 2./3, 1./3);
      vec3d t1t(1./3, 2./3, 1./3), t2t(2./3, 1./3, 2./3);
      t1 -= t1.Floor<int>();
      t2 -= t2.Floor<int>();
      if( (t1.Equals(t1r, 1e-3) && t2.Equals(t2r, 1e-3)) ||
          (t1.Equals(t2r, 1e-3) && t2.Equals(t1r, 1e-3)))
      {
        rv.latt = 3; //'R';
      }
      else if( (t1.Equals(t1s, 1e-3) && t2.Equals(t2s, 1e-3)) ||
               (t1.Equals(t2s, 1e-3) && t2.Equals(t1s, 1e-3)))
      {
        rv.latt = 8; //'S';
      }
      else if( (t1.Equals(t1t, 1e-3) && t2.Equals(t2t, 1e-3)) ||
               (t1.Equals(t2t, 1e-3) && t2.Equals(t1t, 1e-3)))
      {
        rv.latt = 9; //'T';
      }
      rv.translations.AddCopy(t1);
      rv.translations.AddCopy(t2);
    }
    else if( min_a_group ) {
      rv.translations.AddNew(0.0, 0.5, 0.5);
      rv.translations.AddNew(0.5, 0.0, 0.5);
      rv.translations.AddNew(0.5, 0.5, 0.0);
      rv.latt = 4; //'F';
    }
    if( rv.latt < 0 ) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "could not deduce lattice centering");
    }
    for( size_t i=0; i < groups.Count(); i++ )  {
      for (size_t j=0; j < groups.GetValue(i).Count(); j++) {
        groups.GetValue(i)[j].t -=
          groups.GetValue(i)[j].t.template Floor<int>();
      }
      groups.GetValue(i).QuickSorter.SortSF(groups.GetValue(i), &sort_group);
      groups.GetValue(i).SetCount(1);
    }
    // find out if centrosymmetric...
    if( groups.Count() > 1 )  {
      const size_t gc = groups.Count();
      bool inv_t_initialised = false;
      for( size_t i=0; i < groups.Count(); i++ )  {
        const size_t ii = groups.IndexOf(rotation_id::negate(groups.GetKey(i)));
        if( ii == InvalidIndex )
          continue;
        /* check that the thing folds back onto itself */
        vec3d t = (groups.GetValue(ii)[0].t - groups.GetValue(i)[0].t).Abs();
        if (!inv_t_initialised) {
          rv.inv_trans = t;
          inv_t_initialised = true;
        }
        else {
          if (!t.Equals(rv.inv_trans, 1e-3) ) {
            throw TFunctionFailedException(__OlxSourceInfo,
              "inversion translation varies");
          }
        }
        groups.GetValue(ii).AddCopy(groups.GetValue(i)[0]);
        groups.Delete(i--);
      }
      rv.centrosymmetric = (groups.Count()*2 == gc);
      if( rv.centrosymmetric )  {
        for( size_t i=0; i < groups.Count(); i++ )  {
          if( groups.GetValue(i)[0].r.Determinant() < 0 )
            groups.GetValue(i).Delete(0);
          else
            groups.GetValue(i).SetCount(1);
        }
      }
    }
    for( size_t i=0; i < groups.Count(); i++ )  {
      smatd_list& l = groups.GetValue(i);
      for( size_t j=0; j < l.Count(); j++ )
        rv.matrices.AddCopy(l[j]);
    }
    return rv;
  }
  // returns a compact form of symspace
  template <class SP> static InfoEx Compact(const SP& sp)  {
    InfoEx rv;
    Info info = GetInfo(sp);
    rv.centrosymmetric = info.centrosymmetric;
    rv.vertices = info.translations;
    for (size_t i=0; i < info.matrices.Count(); i++)  {
      if (info.matrices[i].IsI())
        rv.matrices.AddCopy(info.matrices[i]);
    }
    return rv;
  }
};  // end namespace SymSpace

EndXlibNamespace()
#endif
