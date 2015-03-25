/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_process_H
#define __olx_process_H
#include "actions.h"
#include "estrlist.h"
#include "edlist.h"
#include "os_util.h"
#include "sortedlist.h"
#include "bapp.h"

#ifdef __WXWIDGETS__
  #include "wx/process.h"
#endif

/* Windows specific implementation is inspired by the Python source code */

#ifdef __WIN32__
  #include <winuser.h>
#endif

const short  // process flags
  spfRedirected        = 0x0001,
  spfSynchronised      = 0x0002,
  spfTerminateOnDelete = 0x0004,
  spfQuiet             = 0x0008,
  spfTerminated        = 0x0010;

enum {
  process_manager_timer,
  process_manager_kill
};

class IProcessOutput {
public:
  virtual ~IProcessOutput() {}
  virtual void write_char(char ch) = 0;
  virtual void write_array(const char* bf, size_t len) = 0;
  virtual olxstr readAll() = 0;
  virtual bool isEmpty() const = 0;
};

class ProcessOutput : public IProcessOutput {
protected:
  TDirectionalList<char> data;
public:
  ProcessOutput()  {}
  void write_char(char ch)  {  data.Write(ch);  }
  void write_array(const char* bf, size_t len)  {  data.Write(bf, len);  }
  bool isEmpty() const {  return data.IsEmpty();  }
  olxstr readAll() {
    olxstr rv;
    data.ToString(rv);
    data.Clear();
    return rv;
  }
};

class SynchronisedProcessOutput : public IProcessOutput {
protected:
  TDirectionalList<char> data;
  mutable olx_critical_section cs;
public:
  SynchronisedProcessOutput() {}
  void write_char(char ch)  {
    volatile olx_scope_cs _cs(cs);
    data.Write(ch);
  }
  void write_array(const char* bf, size_t len)  {
    volatile olx_scope_cs _cs(cs);
    data.Write(bf, len);
  }
  bool isEmpty() const {
    volatile olx_scope_cs _cs(cs);
    return data.IsEmpty();
  }
  olxstr readAll() {
    volatile olx_scope_cs _cs(cs);
    olxstr rv;
    data.ToString(rv);
    data.Clear();
    return rv;
  }
};

class BufferedProcessOutput {
protected:
  IProcessOutput& data;
public:
  BufferedProcessOutput(bool thread_use) :
      data(*(thread_use ?
        (IProcessOutput*)(new SynchronisedProcessOutput)
        : (IProcessOutput*)(new ProcessOutput)))
  {}
  ~BufferedProcessOutput()  {  delete &data;  }
  void Write(char ch)  {  data.write_char(ch);  }
  void Write(const char* bf, size_t len)  {  data.write_array(bf, len);  }
  bool IsEmpty() const {  return data.isEmpty();  }
  olxstr ReadAll() {  return data.readAll();  }
};

class AProcess : public virtual IOlxObject {
private:
  TActionQList Actions;
  TStrList FOnTerminateCmds;
  IOutputStream* DubOutput;
  short Flags;
  olxstr CmdLine;
protected:
  int ProcessId; // must be initialised for async execution in the execute method
  bool IsTerminateOnDelete() const {  return (Flags & spfTerminateOnDelete) != 0;  }
  BufferedProcessOutput Output;
  void SetTerminated()  {  Flags |= spfTerminated;  }
public:
  AProcess(bool use_threads, const olxstr& cmdl, short flags);
  virtual ~AProcess();

  bool IsSynchronised() const {  return (Flags & spfSynchronised) != 0;  }
  bool IsRedirected() const {  return (Flags & spfRedirected) != 0;  }
  bool IsOutputDub() const {  return (Flags & spfRedirected) != 0;  }
  bool IsQuite() const {  return (Flags & spfQuiet) != 0;  }
  bool IsTerminated() const {  return (Flags & spfTerminated) != 0;  }

  const BufferedProcessOutput& GetOutput() const {  return Output;  }
  BufferedProcessOutput& GetOutput()  {  return Output;  }

  const TStrList& OnTerminateCmds() const {  return FOnTerminateCmds;  }
  void SetOnTerminateCmds(const TStrList& l)  {  FOnTerminateCmds.Assign(l);  }
  int GetProcessId() const {  return ProcessId;  }
  const olxstr&  GetCmdLine() const {  return CmdLine;  }
  IOutputStream* GetDubStream() const {  return DubOutput;  }
  // sets stream to dublicate process output, will be deleted
  void SetDubStream(IOutputStream* v) {  DubOutput = v;  }

  virtual void Write(const olxstr& Cmd)=0; // writes to the process output
  virtual void Writenl() = 0; // writes to the process output
  virtual bool Terminate() = 0;
  virtual bool Execute() = 0;
  virtual void Detach() = 0;

  static olxstr PrepareArg(const olxstr &p);

  TActionQueue& OnTerminate;
};

