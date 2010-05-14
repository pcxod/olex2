#ifndef __olx_process_H
#define __olx_process_H
#include "actions.h"
#include "estrlist.h"
#include "edlist.h"
#include "os_util.h"

#ifdef __WXWIDGETS__
  #include "wx/process.h"
#endif

#ifdef __WIN32__
  #include <winuser.h>
#endif

const short  // process flags
  spfRedirected        = 0x0001,
  spfSynchronised      = 0x0002,
  spfTerminateOnDelete = 0x0004,
  spfQuite             = 0x0008,
  spfTerminated        = 0x0010;

class IProcessOutput  {
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
        (IProcessOutput*)(new SynchronisedProcessOutput) : (IProcessOutput*)(new ProcessOutput))) {}
  ~BufferedProcessOutput()  {  delete &data;  }
  void Write(char ch)  {  data.write_char(ch);  }
  void Write(const char* bf, size_t len)  {  data.write_array(bf, len);  }
  bool IsEmpty() const {  return data.isEmpty();  }
  olxstr ReadAll() {  return data.readAll();  }
};

class AProcess : public IEObject {
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
  bool IsQuite() const {  return (Flags & spfQuite) != 0;  }
  bool IsTerminated() const {  return (Flags & spfTerminated) != 0;  }

  const BufferedProcessOutput& GetOutput() const {  return Output;  }
  BufferedProcessOutput& GetOutput()  {  return Output;  }

  TStrList& OnTerminateCmds()  {  return FOnTerminateCmds;  }
  inline int GetProcessId() const {  return ProcessId;  }
  const olxstr&  GetCmdLine() const {  return CmdLine;  }
  IOutputStream* GetDubStream() const {  return DubOutput;  }
  // sets stream to dublicate process output, will be deleted
  void SetDubStream(IOutputStream* v) {  DubOutput = v;  }

  virtual void Write(const olxstr& Cmd)=0; // writes to the process output
  virtual void Writenl() = 0; // writes to the process output
  virtual bool Terminate() = 0;
  virtual bool Execute() = 0;
  virtual void Detach() = 0;

  TActionQueue& OnTerminate;
};

#ifdef __WXWIDGETS__
class TIStream : public wxThread  {
  wxInputStream *FStream;
  AProcess *FParent;
  bool Terminated;
public:
  TIStream(class TWxProcess *Parent, wxInputStream *IS);
  virtual ~TIStream();
  virtual void* Entry();
  void OnTerminate();
};

class TWxProcess : public wxProcess, public AProcess  {
  TIStream *FStream;
  wxInputStream *FInputStream;
  wxOutputStream *FOutputStream;
  wxInputStream *FErrorStream;
protected:
  virtual void OnTerminate(int pid, int status);
  bool InitStreams();
public:
  TWxProcess(const olxstr& cmdl, short flags);
  virtual ~TWxProcess();

  TIStream* GetStream()  {  return FStream; }

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
  bool Dispatch(int MsgId, short MsgSubId, const IEObject *Sender, const IEObject *Data=NULL);
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

#endif
