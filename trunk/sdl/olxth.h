#ifndef __olx_thread_H
#define __olx_thread_H

// sort out the gheader files
#ifdef __WIN32__
  #include <windows.h>
#else
  #include <pthread.h>
#endif
#include "exception.h"
#include "bapp.h"

BeginEsdlNamespace()

class AOlxThread  {
protected:
  int volatile RetVal;
  bool volatile Terminate, Detached, Running;
#ifdef __WIN32__
  HANDLE Handle;
  static unsigned long _stdcall _Run(void* _instance) {
#else
  pthread_t Handle;
  static void* _Run(void* _instance) {
#endif
    ((AOlxThread*)_instance)->Running = true;
    ((AOlxThread*)_instance)->RetVal = ((AOlxThread*)_instance)->Run();
    // running prevents the object deletion...
    ((AOlxThread*)_instance)->Running = false;
    if( ((AOlxThread*)_instance)->Detached )
      delete (AOlxThread*)_instance;
    return 0;
  }
protected:  // do not allow to create externally
  AOlxThread() : 
    Detached(true), 
    Terminate(false),
    Running(false),
    Handle(0), 
    RetVal(0) {  }
public:
  virtual ~AOlxThread()  {
    if( Running )  {  // prevent deleting
      Detached = false;
      Terminate = true;
    }
    while( Running )
      TBasicApp::Sleep(50);
  }

  /* It is crutial to check if the terminate flag is set. In that case the function should
  return a value, or a deadlock situation may arrise. */
  virtual int Run() = 0;

  bool Start() {
#ifdef __WIN32__
    unsigned long thread_id;
    Handle = CreateThread(NULL, 0, _Run, this, 0, &thread_id);
    if( Handle == NULL )  
      return false;
#else  
    return (pthread_create(&Handle, NULL, _Run, this) == 0);
#endif
    return true;
  }
  /* returns true if successful, the process calling Join is responsible for the
  memory deallocation... */
  bool Join()  {
    if( Handle == 0 )
      throw TInvalidArgumentException(__OlxSourceInfo, "the tread must be started at first");
    Detached = false;
#ifdef __WIN32__
    unsigned long ec = STILL_ACTIVE, rv;
    while( ec == STILL_ACTIVE && (rv=GetExitCodeThread(Handle, &ec)) != 0 )
      ;
    return rv != 0;
#else  
    if( pthread_join(Handle, NULL) != 0 )
      return false;
#endif
    return true;
  }
  // this only has effect if the main procedure of the thread checks for this flag...
  void SendTerminate()  {  Terminate = true;  }
  // just to validate that the type is correct
  template <class T> static T* NewThread()  {
    AOlxThread* th = new T;
    return (T*)th;
  }

};

EndEsdlNamespace()
#endif
