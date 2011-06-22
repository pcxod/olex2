/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "bapp.h"
#include "log.h"
#include "efile.h"
#include "etime.h"
#include "egc.h"

#ifdef __WIN32__
  #define OLX_STR(a) (a).u_str()
  #define OLX_CHAR olxch
#else
  #define OLX_STR(a) (a).c_str()
#endif
UseEsdlNamespace()

TBasicApp* TBasicApp::Instance = NULL;
olx_critical_section TBasicApp::app_cs;

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
    olxstr var_val = olx_getenv(var_name);
    if( !var_val.IsEmpty() )
      bd = var_val;
  }
  else  {
	  if( !TEFile::IsDir(path) )
      bd = TEFile::ExtractFilePath(path);
		else
		  bd = path;
    bd = TEFile::ExpandRelativePath(bd, TEFile::CurrentDir());
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
}
//..............................................................................
olxstr TBasicApp::GetPlatformString()  {
  olxstr rv;
#ifdef _WIN64
  rv << "WIN64";
#elif _WIN32
  rv << "WIN32";
#  if _M_IX86_FP == 1
  rv << "_SSE";
#  elif _M_IX86_FP == 2
  rv << "_SSE2";
#  endif

#elif __MAC__
  rv << "MAC";
#elif __linux__
  rv << "Linux";
#  ifdef __LP64__
  rv << "64";
#  else
  rv << "32";
#  endif
#else
  rv << "Other";
#endif
  return rv;
}
//..............................................................................
//..............................................................................
void BAPP_GetArgCount(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(TBasicApp::GetArgCount());
}
void BAPP_GetArg(const TStrObjList& Params, TMacroError& E)  {
  size_t i = Params[0].ToSizeT();
  E.SetRetVal(i < TBasicApp::GetArgCount() ? TBasicApp::GetArg(i) : EmptyString());
}
//..............................................................................
void BAPP_Profiling(const TStrObjList& Params, TMacroError &E)  {
  if( Params.IsEmpty() )
    E.SetRetVal(TBasicApp::IsProfiling());
  else
    TBasicApp::SetProfiling(Params[0].ToBool());
}
//..............................................................................

TLibrary* TBasicApp::ExportLibrary(const olxstr& lib_name)  {
  TLibrary* lib = new TLibrary(lib_name);
  lib->RegisterStaticFunction(new TStaticFunction(BAPP_GetArgCount, "ArgCount", fpNone,
"Returns number of arguments passed to the application") );
  lib->RegisterStaticFunction(new TStaticFunction(BAPP_GetArg, "GetArg", fpOne,
"Returns application argument value by index") );
  lib->RegisterStaticFunction(new TStaticFunction(BAPP_Profiling, "Profiling", fpNone|fpOne,
"Sets/Returns current procedure profiling status") );
 return lib;
}
