//---------------------------------------------------------------------------//
// TXApplication - a wraper for basic crystallographic graphic application
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#ifndef bappH
#define bappH
#include "paramlist.h"

BeginEsdlNamespace()

class TBasicApp: public IEObject  {
  olxstr FBaseDir, SharedDir, FExeName;
protected:
  class TActionQList* FActions;
  static TBasicApp* Instance;
  class TLog* Log;
  short MaxThreadCount;
  bool MainFormVisible, Profiling, BaseDirWriteable;
public:
  TParamList ParamList;
  TStrObjList Arguments;

  TBasicApp(const olxstr& AppName); // the file name of the application with full path
  virtual ~TBasicApp();

  const olxstr& BaseDir() const {  return FBaseDir; }
  bool IsBaseDirWriteable() const {  return BaseDirWriteable;  }
  // valid only if correct string is passed to the constructor
  const olxstr& ExeName() const {  return FExeName;  }
  /* shared dir is independent of current user and to be used for global data
  and locks, must be initialised by the caller otherwise (if empty) the function
  will throw TFunctionfailedException  */
  const olxstr& GetSharedDir() const;
  // will create the folder if does not exist, if fails - throws TFunctionfailedException
  const olxstr& SetSharedDir(const olxstr& cd);
  bool HasSharedDir() const {  return !SharedDir.IsEmpty();  }

  static TLog& GetLog()  {  return *GetInstance()->Log;  }
  static const olxstr& GetBaseDir()  {  return GetInstance()->FBaseDir;  }
  static const olxstr& GetExeName()  {  return GetInstance()->FExeName;  }
  static inline TBasicApp*  GetInstance() {  
    if( Instance == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "Uninitialised application layer...");
    return Instance;  
  }

  static bool HasInstance()  {  return Instance != NULL;  }
  //static printf(const char* format, ...);

  TActionQueue& NewActionQueue(const olxstr &Name);
  inline TActionQueue* ActionQueue(const olxstr &Name){  return FActions->FindQueue(Name); }

  inline TActionQList& Actions() const {  return *FActions; }

  DefPropBIsSet(MainFormVisible)
  DefPropBIsSet(Profiling)

  // default implementtaion is POSIX and windows
  static void Sleep(long msec);
  // implementation might consider drawing scene, update GUI etc..
  virtual void Update()  {  return;  }
  DefPropP(short, MaxThreadCount)
  TActionQueue *OnProgress;
  TActionQueue *OnTimer;
  TActionQueue *OnIdle;
};

EndEsdlNamespace()
#endif
