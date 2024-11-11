/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_log_H
#define __olx_sdl_log_H
#include "talist.h"
#include "egc.h"
#include "datastream.h"
#include "actions.h"
#include "estrlist.h"
#include "os_util.h"
// convenience macro
#define OLX_DISABLE_LOGGING()\
 volatile TLog::Disabler olx_log_disabler = TBasicApp::GetInstance().GetLog().Disable()
BeginEsdlNamespace()

enum  {
  logDefault,
  logVerbose, // as logInfo but may be discarded if TLog is not verbose
  logInfo,
  logWarning,
  logError,
  logException,
  logExceptionTrace
};
class TLog : public IOlxObject, public IDataOutputStream {
  // stream, to delete at the end
  TArrayList<olx_pair_t<IDataOutputStream*, bool> > Streams;
  TActionQList Actions;
  bool verbose, auto_flush;
  int disabled;
protected:
  virtual size_t Write(const void* Data, size_t size);
  virtual uint64_t GetSize() const { return 0; }
  virtual uint64_t GetPosition() const { return 0; }
  virtual void SetPosition(uint64_t) {}

  void Add(const olxstr& str) {
    for (size_t i = 0; i < Streams.Count(); i++) {
      Streams[i].a->Writeln(disabled != 0 ? EmptyString() : str);
    }
  }
  template <class List>
  void Add(const List& lst) {
    for (size_t i = 0; i < lst.Count(); i++) {
      for (size_t j = 0; j < Streams.Count(); j++) {
        Streams[j].a->Writeln(disabled != 0 ? EmptyString() : lst[i]);
      }
    }
  }
public:
  TLog();
  virtual ~TLog();

  bool IsVerbose() const { return verbose; }
  void SetVerbose(bool v) { verbose = v; }

  bool GetAutoFlush() const { return auto_flush; }
  void SetAutoFlush(bool v) { auto_flush = v; }
  // if own is true, the object will be deleted in the end
  void AddStream(IDataOutputStream* stream, bool own) {
    Streams.Add(olx_pair_t<IDataOutputStream*, bool>(stream, own));
  }
  void RemoveStream(IDataOutputStream* stream) {
    for (size_t i = 0; i < Streams.Count(); i++)
      if (Streams[i].GetA() == stream) {
        Streams.Delete(i);
        return;
      }
  }
  // use this operators for unconditional 'printing'
  TLog& operator << (const olxstr& str) {
    for (size_t i = 0; i < Streams.Count(); i++) {
      Streams[i].a->Write(disabled != 0 ? EmptyString() : str);
    }
    if (auto_flush) {
      Flush();
    }
    return *this;
  }
  template <class T>
  TLog& operator << (const TTStrList<T>& lst) {
    Add(lst);
    if (auto_flush) {
      Flush();
    }
    return *this;
  }
  //..............................................................................
  virtual void Flush() {
    for (size_t i = 0; i < Streams.Count(); i++) {
      Streams[i].GetA()->Flush();
    }
  }
  static olx_critical_section& GetCriticalSection() {
    static olx_critical_section cs;
    return cs;
  }

  //..............................................................................
  struct LogEntry {
    TLog& parent;
    olxstr_buf buffer;
    int evt;
    LogEntry(TLog& _parent, int evt, bool annotate, const olxstr& location);
    ~LogEntry();
    LogEntry& operator << (const olxstr& str) {
      buffer << str;
      return *this;
    }
    LogEntry& operator << (const TExceptionBase& e);
    LogEntry& nl() {
      buffer << NewLineSequence();
      return *this;
    }
    template <class T>
    LogEntry& operator << (const TTStrList<T>& lst) {
      buffer << lst.Text(NewLineSequence());
      return *this;
    }
    template <class T>
    LogEntry& operator << (const ConstStrList<T>& lst) {
      buffer << lst.Text(NewLineSequence());
      return *this;
    }
    LogEntry& operator << (const IOlxObject& o) {
      buffer << o.ToString();
      return *this;
    }
  };
  //..............................................................................
  LogEntry NewEntry(int evt = logDefault, bool annotate = false,
    const olxstr& location = EmptyString())
  {
    return LogEntry(*this, evt, annotate, location);
  }
  //..............................................................................
  struct Disabler {
  private:
    TLog& parent;
  public:
    Disabler(TLog& parent)
      : parent(parent)
    {
      parent.disabled++;
    }
    ~Disabler() {
      parent.disabled--;
    }
  };
  //..............................................................................
  Disabler Disable() {
    return Disabler(*this);
  }
  //..............................................................................
  TActionQueue& OnInfo;
  TActionQueue& OnWarning;
  TActionQueue& OnError;
  TActionQueue& OnException;
  TActionQueue& OnPost;
};

EndEsdlNamespace()
#endif
