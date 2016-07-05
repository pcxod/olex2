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
//.............................................................................
void OpTest(OlxTests& t)  {
  t.description = __FUNC__;
  double ta_[12] = { 1, 2, 3, 4, 5, -1, -2, -3, -4, -5, 1, 0 };
  TDoubleList ta(12, &ta_[0]);
  if (ta.Filter(olx_alg::olx_eq(0)).Count() != 1)
    throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
  if (ta.Filter(olx_alg::olx_neq(0)).Count() != 11)
    throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
  if (ta.Filter(olx_alg::olx_neq(1)).Count() != 10)
    throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
  if (ta.Filter(olx_alg::olx_eq(1)).Count() != 2)
    throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
  if (ta.Filter(olx_alg::olx_eq(2)).Count() != 1)
    throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
  if (ta.Filter(olx_alg::olx_lt(0)).Count() != 5)
    throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
  if (ta.Filter(olx_alg::olx_le(0)).Count() != 6)
    throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
  if (ta.Filter(olx_alg::olx_gt(0)).Count() != 6)
    throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
  if (ta.Filter(olx_alg::olx_ge(0)).Count() != 7)
    throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");

  if (ta.Filter(olx_alg::olx_eq(0, TDirectAccessor<double>())).Count() != 1)
    throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
  if (ta.Filter(olx_alg::olx_neq(0, TDirectAccessor<double>())).Count() != 11)
    throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
  if (ta.Filter(olx_alg::olx_neq(1, TDirectAccessor<double>())).Count() != 10)
    throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
  if (ta.Filter(olx_alg::olx_eq(1, TDirectAccessor<double>())).Count() != 2)
    throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
  if (ta.Filter(olx_alg::olx_eq(2, TDirectAccessor<double>())).Count() != 1)
    throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
  if (ta.Filter(olx_alg::olx_lt(0, TDirectAccessor<double>())).Count() != 5)
    throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
  if (ta.Filter(olx_alg::olx_le(0, TDirectAccessor<double>())).Count() != 6)
    throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
  if (ta.Filter(olx_alg::olx_gt(0, TDirectAccessor<double>())).Count() != 6)
    throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
  if (ta.Filter(olx_alg::olx_ge(0, TDirectAccessor<double>())).Count() != 7)
    throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");


  if (ta.Filter(olx_alg::olx_and(olx_alg::olx_ge(0), olx_alg::olx_le(0)))
    .Count() != 1)
  {
    throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
  }
  if (ta.Filter(olx_alg::olx_or(olx_alg::olx_gt(0), olx_alg::olx_lt(0)))
    .Count() != 11)
  {
    throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
  }
  if (ta.Filter(olx_alg::olx_not(olx_alg::olx_eq(0))).Count() !=
    ta.Filter(olx_alg::olx_neq(0)).Count())
  {
    throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
  }
  if (ta.Filter(olx_alg::olx_not(olx_alg::olx_lt(0))).Count() !=
    ta.Filter(olx_alg::olx_ge(0)).Count())
  {
    throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
  }
  if (ta.Filter(olx_alg::olx_not(olx_alg::olx_le(0))).Count() !=
    ta.Filter(olx_alg::olx_gt(0)).Count())
  {
    throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
  }

  if (ta.Count(olx_alg::olx_ge(0, TDirectAccessor<double>())) != 7)
    throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");

  {
    int v = 10;
    TPtrList<int> lst(10);
    lst[0] = lst[3] = lst[9] = &v;

    if (lst.Count(olx_alg::olx_eq((const int *)0)) != 7)
      throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
    if (lst.Count(olx_alg::olx_eq(&v)) != 3)
      throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
  }
  {
    int v = 10;
    TTypeList<int> lst(10, true);
    lst.ForEach(olx_list_init::value(0));
    lst[0] = lst[3] = lst[9] = v;

    if (lst.Count(olx_alg::olx_eq(0)) != 7)
      throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
    if (lst.Count(olx_alg::olx_eq(v)) != 3)
      throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
  }
  {
    int a, b;
    sorted::PointerPointer<int> lst;
    lst.Add(&a);
    lst.Add(&b);
    lst.Add(&a);
    lst.Add(NULL);
    if (lst.Count(olx_alg::olx_eq(&a)) != 2)
      throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
    if (lst.Count(olx_alg::olx_eq((const int *)0)) != 1)
      throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
  }
}
//.............................................................................
void AlgTests(OlxTests& t)  {
  t.Add(&test::IndexAccessorTest)
    .Add(&test::OpTest);
}
};  //namespace test
