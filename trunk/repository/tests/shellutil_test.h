/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "../repository/shellutil.h"

namespace test {

void ArgQuoteTest(OlxTests& t)  {
  t.description = __FUNC__;
  if (TShellUtil::QuoteArg("x") != "x")
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  if (TShellUtil::QuoteArg("x\\") != "x\\")
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  if (TShellUtil::QuoteArg("x\"") != "\"x\\\"\"")
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  if (TShellUtil::QuoteArg("x\t") != "\"x\t\"")
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  if (TShellUtil::QuoteArg("x y") != "\"x y\"")
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  if (TShellUtil::QuoteArg("x \n\"") != "\"x \n\\\"\"")
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
  if (TShellUtil::QuoteArg("C:\\program files\\") != "\"C:\\program files\\\\\"")
    throw TFunctionFailedException(__OlxSourceInfo, "assert");
}

void ShellUtilTests(OlxTests& t) {
  t.Add(&ArgQuoteTest);
}
//...................................................................................................
};  //namespace test
