//---------------------------------------------------------------------------//
//---------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "eprocess.h"
#include "bapp.h"
#include "log.h"
#include "efile.h"
#include "egc.h"
const int ID_Timer = 1;

//..............................................................................
AProcess::AProcess() :
  OnTerminate(Actions.New("ONTERMINATE"))
{
  ProcessId = -1;
  Flags = 0;
  DubOutput = NULL;
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
TWinProcess::TWinProcess()  {
  InWrite = OutRead = NULL;
  OutWrite = ErrWrite = InRead = NULL;
  ProcessInfo.hProcess = NULL;
}
//..............................................................................
TWinProcess::~TWinProcess()  {
  if( IsTerminateOnDelete() )
    Terminate();
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
bool TWinProcess::Execute(const olxstr & Cmd, short Flags)  {
  InitData(Cmd, Flags);
  if( IsRedirected() )  {  // must be async
    if( !InitStreams() )  return false;
    SetBit(false, Flags, spfSynchronised);
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
  if( !CreateProcess( NULL, (LPTSTR)Cmd.u_str(), NULL, NULL,   true,
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
  if( !IsSynchronised() && IsRedirected() ) // Launch a thread to receive output from the child process.
    TBasicApp::GetInstance().OnTimer.Add(this, ID_Timer);

  if( IsSynchronised() )  {
    DWORD Status;
    GetExitCodeProcess(ProcessInfo.hProcess, &Status);
    while( Status == STILL_ACTIVE )  {
      SleepEx(50, TRUE);
      GetExitCodeProcess(ProcessInfo.hProcess, &Status);
    }
  }
  return true;
}
//..............................................................................
bool TWinProcess::Terminate()  {
  TBasicApp::GetInstance().OnTimer.Remove(this);
  CloseStreams();
  OnTerminate.Execute((AEventsDispatcher*)this, NULL);
  if( ProcessInfo.hProcess )  {
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
  WriteFile(InWrite, (LPCTSTR)Str.u_str(), Str.Length(), &Written, NULL);
}
//..............................................................................
void TWinProcess::Writenl()  {
  if( InWrite == NULL )  return;
  DWORD Written;
  WriteFile(InWrite, "\n", 1, &Written, NULL);
}
//..............................................................................
bool TWinProcess::Dispatch(int MsgId, short MsgSubId, const IEObject *Sender, const IEObject *Data)  {
  DWORD dwAvail = 0;
  static int wasted=0;  // acounter for how many times there is nothing to read in the pipe
  const int BfC=512;
  char Bf[BfC];
  DWORD dwRead = 0;
  bool Terminated = false;
  static olxstr Str;
  switch( MsgId )  {
    case ID_Timer:
    if( !PeekNamedPipe(OutRead, NULL, 0, NULL, &dwAvail, NULL) )  {    // error
      Terminated = true;
      break;
    }
    if( dwAvail == 0 )  {  // no data
      wasted++;
      break;
    }
    wasted = 0;
    dwRead = 0;
    if( !ReadFile(OutRead, Bf, olx_min(BfC-1, dwAvail), &dwRead, NULL) || dwRead == 0 )  { // error, the child might ended
      Terminated = true;
      break;
    }
    Str.SetCapacity( Str.Length() + dwRead );
    for( unsigned int i=0; i < dwRead; i++ )  {
      if( Bf[i] == '\r' ) continue;
      if( Bf[i] == '\n' )  {
        AddString(Str);
        if( GetDubStream() != NULL )
          GetDubStream()->Writenl( Str.c_str(), Str.Length() );
        if( !Str.IsEmpty() )
          Str = EmptyString;
        continue;
      }
      Str << Bf[i];
    }
    break;
  }
  // Win98 fix... as PeekNamedPipe does not fail after the process is terminate
  if( wasted > 25 )  {
    unsigned long pec = 0;
    if( !Terminated && GetExitCodeProcess(ProcessInfo.hProcess, &pec) != 0 )
      if( pec != STILL_ACTIVE )
        Terminated = true;
    wasted = 0;
  }
  //
  if( Terminated )  {
    if( !Str.IsEmpty() )  {
      AddString(Str);
      if( GetDubStream() != NULL )
        GetDubStream()->Writenl( Str.c_str(), Str.Length() );
      Str  = EmptyString;
    }
    CloseHandle(ProcessInfo.hProcess);
    ProcessInfo.hProcess = NULL;
    Terminate();
    wasted = 0;
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
TWxProcess::TWxProcess()  {
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
bool TWxProcess::Execute(const olxstr & Cmd, short Flags)  {
  if( Cmd.IsEmpty() )  return false;
  InitData(Cmd, Flags);
  if( AProcess::IsRedirected() )  Redirect();
  if( AProcess::IsRedirected() )  {  // must be async
    SetBit(false, Flags, spfSynchronised);
    TStrList toks;
    TParamList::StrtokParams(Cmd, ' ', toks);
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
      TBasicApp::GetLog().Error(olxstr("Failed to execute '") << cmd << '\'');
      return false;
    }
    InitStreams();
    return true;
  }
  else  {
    if( IsSynchronised() )  {
      ProcessId = wxExecute( Cmd.u_str(), wxEXEC_SYNC, this);
      if( ProcessId == -1 )  return false;
      return true;
    }
    else  {
      wxExecute( Cmd.u_str(), wxEXEC_ASYNC, this);
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
}
//..............................................................................
void TWxProcess::Writenl()  {
  if( !FOutputStream )  return;
  FOutputStream->PutC('\n');
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
  FStream = NULL;
  FParent = NULL;
}
//..............................................................................
void* TIStream::Entry()  {
  static olxstr Str;
  while( (FStream != NULL) && !FStream->Eof() )  {
    char C = FStream->GetC();
    if( C == '\n' )  {
      if( FParent == NULL )  return NULL;
      FParent->AddString(Str);
      if( FParent->GetDubStream() != NULL )
        FParent->GetDubStream()->Writenl( Str.c_str(), Str.Length() );
      Str = EmptyString;
    }
    else  {
      if( C == '\r' )  continue;
      Str << C;
    }
  }
  if( !Str.IsEmpty() && FParent != NULL )  {
    FParent->AddString(Str);  
    Str = EmptyString; 
  }
  return NULL;
  // a small problem - if there is a prompt line printed by the program, it will not 
  //be added to the list until enter is pressed ...
}
//..............................................................................
#endif  // for __WXWIDGETS__

