// simple OS utilities... (c) O DOlomanov, 2009
#ifndef __olx_os_util_H
#define __olx_os_util_H
#include "ebase.h"
#ifdef __WIN32__
  #include <windows.h>
#ifdef __BORLANDC__  // time_t definition...
  #include <time.h>
#endif
#undef Yield
#undef GetAtom
#undef AddAtom
#undef GetObject
#else
  #include <pthread.h>
  #include <time.h>
  #include <errno.h>
#endif

BeginEsdlNamespace()
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
  olx_critical_section() {
    pthread_mutex_init(&cs, NULL);
  }

  ~olx_critical_section()  {
    pthread_mutex_destroy(&cs);
  }
  inline bool tryEnter()  {  return pthread_mutex_trylock(&cs) != EBUSY;  }
  inline void enter() {  pthread_mutex_lock(&cs);  }
  inline void leave() {  pthread_mutex_unlock(&cs);  }
#endif
};
/* 'scope critical section' to be used for automatic mannagement of small portions of code.
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
