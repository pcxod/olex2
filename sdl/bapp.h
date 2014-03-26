/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_bapp_H
#define __olx_bapp_H
#include "paramlist.h"
#include "os_util.h"
#include "library.h"
#include "log.h"
BeginEsdlNamespace()

// app event registry, these might not be implemented
static olxstr
  olxappevent_GL_DRAW("GLDRAW"),
  olxappevent_GL_CLEAR_STYLES("GLDSCLEAR"),
  olxappevent_UPDATE_GUI("UPDATE_GUI");

class TBasicApp: public IEObject  {
  olxstr
/* the directory from which the program is running */
    BaseDir,
/* the instance specific, writable directory */
    InstanceDir,
/* the instance independent, writable directory */
    SharedDir,
    ExeName,
/* the configuration, writable directory, by default = InstanceDir */
    ConfigDir;
protected:
  class TActionQList Actions;
  static TBasicApp *Instance;
  class TLog *Log;
  class TEFile *LogFile;
  short MaxThreadCount;
  bool MainFormVisible, Profiling, BaseDirWriteable;
  void ValidateArgs() const;
  TParamList Options;
  TStrList Arguments;
public:
  // the file name of the application with full path
  TBasicApp(const olxstr& AppName, bool read_options=false);
  virtual ~TBasicApp();
  const TParamList &GetOptions() const { return Options; }
  const TStrList &GetArguments() const { return Arguments; }
  // this can be used to identify version changes
  static const olxstr &GetModuleMD5Hash();
  static olxstr GetModuleName();
  /* initialises Options and Arguments. Options either contain '=' or start
  from '-'
  */
  template <typename ch_t>
  void InitArguments(int argc, ch_t **argv) {
    ValidateArgs(); // throw an excaption if Arguments are not empty
    for (int i=0; i < argc; i++) {
      olxstr arg = argv[i];
      if (arg.Contains('=') || arg.StartsFrom('-'))
        Options.FromString(arg, '=');
      else
        Arguments.Add(arg);
    }
  }
  /*The options are read when the object is constructed, calling it
  consequently will update the values
  */
  void ReadOptions(const olxstr &fn);
  void SaveOptions() const;
  template <typename T>
  void UpdateOption(const T &name, const olxstr &value) {
    Options.AddParam(name, value);
  }
  /* instance dir dependents on the location of the executable and to be used
  to store instance specific data - updates etc.  If unset, the value is equal
  to the BaseDir
  */
  static const olxstr& GetSharedDir();
  /* will create the folder if does not exist, if fails - throws
  TFunctionfailedException. Shared dir is the same for all insatnces of
  Olex2 disregarding the location
  */
  static const olxstr& SetSharedDir(const olxstr& cd);
  static bool HasSharedDir() {  return !GetInstance().SharedDir.IsEmpty();  }
  const olxstr &_GetInstanceDir() const {
    return InstanceDir.IsEmpty() ? GetBaseDir() : InstanceDir;
  }
  virtual bool ToClipboard(const olxstr &) const { return false; }
  bool ToClipboard(const TStrList &text) const {
    return ToClipboard(text.Text(NewLineSequence()));
  }
  /* If the path is absolute - it is used as is, otherwise it is considered to
  be relative to the InstanceDir
  */
  void SetConfigdDir(const olxstr &path);
  bool HasConfigDir() { return !ConfigDir.IsEmpty(); }
  const olxstr &GetConfigDir() const {
    return ConfigDir.IsEmpty() ? _GetInstanceDir() : ConfigDir;
  }
  /* sets instance dir - folder dependent on the exe location
  */
  void SetInstanceDir(const olxstr &d);
  static TLog& GetLog()  {  return *GetInstance().Log;  }
  static TLog::LogEntry NewLogEntry(int evt_type=logDefault,
    bool annotate=false, const olxstr &location=EmptyString())
  {
    return GetInstance().Log->NewEntry(evt_type, annotate, location);
  }
  /* if var_name is not NULL, tries to get its value and combine with file name
  of path if either are empty - the curent folder is used with exename
  'unknown.exe'
  */
  static olxstr GuessBaseDir(const olxstr& path,
    const olxstr& var_name=EmptyString());
  static const olxstr& GetBaseDir()  {  return GetInstance().BaseDir;  }
  /* instance dir dependents on the location of the executable and to be used
  to store insatnce specific data - updates etc.  If unset, the value is equal
  to the BaseDir
  */
  static const olxstr& GetInstanceDir()  {
    return GetInstance()._GetInstanceDir();
  }
  // this resets the ExeName to the file name of the bd
  static const olxstr& SetBaseDir(const olxstr& bd);
  static bool IsBaseDirWriteable() {  return GetInstance().BaseDirWriteable;  }
  // valid only if correct string is passed to the constructor
  static const olxstr& GetExeName()  {  return GetInstance().ExeName;  }
  static TBasicApp& GetInstance() {
    if( Instance == NULL ) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "Uninitialised application layer...");
    }
    return *Instance;
  }

  static bool HasInstance();
  /* Creates a log file, just a name is expected: like olex2 or olex2c, the
  timestamp will be appended to make it 'unique'. If the name is not an
  absolute path - the InsatnceDir is used
  */
  bool CreateLogFile(const olxstr &file_name);
  TEFile *GetLogFile() const { return LogFile; }
  /*Removes .log files in the given folder, if the dir_name is empty - the
  InstanceDir is cleaned up
  */
  void CleanupLogs(const olxstr &dir_name=EmptyString());

  TActionQueue& NewActionQueue(const olxstr& Name);
  TActionQueue* FindActionQueue(const olxstr& Name)  {
    try  {  return Actions.Find(Name);   }
    catch(...)  {  return NULL;  }
  }

  const TActionQList& GetActions() const {  return Actions; }

  static bool IsProfiling()  {  return GetInstance().Profiling;  }
  static void SetProfiling(bool v)  {  GetInstance().Profiling = v;  }

  // implementation might consider drawing scene, update GUI etc..
  virtual void Update()  {}
  static size_t GetArgCount()  {  return GetInstance().Arguments.Count();  }
  static const olxstr& GetArg(size_t i)  {
    return GetInstance().Arguments[i];
  }
  // returns WIN32, WIN64, LINUX32, LINUX64, MAC32 etc
  static olxstr GetPlatformString();
  static bool Is64BitCompilation();
  TLibrary* ExportLibrary(const olxstr& lib_name="app");

  // application layer critical section
  static void EnterCriticalSection()  {  GetCriticalSection().enter();  }
  static void LeaveCriticalSection()  {  GetCriticalSection().leave();  }
  static olx_critical_section& GetCriticalSection() {
    static olx_critical_section app_cs;
    return app_cs;
  }
  DefPropP(short, MaxThreadCount)
  /* Note that objects which may live after the application end (like the ones
  places into the garbage collector - should check that an application instance
  exsists before using any of the actions queues
  */
  TActionQueue& OnProgress;
  TActionQueue& OnTimer;
  TActionQueue& OnIdle;
};

EndEsdlNamespace()
#endif
