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
#include "estopwatch.h"
#include "sha.h"
#include "encodings.h"

#ifdef __WIN32__
  #define OLX_STR(a) (a).u_str()
  #define OLX_CHAR olxch
#else
  #define OLX_STR(a) (a).c_str()
# include <dlfcn.h>
#endif
UseEsdlNamespace()

TBasicApp::TBasicApp(const olxstr& FileName, bool read_options)
  : OnProgress(ActionList.New("PROGRESS")),
    OnTimer(ActionList.New("TIMER")),
    OnIdle(ActionList.New("IDLE"))
{
  if (Instance_() != 0) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "an application instance already exists");
  }
  Instance_() = this;

  MaxThreadCount = 1;
#ifdef __WIN32__
  SYSTEM_INFO si;
  memset(&si, 0, sizeof(si));
  GetSystemInfo(&si);
  MaxThreadCount = (short)si.dwNumberOfProcessors;
#endif
  LogFile = 0;
  Log = new TLog;

  Profiling = MainFormVisible = false;
  // attach GC to the instance, if detached...
  TEGC::Initialise();
  SetBaseDir(FileName);
  if (read_options) {
    ReadOptions(GetBaseDir() + ".options");
  }
  TActionHandler *ah = new TActionHandler();
/* For some reason timer events do not always pass through modal dialogs on Linux/Mac
* So this code works only on Windows!
  OnTimer.Add(ah);
  Sticking to the one that works on all platforms, possibly not the most efficient way?
*/
  OnIdle.Add(ah);
}
//..............................................................................
TBasicApp::~TBasicApp() {
  Instance_() = 0;
  delete Log;
  if (LogFile != 0) {
    delete LogFile;
  }
}
//..............................................................................
olxstr TBasicApp::GetModuleName() {
  olxstr name;
#ifdef __WIN32__
  olx_array_ptr<olxch> bf;
  DWORD rv = MAX_PATH;
  int n=1;
  while (rv == MAX_PATH) {
    if (n > 1) {
      delete[] bf.release();
    }
    bf = new olxch[MAX_PATH*n];
    rv = GetModuleFileName((HMODULE)Module().GetHandle(), bf, MAX_PATH*n);
    n++;
  }
  name = olxstr::FromExternal(bf.release(), rv);
#else
  Dl_info dl_info;
  dladdr((const void *)&TBasicApp::GetModuleMD5Hash, &dl_info);
  name = olxstr::FromCStr(dl_info.dli_fname);
  if (!TEFile::IsAbsolutePath(name)) {
    if (HasInstance()) {
      name = TEFile::ExpandRelativePath(name, GetBaseDir());
    }
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
    TCStrList l = TEFile::ReadCLines(last_dg_fn);
    if (l.Count() > 3) {
      TEFile::FileID fd = TEFile::GetFileID(name);
      if (l[0] == name.ToMBStr() &&
          l[1] == olxcstr(fd.size) &&
          l[2] == olxcstr(fd.timestamp))
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
      l << name.ToMBStr();
      l << fd.size;
      l << fd.timestamp;
      l << Digest;
      TEFile::WriteLines(last_dg_fn, l);
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
  return Instance_() != 0;
}
//..............................................................................
void TBasicApp::ReadOptions(const olxstr &fn) {
  try {
    TStrList path_suffix, path_prefix;
#ifndef __WIN32__
    path_suffix.Add("/usr/local/bin");
#endif
    if (TEFile::Exists(fn)) {
      TSettingsFile sf(fn);
      for (size_t i=0; i < sf.ParamCount(); i++) {
        Options.AddParam(sf.ParamName(i), sf.ParamValue(i), true);
        if (sf.ParamName(i) == "profiling" && sf.ParamValue(i).IsBool()) {
          SetProfiling(sf.ParamValue(i).ToBool());
        }
        else if (sf.ParamName(i).Equalsi("PATH")) {
          olxstr path_ext = olxstr(sf.ParamValue(i)).TrimWhiteChars();
          path_prefix.Strtok(path_ext, olx_env_sep());
        }
      }
    }
    if (!path_suffix.IsEmpty() || !path_prefix.IsEmpty()) {
      olxstr current_path = olx_getenv("PATH"), path;
      if (!current_path.EndsWith(olx_env_sep())) {
        current_path << olx_env_sep();
      }
      for (size_t i = 0; i < path_prefix.Count(); i++) {
        if (!current_path.Contains(path_prefix[i] + olx_env_sep())) {
          path << path_prefix[i] << olx_env_sep();
        }
      }
      path << current_path;
      for (size_t i = 0; i < path_suffix.Count(); i++) {
        if (!path.Contains(path_suffix[i] + olx_env_sep())) {
          path << path_suffix[i] << olx_env_sep();
        }
      }
      if (path.EndsWith(olx_env_sep())) {
        path.SetLength(path.Length()-1);
      }
      olx_setenv("PATH", path);
    }
  }
  catch(...)  {}
}
//..............................................................................
void TBasicApp::SaveOptions() const {
  try {
    TSettingsFile st;
    olxstr fn = GetConfigDir() + ".options";
    if (TEFile::Exists(fn)) {
      st.LoadSettings(fn);
    }
    for (size_t i = 0; i < Options.Count(); i++) {
      st.SetParam(Options.GetName(i), Options.GetValue(i));
    }
    st.SaveSettings(fn);
  }
  catch (const TExceptionBase &e) {
    NewLogEntry(logExceptionTrace) << e;
  }
}
//..............................................................................
olxstr TBasicApp::GuessBaseDir(const olxstr& _path, const olxstr& var_name) {
  olxstr bd, path;
  TStrList toks;
  if (!_path.StartsFrom(' ')) { // empty executable name
    TParamList::StrtokParams(_path, ' ', toks);
  }
  if (!toks.IsEmpty()) {
    path = toks[0];
  }
  if (!var_name.IsEmpty()) {
    bd = olx_getenv(var_name);
  }
  else {
    if (!TEFile::IsDir(path)) {
      bd = TEFile::ExtractFilePath(path);
    }
    else {
      bd = path;
    }
    bd = TEFile::ExpandRelativePath(bd, TEFile::CurrentDir());
  }
  if (bd.IsEmpty() || !TEFile::Exists(bd)) {
    bd = TEFile::CurrentDir();
  }
  TEFile::AddPathDelimeterI(bd);
  olxstr en = TEFile::ExtractFileName(path);
  if (en.IsEmpty()) {
    en = "unknown.exe";
  }
  return bd << en;
}
//..............................................................................
const olxstr& TBasicApp::SetBaseDir(const olxstr& _bd) {
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
  if (TEFile::Exists(tmp_dir)) {  // would be odd
    if (TEFile::RmDir(tmp_dir)) {
      inst.BaseDirWriteable = true;
    }
  }
  else if (TEFile::MakeDir(tmp_dir)) {
    if (TEFile::RmDir(tmp_dir)) {
      inst.BaseDirWriteable = true;
    }
  }
  return inst.BaseDir;
}
//..............................................................................
const olxstr& TBasicApp::SetSharedDir(const olxstr& cd) {
  TBasicApp& inst = GetInstance();
  if (!TEFile::Exists(cd)) {
    if (!TEFile::MakeDir(cd))
      if (!TEFile::MakeDirs(cd)) {
        throw TFunctionFailedException(__OlxSourceInfo,
          olxstr("Could not create common dir: ").quote() << cd);
      }
  }
  else if (!TEFile::IsDir(cd)) {
    throw TFunctionFailedException(__OlxSourceInfo,
      olxstr("Invalid shared dir: ").quote() << cd);
  }
  return (inst.SharedDir = TEFile::AddPathDelimeter(cd));
}
//..............................................................................
const olxstr& TBasicApp::GetSharedDir() {
  if (GetInstance().SharedDir.IsEmpty()) {
    return GetInstance().BaseDir;
  }
  return GetInstance().SharedDir;
}
//..............................................................................
void TBasicApp::SetInstanceDir(const olxstr &d) {
  if (!TEFile::Exists(d)) {
    TEFile::MakeDirs(d);
  }
  InstanceDir = TEFile::AddPathDelimeter(d);
  //2013.06.25 - do it manually only!
  //// read user settings
  //if (!InstanceDir.Equals(BaseDir))
  //  ReadOptions(InstanceDir + ".options");
}
//..............................................................................
void TBasicApp::SetConfigdDir(const olxstr &cd) {
  ConfigDir = (TEFile::IsAbsolutePath(cd) ? cd : _GetInstanceDir() + cd);
  TEFile::AddPathDelimeterI(ConfigDir);
  if (!TEFile::Exists(ConfigDir)) {
    if (!TEFile::MakeDirs(ConfigDir)) {
      throw TInvalidArgumentException(__OlxSourceInfo, "ConfigDir");
    }
  }
}
//..............................................................................
TActionQueue& TBasicApp::NewActionQueue(const olxstr& Name) {
  TActionQueue *aq = ActionList.Find(Name);
  return (aq != 0) ? *aq : ActionList.New(Name);
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
olxstr TBasicApp::GetPlatformString_(bool) const {
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
  if (LogFile != 0) {
    return false;
  }
  olxstr l_name=file_name;
  if (!TEFile::IsAbsolutePath(file_name)) {
    if (!TEFile::IsDir(InstanceDir)) {
      return false;
    }
    olxstr ld = InstanceDir + "logs";
    if (!TEFile::Exists(ld) && !TEFile::MakeDir(ld)) {
      return false;
    }
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
  if (dn.IsEmpty()) {
    dn = InstanceDir + "/logs";
  }
  if (!TEFile::IsDir(dn)) return;
  TStrList log_files;
  TEFile::ListDir(dn, log_files, "*.log", sefAll);
  TEFile::AddPathDelimeterI(dn);
  time_t now = TETime::EpochTime();
  for (size_t i=0; i < log_files.Count(); i++) {
    if (!log_files[i].EndsWith("olx.log")) {
      continue;
    }
    olxstr fn = dn+log_files[i];
    time_t fa = TEFile::FileAge(fn);
    if ((fa + (3600 * 24)) < now) { // 1 days in seconds
      TEFile::DelFile(fn);
    }
  }
}
//..............................................................................
void TBasicApp::ValidateArgs() const {
  if (!Arguments.IsEmpty()) {
    throw TFunctionFailedException(__OlxSourceInfo, "already initialised");
  }
}
//..............................................................................
void TBasicApp::PostAction(IOlxAction *a) {
  olx_scope_cs cs_(TBasicApp::GetCriticalSection());
  TBasicApp::GetInstance().Actions.Add(a);
}
//..............................................................................
bool TBasicApp::TActionHandler::Execute(const IOlxObject *, const IOlxObject *,
  TActionQueue *)
{
  olx_scope_cs cs_(TBasicApp::GetCriticalSection());
  TBasicApp& app = TBasicApp::GetInstance();
  bool update = !app.Actions.IsEmpty();
  if (update) {
    while (!app.Actions.IsEmpty()) {
      IOlxAction &a = app.Actions.Release(0);
      a.Run();
      delete &a;
    }
    app.Update();
  }
  return true;
}
//..............................................................................
//..............................................................................
void BAPP_GetArgCount(const TStrObjList&, TMacroData& E)  {
  E.SetRetVal(TBasicApp::GetArgCount());
}
void BAPP_GetArg(const TStrObjList& Params, TMacroData& E)  {
  size_t i = Params[0].ToSizeT();
  E.SetRetVal(i < TBasicApp::GetArgCount() ? TBasicApp::GetArg(i) : EmptyString());
}
//..............................................................................
void BAPP_GetOptCount(const TStrObjList&, TMacroData& E)  {
  E.SetRetVal(TBasicApp::GetInstance().GetOptions().Count());
}
void BAPP_GetOpt(const TStrObjList& Params, TMacroData& E)  {
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
void BAPP_GetOptValue(const TStrObjList& Params, TMacroData& E)  {
  E.SetRetVal(TBasicApp::GetInstance().GetOptions().FindValue(
    Params[0], Params.Count() == 2 ? Params[1] : EmptyString()));
}
void BAPP_SetOpt(const TStrObjList& Params, TMacroData& E) {
  olxstr key, value;
  if (Params.Count() == 1) {
    size_t idx = Params[0].IndexOf('=');
    if (idx == InvalidIndex) {
      E.ProcessingError(__OlxSrcInfo, "name=value string is expected");
      return;
    }
    key = Params[0].SubStringTo(idx);
    value = Params[0].SubStringFrom(idx+1);
  }
  else {
    key = Params[0];
    value = Params[1];
  }
  TBasicApp &a = TBasicApp::GetInstance();
  a.UpdateOption(key, value);
  try {
    a.SaveOptions();
  }
  catch (const TExceptionBase &e) {
    E.ProcessingException(__OlxSourceInfo, e);
  }
}
void BAPP_SaveOptions(const TStrObjList&, TMacroData& E)  {
  TBasicApp::GetInstance().SaveOptions();
}
//..............................................................................
void BAPP_Profiling(const TStrObjList& Params, TMacroData &E)  {
  if (Params.IsEmpty()) {
    E.SetRetVal(TBasicApp::IsProfiling());
  }
  else {
    TBasicApp::SetProfiling(Params[0].ToBool());
  }
}
//..............................................................................
void BAPP_Verbose(const TStrObjList& Params, TMacroData& E) {
  if (Params.IsEmpty()) {
    E.SetRetVal(TBasicApp::GetLog().IsVerbose());
  }
  else {
    TBasicApp::GetLog().SetVerbose(Params[0].ToBool());
  }
}
//..............................................................................
void BAPP_AutoFlush(const TStrObjList& Params, TMacroData& E) {
  if (Params.IsEmpty()) {
    E.SetRetVal(TBasicApp::GetLog().GetAutoFlush());
  }
  else {
    TBasicApp::GetLog().SetAutoFlush(Params[0].ToBool());
  }
}
//..............................................................................
void BAPP_LogFileName(const TStrObjList& Params, TMacroData &E)  {
  TEFile *f = TBasicApp::GetInstance().GetLogFile();
  E.SetRetVal(f == 0 ? EmptyString() : f->GetName());
}
//..............................................................................
void BAPP_BaseDir(const TStrObjList& Params, TMacroData &E)  {
  E.SetRetVal(
    TBasicApp::GetInstance().GetBaseDir().SubStringFrom(0,1));
}
//..............................................................................
void BAPP_InstanceDir(const TStrObjList& Params, TMacroData &E)  {
  E.SetRetVal(
    TBasicApp::GetInstance().GetInstanceDir().SubStringFrom(0,1));
}
//..............................................................................
void BAPP_SharedDir(const TStrObjList& Params, TMacroData &E)  {
  E.SetRetVal(
    TBasicApp::GetInstance().GetSharedDir().SubStringFrom(0,1));
}
//..............................................................................
void BAPP_ConfigDir(const TStrObjList& Params, TMacroData &E)  {
  E.SetRetVal(
    TBasicApp::GetInstance().GetConfigDir().SubStringFrom(0,1));
}
//..............................................................................
void BAPP_Platform(const TStrObjList& Params, TMacroData &E)  {
  E.SetRetVal(TBasicApp::GetPlatformString(true));
}
//..............................................................................
void BAPP_ModuleHash(const TStrObjList& Params, TMacroData &E)  {
  E.SetRetVal(TBasicApp::GetModuleMD5Hash());
}
//..............................................................................
void BAPP_IsBaseDirWritable(const TStrObjList& Params, TMacroData &E)  {
  E.SetRetVal(TBasicApp::IsBaseDirWriteable());
}
//..............................................................................
void BAPP_IsDebugBuild(const TStrObjList& Params, TMacroData &E)  {
#ifdef _DEBUG
  E.SetRetVal(TrueString());
#else
  E.SetRetVal(FalseString());
#endif
}
//..............................................................................
void BAPP_Digest(const TStrObjList& Params, TMacroData &E) {
  TStopWatch sw(__FUNC__);
  sw.start("Digesting a file...");
  TEFile f(Params[0], "rb");
  if (Params[1].Equalsi("MD5")) {
    E.SetRetVal(MD5::Digest(f));
  }
  else if (Params[1].Equalsi("SHA1")) {
    E.SetRetVal(SHA1::Digest(f));
  }
  else if (Params[1].Equalsi("SHA224")) {
    E.SetRetVal(SHA224::Digest(f));
  }
  else if (Params[1].Equalsi("SHA256")) {
    E.SetRetVal(SHA256::Digest(f));
  }
  else {
    E.ProcessingError(__OlxSrcInfo, "Unknow digest requested");
  }
  sw.stop();
}
//..............................................................................
void BAPP_Encode(const TStrObjList& Params, TMacroData &E) {
  TStopWatch sw(__FUNC__);
  sw.start("Encoding...");
  TEFile in(Params[0], "rb");
  TEFile out(Params[1], "w+b");
  if (Params[2].Equalsi("85")) {
    size_t bf_sz = 4 * 64 * 256;
    olx_array_ptr<uint8_t> bf(bf_sz);
    uint64_t available = in.GetSize();
    while (available > bf_sz) {
      in.Read(bf, bf_sz);
      out.Write(encoding::base85::encode(bf, bf_sz));
      available -= bf_sz;
    }
    if (available > 0) {
      in.Read(bf, (size_t)available);
      out.Write(encoding::base85::encode(bf, (size_t)available));
    }
  }
  else {
    E.ProcessingError(__OlxSrcInfo, "Unknow encoding requested");
  }
  sw.stop();
}
//..............................................................................
void BAPP_Decode(const TStrObjList& Params, TMacroData &E) {
  TStopWatch sw(__FUNC__);
  sw.start("Decoding...");
  TEFile in(Params[0], "rb");
  TEFile out(Params[1], "w+b");
  if (Params[2].Equalsi("85")) {
    size_t bf_sz = 5 * 64 * 256;
    olx_array_ptr<uint8_t> bf(bf_sz);
    uint64_t available = in.GetSize();
    while (available > bf_sz) {
      in.Read(bf, bf_sz);
      out.Write(encoding::base85::decode(bf, bf_sz));
      available -= bf_sz;
    }
    if (available > 0) {
      in.Read(bf, (size_t)available);
      out.Write(encoding::base85::decode(bf, (size_t)available));
    }
  }
  else {
    E.ProcessingError(__OlxSrcInfo, "Unknow encoding requested");
  }
  sw.stop();
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
  lib->Register(new TStaticFunction(BAPP_SetOpt,
    "SetOption", fpOne|fpTwo,
    "Sets application given like 'option=value' or name and value pair and saves"
    " the options. Application may need to be restarted for the options to take "
    "effect"));
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
  lib->Register(new TStaticFunction(BAPP_Verbose,
    "Verbose", fpNone | fpOne,
    "Sets/Returns if the verbose messages to be logged or discared"));
  lib->Register(new TStaticFunction(BAPP_AutoFlush,
    "AutoFlushLog", fpNone | fpOne,
    "Sets/Returns Log's auto flushing status"));
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
  lib->Register(new TStaticFunction(BAPP_Digest,
    "Digest", fpTwo,
    "Returns requested digest for a file. Use: 'digest file code', code is MD5, "
    "SHA1, SHA224 or SHA256"));
  lib->Register(new TStaticFunction(BAPP_Encode,
    "Encode", fpThree,
    "Encodes given file in the requested encoding: encode file_in file_out code, "
    "code is 16, 64 or 85"));
  lib->Register(new TStaticFunction(BAPP_Decode,
    "Decode", fpThree,
    "Decodes given file: decode file_in file_out code, "
    "code is 16, 64 or 85"));
  return lib;
}
