/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "symmspace.h"

using namespace SymmSpace;

int SymmSpace::sort_group(const smatd &m1, const smatd &m2) {
  int r = olx_cmp(m1.t.QLength(), m2.t.QLength());
  if (r != 0) return r;
  r = olx_cmp(m1.t[0], m2.t[0]);
  if (r != 0) return r;
  r = olx_cmp(m1.t[1], m2.t[1]);
  if (r != 0) return r;
  return olx_cmp(m1.t[2], m2.t[2]);
}
//.............................................................................
//.............................................................................
bool Info::operator == (const Info &info) const {
  if (centrosymmetric != info.centrosymmetric ||
    latt != info.latt ||
    !inv_trans.Equals(info.inv_trans, 1e-3) ||
    matrices.Count() != info.matrices.Count())
  {
    return false;
  }
  // matrices are sorted, so 1:1 match is expected
  for (size_t i=0; i < matrices.Count(); i++)
    if (!matrices[i].Equals(info.matrices[i]))
      return false;
  return true;
}
//.............................................................................
vec3d Info::normalise_t(const vec3d &t_) {
  vec3d t = t_;
  for (int j=0; j < 3; j++) {
    t[j] -= olx_floor(t[j]);
    if (olx_abs(t[j]-1) < 1e-3) t[j] = 0;
  }
  return t;
}
//.............................................................................
void Info::normalise(const vec3d_list &translations) {
  for (size_t i=0; i < matrices.Count(); i++) {
    vec3d t = normalise_t(matrices[i].t),
      _t = t;
    for (size_t j=0; j < translations.Count(); j++) {
      vec3d at = normalise_t(_t + translations[j]);
      double st = t.Sum(), ast = at.Sum();
      if (st == ast) {
        if ((at[0] <  t[0]) ||
            (at[0] == t[0] && at[1] < t[1]) ||
            (at[0] == t[0] && at[1] == t[1] && at[2] < t[2]))
        {
          t = at;
        }
      }
      else if (ast < st)
        t = at;
    }
    matrices[i].t = t;
  }
  normalise();
}
//.............................................................................
