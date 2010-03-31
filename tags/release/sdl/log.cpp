//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#include "log.h"

TLog::TLog() : 
  OnInfo(Actions.New("ONINF")),
  OnWarning(Actions.New("ONWRN")),
  OnError(Actions.New("ONERR")),
  OnException(Actions.New("ONEXC"))  {}
//..............................................................................
TLog::~TLog()  {
  for( size_t i=0; i < Streams.Count(); i++ )
    if( Streams[i].GetB() )
      delete Streams[i].GetA();
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

