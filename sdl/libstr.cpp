/******************************************************************************
* Copyright (c) 2004-2024 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/
#include "libstr.h"
#include "wildcard.h"

void LibStr::ToUpper(const TStrObjList& Params, TMacroData& E) {
  E.SetRetVal(Params[0].ToUpperCase());
}

void LibStr::ToLower(const TStrObjList& Params, TMacroData& E) {
  E.SetRetVal(Params[0].ToLowerCase());
}

void LibStr::Add(const TStrObjList& Params, TMacroData& E) {
  E.SetRetVal(Params[0] + Params[1]);
}

void LibStr::Eq(const TStrObjList& Params, TMacroData& E) {
  bool cs = Params.Count() < 3 ? true : Params[2].ToBool();
  E.SetRetVal(
    cs ? Params[0].Equals(Params[1]) : Params[0].Equalsi(Params[1]));
}

void LibStr::Like(const TStrObjList& Params, TMacroData& E) {
  bool cs = Params.Count() < 3 ? false : Params[2].ToBool();
  Wildcard wc(Params[1], cs);
  E.SetRetVal(wc.DoesMatch(Params[0]));
}

TLibrary* LibStr::ExportLibrary(const olxstr& name) {
  TLibrary* lib = new TLibrary(name.IsEmpty() ? olxstr("ostr") : name);
  lib->Register(
    new TStaticFunction(&LibStr::ToUpper, "toupper", fpOne,
      "Returns upper case version of the iput")
  );
  lib->Register(
    new TStaticFunction(&LibStr::ToLower, "tolower", fpOne,
      "Returns lower case version of the iput")
  );
  lib->Register(
    new TStaticFunction(&LibStr::Add, "add", fpTwo,
      "Returns concatenated string")
  );
  lib->Register(
    new TStaticFunction(&LibStr::Eq, "eq", fpTwo | fpThree,
      "Returns true if strings are equal."
      " Case sensitive by default. Third bool parameter can cnotrol it.")
  );
  lib->Register(
    new TStaticFunction(&LibStr::Like, "like", fpTwo | fpThree,
      "Returns true if the first argument matches the wildcard."
      " Case insenstitive by default. Third bool parameter can cnotrol it.")
  );
  return lib;
}
