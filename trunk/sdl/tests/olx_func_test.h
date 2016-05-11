/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "olxfunc.h"

namespace test {

  void FuncTest1(OlxTests& t) {
    t.description = __FUNC__;
    TStrList l1;
    for (int i = 0; i < 10; i++) {
      l1.Add(' ') << rand() << ' ';
    }
    // just compilation test...
    l1.ForEachString(olx_func::make(&olxstr::SubString, 1, 1));
  }
  //...................................................................................................
  void FuncTests(OlxTests &t) {
    t.Add(&FuncTest1);
  }
};  //namespace test
