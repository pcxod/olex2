//---------------------------------------------------------------------------//
// TXApplication - a wraper for basic crystallographic graphic application
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#ifndef bappH
#define bappH
#include "paramlist.h"

BeginEsdlNamespace()

class TBasicApp: public IEObject  {
  olxstr FBaseDir;
protected:
  class TActionQList* FActions;
  static TBasicApp* Instance;
  class TLog* Log;
  short MaxThreadCount;
  bool MainFormVisible, Profiling;
public:
  TParamList ParamList;
  TStrObjList Arguments;

  TBasicApp(const olxstr& AppName); // the file anme of the application with full path
  virtual ~TBasicApp();

  const olxstr& BaseDir() const {  return FBaseDir; }

  static inline TLog& GetLog()  {  return *GetInstance()->Log;  }

  static inline TBasicApp*  GetInstance() {  return Instance;  }

  //static printf(const char* format, ...);

  TActionQueue& NewActionQueue(const olxstr &Name);
  inline TActionQueue* ActionQueue(const olxstr &Name){  return FActions->FindQueue(Name); }

  inline TActionQList& Actions() const {  return *FActions; }

  DefPropB(MainFormVisible)
  DefPropB(Profiling)

  // default implementtaion is POSIX and windows
  void Sleep(long msec);
  // implementation might consider drawing scene, update GUI etc..
  virtual void Update()  {  return;  }
  DefPropP(short, MaxThreadCount)
  TActionQueue *OnProgress;
  TActionQueue *OnTimer;
  TActionQueue *OnIdle;
};

EndEsdlNamespace()
#endif
