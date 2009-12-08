#ifndef __olx_process_H
#define __olx_process_H
#include "actions.h"
#include "evtypes.h"
#include "emath.h"

#ifdef __WXWIDGETS__
  #include "wx/process.h"
#endif

#ifdef __WIN32__
  #include <winuser.h>
#endif

const short   spfRedirected        = 0x0001,
              spfSynchronised      = 0x0002,
              spfTerminateOnDelete = 0x0004,
              spfQuite             = 0x0008;
class AProcess : public IEObject {
private:
  TStrList Output;
  TActionQList Actions;
  TStrList FOnTerminateCmds;
  IOutputStream* DubOutput;
protected:
  int ProcessId; // must be initialised for async execution in the execute method
  int Flags;
  olxstr CmdLine;
  bool IsTerminateOnDelete() const {  return (Flags & spfTerminateOnDelete) != 0;  }
  void InitData(const olxstr& cmdl, int flags)  {
    CmdLine = cmdl;
    Flags = flags;
  }
public:
  AProcess();
  virtual ~AProcess();

  bool IsSynchronised() const {  return (Flags & spfSynchronised) != 0;  }
  bool IsRedirected() const {  return (Flags & spfRedirected) != 0;  }
  bool IsOutputDub() const {  return (Flags & spfRedirected) != 0;  }
  bool IsQuite() const {  return (Flags & spfQuite) != 0;  }

  size_t StrCount() const {  return Output.Count();  }
  const olxstr& GetString(size_t i) const {  return Output[i];  }
  TStrList& GetOutput()  {  return Output;  }
  void DeleteStr(size_t i)  {  Output.Delete(i);  }
  void AddString(const olxstr& S)  {  
    if( !IsQuite() )  
      Output.Add(S);  
  }

  TStrList& OnTerminateCmds()  {  return FOnTerminateCmds;  }
  inline int GetProcessId() const {  return ProcessId;  }
  const olxstr&  GetCmdLine() const {  return CmdLine;  }
  IOutputStream* GetDubStream() const {  return DubOutput;  }
  // sets stream to dublicate process output, will be deleted
  void SetDubStream(IOutputStream* v) {  DubOutput = v;  }

  virtual void Write(const olxstr& Cmd)=0; // writes to the process output
  virtual void Writenl() = 0; // writes to the process output
  virtual bool Terminate() = 0;
  virtual bool Execute(const olxstr & Cmd, short Flags) = 0;
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
  TWxProcess();
  virtual ~TWxProcess();

  TIStream* GetStream()  {  return FStream; }

  virtual void Write(const olxstr &Cmd);
  virtual void Writenl();
  virtual bool Execute(const olxstr& Cmd, short Flags);
  virtual bool Terminate();
  virtual void Detach();
};
#endif // ! __WXWIDGETS__

#ifdef __WIN32__
class TWinProcess: public AEventsDispatcher, public AProcess {
private:
protected:
  PROCESS_INFORMATION ProcessInfo;
  HANDLE OutRead, InWrite;
  HANDLE InRead, OutWrite, ErrWrite;  // initialised by InitStreams func
  bool InitStreams();
  void CloseStreams();
  void CloseThreadStreams();
  bool Dispatch(int MsgId, short MsgSubId, const IEObject *Sender, const IEObject *Data=NULL);
//  void Execute
public:
  TWinProcess();
  virtual ~TWinProcess();

  virtual bool Execute(const olxstr& Cmd, short Flags);
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
