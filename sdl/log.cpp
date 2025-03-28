/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "log.h"
#include "etime.h"

TLog::TLog()
  : verbose(false), auto_flush(false),
  disabled(0),
  OnInfo(Actions.New("ONINF")),
  OnWarning(Actions.New("ONWRN")),
  OnError(Actions.New("ONERR")),
  OnException(Actions.New("ONEXC")),
  OnPost(Actions.New("ONPOST"))
  {}
//..............................................................................
TLog::~TLog() {
  for (size_t i = 0; i < Streams.Count(); i++) {
    if (Streams[i].GetB()) {
      delete Streams[i].GetA();
    }
  }
}
//..............................................................................
size_t TLog::Write(const void* Data, size_t size) {
  throw TNotImplementedException(__OlxSourceInfo);
}
//..............................................................................
//..............................................................................
//..............................................................................
TLog::LogEntry::LogEntry(TLog& _parent, int _evt, bool annotate,
  const olxstr &location)
  : parent(_parent), evt(_evt)
{
  if (annotate) {
    if (!location.IsEmpty()) {
      buffer << "New log entry at [" << location <<  "]: "<<
        TETime::FormatDateTime(TETime::Now()) << NewLineSequence();
    }
    else {
      buffer << "New log entry at: " << TETime::FormatDateTime(TETime::Now()) <<
        NewLineSequence();
    }
  }
  else if (!location.IsEmpty()) {
    buffer << "At [" << location <<  "]: " << NewLineSequence();
  }
}
//..............................................................................
TLog::LogEntry::~LogEntry() {
  volatile olx_scope_cs cs_(parent.GetCriticalSection());

  if (evt == logVerbose && !parent.IsVerbose()) {
    return;
  }
  buffer << NewLineSequence();
  olxstr res = buffer;
  parent.OnPost.Execute(&parent, &res);
  if (evt == logDefault) {
    parent << res;
  }
  else {
    TActionQueue* ac = 0;
    switch (evt) {
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
    case logExceptionTrace:
      ac = &parent.OnException;
      break;
    default:
      ac = &parent.OnInfo;
      break;
    }
    if (!ac->Enter(&parent, &res)) {
      parent << res;
    }
    ac->Exit(&parent);
  }
}
//..............................................................................
TLog::LogEntry& TLog::LogEntry::operator << (const TExceptionBase &e) {
  if (evt == logExceptionTrace) {
    TStrList l;
    e.GetException()->GetStackTrace(l);
    buffer << l.Text(NewLineSequence());
  }
  else {
    buffer << e.GetException()->GetFullMessage();
  }
  return *this;
}
//..............................................................................
