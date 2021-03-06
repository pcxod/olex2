/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "evalue.h"
namespace test {

  struct EValueTestData {
    double v, e;
    olxcstr res;
  };
void EValueTests(OlxTests& t)  {
  t.description = __FUNC__;
  EValueTestData data[] = {
    100, 10, "100(10)",
    -100, 10, "-100(10)",
    100, 50, "100(50)",
    -100, 50, "-100(50)",
    100, 0.1, "100.0(1)",
    -100, 0.1, "-100.0(1)",
    100, 0.01, "100.00(1)",
    -100, 0.01, "-100.00(1)",
    100, 1, "100.0(10)",
  };
  EValueTestData fwd_only_data[] = {
    100.01, 0.022, "100.01(2)",
    -100.01, 0.022, "-100.01(2)",
    -100.015, 0.025, "-100.02(3)",
  };
  const size_t sz = sizeof(data)/sizeof(data[0]);
  for( size_t i=0; i < sz; i++ )  {
    TEValue<double> ev(data[i].v, data[i].e);
    if (ev.ToCStr() != data[i].res) {
      throw TFunctionFailedException(__OlxSourceInfo,
        olxstr("Expected: ") << data[i].res << ", got: " << ev.ToCStr());
    }
    ev = data[i].res;
    if (ev.GetV() != data[i].v || ev.GetE() != data[i].e) {
      throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
    }
  }
  const size_t fwd_only_sz = sizeof(fwd_only_data)/sizeof(fwd_only_data[0]);
  for( size_t i=0; i < fwd_only_sz; i++ )  {
    TEValue<double> ev(fwd_only_data[i].v, fwd_only_data[i].e);
    if (ev.ToCStr() != fwd_only_data[i].res) {
      throw TFunctionFailedException(__OlxSourceInfo,
        olxstr("Expected: ") << fwd_only_data[i].res << ", got: " << ev.ToCStr());
    }
  }
  TEValueD A, B, C, tst;
  double data1[][4] = {
    { 10,2, -3, 1 },
    { -10,0.2, 3, 10 },
    { 110,2, -3, 1 },
    { 10,2, 3, 2 },
  };
  const size_t sz1 = sizeof(data1) / sizeof(data1[1]);
  for (size_t i = 0; i < sz1; i++) {
    A = TEValueD(data1[i][0], data1[i][1]);
    B = TEValueD(data1[i][2], data1[i][3]);
    C = A*B;
    tst = TEValueD(A.V()*B.V(), olx_abs(A.V()*B.V()) * sqrt(olx_sqr(A.E() / A.V()) + olx_sqr(B.E() / B.V())));
    if (!C.Equals(tst, 0.001)) {
      throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
    }
    C = A / B;
    tst = TEValueD(A.V() / B.V(), olx_abs(A.V() / B.V()) * sqrt(olx_sqr(A.E() / A.V()) + olx_sqr(B.E() / B.V())));
    if (!C.Equals(tst, 0.001)) {
      throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
    }

    C = A + B;
    tst = TEValueD(A.V() + B.V(), sqrt(olx_sqr(A.E()) + olx_sqr(B.E())));
    if (!C.Equals(tst, 0.001)) {
      throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
    }

    C = B - A;
    tst = TEValueD(B.V() - A.V(), sqrt(olx_sqr(A.E()) + olx_sqr(B.E())));
    if (!C.Equals(tst, 0.001)) {
      throw TFunctionFailedException(__OlxSourceInfo, "unexpected result");
    }
  }
}
//...................................................................................................
};  //namespace test
