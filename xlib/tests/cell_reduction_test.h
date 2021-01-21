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
  evecd cell = evecd::build(6, 25.0822, 5.04549, 29.4356, 90, 103.108, 90);
  Niggli::reduce(7, cell);
  if( cell.DistanceTo(*evecd::build(6, 5.04549, 12.7923, 29.3711, 77.7182, 85.0727, 78.6263)) > 1e-3)
    throw TFunctionFailedException(__OlxSourceInfo, "result mismatch expectations");
  // Acta Cryst. (1976), A32, 297-298, this reduction does not match the one in the paper
  evecd cell1 = evecd::build(6, 3, 5.196, 2, 103.55, 109.28, 134.53);
  Niggli::reduce(1, cell1);
  //if( cell1.DistanceTo(evecd(2, 3, 3, 60, 75.31, 70.32)) > 1) - this is in the paper
  if( cell1.DistanceTo(*evecd::build(6, 2, 3, 3.05, 115.315, 93.909, 109.28)) > 1e-3)
    throw TFunctionFailedException(__OlxSourceInfo, "result mismatch expectations");
  evecd cell2 = evecd::build(6, 17.1947, 17.1947, 84.661, 90, 90, 120);
  Niggli::reduce(3, cell2); //R
  if( cell2.DistanceTo(*evecd::build(6, 17.1947, 17.1947, 29.9155, 73.2984, 73.2984, 60)) > 1e-3)
    throw TFunctionFailedException(__OlxSourceInfo, "result mismatch expectations");
  cell2 = *evecd::build(6, 17.1947, 17.1947, 84.661, 90, 90, 120);
  Niggli::reduce(1, cell2); //P
  if( cell2.DistanceTo(*evecd::build(6, 17.1947, 17.1947, 84.661, 90, 90, 120)) > 1e-3)
    throw TFunctionFailedException(__OlxSourceInfo, "result mismatch expectations");
  cell2 = *evecd::build(6, 17.1947, 17.1947, 84.661, 90, 90, 120);
  Niggli::reduce(2, cell2);  //I
  if( cell2.DistanceTo(*evecd::build(6, 17.1947, 17.1947, 43.1947, 84.2886, 78.5194, 60)) > 1e-3)
    throw TFunctionFailedException(__OlxSourceInfo, "result mismatch expectations");
  cell2 = *evecd::build(6, 17.1947, 17.1947, 84.661, 90, 90, 120);
  Niggli::reduce(4, cell2);  //F
  if( cell2.DistanceTo(*evecd::build(6, 8.59735, 14.891, 43.1947, 99.9257, 95.7114, 90)) > 1e-3)
    throw TFunctionFailedException(__OlxSourceInfo, "result mismatch expectations");
  cell2 = *evecd::build(6, 17.1947, 17.1947, 84.661, 90, 90, 120);
  Niggli::reduce(5, cell2); //A
  if( cell2.DistanceTo(*evecd::build(6, 17.1947, 17.1947, 43.1947, 84.2886, 78.5194, 60)) > 1e-3)
    throw TFunctionFailedException(__OlxSourceInfo, "result mismatch expectations");
  cell2 = *evecd::build(6, 17.1947, 17.1947, 84.661, 90, 90, 120);
  Niggli::reduce(6, cell2);  //B
  if( cell2.DistanceTo(*evecd::build(6, 17.1947, 17.1947, 43.1947, 84.2886, 78.5194, 60)) > 1e-3)
    throw TFunctionFailedException(__OlxSourceInfo, "result mismatch expectations");
  cell2 = *evecd::build(6, 17.1947, 17.1947, 84.661, 90, 90, 120);
  Niggli::reduce(7, cell2); //C
  if( cell2.DistanceTo(*evecd::build(6, 8.59735, 14.891, 84.661, 90, 90, 90)) > 1e-3)
    throw TFunctionFailedException(__OlxSourceInfo, "result mismatch expectations");
  // cctbx tests
  cell2 = *evecd::build(6, 12.7366, 29.2300, 5.0242, 94.6570, 100.8630, 99.7561);
  Niggli::reduce(1, cell2);
  if( cell2.DistanceTo(*evecd::build(6, 5.0242, 12.7366, 29.23, 99.7561, 94.657, 100.863)) > 1e-3)
    throw TFunctionFailedException(__OlxSourceInfo, "result mismatch expectations");
  cell2 = *evecd::build(6, 12.7806, 12.7366, 29.457, 103.42, 103.57, 22.71);
  Niggli::reduce(1, cell2);
  if( cell2.DistanceTo(*evecd::build(6, 5.0242, 12.7366, 29.23, 99.7561, 94.657, 100.863)) > 1e-3)
    throw TFunctionFailedException(__OlxSourceInfo, "result mismatch expectations");
  return;
}
};  //namespace test
