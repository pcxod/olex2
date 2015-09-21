/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "mat_id.h"

namespace test {

void rotation_id_test(OlxTests& t)  {
  t.description = __OlxSrcInfo;
  mat3d m(0, 1, 1, 1, 0, 1, 0, 0, -1);
  int id = rotation_id::get(m);
  mat3d id_r = rotation_id::get(id);
  if( m != id_r )
    throw TFunctionFailedException(__OlxSourceInfo, "m != id_r");
  int i_id = rotation_id::negate(id);
  mat3d i_id_r = rotation_id::get(i_id);
  if( m != -i_id_r )
    throw TFunctionFailedException(__OlxSourceInfo, "m != -i_id_r");
  if( id != rotation_id::negate(rotation_id::negate(id)) )
    throw TFunctionFailedException(__OlxSourceInfo, "m != -i_id_r");
}


void smatd_id_test(OlxTests& t)  {
  t.description = __OlxSrcInfo;
  smatd m(mat3d(0, 1, 1, 0, 0, 1, 0, 0, -1), vec3d(0.5, 1./3, 1./12));
  int id = sg_mat_id::get(m);
  mat3d id_r = rotation_id::get(id);
  if( m.r != id_r )
    throw TFunctionFailedException(__OlxSourceInfo, "m.r != id_r");
  smatd id_s = sg_mat_id::get(id);
  if( !(m == id_s) )
    throw TFunctionFailedException(__OlxSourceInfo, "m != id_s");
}

template <int base>
void full_id_test(OlxTests& t) {
  t.description = __OlxSrcInfo;
  t.description << '_' << base;
  smatd m(mat3d(-1, 1, 1, 1, 0, 1, 0, 0, -1), vec3d(-1./base, 10000./base, -21./base));
  uint64_t id = full_smatd_id<base>::get(m);
  smatd id_r = full_smatd_id<base>::get(id);
  if( m.r != id_r.r || m.t.QDistanceTo(id_r.t) > 1e-15 )  {
    math::alg::print0_2(m.r, "Source:");
    math::alg::print0_1(m.t);
    math::alg::print0_2(id_r.r, "Result:");
    math::alg::print0_1(id_r.t);
    throw TFunctionFailedException(__OlxSourceInfo, "m != id_r");
  }
  uint64_t i_id = full_smatd_id<base>::negate(id);
  smatd i_id_r = full_smatd_id<base>::get(i_id);
  i_id_r *= -1;
  if( m.r != i_id_r.r || m.t.QDistanceTo(i_id_r.t) > 1e-15 )  {
    math::alg::print0_2(m.r, "Source:");
    math::alg::print0_1(m.t);
    math::alg::print0_2(id_r.r, "Result:");
    math::alg::print0_1(id_r.t);
    throw TFunctionFailedException(__OlxSourceInfo, "m != -i_id_r");
  }
}
void MatIdTests(OlxTests& t)  {
  t.description = __OlxSrcInfo;
  t.Add(&test::rotation_id_test)
    .Add(&test::smatd_id_test)
    .Add(&test::full_id_test<12>)
    .Add(&test::full_id_test<256>)
;}
};  //namespace test
