#ifndef __olx_bapp_H
#define __olx_bapp_H
#include "paramlist.h"
#include "os_util.h"
#include "library.h"

BeginEsdlNamespace()

// app event registry, these might not be implemented
static olxstr 
  olxappevent_GL_DRAW("GLDRAW"),
  olxappevent_GL_CLEAR_STYLES("GLDSCLEAR"),
  olxappevent_GL_BEFORE_DRAW("BGLDRAW");

class TBasicApp: public IEObject  {
  olxstr BaseDir, SharedDir, ExeName;
protected:
  class TActionQList Actions;
  static TBasicApp* Instance;
  class TLog* Log;
  short MaxThreadCount;
  bool MainFormVisible, Profiling, BaseDirWriteable;
  static olx_critical_section app_cs;
public:
  TParamList Options;
  TStrList Arguments;

  TBasicApp(const olxstr& AppName); // the file name of the application with full path
  virtual ~TBasicApp();

  /* shared dir is independent of current user and to be used for global data
  and locks, must be initialised by the caller otherwise (if empty) the function
  will throw TFunctionfailedException  */
  static const olxstr& GetSharedDir();
  // will create the folder if does not exist, if fails - throws TFunctionfailedException
  static const olxstr& SetSharedDir(const olxstr& cd);
  static bool HasSharedDir() {  return !GetInstance().SharedDir.IsEmpty();  }

  static TLog& GetLog()  {  return *GetInstance().Log;  }
  /* if var_name is not NULL, tries to get its value and combine with file name of path 
  if either are empty - the curent folder is used with exename 'unknown.exe' */
  static olxstr GuessBaseDir(const olxstr& path, const olxstr& var_name=EmptyString);
  static const olxstr& GetBaseDir()  {  return GetInstance().BaseDir;  }
  // this resets the ExeName to the file name of the bd
  static const olxstr& SetBaseDir(const olxstr& bd);
  static bool IsBaseDirWriteable() {  return GetInstance().BaseDirWriteable;  }
  // valid only if correct string is passed to the constructor
  static const olxstr& GetExeName()  {  return GetInstance().ExeName;  }
  static inline TBasicApp& GetInstance() {  
    if( Instance == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "Uninitialised application layer...");
    return *Instance;  
  }

  static bool HasInstance()  {  return Instance != NULL;  }

  TActionQueue& NewActionQueue(const olxstr& Name);
  inline TActionQueue* ActionQueue(const olxstr& Name){  
    try  {  return Actions.Find(Name);   }
    catch(...)  {  return NULL;  }
  }

  const TActionQList& GetActions() const {  return Actions; }

  DefPropBIsSet(MainFormVisible)
  DefPropBIsSet(Profiling)

  // implementation might consider drawing scene, update GUI etc..
  virtual void Update()  {}
  static size_t GetArgCount()  {  return GetInstance().Arguments.Count();  }
  static const olxstr& GetArg(size_t i)  {  return GetInstance().Arguments[i];  }
  TLibrary* ExportLibrary(const olxstr& lib_name="app");

  // application layer critical section
  inline static void EnterCriticalSection()  {  app_cs.enter();  }
  inline static void LeaveCriticalSection()  {  app_cs.leave();  }
  inline static olx_critical_section& GetCriticalSection() {  return app_cs;  }
  DefPropP(short, MaxThreadCount)
  TActionQueue& OnProgress;
  TActionQueue& OnTimer;
  TActionQueue& OnIdle;
};

EndEsdlNamespace()
#endif
