/******************************************************************************
* Copyright (c) 2004-2015 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "../roman.h"
namespace test {
  void TestRoman(OlxTests& t) {
    t.description = __FUNC__;
    for (size_t i = 0; i < 100; i++) {
      size_t x = rand() % (4000);
      olxstr r = RomanNumber::To(x);
      size_t ri = RomanNumber::From(r);
      if (ri != x) {
        throw TFunctionFailedException(__OlxSourceInfo,
          olxstr(x) << " != " << ri << ", " << r);
        TBasicApp::NewLogEntry() << x;
      }
    }

  }
};  //namespace test
