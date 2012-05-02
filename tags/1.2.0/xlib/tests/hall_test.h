/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "../hall.h"
#include "../symmlib.h"

namespace test {

void HallTest(OlxTests& t)  {
  t.description = __FUNC__;
  TSymmLib &sl = TSymmLib::GetInstance();
  for (size_t i=0; i < sl.SGCount(); i++) {
    TSpaceGroup &sg = sl.GetGroup(i);
    SymmSpace::Info si = sg.GetInfo();
    olxstr hs = HallSymbol::Evaluate(si);
    if (hs != sg.GetHallSymbol()) {
      throw TFunctionFailedException(__OlxSourceInfo,
        olxstr("Hall symbol mismatch: ").quote() << hs <<
        (olxstr("vs ").quote() << sg.GetHallSymbol()));
    }
    SymmSpace::Info r_si = HallSymbol::Expand(hs);
    if (si != r_si) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "Hall symbol expansion failed");
    }
  }
}
//...................................................................................................
void HallTests(OlxTests& t)  {
  t.Add(test::HallTest);
}
};  //namespace test
