/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "bapp.h"
#include "log.h"
#include "edict.h"
#include "outstream.h"
#include "ematrix.h"

#include "xapp.h"


int main(int argc, char** argv)  {
  TOutStream out;
  puts(argv[0]);
  try  {
    TXApp app(TBasicApp::GuessBaseDir(argv[0]));
    app.GetLog().AddStream(new TOutStream, true);
    TStrObjList args("C12H22O11 C:40.1 H:6 N:0 H2O CCl3H", ' ');
    TParamList params;
    TMacroError me;
    ABasicFunction* macFitCHN = app.GetLibrary().FindMacro("FitCHN");
    macFitCHN->Run(args, params, me);
  }
  catch(const TExceptionBase& e)  {
    out.Writeln(e.GetException()->GetError());
  }
}
