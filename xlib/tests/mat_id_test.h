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
  mat3d m(0, 1, 1, 1, 0, 1, 0, 0, 0);
  int id = rotation_id::get(m);
  mat3d id_r = rotation_id::get(id);
  if( m != id_r )
    throw TFunctionFailedException(__OlxSourceInfo, "m != id_r");
  int i_id = rotation_id::negate(id);
  mat3d i_id_r = rotation_id::get(i_id);
  if( m != -i_id_r )
    throw TFunctionFailedException(__OlxSourceInfo, "m != -i_id_r");
}
};  //namespace test
