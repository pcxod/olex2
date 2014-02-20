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
#include "md5.h"

#ifdef __WIN32__
  #define OLX_STR(a) (a).u_str()
  #define OLX_CHAR olxch
#else
  #define OLX_STR(a) (a).c_str()
# include <dlfcn.h>
#endif
UseEsdlNamespace()

TBasicApp* TBasicApp::Instance = NULL;

TBasicApp::TBasicApp(const olxstr& FileName, bool read_options)
  : OnProgress(Actions.New("PROGRESS")),
    OnTimer(Actions.New("TIMER")), OnIdle(Actions.New("IDLE"))
{
  if( Instance != NULL ) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "an application instance already exists");
  }
  Instance = this;

  MaxThreadCount = 1;
#ifdef __WIN32__
  SYSTEM_INFO si;
  memset(&si, 0, sizeof(si));
  GetSystemInfo(&si);
  MaxThreadCount = (short)si.dwNumberOfProcessors;
#endif
  LogFile = NULL;
  Log = new TLog;

  Profiling = MainFormVisible = false;
  // attach GC to the instance, if detached...
  TEGC::Initialise();
  SetBaseDir(FileName);
  if (read_options)
    ReadOptions(GetBaseDir() + ".options");
}
//..............................................................................
TBasicApp::~TBasicApp()  {
  delete Log;
  if (LogFile != NULL)
    delete LogFile;
  Instance = NULL;
}
//..............................................................................
olxstr TBasicApp::GetModuleName() {
  olxstr name;
#ifdef __WIN32__
  olxch * bf;
  DWORD rv = MAX_PATH;
  int n=1;
  while (rv == MAX_PATH) {
    if (n > 1) delete [] bf;
    bf = new olxch[MAX_PATH*n];
    rv = GetModuleFileName((HMODULE)Module().GetHandle(), bf, MAX_PATH*n);
    n++;
  }
  name = olxstr::FromExternal(bf, rv);
#else
  Dl_info dl_info;
  dladdr((const void *)&TBasicApp::GetModuleMD5Hash, &dl_info);
  name = dl_info.dli_fname;
  if (!TEFile::IsAbsolutePath(name)) {
    if (HasInstance())
      name = TEFile::ExpandRelativePath(name, GetBaseDir());
    else {
      name = TEFile::ExpandRelativePath(name, TEFile::CurrentDir());
    }
  }
#endif
  return name;
}
//..............................................................................
const olxstr &TBasicApp::GetModuleMD5Hash() {
  static olxstr Digest;
  if (!Digest.IsEmpty()) return Digest;
  olxstr name = GetModuleName();
  bool do_calculate = true;
  olxstr last_dg_fn = GetInstanceDir() + "app.md5";
  if (TEFile::Exists(last_dg_fn)) {
    TCStrList l;
    l.LoadFromFile(last_dg_fn);
    if (l.Count() > 3) {
      TEFile::FileID fd = TEFile::GetFileID(name);
      if (l[0] == name &&
          l[1] == olxstr(fd.size) &&
          l[2] == olxstr(fd.timestamp))
      {
        Digest = l[3];
        do_calculate = false;
      }
    }
  }
  if (do_calculate) {
    try {
      TEFile f(name, "rb");
      Digest = MD5::Digest(f);
      TCStrList l;
      TEFile::FileID fd = TEFile::GetFileID(name);
      l << name;
      l << fd.size;
      l << fd.timestamp;
      l << Digest;
      l.SaveToFile(last_dg_fn);
    }
    catch (const TExceptionBase &e) {
      TBasicApp::NewLogEntry(logExceptionTrace) << e;
      // make somewhat new every time
      Digest = TETime::msNow();
    }
  }
  return Digest;
}
//..............................................................................
bool TBasicApp::HasInstance()  {
  return Instance != NULL;
}
//..............................................................................
void TBasicApp::ReadOptions(const olxstr &fn) {
  try {
    if (TEFile::Exists(fn)) {
      TSettingsFile sf(fn);
      for (size_t i=0; i < sf.ParamCount(); i++) {
        Options.AddParam(sf.ParamName(i), sf.ParamValue(i), true);
        if (sf.ParamName(i) == "profiling" && sf.ParamValue(i).IsBool()) {
          SetProfiling(sf.ParamValue(i).ToBool());
        }
      }
    }
  }
  catch(...)  {}
}
//..............................................................................
void TBasicApp::SaveOptions() const {
  try {
    TSettingsFile st;
    olxstr fn = GetConfigDir() + ".options";
    if (TEFile::Exists(fn))
      st.LoadSettings(fn);
    for (size_t i=0; i < Options.Count(); i++)
      st.SetParam(Options.GetName(i), Options.GetValue(i));
    st.SaveSettings(fn);
  }
  catch (const TExceptionBase &e) {
    NewLogEntry(logExceptionTrace) << e;
  }
}
//..............................................................................
olxstr TBasicApp::GuessBaseDir(const olxstr& _path, const olxstr& var_name)  {
  olxstr bd, path;
  TStrList toks;
  if (!_path.StartsFrom(' ')) { // empty executable name
    TParamList::StrtokParams(_path, ' ', toks);
  }
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
  if( bd.IsEmpty() || !TEFile::Exists(bd) ) {
    bd = TEFile::CurrentDir();
  }
  TEFile::AddPathDelimeterI(bd);
  olxstr en = TEFile::ExtractFileName(path);
  if( en.IsEmpty() )
    en = "unknown.exe";
  return bd << en;
}
//..............................................................................
const olxstr& TBasicApp::SetBaseDir(const olxstr& _bd)  {
  olxstr bd = TEFile::ExtractFilePath(olxstr(_bd).Trim('"').Trim('\''));
  if (!TEFile::Exists(bd) || !TEFile::IsDir(bd)) {
    bd = TEFile::ExtractFilePath(TBasicApp::GetModuleName());
  }
  if (!TEFile::Exists(bd) || !TEFile::IsDir(bd)) {
    throw TFunctionFailedException(__OlxSourceInfo,
      olxstr("Invalid basedir: ").quote() << bd);
  }
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
  //2013.06.25 - do it manually only!
  //// read user settings
  //if (!InstanceDir.Equals(BaseDir))
  //  ReadOptions(InstanceDir + ".options");
}
//..............................................................................
void TBasicApp::SetConfigdDir(const olxstr &cd) {
  if (TEFile::IsAbsolutePath(cd))
    ConfigDir = cd;
  else
    ConfigDir = _GetInstanceDir() + cd;
  TEFile::AddPathDelimeterI(ConfigDir);
  if (!TEFile::Exists(ConfigDir)) {
    if (!TEFile::MakeDirs(ConfigDir))
      throw TInvalidArgumentException(__OlxSourceInfo, "ConfigDir");
  }
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
  if (i >= a.GetOptions().Count()) {
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
void BAPP_SaveOptions(const TStrObjList&, TMacroError& E)  {
  TBasicApp::GetInstance().SaveOptions();
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
void BAPP_BaseDir(const TStrObjList& Params, TMacroError &E)  {
  E.SetRetVal(
    TBasicApp::GetInstance().GetBaseDir().SubStringFrom(0,1));
}
//..............................................................................
void BAPP_InstanceDir(const TStrObjList& Params, TMacroError &E)  {
  E.SetRetVal(
    TBasicApp::GetInstance().GetInstanceDir().SubStringFrom(0,1));
}
//..............................................................................
void BAPP_SharedDir(const TStrObjList& Params, TMacroError &E)  {
  E.SetRetVal(
    TBasicApp::GetInstance().GetSharedDir().SubStringFrom(0,1));
}
//..............................................................................
void BAPP_ConfigDir(const TStrObjList& Params, TMacroError &E)  {
  E.SetRetVal(
    TBasicApp::GetInstance().GetConfigDir().SubStringFrom(0,1));
}
//..............................................................................
void BAPP_Platform(const TStrObjList& Params, TMacroError &E)  {
  E.SetRetVal(TBasicApp::GetPlatformString());
}
//..............................................................................
void BAPP_ModuleHash(const TStrObjList& Params, TMacroError &E)  {
  E.SetRetVal(TBasicApp::GetModuleMD5Hash());
}
//..............................................................................
void BAPP_IsBaseDirWritable(const TStrObjList& Params, TMacroError &E)  {
  E.SetRetVal(TBasicApp::IsBaseDirWriteable());
}
//..............................................................................
void BAPP_IsDebugBuild(const TStrObjList& Params, TMacroError &E)  {
#ifdef _DEBUG
  E.SetRetVal(TrueString());
#else
  E.SetRetVal(FalseString());
#endif
}
//..............................................................................
TLibrary* TBasicApp::ExportLibrary(const olxstr& lib_name)  {
  TLibrary* lib = new TLibrary(lib_name);
  lib->Register(new TStaticFunction(BAPP_GetArgCount,
    "ArgCount", fpNone,
    "Returns number of arguments passed to the application"));
  lib->Register(new TStaticFunction(BAPP_GetArg,
    "GetArg", fpOne,
    "Returns application argument value by index"));
  lib->Register(new TStaticFunction(BAPP_GetOptCount,
    "OptCount", fpNone,
    "Returns number of options passed to the application"));
  lib->Register(new TStaticFunction(BAPP_GetOpt,
    "GetOpt", fpOne,
    "Returns application 'option=value' value by index. '=' only added if the"
    " values is not empty"));
  lib->Register(new TStaticFunction(BAPP_GetOptValue,
    "OptValue", fpOne|fpTwo,
    "Returns value of the given option (default may be provided)"));
  lib->Register(new TStaticFunction(BAPP_SaveOptions,
    "SaveOptions", fpNone, "Saves options to ConfigDir/.options file"));
  lib->Register(new TStaticFunction(BAPP_Profiling,
    "Profiling", fpNone|fpOne,
    "Sets/Returns current procedure profiling status"));
  lib->Register(new TStaticFunction(BAPP_LogFileName,
    "GetLogName", fpNone,
    "Returns current log file name"));
  lib->Register(new TStaticFunction(BAPP_BaseDir,
    "BaseDir", fpNone,
    "Returns the directory from which the application is launched."));
  lib->Register(new TStaticFunction(BAPP_InstanceDir,
    "InstanceDir", fpNone,
    "Returns the instance specific, writable directory."));
  lib->Register(new TStaticFunction(BAPP_SharedDir,
    "SharedDir", fpNone,
    "Returns a generic writable directory."));
  lib->Register(new TStaticFunction(BAPP_ConfigDir,
    "ConfigDir", fpNone,
    "Returns the configuration directory. If it is not set, the InstanceDir is"
    " returned"));
  lib->Register(new TStaticFunction(BAPP_Platform,
    "Platform", fpNone,
    "Returns current platform like WIN, MAC, Linux 32/64"));
  lib->Register(new TStaticFunction(BAPP_ModuleHash,
    "ModuleHash", fpNone,
    "Returns current module MD5 hash"));
  lib->Register(new TStaticFunction(BAPP_IsBaseDirWritable,
    "IsBaseDirWritable", fpNone,
    "Returns true if the application can write to the BaseDir()"));
  lib->Register(new TStaticFunction(BAPP_IsDebugBuild,
    "IsDebugBuild", fpNone,
    "Returns true if the application is built with debug info"));
 return lib;
}
