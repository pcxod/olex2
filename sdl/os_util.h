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
bool olx_setenv(const olxstr& name, const olxstr& val);
olxstr olx_getenv(const olxstr& name);
// a convenience function, takin key=val string
bool olx_setenv(const olxstr& v);
#ifdef __WIN32__
  inline char olx_env_sep() { return ';'; }
  bool IsWow64();
  /* this to work around the DLL handle. A DLL should set the handle in the
  DLLMain function, otherwise, module handle dependent functions will use the
  module of the executable which has loaded the DLL
  */
  struct Module {
    static HANDLE handle;
  public:
    Module() {}
    HANDLE GetHandle() const;
    HANDLE SetHandle(HANDLE h);
  };
#else
  inline char olx_env_sep() { return ':'; }
#endif
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
    pthread_mutexattr_settype(&csa, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&cs, &csa);
  }

  ~olx_critical_section()  {
    pthread_mutex_destroy(&cs);
    pthread_mutexattr_destroy(&csa);
  }
  bool tryEnter()  {  return (pthread_mutex_trylock(&cs) != EBUSY);  }
  void enter() {  pthread_mutex_lock(&cs);  }
  void leave() {  pthread_mutex_unlock(&cs);  }
#endif
};
/* 'scope critical section' to be used for automatic management of small
portions of code. Use it like:
  {
    // make sure that optimiser does not delete it at once...
    volatile olx_scope_cs _cs(TBasicApp::GetCriticalSection());
    { code.. }
  }
*/
struct olx_scope_cs  {
protected:
  olx_critical_section* cs;
public:
  olx_scope_cs(olx_critical_section* _cs) : cs(_cs)  {
    if (cs != 0) {
      cs->enter();
    }
  }
  olx_scope_cs(olx_critical_section& _cs) : cs(&_cs)  {  cs->enter();  }
  ~olx_scope_cs() {
    if (cs != 0) {
      cs->leave();
    }
  }
};

inline void olx_sleep(time_t msec)  {
#ifdef __WIN32__
  SleepEx((DWORD)msec, TRUE);
#else
  timespec tm;
  tm.tv_sec = msec/1000;
  tm.tv_nsec = (msec%1000)*1000*1000;
  nanosleep(&tm, NULL);
#endif
}
/* after some time with ChatGPT! */
template <typename T>
class OlxVolatileValue {
  T value;
  mutable olx_critical_section cs;

public:
  OlxVolatileValue(const T& v) : value(v) {}

  // Thread-safe getter
  T get() const {
    olx_scope_cs lock(cs);
    return value;
  }

  // Thread-safe setter
  void set(const T& v) {
    *this = v;  // reuse operator=
  }

  // operator() shorthand
  T operator()() const {
    return get();
  }

  // Assign from T
  OlxVolatileValue<T>& operator=(const T& v) {
    olx_scope_cs lock(cs);
    value = v;
    return *this;
  }

  // Assign from another OlxVolatileValue<T>
  OlxVolatileValue<T>& operator=(const OlxVolatileValue<T>& other) {
    if (this != &other) {
      // To avoid deadlock in multithreaded assignment, lock in address order
      if (&cs < &other.cs) {
        olx_scope_cs lock1(cs);
        olx_scope_cs lock2(other.cs);
        value = other.value;
      }
      else {
        olx_scope_cs lock1(other.cs);
        olx_scope_cs lock2(cs);
        value = other.value;
      }
    }
    return *this;
  }

  // Lambda-style in-place access (non-const)
  template <typename F>
  void with_lock(F func) {
    olx_scope_cs lock(cs);
    func(value);
  }

  // Lambda-style in-place access (const)
  template <typename F>
  void with_lock(F func) const {
    olx_scope_cs lock(cs);
    func(value);
  }
};

EndEsdlNamespace()
#endif
