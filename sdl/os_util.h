/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_os_util_H
#define __olx_sdl_os_util_H
#include "ebase.h"
#ifdef __BORLANDC__  // time_t definition...
  #include <time.h>
#endif
#undef Yield
#undef GetAtom
#undef AddAtom
#undef GetObject
// includes...
#ifdef __WIN32__
#  include <process.h>
#else
#  include <pthread.h>
#  include <time.h>
#  include <errno.h>
#  include <unistd.h>
#endif

BeginEsdlNamespace()

// simple OS utilities...
#ifdef __WIN32__
#  ifdef _MSC_VER
#    ifdef _UNICODE
#      define OLX_GETENV _wgetenv_s
#      define OLX_PUTENV _wputenv_s
#    else
#      define OLX_GETENV getenv_s
#      define OLX_PUTENV _putenv_s
#    endif
   static bool olx_setenv(const olxstr& name, const olxstr& val)  {
     return OLX_PUTENV(name.u_str(), val.u_str()) == 0;
   }
   static olxstr olx_getenv(const olxstr& name)  {
     olxch* val=NULL;
     size_t sz;
     OLX_GETENV(&sz, NULL, 0, name.u_str());
     if( sz == 0 )  return EmptyString();
     val = olx_malloc<olxch>(sz);
     OLX_GETENV(&sz, val, sz, name.u_str());
     return olxstr::FromExternal(val, sz-1);
   }
#  else // not MSVC
   static bool olx_setenv(const olxstr& name, const olxstr& val)  {
     return putenv((olxstr(name) << '=' << val).c_str()) == 0;
   }
   static olxstr olx_getenv(const olxstr& name)  {
      return getenv(name.c_str());
   }
#  endif  // MSVC and others WIN compilers
/*http://msdn.microsoft.com/en-us/library/ms684139.aspx
return true if the process is running under wow64 - ie on x64 bit Windows,
might throw an exception... */
static bool IsWow64()  {
  typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
  LPFN_ISWOW64PROCESS fnIsWow64Process;
  BOOL bIsWow64 = FALSE;
  fnIsWow64Process = (LPFN_ISWOW64PROCESS) GetProcAddress(
    GetModuleHandle(TEXT("kernel32")),"IsWow64Process");
  if( fnIsWow64Process != NULL )  {
    if( !fnIsWow64Process(GetCurrentProcess(),&bIsWow64))
      throw TFunctionFailedException(__OlxSourceInfo, "to call IsWow64Process");
  }
  return bIsWow64 == TRUE;
}
#else  // not WIN
   static bool olx_setenv(const olxstr& name, const olxstr& val)  {
     return setenv(name.c_str(), val.c_str(), 1) == 0;
   }
   static olxstr olx_getenv(const olxstr& name)  {
     return getenv(name.c_str());
   }
#endif
// a convenience function
static bool olx_setenv(const olxstr& v)  {
  size_t ei = v.IndexOf('=');
  if( ei == InvalidIndex )  return false;
  return olx_setenv(v.SubStringTo(ei).u_str(), v.SubStringFrom(ei+1).u_str());
}


// http://en.wikipedia.org/wiki/Critical_section
struct olx_critical_section  {
#ifdef __WIN32__
  CRITICAL_SECTION cs;
  olx_critical_section()  {
    InitializeCriticalSection(&cs);
  }
  ~olx_critical_section()  {
    DeleteCriticalSection(&cs);
  }
  inline bool tryEnter()  {  return TryEnterCriticalSection(&cs) != 0;  }
  inline void enter() {  EnterCriticalSection(&cs);  }
  inline void leave() {  LeaveCriticalSection(&cs);  }
#else
  pthread_mutex_t cs;
  pthread_mutexattr_t csa;
  olx_critical_section() {
    pthread_mutexattr_init(&csa);
    pthread_mutexattr_settype(&csa, PTHREAD_MUTEX_DEFAULT);
    pthread_mutex_init(&cs, &csa);
  }

  ~olx_critical_section()  {
    pthread_mutex_destroy(&cs);
    pthread_mutexattr_destroy(&csa);
  }
  inline bool tryEnter()  {  return pthread_mutex_trylock(&cs) != EBUSY;  }
  inline void enter() {  pthread_mutex_lock(&cs);  }
  inline void leave() {  pthread_mutex_unlock(&cs);  }
#endif
};
/* 'scope critical section' to be used for automatic management of small portions of code.
use it like:
  {
    // make sure that optimiser does not delete it at once...
    volatile olx_scope_cs _cs(TBasicApp::GetCriticalSection());
    { code.. }
  }
*/
struct olx_scope_cs  {
protected:
  olx_critical_section& cs;
public:
  olx_scope_cs(olx_critical_section& _cs) : cs(_cs)  {  cs.enter();  }
  ~olx_scope_cs() {  cs.leave();  }
};

static void olx_sleep(time_t msec)  {
#ifdef __WIN32__
  SleepEx((DWORD)msec, TRUE);
#else
  timespec tm;
  tm.tv_sec = msec/1000;
  tm.tv_nsec = (msec%1000)*1000*1000;
  nanosleep(&tm, NULL);
#endif
}

EndEsdlNamespace()
#endif
