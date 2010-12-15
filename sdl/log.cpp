//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#include "log.h"
#include "etime.h"

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
//..............................................................................
//..............................................................................
TLog::LogEntry::LogEntry(TLog& _parent, int _evt, bool annotate) : parent(_parent), evt(_evt)  {
  if( annotate )
    buffer << "New log entry at: " << TETime::FormatDateTime(TETime::Now()) << NewLineSequence;
}
//..............................................................................
TLog::LogEntry::~LogEntry()  {
  buffer << NewLineSequence;
  if( evt == logDefault )  {
    parent << buffer;
    //parent.Flush();
  }
  else  {
    TActionQueue* ac = NULL;
    switch( evt )  {
    case logInfo: 
      ac = &parent.OnInfo;
      break;
    case logWarning: 
      ac = &parent.OnWarning;
      break;
    case logError: 
      ac = &parent.OnError;
      break;
    case logException: 
      ac = &parent.OnException;
      break;
    default: 
      ac = &parent.OnInfo;
      break;
    }
    if( !ac->Enter(&parent, &buffer) )  {
      parent << buffer;
      //parent.Flush();
    }
    ac->Exit(&parent);
  }
}
//..............................................................................

