/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "eprocess.h"
#include "bapp.h"
#include "log.h"
#include "efile.h"
#include "egc.h"
#include "emath.h"

const int ID_Timer = 1;
//.............................................................................
AProcess::AProcess(bool use_threads, const olxstr& cmdl, short flags) :
  Flags(flags),
  CmdLine(cmdl),
  Output(use_threads), OnTerminate(Actions.New("ONTERMINATE"))
{
  ProcessId = -1;
  DubOutput = NULL;
  if (IsRedirected())  // must be async
    olx_set_bit(false, Flags, spfSynchronised);
}
//.............................................................................
AProcess::~AProcess() {
  if (DubOutput != NULL)
    delete DubOutput;
}
//.............................................................................
olxstr AProcess::PrepareArg(const olxstr &p) {
  using namespace exparse::parser_util;
  if (is_quoted(p)) return p;
  bool has_ws = false;
  for (size_t i=0; i < p.Length(); i++) {
    if (is_quote(p.CharAt(i))) {
      skip_string(p, i);
      continue;
    }
    if (p.CharAt(i) == '=') {
      if (has_ws) break;
      olxstr argv = p.SubStringFrom(i+1);
      if (is_quoted(argv)) return p;
      size_t spi = argv.IndexOf(' ');
      if (spi != InvalidIndex)
        return olxstr(p.SubStringTo(i+1)).quote('"') << argv;
      return p;
    }
    else if (p.CharAt(i) == ' ')
      has_ws = true;
  }
  if (has_ws)
    return olxstr().quote('"') << p;
  return p;
}
//.............................................................................

