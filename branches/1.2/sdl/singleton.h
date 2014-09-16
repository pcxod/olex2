/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_singleton__H
#define __olx_singleton__H
#include "ebase.h"
#include "edict.h"
#include "olxth.h"
BeginEsdlNamespace()

/* the Impl class must provide the GetInstance method, which will allow it to monitor
the calls to the GetInstance and if required - to do any special processing */
template <class Impl> class SingletonBase  {
protected:
  static Impl Instance;
  SingletonBase()  {}
public:
  static inline Impl& GetInstance()  {  return Instance.GetInstance();  }
};
template <class T> T SingletonBase<T>::Instance;

/* Impl should be as above */
template <class Impl> struct ThreadSingletonMainInstance  {
  Impl Instance;
  olxdict<unsigned long, Impl*, TPrimitiveComparator> Instances;
  unsigned long main_thread_id;
  ThreadSingletonMainInstance() : main_thread_id(AOlxThread::GetCurrentThreadId())  {}
  ~ThreadSingletonMainInstance()  {
    for( int i=0; i < Instances.Count(); i++ )
      delete Instances.GetValue(i);
    Instances.Clear();
  }
};
template <class Impl> class ThreadSingletonBase {
protected:
  static ThreadSingletonMainInstance<Impl> Instance;
  static olx_critical_section cs;
  ThreadSingletonBase() : Impl() {}
  virtual ~ThreadSingletonBase()  {}
public:
  static Impl& GetInstance()  {
    if( Instance.main_thread_id == AOlxThread::GetCurrentThreadId() )
      return Instance.Instance.GetInstance();
    else  {
      unsigned long th_id = AOlxThread::GetCurrentThreadId();
      cs.enter();
      Impl* rv = NULL;
      try  {
        if( Instance.Instances.HasKey(th_id) )
          rv = &Instance.Instances[th_id]->GetInstance();
        else
          rv = &Instance.Instances.Add(th_id, new Impl)->GetInstance();
      }
      catch(...) {}
      cs.leave();
      if( rv == NULL )
        throw TFunctionFailedException(__OlxSourceInfo, "could not acquire object instance");
      return *rv;
    }
  }
  static bool HasInstance()  {
    unsigned long th_id = AOlxThread::GetCurrentThreadId();
    if( th_id == Instance.main_thread_id )
      return true;
    else
      return Instance.Instances.HasKey(th_id);
  }
};

template <class T> ThreadSingletonMainInstance<T> ThreadSingletonBase<T>::Instance;

EndEsdlNamespace()
#endif
