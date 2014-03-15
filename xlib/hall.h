/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xlib_Hall_H
#define __olx_xlib_Hall_H
#include "mat_id.h"
#include "edict.h"
#include "symmat.h"
#include "symmspace.h"

BeginXlibNamespace()

struct HallSymbol  {
protected:
  struct symop {
    int rot_id;
    vec3d t;
  };
  void init();
  TTypeList<olx_pair_t<vec3d, olxstr> > trans;
  //const TTypeList<olx_pair_t<mat3d, olxstr> > rotations;
  TTypeList<olx_pair_t<int,olxstr> >
    rotx, roty, rotz, rotx1, roty1, rotz1, rot3;
  olxstr_dict<int> r_dict;
  olxstr_dict<vec3d*> t_dict;
  olxstr FindT(const vec3d& t, int order) const;
  olxstr FindTR(const vec3d& t, int order) const;
  int FindR(olxstr& hs, TTypeList<symop>& matrs,
    const TTypeList<olx_pair_t<int,olxstr> >& rot, bool full) const;
  vec3d get_screw_axis_t(int dir, int order) const;
  // dir - 1 for x, 2 for y, 3 for z, which - ' or "
  int find_diagonal(int dir, olxch which) const;
  static HallSymbol &GetInstance() {
    static HallSymbol i_;
    i_.init();
    return i_;
  }
  olxstr Evaluate_(int latt, const smatd_list& matrices) const;
  olxstr EvaluateEx_(int latt, const smatd_list& matrices) const;
  olxstr Evaluate_(const SymmSpace::Info& si) const;
  SymmSpace::Info Expand_(const olxstr &hs) const;
public:
  // a compact list of matrices is taken
  static olxstr Evaluate(int latt, const smatd_list& matrices) {
    return GetInstance().Evaluate_(latt, matrices);
  }
  /* a compact list of matrices is taken, expanded and compacted again -
  sometimes the asymmetric unit will have inversion expanded
  */
  static olxstr EvaluateEx(int latt, const smatd_list& matrices) {
    return GetInstance().EvaluateEx_(latt, matrices);
  }
  // takes full list of matrices
  static olxstr Evaluate(const smatd_list& matrices) {
    return Evaluate(SymmSpace::GetInfo(matrices));
  }
  static olxstr Evaluate(const SymmSpace::Info& si) {
    return GetInstance().Evaluate_(si);
  }
  static SymmSpace::Info Expand(const olxstr &hs) {
    return GetInstance().Expand_(hs);
  }
};

EndXlibNamespace()
#endif
