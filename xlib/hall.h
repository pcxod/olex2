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
  static void init();
  static TTypeList<AnAssociation2<vec3d, olxstr> > trans;
  //const TTypeList<AnAssociation2<mat3d, olxstr> > rotations;
  static TTypeList<AnAssociation2<int,olxstr> >
    rotx, roty, rotz, rotx1, roty1, rotz1, rot3;
  static olxstr_dict<int> r_dict;
  static olxstr_dict<vec3d*> t_dict;
  static olxstr FindT(const vec3d& t, int order);
  static olxstr FindTR(const vec3d& t, int order);
  static int FindR(olxstr& hs, TTypeList<symop>& matrs,
    const TTypeList<AnAssociation2<int,olxstr> >& rot, bool full);
  static vec3d get_screw_axis_t(int dir, int order);
  // dir - 1 for x, 2 for y, 3 for z, which - ' or "
  static int find_diagonal(int dir, olxch which);
public:
  // a compact list of matrices is taken
  static olxstr Evaluate(int latt, const smatd_list& matrices);
  /* a compact list of matrices is taken, expanded and compacted again -
  sometimes the asymmetric unit will have inversion expanded
  */
  static olxstr EvaluateEx(int latt, const smatd_list& matrices);
  // takes full list of matrices
  static olxstr Evaluate(const smatd_list& matrices) {
    return Evaluate(SymmSpace::GetInfo(matrices));
  }
  static olxstr Evaluate(const SymmSpace::Info& si);
  static SymmSpace::Info Expand(const olxstr &hs);
};

EndXlibNamespace()
#endif
