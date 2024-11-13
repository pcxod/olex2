/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/
// matrix differentiantion helper functions
#ifndef __olx_xlib_dmat_H
#define __olx_xlib_dmat_H
#include "../threex3.h"
BeginEsdlNamespace()

namespace math { struct dmat {

  static mat3d M_x_One(const mat3d &m, size_t i, size_t j) {
    mat3d r;
    for (size_t idx = 0; idx < 3; idx++) {
      r[idx][j] = m[idx][i];
    }
    return r;
  }

  static mat3d One_x_M(const mat3d &m, size_t i, size_t j) {
    mat3d r;
    for (size_t idx = 0; idx < 3; idx++) {
      r[i][idx] = m[j][idx];
    }
    return r;
  }

  static mat3d M_x_OneSym(const mat3d &m, size_t i, size_t j) {
    mat3d r;
    for (size_t idx=0; idx < 3; idx++) {
      r[idx][j] = m[idx][i];
      r[idx][i] = m[idx][j];
    }
    return r;
  }

  static mat3d OneSym_x_M(const mat3d &m, size_t i, size_t j) {
    mat3d r;
    for (size_t idx=0; idx < 3; idx++) {
      r[i][idx] = m[j][idx];
      r[j][idx] = m[i][idx];
    }
    return r;
  }

  static mat3d M_x_One_x_Mt(const mat3d &m, size_t i, size_t j) {
    mat3d r;
    for (size_t i1=0; i1 < 3; i1++)
      for (size_t i2=0; i2 < 3; i2++) {
        r[i1][i2] = m[i1][i]*m[i2][j];
      }
      return r;
  }

  static mat3d M_x_OneSym_x_Mt(const mat3d &m, size_t i, size_t j) {
    if (i == j) {
      return M_x_One_x_Mt(m, i, j);
    }
    mat3d r;
    for (size_t i1=0; i1 < 3; i1++)
      for (size_t i2=0; i2 < 3; i2++) {
        r[i1][i2] = m[i1][i]*m[i2][j] + m[i1][j]*m[i2][i];
      }
      return r;
  }
}; // struct dmat

// symmetrix matrix helper functions
struct linear_to_sym_base {
  static size_t*get_i_j(size_t i) {
    static size_t a[6][2] = { {0,0}, {0,1}, {0,2}, {1,1}, {1,2}, {2,2} };
#ifdef OLX_DEBUG
    if (i > 5) {
      throw TIndexOutOfRangeException(__OlxSourceInfo, i, 0, 5);
    }
#endif
    return &a[i][0];
  }
  static size_t get_i(size_t i) {  return get_i_j(i)[0]; }
  static size_t get_j(size_t i) {  return get_i_j(i)[1]; }
};

template <typename mat_t> struct linear_from_sym {
  typedef typename mat_t::number_type FT;
  evecd data;
  linear_from_sym(const mat_t &m_) {
    size_t rc = m_.RowCount();
    if (rc != m_.ColCount()) {
      throw TInvalidArgumentException(__OlxSourceInfo, "symmetric matrix");
    }
    data.Resize((rc*(rc+1))/2);
    size_t idx=0;
    for (size_t i=0; i < rc; i++) {
      for (size_t j=i; j < rc; j++, idx++) {
        data(idx) = m_(i,j);
#ifdef OLX_DEBUG
        if (olx_cmp_float(m_(i, j), m_(j, i), 1e-8) != 0) {
          throw TInvalidArgumentException(__OlxSourceInfo, "symmetric matrix");
        }
#endif
      }
    }
  }
  FT operator () (size_t i) const { return data(i);  }
};

template <typename mat_t>
struct linear_to_sym : public linear_to_sym_base {
  typedef typename mat_t::number_type FT;
  mat_t &m;
  linear_to_sym(mat_t &m_) : m(m_) {}
  struct av {
    FT &a, &b;
    av(FT &a_, FT &b_) : a(a_), b(b_) {}
    FT operator = (FT v) { return (a=b=v); }
  };
  av operator () (size_t i) const {
    return av(m(get_i(i), get_j(i)), m(get_j(i), get_i(i)));
  }
};
} // namespace math
EndEsdlNamespace()
#endif
