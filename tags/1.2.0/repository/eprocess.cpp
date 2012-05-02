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
//..............................................................................
AProcess::AProcess(bool use_threads, const olxstr& cmdl, short flags) : 
  Flags(flags), 
  CmdLine(cmdl),
  Output(use_threads), OnTerminate(Actions.New("ONTERMINATE"))
{
  ProcessId = -1;
  DubOutput = NULL;
  if( IsRedirected() )  // must be async
    olx_set_bit(false, Flags, spfSynchronised);
}
//..............................................................................
AProcess::~AProcess()  {
  if( DubOutput != NULL )
    delete DubOutput; 
}
//..............................................................................

#ifdef __WIN32__
//---------------------------------------------------------------------------//
// TWinProcess Def
//---------------------------------------------------------------------------//
TWinProcess::TWinProcess(const olxstr& cmdl, short flags) : AProcess(false, cmdl, flags) {
  InWrite = OutRead = NULL;
  OutWrite = ErrWrite = InRead = NULL;
  ProcessInfo.hProcess = NULL;
  CallsWasted = 0;
}
//..............................................................................
TWinProcess::~TWinProcess()  {
  volatile olx_scope_cs cs(TBasicApp::GetCriticalSection());
  if( IsTerminateOnDelete() )
    Terminate();
  if (TBasicApp::HasInstance())
    TBasicApp::GetInstance().OnTimer.Remove(this);
  CloseThreadStreams();
  CloseStreams();
  if( ProcessInfo.hProcess != NULL )
    CloseHandle(ProcessInfo.hProcess);
}
//..............................................................................
bool TWinProcess::InitStreams()  {
  HANDLE  TmpOutRead = NULL;  // parent stdout read handle
  HANDLE TmpInWrite = NULL;      // parent stdin write handle
  CloseThreadStreams(); // just in case
  SECURITY_ATTRIBUTES sa;

  sa.nLength = sizeof(SECURITY_ATTRIBUTES);
  sa.lpSecurityDescriptor = NULL;
  sa.bInheritHandle = true;

  if( !CreatePipe(&TmpOutRead, &OutWrite, &sa, 0)) // Create a child stdout pipe
    return false;
  
   // Create a duplicate of the stdout write handle for the std
  if( !DuplicateHandle( GetCurrentProcess(), OutWrite, GetCurrentProcess(),
                          &ErrWrite,0, true, DUPLICATE_SAME_ACCESS))
    return false;

  if( !CreatePipe(&InRead, &TmpInWrite, &sa, 0)) // Create a child stdin pipe.
    return false;

    // Create new stdout read handle and the stdin write handle.
    // Set the inheritance properties to FALSE. Otherwise, the child
    // inherits these handles; resulting in non-closeable
    // handles to the pipes being created.
  if( !DuplicateHandle( GetCurrentProcess(), TmpOutRead, GetCurrentProcess(),
        &OutRead,  0, false,      // make it uninheritable.
        DUPLICATE_SAME_ACCESS))
    return false;

  if( !DuplicateHandle( GetCurrentProcess(), TmpInWrite, GetCurrentProcess(),
        &InWrite,  0, false,      // make it uninheritable.
        DUPLICATE_SAME_ACCESS))
    return false;

    // Close inheritable copies of the handles we do not want to
    // be inherited.
  CloseHandle(TmpOutRead);  TmpOutRead = NULL;
  CloseHandle(TmpInWrite);  TmpInWrite = NULL;

  SetHandleInformation( OutRead, HANDLE_FLAG_INHERIT, 0);
  SetHandleInformation( InWrite, HANDLE_FLAG_INHERIT, 0);
  return true;
}
//..............................................................................
void TWinProcess::CloseThreadStreams()  {
 if( InRead )     { CloseHandle(InRead);    InRead = NULL; }
 if( OutWrite )   { CloseHandle(OutWrite);  OutWrite = NULL;   }
 if( ErrWrite )   { CloseHandle(ErrWrite);  ErrWrite = NULL;   }
}
//..............................................................................
void TWinProcess::CloseStreams()  {
 if( OutRead != NULL )  { CloseHandle(OutRead);   OutRead = NULL; }
 if( InWrite != NULL )  { CloseHandle(InWrite); InWrite = NULL;   }
}
//..............................................................................
bool TWinProcess::Execute()  {
  if( IsRedirected() )  {  // must be async
    if( !InitStreams() )  return false;
  }
  STARTUPINFO si;
  ZeroMemory(&si, sizeof(STARTUPINFO));
  si.cb = sizeof(STARTUPINFO);
  if( IsRedirected() )  {
    si.hStdOutput = OutWrite;
    si.hStdInput = InRead;
    si.hStdError = ErrWrite;
  }
  if( !IsSynchronised() && IsRedirected() )
    si.wShowWindow = SW_HIDE;
  else
    si.wShowWindow = SW_SHOW;

  si.dwFlags = STARTF_USESHOWWINDOW;
  if( IsRedirected() )
    si.dwFlags |= STARTF_USESTDHANDLES;

  // Launch the child process.
  if( !CreateProcess(NULL, (LPTSTR)GetCmdLine().u_str(), NULL, NULL,   true,
                      CREATE_NEW_CONSOLE|CREATE_SEPARATE_WOW_VDM, NULL, NULL, &si, &ProcessInfo))
  {
    CloseStreams();
    return false;
  }
  CloseHandle(ProcessInfo.hThread);  // Close any unuseful handles

  ProcessId = ProcessInfo.dwProcessId;
  // Child is launched. Close the parents copy of those pipe
  // handles that only the child should have open.
  // Make sure that no handles to the write end of the stdout pipe
  // are maintained in this process or else the pipe will not
  // close when the child process exits and ReadFile will hang.
  CloseThreadStreams();
  if( !IsSynchronised() ) // Launch a thread to receive output from the child process.
    TBasicApp::GetInstance().OnTimer.Add(this, ID_Timer);

  if( IsSynchronised() )  {
    DWORD Status;
    GetExitCodeProcess(ProcessInfo.hProcess, &Status);
    while( Status == STILL_ACTIVE )  {
      SleepEx(50, TRUE);
      GetExitCodeProcess(ProcessInfo.hProcess, &Status);
    }
    SetTerminated();
  }
  return true;
}
//..............................................................................
bool TWinProcess::Terminate()  {
  volatile olx_scope_cs cs(TBasicApp::GetCriticalSection());
  if (TBasicApp::HasInstance())
    TBasicApp::GetInstance().OnTimer.Remove(this);
  SetTerminated();
  CloseStreams();
  OnTerminate.Execute((AEventsDispatcher*)this, NULL);
  if( ProcessInfo.hProcess != NULL )  {
    bool res = TerminateProcess(ProcessInfo.hProcess, 0) != 0;
    CloseHandle(ProcessInfo.hProcess);
    ProcessInfo.hProcess = NULL;
    return res;
  }
  return true;
}
//..............................................................................
// write data to the child's stdin
void TWinProcess::Write(const olxstr &Str)  {
  if( InWrite == NULL )  return;
  DWORD Written;
  WriteFile(InWrite, Str.c_str(), Str.Length(), &Written, NULL);
  GetOutput().Write(Str.c_str(), Str.Length());
}
//..............................................................................
void TWinProcess::Writenl()  {
  if( InWrite == NULL )  return;
  DWORD Written;
  WriteFile(InWrite, "\n", 1, &Written, NULL);
  GetOutput().Write('\n');
}
//..............................................................................
bool TWinProcess::Dispatch(int MsgId, short MsgSubId, const IEObject *Sender, const IEObject *Data)  {
  bool Terminated = false;
  if( MsgId == ID_Timer )  {
    if( IsRedirected() )  {
      DWORD dwAvail = 0;
      const size_t BfC=512;
      char Bf[BfC];
      DWORD dwRead = 0;
      if( PeekNamedPipe(OutRead, NULL, 0, NULL, &dwAvail, NULL) == 0 )  // error
        Terminated = true;
      else  if( dwAvail == 0 )  {  // no data
        CallsWasted++;
      }
      else  {
        CallsWasted = 0;
        dwRead = 0;
        if( ReadFile(OutRead, Bf, olx_min(BfC-1, dwAvail), &dwRead, NULL) == 0 || dwRead == 0 )  // error, the child might ended
          Terminated = true;
        else  {
          Output.Write(Bf, dwRead);
          if( GetDubStream() != NULL )
            GetDubStream()->Write(Bf, dwRead);
        }
      }
    }
    else  {  // just check if still valid
      DWORD Status;
      if( GetExitCodeProcess(ProcessInfo.hProcess, &Status) == 0 || Status != STILL_ACTIVE )
        Terminated = true;
    }
  }
  // Win98 fix... as PeekNamedPipe does not fail after the process is terminate
  if( CallsWasted > 15 )  {
    unsigned long pec = 0;
    if( !Terminated && GetExitCodeProcess(ProcessInfo.hProcess, &pec) != 0 )  {
      if( pec != STILL_ACTIVE )
        Terminated = true;
    }
    CallsWasted = 0;
  }
  //
  if( Terminated )  {
    CloseHandle(ProcessInfo.hProcess);
    ProcessInfo.hProcess = NULL;
    Terminate();
    CallsWasted = 0;
  }
  return true;
}
void TWinProcess::Detach()  {
  TEGC::Add((AEventsDispatcher*)this);
}
//..............................................................................
//---------------------------------------------------------------------------//
// TWinWinCmd implementation
//---------------------------------------------------------------------------//
bool TWinWinCmd::SendWindowCmd(const olxstr& WndName, const olxstr& Cmd)  {
  HWND Window = FindWindow(WndName.u_str(), NULL);
  int TO=50;
  if( Window == NULL )  {
    SleepEx(20, FALSE);
    Window = FindWindow(WndName.u_str(), NULL);
    if( --TO < 0 )
      return false;
  }
  for( size_t i=0; i < Cmd.Length(); i++ )
    PostMessage(Window, WM_CHAR, Cmd[i], 0);
  return true;
}
#endif  // for __WIN32__
#ifdef __WXWIDGETS__
//..............................................................................
//---------------------------------------------------------------------------//
// TWxProcess implementation
//---------------------------------------------------------------------------//
TWxProcess::TWxProcess(const olxstr& cmdl, short flags) : AProcess(true, cmdl, flags) {
  FStream = NULL;
  FInputStream = NULL;
  FOutputStream = NULL;
}
//..............................................................................
TWxProcess::~TWxProcess()  {
  if( FStream != NULL )  {
    FStream->OnTerminate();
    FStream->Wait();
    delete FStream;
    FStream = NULL;  //Terminate uses it
  }
  if( IsTerminateOnDelete() )
    Terminate();
}
//..............................................................................
bool TWxProcess::Terminate()  {
  SetTerminated();
  if( ProcessId >= 0 )  {
    if( FStream != NULL )  {
      FStream->OnTerminate();
      FStream->Wait();
      delete FStream;
      FStream = NULL;
    }
    int pid = ProcessId;
    ProcessId = -1;
    return wxKill(pid, wxSIGKILL) == 0;
    // the function hsould trigger the OnTerminate function
  }
  return false;
}
//..............................................................................
void TWxProcess::OnTerminate(int pid, int status)  {
  ProcessId = -1;
  SetTerminated();
  FInputStream = NULL;
  FOutputStream = NULL;
  if( FStream != NULL )  {
    FStream->OnTerminate();
    FStream->Wait();
    delete FStream;
    FStream = NULL;
  }
  AProcess::OnTerminate.Execute(this);
}
//..............................................................................
void TWxProcess::Detach()  {  
  AProcess::OnTerminate.Clear();
  wxProcess::Detach();  
  TEGC::AddP(this); // according to docs, we should not delete the object 
}
//..............................................................................
bool TWxProcess::Execute()  {
  if( GetCmdLine().IsEmpty() )  return false;
  if( AProcess::IsRedirected() )  Redirect();
  if( AProcess::IsRedirected() )  {  // must be async
    TStrList toks;
    TParamList::StrtokParams(GetCmdLine(), ' ', toks);
#ifdef __WIN32__
    toks[0] = TEFile::ChangeFileExt(toks[0], "exe");
#endif
    olxstr cmd( TEFile::Which(TEFile::ExtractFileName(toks[0])) );
    if( toks.Count() > 1 )  {
      for( size_t i=1; i < toks.Count(); i++ )  {
        if( toks[i].IndexOf(' ') == InvalidIndex )
          cmd << ' '  << toks[i];
        else
          cmd << " \'" << toks[i] << '\'';
      }
    }
    ProcessId = wxExecute( cmd.u_str(), wxEXEC_ASYNC, this);
    if( ProcessId <= 0 )  {
      TBasicApp::NewLogEntry(logError) << "Failed to execute '" << cmd << '\'';
      return false;
    }
    InitStreams();
    return true;
  }
  else  {
    if( IsSynchronised() )  {
      ProcessId = wxExecute(GetCmdLine().u_str(), wxEXEC_SYNC, this);
      if( ProcessId == -1 )  return false;
      return true;
    }
    else  {
      wxExecute(GetCmdLine().u_str(), wxEXEC_ASYNC, this);
      ProcessId = -1;
    }
  }
  return true;
}
bool TWxProcess::InitStreams()  {
  FInputStream = GetInputStream();
  FOutputStream = wxProcess::GetOutputStream();
  if( FInputStream != NULL )  {
    FStream = new TIStream(this, FInputStream);
    FStream->Create();
    FStream->Run();
    return true;
  }
  return false;
}
//..............................................................................
void TWxProcess::Write(const olxstr &Cmd)  {
  if( !FOutputStream )  return;
  FOutputStream->Write(Cmd.c_str(), Cmd.Length());
  Output.Write(Cmd.c_str(), Cmd.Length());
}
//..............................................................................
void TWxProcess::Writenl()  {
  if( !FOutputStream )  return;
  FOutputStream->PutC('\n');
  Output.Write('\n');
}
//..............................................................................
//---------------------------------------------------------------------------//

//---------------------------------------------------------------------------//
TIStream::TIStream(TWxProcess *Parent, wxInputStream *IS) : wxThread(wxTHREAD_JOINABLE)  {
  FStream = IS;
  FParent = Parent;
  Terminated = false;
}
//..............................................................................
TIStream::~TIStream(){ 
  FParent = NULL;
  FStream = NULL;
}
//..............................................................................
void TIStream::OnTerminate()  {
  volatile olx_scope_cs _cs(TBasicApp::GetCriticalSection());
  FStream = NULL;
  FParent = NULL;
}
//..............................................................................
void* TIStream::Entry()  {
  while( (FStream != NULL) && !FStream->Eof() )  {
    const char ch = (char)FStream->GetC();
    if( FStream != NULL && !FStream->Eof() ) {
      FParent->GetOutput().Write(ch);
      if( FParent->GetDubStream() != NULL )
        FParent->GetDubStream()->Write(&ch, 1);
    }
  }
  return NULL;
}
//..............................................................................
#endif  // for __WXWIDGETS__
