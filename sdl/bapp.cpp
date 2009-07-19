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
#include "egc.h"

#ifdef __WIN32__
  #include <windows.h>
  #define OLX_STR(a) (a).u_str()
  #define OLX_CHAR olxch
  #ifdef _UNICODE
    #define OLX_GETENV _wgetenv
  #else
    #define OLX_GETENV getenv
  #endif
#else
  #include <time.h>  //POSIX for nanosleep
  #define OLX_STR(a) (a).c_str()
  #define OLX_GETENV getenv
  #define OLX_CHAR char
#endif
UseEsdlNamespace()

TBasicApp* TBasicApp::Instance = NULL;
//----------------------------------------------------------------------------//
//TBasicApp function bodies
//----------------------------------------------------------------------------//
TBasicApp::TBasicApp(const olxstr& FileName)  {
  if( Instance != NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "an application instance already exists");
  Instance = this;

  MaxThreadCount = 1;
  Log = new TLog;

  FActions = new TActionQList;
  OnProgress = &FActions->NewQueue("PROGRESS");
  OnTimer    = &FActions->NewQueue("TIMER");
  OnIdle     = &FActions->NewQueue("IDLE");
  Profiling = MainFormVisible = false;
  // attach GC to the instance, if detached...
  TEGC::Initialise();
  SetBaseDir(FileName);
}
//..............................................................................
TBasicApp::~TBasicApp()  {
  Instance = NULL;
  delete FActions;
  delete Log;
}
//..............................................................................
olxstr TBasicApp::GuessBaseDir(const olxstr& _path, const olxstr& var_name)  {
  olxstr path = _path.Trim(' ').Trim('"').Trim('\'');
  olxstr bd;
  if( !var_name.IsEmpty() )  {
    OLX_CHAR* var_val = OLX_GETENV(OLX_STR(var_name));
    if( var_val != NULL )
      bd = var_val;
  }
  else  {
	  if( !TEFile::IsDir(path) )
      bd = TEFile::ExtractFilePath( path );
		else
		  bd = path;
    if( bd.EndsWith('.') || bd.EndsWith("..") )
      bd = TEFile::AbsolutePathTo(TEFile::CurrentDir(), bd);
  }
  if( bd.IsEmpty() || !TEFile::Exists(bd) )
    bd = TEFile::CurrentDir();
  TEFile::AddTrailingBackslashI(bd);
  olxstr en = TEFile::ExtractFileName(path);
  if( en.IsEmpty() )
    en = "unknown.exe";
  return bd << en;
}
//..............................................................................
const olxstr& TBasicApp::SetBaseDir(const olxstr& _bd)  {
  olxstr bd = TEFile::ExtractFilePath(_bd);
  if( !TEFile::Exists(bd) || !TEFile::IsDir(bd) )
    throw TFunctionFailedException(__OlxSourceInfo, olxstr("Invalid basedir: ") << bd);
  TBasicApp& inst = GetInstance();
  inst.BaseDir = bd;
  inst.ExeName = TEFile::ExtractFileName(_bd);
  TEFile::AddTrailingBackslashI(inst.BaseDir);  // just in case!
  inst.BaseDirWriteable = false;
  // and test it now
  const olxstr tmp_dir = inst.BaseDir + "_t__m___p____DIR";
  if( TEFile::Exists(tmp_dir) )  {  // would be odd
    if( TEFile::RmDir(tmp_dir) )
      inst.BaseDirWriteable = true;
  }
  else if( TEFile::MakeDir(tmp_dir) )  {
    if( TEFile::RmDir(tmp_dir) )
      inst.BaseDirWriteable = true;
  }
  return inst.BaseDir;
}
//..............................................................................
const olxstr& TBasicApp::SetSharedDir(const olxstr& cd) {
  TBasicApp& inst = GetInstance();
  if( !TEFile::Exists(cd) )  {
    if( !TEFile::MakeDir(cd) )
      if( !TEFile::MakeDirs(cd) )
        throw TFunctionFailedException(__OlxSourceInfo, olxstr("Could not create common dir:") << cd);
  }
  else if( !TEFile::IsDir(cd) )  
    throw TFunctionFailedException(__OlxSourceInfo, olxstr("Invalid config dir:") << cd);
  return (inst.SharedDir = TEFile::AddTrailingBackslash(cd));
}
//..............................................................................
const olxstr& TBasicApp::GetSharedDir() {  
  if( GetInstance().SharedDir.IsEmpty() )
    throw TFunctionFailedException(__OlxSourceInfo, "The common directory is undefined");
  return GetInstance().SharedDir; 
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


