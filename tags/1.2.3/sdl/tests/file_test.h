/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "efile.h"

namespace test {

void RelativePathTest(OlxTests& t)  {
  t.description = __FUNC__;
  olxstr base1="c:\\windows\\test",
         base2="c:\\windows\\drivers\\test",
         base3="/tmp/test";
  olxstr path1="c:\\windows\\sys32",
         path2="c:\\test",
         path3="/tmp/test/test/tmp";
  olxstr rel1=TEFile::CreateRelativePath(path1,base1),
         rel2=TEFile::CreateRelativePath(path2,base1),
         rel3=TEFile::CreateRelativePath(path1,base2),
         rel4=TEFile::CreateRelativePath(path2,base2),
         rel5=TEFile::CreateRelativePath(path3,base3);
  if( TEFile::WinPath(rel1) != "..\\sys32" )
    throw TFunctionFailedException(__OlxSourceInfo, "failed to compose relative path");
  if( TEFile::WinPath(rel2) != "..\\..\\test" )
    throw TFunctionFailedException(__OlxSourceInfo, "failed to compose relative path");
  if( TEFile::WinPath(rel3) != "..\\..\\sys32" )
    throw TFunctionFailedException(__OlxSourceInfo, "failed to compose relative path");
  if( TEFile::WinPath(rel4) != "..\\..\\..\\test" )
    throw TFunctionFailedException(__OlxSourceInfo, "failed to compose relative path");
  if( TEFile::UnixPath(rel5) != "./test/tmp" )
    throw TFunctionFailedException(__OlxSourceInfo, "failed to compose relative path");
  if( TEFile::WinPath(TEFile::CreateRelativePath(path2,"d:\\test")) != path2 )
    throw TFunctionFailedException(__OlxSourceInfo, "failed to compose relative path");

  if( TEFile::WinPath(TEFile::ExpandRelativePath(rel1,base1)) != path1 )
    throw TFunctionFailedException(__OlxSourceInfo, "failed to expand relative path");
  if( TEFile::WinPath(TEFile::ExpandRelativePath(rel2,base1)) != path2 )
    throw TFunctionFailedException(__OlxSourceInfo, "failed to expand relative path");
  if( TEFile::WinPath(TEFile::ExpandRelativePath(rel3,base2)) != path1 )
    throw TFunctionFailedException(__OlxSourceInfo, "failed to expand relative path");
  if( TEFile::WinPath(TEFile::ExpandRelativePath(rel4,base2)) != path2 )
    throw TFunctionFailedException(__OlxSourceInfo, "failed to expand relative path");
  if( TEFile::UnixPath(TEFile::ExpandRelativePath(rel5,base3)) != path3 )
    throw TFunctionFailedException(__OlxSourceInfo, "failed to expand relative path");
}
//...................................................................................................
void PathQuoteTest(OlxTests& t)  {
  t.description = __FUNC__;
  char *paths[] = {
    "x", "x",
    "x\\", "x\\",
    "x\"", "\"x\\\"\"",
    "x\t", "\"x\t\"",
    "x y", "\"x y\"",
    "x \n\"", "\"x \n\\\"\"",
    "C:\\program files\\", "\"C:\\program files\\\\\"",
    NULL
  };

  int i=0;
  while (paths[i] != NULL) {
    if (TEFile::QuotePath(paths[i]) != paths[i+1])
      throw TFunctionFailedException(__OlxSourceInfo, "path quoting failed");
    if (TEFile::UnquotePath(paths[i+1]) != paths[i])
      throw TFunctionFailedException(__OlxSourceInfo, "path unquoting failed");
    i+=2;
  }
}
void FileTests(OlxTests &t) {
  t.Add(&RelativePathTest)
    .Add(&PathQuoteTest);
}
};  //namespace test
