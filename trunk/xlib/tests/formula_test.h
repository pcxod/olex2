/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "../chemdata.h"

namespace test {

void FormulaParseTest(OlxTests& t)  {
  t.description = __FUNC__;
  TStrList s_toks;
  TDoubleList n_toks;
  s_toks << "Pb" << "Co" << "O2";
  n_toks << 1 << 1 << 2;
  ContentList cl = XElementLib::ParseElementString("PbCoO2");
  if (cl.Count() != s_toks.Count()) {
    throw TFunctionFailedException(__OlxSourceInfo,
        olxstr("Unexpected number of tokens: ") << cl.Count() <<
        "vs expected " << s_toks.Count());
  }
  else {
    for (size_t i=0; i < cl.Count(); i++) {
      if (!s_toks[i].StartsFromi(cl[i].element.symbol)) {
        throw TFunctionFailedException(__OlxSourceInfo,
          olxstr("Mismatching element type. Expected ").quote() << s_toks[i] <<
            " got " << cl[i].element.symbol);
      }
      if (olx_abs(cl[i].count-n_toks[i]) > 1e-3) {
        throw TFunctionFailedException(__OlxSourceInfo,
          olxstr("Mismatching element count. Expected ").quote() << n_toks[i] <<
            " got " << cl[i].count);
      }
    }
  }
}
//...................................................................................................
void FormulaTests(OlxTests& t)  {
  t.Add(test::FormulaParseTest);
}
};  //namespace test
