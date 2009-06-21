//---------------------------------------------------------------------------//
// TXApplication - a wraper for basic crystallographic graphic application
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#ifndef bappH
#define bappH
#include "paramlist.h"

BeginEsdlNamespace()

class TBasicApp: public IEObject  {
  olxstr FBaseDir, ConfigDir;
protected:
  class TActionQList* FActions;
  static TBasicApp* Instance;
  class TLog* Log;
  short MaxThreadCount;
  bool MainFormVisible, Profiling;
public:
  TParamList ParamList;
  TStrObjList Arguments;

  TBasicApp(const olxstr& AppName); // the file name of the application with full path
  virtual ~TBasicApp();

  const olxstr& BaseDir() const {  return FBaseDir; }

  /* config dir is independent of current user and to be used for global data
  and locks, mut be initialised by the caller */
  const olxstr& GetConfigDir() const {  return ConfigDir; }
  // will create the folder if does not exist
  const olxstr& SetConfigDir(const olxstr& cd);

  static inline TLog& GetLog()  {  return *GetInstance()->Log;  }

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