#ifdef __WXWIDGETS__
class TWxProcess
  : protected wxProcess, public AProcess, public AEventsDispatcher
{
  bool Dispatch(int MsgId, short MsgSubId, const IOlxObject *Sender,
    const IOlxObject *Data, TActionQueue *);
  // override to stop automatic deletion of the object
  virtual void OnTerminate(int pid, int status);
public:
  TWxProcess(const olxstr& cmdl, short flags);
  ~TWxProcess();

  virtual void Write(const olxstr& Cmd);
  virtual void Writenl();
  virtual bool Execute();
  virtual bool Terminate();
  virtual void Detach();
};
#endif // ! __WXWIDGETS__

#ifdef __WIN32__
class TWinProcess: public AEventsDispatcher, public AProcess {
private:
protected:
  size_t CallsWasted;
  PROCESS_INFORMATION ProcessInfo;
  HANDLE OutRead, InWrite;
  HANDLE InRead, OutWrite, ErrWrite;  // initialised by InitStreams func
  bool InitStreams();
  void CloseStreams();
  void CloseThreadStreams();
  bool Dispatch(int MsgId, short MsgSubId, const IOlxObject *Sender,
    const IOlxObject *Data, TActionQueue *);
public:
  TWinProcess(const olxstr& cmdl, short flags);
  virtual ~TWinProcess();

  virtual bool Execute();
  virtual void Write(const olxstr& Str);
  virtual void Writenl();
  virtual bool Terminate();
  virtual void Detach();
};

class TWinWinCmd  {
public:
  static bool SendWindowCmd(const olxstr& WndName, const olxstr& Cmd);
};
#endif // !__WIN32__

class ProcessManager : public AEventsDispatcher  {
public:
  class IProcessHandler  {
  public:
    virtual ~IProcessHandler() {}
    virtual void BeforePrint() {}
    virtual void Print(const olxstr& line) = 0;
    virtual void AfterPrint() {}
    virtual void OnWait() {}
    virtual void OnTerminate(const AProcess& p) = 0;
  };
private:
  /* there could be many processes running at the same time, however only one
   *  process a time can be redirected (i.e. get input) and only one process to
   *  wait for... */
  sorted::PointerPointer<AProcess> Processes;
  AProcess* Redirected, *Current, *Last;
protected:
  IProcessHandler& OutputHandler;
  virtual bool Dispatch(int MsgId, short MsgSubId, const IOlxObject* Sender,
  const IOlxObject* Data, TActionQueue *)
  {
    if( MsgSubId != msiExecute )  return false;
    if( MsgId == process_manager_timer )  {
      if( !Processes.IsEmpty() )  {
        OutputHandler.BeforePrint();
        for( size_t i=0; i < Processes.Count(); i++ )  {
          if( !Processes[i]->GetOutput().IsEmpty() )  {
            olxstr rv = Processes[i]->GetOutput().ReadAll();
            OutputHandler.Print(rv);
          }
        }
        OutputHandler.AfterPrint();
      }
    }
    else if( MsgId == process_manager_kill )  {
      OnTerminate(dynamic_cast<const AProcess&>(*Sender));
    }
    else
      return false;
    return true;
  }
public:
  ProcessManager(IProcessHandler& outputHandler)
  : Redirected(NULL), Current(NULL), Last(NULL),
    OutputHandler(outputHandler)
  {
    TBasicApp::GetInstance().OnTimer.Add(this, process_manager_timer);
  }
  //..............................................................................
  ~ProcessManager()  {
    TBasicApp::GetInstance().OnTimer.Remove(this);
    for( size_t i=0; i < Processes.Count(); i++ )  {
      Processes[i]->OnTerminate.Clear();
      Processes[i]->Terminate();
      delete Processes[i];
    }
  }
  void OnCreate(AProcess& Process)  {
    Process.OnTerminate.Add(this, process_manager_kill);
    while( Current != NULL && !Current->IsTerminated() )  {
      OutputHandler.OnWait();
      olx_sleep(50);
    }
    Current = NULL;
    Processes.Add(&Process);
    if( Process.IsRedirected() )
      Redirected = &Process;
    Last = &Process;
  }
  //..............................................................................
  void OnTerminate(const AProcess& _process)  {
    const size_t pi = Processes.IndexOf(&_process);
    if( pi == InvalidIndex )  // howm come?
      return;
    AProcess& Process = *Processes[pi];
    if( &Process == Redirected )  Redirected = NULL;
    if( &Process == Current )  Current = NULL;
    if( &Process == Last )  Last = NULL;
    if( !Process.GetOutput().IsEmpty() )  {
      OutputHandler.BeforePrint();
      const olxstr rv = Process.GetOutput().ReadAll();
      OutputHandler.Print(rv);
      OutputHandler.AfterPrint();
    }
    OutputHandler.OnTerminate(_process);
    TEGC::Add(Processes[pi]);
    Processes.Delete(pi);
  }
  //..............................................................................
  void WaitForLast()  {
    Current = Last;
    while( Current != NULL && !Current->IsTerminated() )  {
      OutputHandler.OnWait();
      olx_sleep(50);
    }
    Current = NULL;
  }
  //..............................................................................
  AProcess* GetCurrent() const {  return Current;  }
  AProcess* GetLast() const {  return Last;  }
  AProcess* GetRedirected() const {  return Redirected;  }
};

#endif
