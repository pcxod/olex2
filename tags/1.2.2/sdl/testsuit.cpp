/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "testsuit.h"
#include "bapp.h"

void OlxTests::run()  {
  int failed_cnt = 0;
  for( size_t i=0; i < tests.Count(); i++ )  {
    try  { 
      description.SetLength(0);
      tests[i].run(*this);
      if (description.IsEmpty())
        description = "Test container...";
      TBasicApp::NewLogEntry() << "Running test " << i+1 << '/' <<
        tests.Count() << ": " << description;
      TBasicApp::NewLogEntry() << "Done";
    }
    catch( TExceptionBase& exc )  {
      TBasicApp::NewLogEntry() << "Running test " << i+1 << '/' <<
        tests.Count() << ": " << description;
      TBasicApp::NewLogEntry(logError) << exc.GetException()->GetFullMessage();
      TBasicApp::NewLogEntry() << "Failed";
      failed_cnt++;
    }
  }
  if( failed_cnt != 0 )
    TBasicApp::NewLogEntry() << failed_cnt << '/' << tests.Count() << " have failed";
}
