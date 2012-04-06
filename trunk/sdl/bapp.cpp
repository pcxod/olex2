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
#include "settingsfile.h"
#include "utf8file.h"

#ifdef __WIN32__
  #define OLX_STR(a) (a).u_str()
  #define OLX_CHAR olxch
#else
  #define OLX_STR(a) (a).c_str()
#endif
UseEsdlNamespace()

TBasicApp* TBasicApp::Instance = NULL;

TBasicApp::TBasicApp(const olxstr& FileName)
  : OnProgress(Actions.New("PROGRESS")),
    OnTimer(Actions.New("TIMER")), OnIdle(Actions.New("IDLE"))
{
  if( Instance != NULL ) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "an application instance already exists");
  }
  Instance = this;

  MaxThreadCount = 1;
  LogFile = NULL;
  Log = new TLog;

  Profiling = MainFormVisible = false;
  // attach GC to the instance, if detached...
  TEGC::Initialise();
  SetBaseDir(FileName);
  ReadOptions(GetBaseDir() + ".options");
}
//..............................................................................
TBasicApp::~TBasicApp()  {
  EnterCriticalSection();
  delete Log;
  if (LogFile != NULL)
    delete LogFile;
  Instance = NULL;
  LeaveCriticalSection();
}
//..............................................................................
bool TBasicApp::HasInstance()  {
  volatile olx_scope_cs cs(GetCriticalSection());
  return Instance != NULL;
}
//..............................................................................
void TBasicApp::ReadOptions(const olxstr &fn) {
  try {
    if( TEFile::Exists(fn) )  {
      TSettingsFile sf(fn);
      for( size_t i=0; i < sf.ParamCount(); i++ )
        Options.AddParam(sf.ParamName(i), sf.ParamValue(i), false);
    }
  }
  catch(...)  {}
}
//..............................................................................
olxstr TBasicApp::GuessBaseDir(const olxstr& _path, const olxstr& var_name)  {
  olxstr bd, path;
  TStrList toks;
  TParamList::StrtokParams(_path, ' ', toks);
  if( !toks.IsEmpty() )
    path = toks[0];
  if( !var_name.IsEmpty() )  {
    bd = olx_getenv(var_name);
  }
  else {
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
      if( !TEFile::MakeDirs(cd) ) {
        throw TFunctionFailedException(__OlxSourceInfo,
          olxstr("Could not create common dir: ").quote() << cd);
      }
  }
  else if( !TEFile::IsDir(cd) )  {
    throw TFunctionFailedException(__OlxSourceInfo,
      olxstr("Invalid shared dir: ").quote() << cd);
  }
  return (inst.SharedDir = TEFile::AddPathDelimeter(cd));
}
//..............................................................................
const olxstr& TBasicApp::GetSharedDir() {  
  if( GetInstance().SharedDir.IsEmpty() )
    return GetInstance().BaseDir;
  return GetInstance().SharedDir; 
}
//..............................................................................
void TBasicApp::SetInstanceDir(const olxstr &d) {  
  if (!TEFile::Exists(d))
    TEFile::MakeDirs(d);
  InstanceDir = TEFile::AddPathDelimeter(d); 
  // read user settings
  if (!InstanceDir.Equals(BaseDir))
    ReadOptions(InstanceDir + ".options");
}
//..............................................................................
TActionQueue& TBasicApp::NewActionQueue(const olxstr& Name) {
  if( Actions.Exists(Name) )
    return *Actions.Find(Name);
  else
    return Actions.New(Name);
}
//..............................................................................
bool TBasicApp::Is64BitCompilation() {
#if defined(_WIN64)
  return true;
#elif defined(__MAC__)
# if defined(__LP64__) || defined(__x86_64__)
  return true;
#endif
#elif defined(__linux__)
# if defined(__LP64__) || defined(__x86_64__)
  return true;
#endif
#endif
  return false;
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
#  if defined(__LP64__) || defined(__x86_64__)
  rv << "64";
#  else
  rv << "32";
#  endif
#elif __linux__
  rv << "Linux";
#  if defined(__LP64__) || defined(__x86_64__)
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
bool TBasicApp::CreateLogFile(const olxstr &file_name) {
  if (LogFile != NULL) return false;
  olxstr l_name=file_name;
  if (!TEFile::IsAbsolutePath(file_name)) {
    if (!TEFile::IsDir(InstanceDir)) return false;
    olxstr ld = InstanceDir + "logs";
    if (!TEFile::Exists(ld) && !TEFile::MakeDir(ld))
      return false;
    l_name = TEFile::AddPathDelimeterI(ld) << file_name;
    olxstr ts = TETime::FormatDateTime("yyMMdd-hhmmss", TETime::Now());
    l_name << '-' << ts << ".olx.log";
  }
  try {
    LogFile = TUtf8File::Create(l_name);
    Log->AddStream(LogFile, false);
    return true;
  }
  catch(...) {
    return false;
  }
}
//..............................................................................
void TBasicApp::CleanupLogs(const olxstr &dir_name) {
  olxstr dn = dir_name;
  if (dn.IsEmpty())
    dn = InstanceDir + "/logs";
  if (!TEFile::IsDir(dn)) return;
  TStrList log_files;
  TEFile::ListDir(dn, log_files, "*.log", sefAll);
  TEFile::AddPathDelimeterI(dn);
  time_t now = TETime::EpochTime();
  for (size_t i=0; i < log_files.Count(); i++) {
    if (!log_files[i].EndsWith("olx.log")) continue;
    olxstr fn = dn+log_files[i];
    time_t fa = TEFile::FileAge(fn);
    if ((fa+(3600*24)) < now) // 1 days in seconds
      TEFile::DelFile(fn);
  }
}
//..............................................................................
void TBasicApp::ValidateArgs() const {
  if (!Arguments.IsEmpty())
    throw TFunctionFailedException(__OlxSourceInfo, "already initialised");
}
//..............................................................................
void TBasicApp::TryToCombineArguments() {
  TStrList args;
  for (size_t i=0; i < Arguments.Count(); i++) {
    if (Arguments[i].Contains(' '))
      args.Add('"') << Arguments[i] << '"';
    else
      args.Add(Arguments[i]);
  }
  Arguments.Clear();
  TParamList::StrtokParams(args.Text(' '), ' ', Arguments);
}
//..............................................................................
//..............................................................................
void BAPP_GetArgCount(const TStrObjList&, TMacroError& E)  {
  E.SetRetVal(TBasicApp::GetArgCount());
}
void BAPP_GetArg(const TStrObjList& Params, TMacroError& E)  {
  size_t i = Params[0].ToSizeT();
  E.SetRetVal(i < TBasicApp::GetArgCount() ? TBasicApp::GetArg(i) : EmptyString());
}
//..............................................................................
void BAPP_GetOptCount(const TStrObjList&, TMacroError& E)  {
  E.SetRetVal(TBasicApp::GetInstance().GetOptions().Count());
}
void BAPP_GetOpt(const TStrObjList& Params, TMacroError& E)  {
  size_t i = Params[0].ToSizeT();
  TBasicApp &a = TBasicApp::GetInstance();
  if (i > a.GetOptions().Count()) {
    E.SetRetVal(EmptyString());
    return;
  }
  olxstr n = a.GetOptions().GetName(i),
    v = a.GetOptions().GetValue(i);
  E.SetRetVal(v.IsEmpty() ? n : (n << '=' << v));
}
void BAPP_GetOptValue(const TStrObjList& Params, TMacroError& E)  {
  E.SetRetVal(TBasicApp::GetInstance().GetOptions().FindValue(
    Params[0], Params.Count() == 2 ? Params[1] : EmptyString()));
}
//..............................................................................
void BAPP_Profiling(const TStrObjList& Params, TMacroError &E)  {
  if( Params.IsEmpty() )
    E.SetRetVal(TBasicApp::IsProfiling());
  else
    TBasicApp::SetProfiling(Params[0].ToBool());
}
//..............................................................................
void BAPP_LogFileName(const TStrObjList& Params, TMacroError &E)  {
  TEFile *f = TBasicApp::GetInstance().GetLogFile();
  E.SetRetVal(f == NULL ? EmptyString() : f->GetName());
}
//..............................................................................
TLibrary* TBasicApp::ExportLibrary(const olxstr& lib_name)  {
  TLibrary* lib = new TLibrary(lib_name);
  lib->RegisterStaticFunction(new TStaticFunction(BAPP_GetArgCount,
    "ArgCount", fpNone,
    "Returns number of arguments passed to the application"));
  lib->RegisterStaticFunction(new TStaticFunction(BAPP_GetArg,
    "GetArg", fpOne,
    "Returns application argument value by index"));
  lib->RegisterStaticFunction(new TStaticFunction(BAPP_GetOptCount,
    "OptCount", fpNone,
    "Returns number of options passed to the application"));
  lib->RegisterStaticFunction(new TStaticFunction(BAPP_GetOpt,
    "GetOpt", fpOne,
    "Returns application 'option=value' value by index. '=' only added if the"
    " values is not empty"));
  lib->RegisterStaticFunction(new TStaticFunction(BAPP_GetOptValue,
    "OptValue", fpOne|fpTwo,
    "Returns value of the given option (default may be provided)"));
  lib->RegisterStaticFunction(new TStaticFunction(BAPP_Profiling,
    "Profiling", fpNone|fpOne,
    "Sets/Returns current procedure profiling status"));
  lib->RegisterStaticFunction(new TStaticFunction(BAPP_LogFileName,
    "GetLogName", fpNone,
    "Returns current log file name"));
 return lib;
}
