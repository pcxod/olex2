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
  for (int itr=0; itr <= 4; itr++) {
    if (itr == 0) {
      s_toks << "Pb" << "Co" << "O2";
      n_toks << 1 << 1 << 2;
    }
    else if (itr == 1) {
      s_toks.Clear();
      n_toks.Clear();
      s_toks << "P" << "B" << "C" << "OO2";
      n_toks << 1 << 1 << 1 << 3;
    }
    else if (itr == 2) {
      s_toks.Clear();
      n_toks.Clear();
      s_toks << "P" << "B1" << "C" << "O1OO2";
      n_toks << 1 << 1 << 1 << 4;
    }
    else if (itr == 3) {
      s_toks.Clear();
      n_toks.Clear();
      s_toks << "P1" << "B1" << "C" << "O1OO2";
      n_toks << 1 << 1 << 1 << 4;
    }
    else if (itr == 4) {
      s_toks.Clear();
      n_toks.Clear();
      s_toks << "P" << "Br" << "O3";
      n_toks << 1 << 1 << 3;
    }
    else
      break;
    ContentList cl = XElementLib::ParseElementString(s_toks.Text(EmptyString()));
    if (cl.Count() != s_toks.Count()) {
      throw TFunctionFailedException(__OlxSourceInfo,
        olxstr("Unexpected number of tokens: ") << cl.Count() <<
        " vs expected " << s_toks.Count());
    }
    else {
      for (size_t i=0; i < cl.Count(); i++) {
        if (!s_toks[i].StartsFromi(cl[i].element->symbol)) {
          throw TFunctionFailedException(__OlxSourceInfo,
            olxstr("Mismatching element type. Expected ").quote() << s_toks[i] <<
            " got " << cl[i].element->symbol);
        }
        if (olx_abs(cl[i].count-n_toks[i]) > 1e-3) {
          throw TFunctionFailedException(__OlxSourceInfo,
            olxstr("Mismatching element count. Expected ").quote() << n_toks[i] <<
            " got " << cl[i].count);
        }
      }
    }
  }
}
//...................................................................................................
void FormulaTests(OlxTests& t)  {
  t.Add(test::FormulaParseTest);
}
};  //namespace test
