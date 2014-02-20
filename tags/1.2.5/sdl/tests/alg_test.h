/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "../talist.h"
#include "../threex3.h"

namespace test {

void IndexAccessorTest(OlxTests& t)  {
  t.description = __FUNC__;
  double ta_[5] = {1, 2, 3, 4, 5};
  TDoubleList ta(5, &ta_[0]);
  olx_list_reverse::ReverseList_<TDoubleList> rl =
    olx_list_reverse::Make(ta);
  for (size_t i=0; i < rl.Count(); i++) {
    if (rl[i] != ta[ta.Count()-i-1])
      throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
  }
  olx_reverse(ta);
  for (size_t i=0; i < rl.Count(); i++) {
    if (rl[i] != i+1)
      throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
  }
  olx_reverse(ta);

  TSizeList indices(5, olx_list_init::index());
  double v = olx_mean(ta);
  if (olx_abs(v-double(1+2+3+4+5)/5) > 1e-8)
    throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
  IndexAccessor::IndexAccessor_<TDoubleList> la = IndexAccessor::Make(ta);
  v = olx_mean(indices, la);
  if (olx_abs(v-double(1+2+3+4+5)/5) > 1e-8)
    throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
  indices.Delete(2);
  v = olx_mean(indices, la);
  if (olx_abs(v-double(1+2+4+5)/4) > 1e-8)
    throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
  indices.Delete(0);
  v = olx_mean(indices, la);
  if (olx_abs(v-double(2+4+5)/3) > 1e-8)
    throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");

  vec3d_alist vl(3);
  indices.SetCount(3);
  indices.ForEach(olx_list_init::index());
  IndexAccessor::IndexAccessor_<vec3d_alist> vla = IndexAccessor::Make(vl);
  vl[0][0] = vl[1][1] = vl[2][2] = 3;
  vec3d mv = olx_mean(vl);
  if (mv.DistanceTo(olx_mean(indices, vla)) > 1e-8)
    throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
  if (mv.DistanceTo(vec3d(1)) > 1e-8)
    throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");

}
//...................................................................................................
void AlgTests(OlxTests& t)  {
  t.Add(test::IndexAccessorTest);
}
};  //namespace test