#ifdef __WIN32__
//---------------------------------------------------------------------------//
// TWinProcess Def
//---------------------------------------------------------------------------//
TWinProcess::TWinProcess(const olxstr& cmdl, short flags)
  : AProcess(false, cmdl, flags)
{
  InWrite = OutRead = NULL;
  OutWrite = ErrWrite = InRead = NULL;
  ProcessInfo.hProcess = NULL;
  CallsWasted = 0;
}
//.............................................................................
TWinProcess::~TWinProcess() {
  volatile olx_scope_cs cs(TBasicApp::GetCriticalSection());
  if (IsTerminateOnDelete())
    Terminate();
  if (TBasicApp::HasInstance())
    TBasicApp::GetInstance().OnTimer.Remove(this);
  CloseThreadStreams();
  CloseStreams();
  if (ProcessInfo.hProcess != NULL)
    CloseHandle(ProcessInfo.hProcess);
}
//.............................................................................
bool TWinProcess::InitStreams() {
  HANDLE  TmpOutRead = NULL;  // parent stdout read handle
  HANDLE TmpInWrite = NULL;  // parent stdin write handle
  CloseThreadStreams(); // just in case
  SECURITY_ATTRIBUTES sa;

  sa.nLength = sizeof(SECURITY_ATTRIBUTES);
  sa.lpSecurityDescriptor = NULL;
  sa.bInheritHandle = true;

  if (!CreatePipe(&TmpOutRead, &OutWrite, &sa, 0)) // Create a child stdout pipe
    return false;

   // Create a duplicate of the stdout write handle for the std
  if (!DuplicateHandle( GetCurrentProcess(), OutWrite, GetCurrentProcess(),
        &ErrWrite,0, true, DUPLICATE_SAME_ACCESS))
  {
    return false;
  }
  // Create a child stdin pipe.
  if (!CreatePipe(&InRead, &TmpInWrite, &sa, 0))
    return false;

  // Create new stdout read handle and the stdin write handle.
  // Set the inheritance properties to FALSE. Otherwise, the child
  // inherits these handles; resulting in non-closeable
  // handles to the pipes being created.
  if (!DuplicateHandle(GetCurrentProcess(), TmpOutRead, GetCurrentProcess(),
        &OutRead,  0, false,  // make it uninheritable.
        DUPLICATE_SAME_ACCESS))
  {
    return false;
  }
  if (!DuplicateHandle(GetCurrentProcess(), TmpInWrite, GetCurrentProcess(),
        &InWrite,  0, false,  // make it uninheritable.
        DUPLICATE_SAME_ACCESS))
  {
    return false;
  }
  // Close inheritable copies of the handles we do not want to
  // be inherited.
  CloseHandle(TmpOutRead);  TmpOutRead = NULL;
  CloseHandle(TmpInWrite);  TmpInWrite = NULL;

  SetHandleInformation( OutRead, HANDLE_FLAG_INHERIT, 0);
  SetHandleInformation( InWrite, HANDLE_FLAG_INHERIT, 0);
  return true;
}
//.............................................................................
void TWinProcess::CloseThreadStreams()  {
 if (InRead != NULL)    { CloseHandle(InRead);    InRead = NULL; }
 if (OutWrite != NULL)  { CloseHandle(OutWrite);  OutWrite = NULL; }
 if (ErrWrite != NULL)  { CloseHandle(ErrWrite);  ErrWrite = NULL; }
}
//.............................................................................
void TWinProcess::CloseStreams()  {
 if (OutRead != NULL)  { CloseHandle(OutRead);   OutRead = NULL; }
 if (InWrite != NULL)  { CloseHandle(InWrite); InWrite = NULL; }
}
//.............................................................................
bool TWinProcess::Execute() {
  if (IsRedirected()) {  // must be async
    if (!InitStreams())
      return false;
  }
  STARTUPINFO si = {0};
  si.cb = sizeof(STARTUPINFO);
  if (IsRedirected()) {
    si.hStdOutput = OutWrite;
    si.hStdInput = InRead;
    si.hStdError = ErrWrite;
  }
  if (!IsSynchronised() && IsRedirected())
    si.wShowWindow = SW_HIDE;
  else
    si.wShowWindow = SW_SHOW;

  si.dwFlags = STARTF_USESHOWWINDOW;
  if (IsRedirected())
    si.dwFlags |= STARTF_USESTDHANDLES;

  // Launch the child process.
  if (!CreateProcess(NULL, (LPTSTR)GetCmdLine().u_str(), NULL, NULL, true,
        CREATE_NEW_CONSOLE|CREATE_SEPARATE_WOW_VDM, NULL, NULL, &si,
        &ProcessInfo))
  {
    CloseStreams();
    return false;
  }
  CloseHandle(ProcessInfo.hThread);  // Close any unuseful handles

  ProcessId = ProcessInfo.dwProcessId;
  CloseThreadStreams();
  // Launch a thread to receive output from the child process.
  if (!IsSynchronised())
    TBasicApp::GetInstance().OnTimer.Add(this, ID_Timer);

  if (IsSynchronised()) {
    DWORD Status;
    GetExitCodeProcess(ProcessInfo.hProcess, &Status);
    while (Status == STILL_ACTIVE) {
      SleepEx(50, TRUE);
      GetExitCodeProcess(ProcessInfo.hProcess, &Status);
    }
    SetTerminated();
  }
  return true;
}
//.............................................................................
bool TWinProcess::Terminate() {
  volatile olx_scope_cs cs(TBasicApp::GetCriticalSection());
  if (TBasicApp::HasInstance())
    TBasicApp::GetInstance().OnTimer.Remove(this);
  SetTerminated();
  CloseStreams();
  OnTerminate.Execute((AEventsDispatcher*)this, NULL);
  if (ProcessInfo.hProcess != NULL) {
    bool res = TerminateProcess(ProcessInfo.hProcess, 0) != 0;
    CloseHandle(ProcessInfo.hProcess);
    ProcessInfo.hProcess = NULL;
    return res;
  }
  return true;
}
//.............................................................................
// write data to the child's stdin
void TWinProcess::Write(const olxstr &Str) {
  if (InWrite == NULL) return;
  DWORD Written;
  WriteFile(InWrite, Str.c_str(), Str.Length(), &Written, NULL);
  GetOutput().Write(Str.c_str(), Str.Length());
}
//.............................................................................
void TWinProcess::Writenl() {
  if (InWrite == NULL) return;
  DWORD Written;
  WriteFile(InWrite, "\n", 1, &Written, NULL);
  GetOutput().Write('\n');
}
//.............................................................................
bool TWinProcess::Dispatch(int MsgId, short MsgSubId, const IOlxObject *Sender,
  const IOlxObject *Data, TActionQueue *)
{
  bool Terminated = false;
  if (MsgId == ID_Timer) {
    if (IsRedirected()) {
      DWORD dwAvail = 0;
      const int BfC=512;
      olx_array_ptr<char> Bf(new char[BfC]);
      DWORD dwRead = 0;
      if (PeekNamedPipe(OutRead, NULL, 0, NULL, &dwAvail, NULL) == 0) // error
        Terminated = true;
      else if (dwAvail == 0) {  // no data
        CallsWasted++;
      }
      else  {
        CallsWasted = 0;
        dwRead = 0;
        if (ReadFile(OutRead, Bf(), olx_min(BfC-1, dwAvail), &dwRead, NULL) == 0
            || dwRead == 0)  // error, the child might ended
        {
          Terminated = true;
        }
        else {
          Output.Write(Bf, dwRead);
          if (GetDubStream() != NULL)
            GetDubStream()->Write(Bf, dwRead);
        }
      }
    }
    else {  // just check if still valid
      DWORD Status;
      if (GetExitCodeProcess(ProcessInfo.hProcess, &Status) == 0 ||
          Status != STILL_ACTIVE)
      {
        Terminated = true;
      }
    }
  }
  // Win98 fix... as PeekNamedPipe does not fail after the process is terminate
  if (CallsWasted > 15) {
    unsigned long pec = 0;
    if (!Terminated && GetExitCodeProcess(ProcessInfo.hProcess, &pec) != 0) {
      if (pec != STILL_ACTIVE)
        Terminated = true;
    }
    CallsWasted = 0;
  }
  //
  if (Terminated) {
    CloseHandle(ProcessInfo.hProcess);
    ProcessInfo.hProcess = NULL;
    Terminate();
    CallsWasted = 0;
  }
  return true;
}
void TWinProcess::Detach() {
  TEGC::Add((AEventsDispatcher*)this);
}
//.............................................................................
//---------------------------------------------------------------------------//
// TWinWinCmd implementation
//---------------------------------------------------------------------------//
bool TWinWinCmd::SendWindowCmd(const olxstr& WndName, const olxstr& Cmd) {
  HWND Window = FindWindow(WndName.u_str(), NULL);
  int TO=50;
  if (Window == NULL) {
    SleepEx(20, FALSE);
    Window = FindWindow(WndName.u_str(), NULL);
    if (--TO < 0)
      return false;
  }
  for (size_t i=0; i < Cmd.Length(); i++)
    PostMessage(Window, WM_CHAR, Cmd[i], 0);
  return true;
}
#endif  // for __WIN32__
#ifdef __WXWIDGETS__
//.............................................................................
//---------------------------------------------------------------------------//
// TWxProcess implementation
//---------------------------------------------------------------------------//
TWxProcess::TWxProcess(const olxstr& cmdl, short flags)
  : AProcess(true, cmdl, flags)
{}
//.............................................................................
TWxProcess::~TWxProcess() {
  if (TBasicApp::HasInstance()) {
    TBasicApp::GetInstance().OnTimer.Remove(this);
  }
  if (IsTerminateOnDelete()) {
    Terminate();
  }
}
//.............................................................................
bool TWxProcess::Terminate() {
  SetTerminated();
  if (TBasicApp::HasInstance()) {
    TBasicApp::GetInstance().OnTimer.Remove(this);
  }
  AProcess::OnTerminate.Execute((AProcess *)this);
  if (ProcessId >= 0) {
    int pid = ProcessId;
    ProcessId = -1;
    return wxKill(pid, wxSIGKILL) == 0;
    // the function should trigger the OnTerminate function
  }
  return false;
}
//.............................................................................
void TWxProcess::Detach() {
  AProcess::OnTerminate.Clear();
  wxProcess::Detach();
  // according to docs, we should not delete the object ?
  TEGC::AddP((AProcess *)this);
}
//.............................................................................
bool TWxProcess::Execute() {
  if (GetCmdLine().IsEmpty()) {
    return false;
  }
  if (AProcess::IsRedirected()) {
    Redirect();
  }
  if (AProcess::IsRedirected()) {  // must be async
    TStrList toks;
    TParamList::StrtokParams(GetCmdLine(), ' ', toks);
#ifdef __WIN32__
    toks[0] = TEFile::ChangeFileExt(toks[0], "exe");
#endif
    olxstr cmd( TEFile::Which(TEFile::ExtractFileName(toks[0])) );
    if (toks.Count() > 1) {
      for (size_t i=1; i < toks.Count(); i++) {
        if (!toks[i].Contains(' ')) {
          cmd << ' ' << toks[i];
        }
        else {
          cmd << " \'" << toks[i] << '\'';
        }
      }
    }
    ProcessId = wxExecute( cmd.u_str(), wxEXEC_ASYNC, this);
    if (ProcessId <= 0) {
      TBasicApp::NewLogEntry(logError) << "Failed to execute '" << cmd << '\'';
      return false;
    }
  }
  else {
    if (IsSynchronised()) {
      ProcessId = wxExecute(GetCmdLine().u_str(), wxEXEC_SYNC, this);
      if (ProcessId == -1) {
        return false;
      }
      return true;
    }
    else {
      wxExecute(GetCmdLine().u_str(), wxEXEC_ASYNC, this);
      SetTerminated();
    }
  }
  if (!IsSynchronised()) {
    TBasicApp::GetInstance().OnTimer.Add(this, ID_Timer);
  }
  return true;
}
//.............................................................................
bool TWxProcess::Dispatch(int MsgId, short MsgSubId, const IOlxObject *Sender,
  const IOlxObject *Data, TActionQueue *q)
{
  bool Terminated = false;
  if (MsgId == ID_Timer) {
    wxInputStream *in = wxProcess::GetInputStream();
    if (in != 0) {
      const int BfC=512;
      olx_array_ptr<char> Bf(new char[BfC]);
      if (in->CanRead()) {
        if (in->Read(Bf(), BfC).Eof()) {
          Terminated = true;
        }
        size_t lr = in->LastRead();
         Output.Write(Bf, lr);
         if (GetDubStream() != 0) {
           GetDubStream()->Write(Bf(), lr);
         }
      }
    }
    else {  // just check if still valid
      Terminated = !wxProcess::Exists(ProcessId);
    }
  }
  if (Terminated) {
    if (TBasicApp::HasInstance()) {
      TBasicApp::GetInstance().OnTimer.Remove(this);
    }
  }
  return true;
}
//.............................................................................
void TWxProcess::OnTerminate(int pid, int status) {
  if (ProcessId == pid) {
    // make sure that nothing is discarded!
    Dispatch(ID_Timer, 0, 0, 0, 0);
    ProcessId = -1;
    Terminate();
  }
}
//.............................................................................
void TWxProcess::Write(const olxstr &Cmd) {
  wxOutputStream *out = wxProcess::GetOutputStream();
  if (out == 0) {
    return;
  }
  out->Write(Cmd.c_str(), Cmd.Length());
  Output.Write(Cmd.c_str(), Cmd.Length());
}
//.............................................................................
void TWxProcess::Writenl()  {
  wxOutputStream *out = wxProcess::GetOutputStream();
  if (out == 0) {
    return;
  }
  out->PutC('\n');
  Output.Write('\n');
}
//.............................................................................
#endif  // for __WXWIDGETS__
