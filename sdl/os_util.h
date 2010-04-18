// simple OS utilities... (c) O DOlomanov, 2009
#ifndef __olx_os_util_H
#define __olx_os_util_H
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

#ifdef __WIN32__
#  ifdef _MSC_VER
#    ifdef _UNICODE
#      define OLX_GETENV _wgetenv_s
#      define OLX_PUTENV _wputenv_s
#    else
#      define OLX_GETENV getenv_s
#      define OLX_PUTENV _putenv_s
#    endif
   static void olx_setenv(const olxstr& name, const olxstr& val)  {
     OLX_PUTENV(name.u_str(), val.u_str());
   }
   static olxstr olx_getenv(const olxstr& name)  {
     olxch* val=NULL;
     size_t sz;
     OLX_GETENV(&sz, NULL, 0, name.u_str());
     if( sz == 0 )  return EmptyString;
     val = new olxch[sz];
     OLX_GETENV(&sz, val, sz, name.u_str());
     return olxstr::FromExternal(val, sz-1);
   }
#  else // not MSVC
   static void olx_setenv(const olxstr& name, const olxstr& val)  {
     putenv((olxstr(name) << '=' << val).c_str());
   }
   static olxstr olx_getenv(const olxstr& name)  {
      return getenv(name.c_str());
   }
#  endif  // MSVC and others WIN compilers
#else  // not WIN
   static void olx_setenv(const olxstr& name, const olxstr& val)  {
     setenv(name.c_str(), val.c_str(), 1);
   }
   static olxstr olx_getenv(const olxstr& name)  {
     return getenv(name.c_str());
   }
#endif
// a convinience function
static void olx_setenv(const olxstr& v)  {
  size_t ei = v.IndexOf('=');
  if( ei == InvalidIndex )  return;
  olx_setenv(v.SubStringTo(ei).u_str(), v.SubStringFrom(ei+1).u_str());
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
    volatile olx_scope_cs _cs(TBasicApp::GetCriticalSection());  // make sure that optimised does not delete it at once...
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
