//---------------------------------------------------------------------------//
// get a response
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#ifdef __BORLANDC__
  #pragma hdrstop
#endif
#include "log.h"
#include "efile.h"
#include "actions.h"
// using cin is very hevy - size increases drastically!
//#include <iostream>
#include <stdio.h>

UseEsdlNamespace()
//----------------------------------------------------------------------------//
//TLog class implementation
//----------------------------------------------------------------------------//
TLog::TLog()  {
  FActions = new TActionQList;
  OnInfo    = &FActions->NewQueue("ONINF");
  OnWarning = &FActions->NewQueue("ONWRN");
  OnError   = &FActions->NewQueue("ONERR");
  OnException = &FActions->NewQueue("ONEXC");
}
//..............................................................................
TLog::~TLog()  {
  for( int i=0; i < Streams.Count(); i++ )
    if( Streams[i].GetB() )
      delete Streams[i].GetA();
  delete FActions;
}
//..............................................................................
size_t TLog::Write(const void *Data, size_t size)  {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
size_t TLog::Writenl(const void *Data, size_t size)  {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................

