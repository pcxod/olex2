/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "cell_reduction.h"

namespace test {
// using some of the occtbx borrowed tests...
void cell_reduction_test(OlxTests& t)  {
  t.description = __OlxSrcInfo;
  vec3d s(2,3,5), a(80,90,100);
  Niggli::reduce(1, s, a);
  if( s.DistanceTo(vec3d(2,3,5)) > 1e-8 ||
      a.DistanceTo(vec3d(100,90,100)) > 1e-8 )
    throw TFunctionFailedException(__OlxSourceInfo, "result mismatch expectations");
  bool res = false;
  try {
    vec3d s1(2,3,5), a1(70,120,50);
    Niggli::reduce(1, s1, a1);
  }
  catch(...)  {  res = true;  }
  if( !res )
    throw TFunctionFailedException(__OlxSourceInfo, "expected eception not thrown");
  //http://www.ccdc.cam.ac.uk/support/documentation/conquest/ConQuest/conquest.3.340.html
  s = vec3d(12.132, 14.662, 13.153);
  a = vec3d(90, 127.84, 90);
  Niggli::reduce(1, s, a);
  if( s.DistanceTo(vec3d(11.154,12.132,14.663)) > 1e-2 ||
      a.DistanceTo(vec3d(90,90,111.36)) > 1e-2 )
    throw TFunctionFailedException(__OlxSourceInfo, "result mismatch expectations");
  evecd cell(25.0822, 5.04549, 29.4356, 90, 103.108, 90);
  Niggli::reduce(7, cell);
  if( cell.DistanceTo(evecd(5.04549, 12.7923, 29.3711, 77.7182, 85.0727, 78.6263)) > 1e-3)
    throw TFunctionFailedException(__OlxSourceInfo, "result mismatch expectations");
  // Acta Cryst. (1976), A32, 297-298, this reduction does not match the one in the paper
  evecd cell1(3, 5.196, 2, 103.55, 109.28, 134.53);
  Niggli::reduce(1, cell1);
  //if( cell1.DistanceTo(evecd(2, 3, 3, 60, 75.31, 70.32)) > 1) - this is in the paper
  if( cell1.DistanceTo(evecd(2, 3, 3.05, 115.315, 93.909, 109.28)) > 1e-3)
    throw TFunctionFailedException(__OlxSourceInfo, "result mismatch expectations");
  return;
}
};  //namespace test
