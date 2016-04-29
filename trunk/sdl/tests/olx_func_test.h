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
    TStrList strl;
    for (int i = 0; i < 10; i++) {
      strl << ' ' << rand() << ' ';
    }
    strl.ForEachString(
      olx_func::make(&olxstr::SubString, 0, 1));
    for (size_t i = 0; i < strl.Count(); i++) {
      if (olxstr(strl[i]) != strl[i].TrimWhiteChars()) {
        throw TFunctionFailedException(__OlxSourceInfo, "n/a");
      }
    }
  }
  //...................................................................................................
  void FuncTests(OlxTests &t) {
    t.Add(&FuncTest1);
  }
};  //namespace test
