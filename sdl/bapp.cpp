//----------------------------------------------------------------------------//
// TXApplication - a wraper for basic crystallographic graphic application
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "bapp.h"
#include "log.h"
#include "efile.h"
#include "etime.h"

#ifdef __WIN32__
  #include <windows.h>
#else
  #include <time.h>  //POSIX for nanosleep
#endif
UseEsdlNamespace()

TBasicApp* TBasicApp::Instance = NULL;
//----------------------------------------------------------------------------//
//TBasicApp function bodies
//----------------------------------------------------------------------------//
TBasicApp::TBasicApp(const olxstr & FileName)  {
  if( Instance != NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "an application instance already exists");
  Instance = this;

  MaxThreadCount = 1;
  FBaseDir = TEFile::ExtractFilePath(FileName);
  TEFile::AddTrailingBackslashI(FBaseDir);  // just in case!
  olxstr LogFN;
  Log = new TLog;

  FActions = new TActionQList;
  OnProgress = &FActions->NewQueue("PROGRESS");
  OnTimer    = &FActions->NewQueue("TIMER");
  OnIdle     = &FActions->NewQueue("IDLE");
  Profiling = MainFormVisible = false;
}
//..............................................................................
TBasicApp::~TBasicApp()  {
  Instance = NULL;
  delete FActions;
  delete Log;
}
//..............................................................................
const olxstr& TBasicApp::SetConfigDir(const olxstr& cd) {
  if( !TEFile::Exists(cd) )  {
    if( !TEFile::MakeDir(cd) )
      if( !TEFile::MakeDirs(cd) )
        throw TFunctionFailedException(__OlxSourceInfo, olxstr("Could not create config dir:") << cd);
  }
  else if( !TEFile::IsDir(cd) )  
    throw TFunctionFailedException(__OlxSourceInfo, olxstr("Invalid config dir:") << cd);
  return (ConfigDir = TEFile::AddTrailingBackslash(cd));
}
//..............................................................................
TActionQueue& TBasicApp::NewActionQueue(const olxstr &Name) {
  if( FActions->QueueExists(Name) )
    return *FActions->FindQueue(Name);
  else
    return FActions->NewQueue(Name);

  //TActionQueue* q = FActions->FindQueue(Name);
  //return (q!=NULL) ? *q : FActions->NewQueue(Name);
}
//..............................................................................
void TBasicApp::Sleep(long msec)  {
#ifdef __WIN32__
  SleepEx(msec, TRUE);
#else
//#ifdef __WXWIDGETS__
//    wxMilliSleep( msec );
//  #else  // POSIX, <time.h>
    timespec tm;
    tm.tv_sec = msec/1000;
    tm.tv_nsec = (msec%1000)*1000*1000;
    nanosleep(&tm, NULL);
//  #endif
#endif
}


