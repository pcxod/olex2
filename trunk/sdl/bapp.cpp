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
  #define OLX_STR(a) (a).u_str()
  #define OLX_CHAR olxch
  #ifdef _UNICODE
    #define OLX_GETENV _wgetenv
  #else
    #define OLX_GETENV getenv
  #endif
#else
  #define OLX_STR(a) (a).c_str()
  #define OLX_GETENV getenv
  #define OLX_CHAR char
#endif
UseEsdlNamespace()

TBasicApp* TBasicApp::Instance = NULL;
olx_critical_section TBasicApp::app_cs;
//----------------------------------------------------------------------------//
//TBasicApp function bodies
//----------------------------------------------------------------------------//
TBasicApp::TBasicApp(const olxstr& FileName) : OnProgress(Actions.New("PROGRESS")),
  OnTimer(Actions.New("TIMER")), OnIdle(Actions.New("IDLE"))
{
  if( Instance != NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "an application instance already exists");
  Instance = this;

  MaxThreadCount = 1;
  Log = new TLog;

  Profiling = MainFormVisible = false;
  // attach GC to the instance, if detached...
  TEGC::Initialise();
  SetBaseDir(FileName);
}
//..............................................................................
TBasicApp::~TBasicApp()  {
  Instance = NULL;
  delete Log;
}
//..............................................................................
olxstr TBasicApp::GuessBaseDir(const olxstr& _path, const olxstr& var_name)  {
  olxstr bd, path;
  TStrList toks;
  TParamList::StrtokParams(_path, ' ', toks);
  if( !toks.IsEmpty() )
    path = toks[0];
  if( !var_name.IsEmpty() )  {
    OLX_CHAR* var_val = OLX_GETENV(OLX_STR(var_name));
    if( var_val != NULL )
      bd = var_val;
  }
  else  {
	  if( !TEFile::IsDir(path) )
      bd = TEFile::ExtractFilePath(path);
		else
		  bd = path;
    if( bd.EndsWith('.') || bd.EndsWith("..") )
      bd = TEFile::AbsolutePathTo(TEFile::CurrentDir(), bd);
  }
  if( bd.IsEmpty() || !TEFile::Exists(bd) )
    bd = TEFile::CurrentDir();
  TEFile::AddPathDelimeterI(bd);
  olxstr en = TEFile::ExtractFileName(path);
  if( en.IsEmpty() )
    en = "unknown.exe";
  return bd << en;
}
//..............................................................................
const olxstr& TBasicApp::SetBaseDir(const olxstr& _bd)  {
  olxstr bd = TEFile::ExtractFilePath(olxstr(_bd).Trim('"').Trim('\''));
  if( !TEFile::Exists(bd) || !TEFile::IsDir(bd) )
    throw TFunctionFailedException(__OlxSourceInfo, olxstr("Invalid basedir: ") << bd);
  TBasicApp& inst = GetInstance();
  inst.BaseDir = bd;
  inst.ExeName = TEFile::ExtractFileName(_bd);
  TEFile::AddPathDelimeterI(inst.BaseDir);  // just in case!
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
  return (inst.SharedDir = TEFile::AddPathDelimeter(cd));
}
//..............................................................................
const olxstr& TBasicApp::GetSharedDir() {  
  if( GetInstance().SharedDir.IsEmpty() )
    throw TFunctionFailedException(__OlxSourceInfo, "The common directory is undefined");
  return GetInstance().SharedDir; 
}
//..............................................................................
TActionQueue& TBasicApp::NewActionQueue(const olxstr& Name) {
  if( Actions.Exists(Name) )
    return *Actions.Find(Name);
  else
    return Actions.New(Name);

  //TActionQueue* q = FActions->FindQueue(Name);
  //return (q!=NULL) ? *q : FActions->NewQueue(Name);
}
