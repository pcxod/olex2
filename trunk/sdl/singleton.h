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
  olxdict<int, Impl*, TPrimitiveComparator> Instances;
  int main_thread_id;
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
  ThreadSingletonBase() : Impl() {}
  virtual ~ThreadSingletonBase()  {}
public:
  static Impl& GetInstance()  {
    if( Instance.main_thread_id == AOlxThread::GetCurrentThreadId() )
      return Instance.Instance.GetInstance();
    else  {
      int th_id = AOlxThread::GetCurrentThreadId();
      if( Instance.Instances.HasKey(th_id) )
        return Instance.Instances[th_id]->GetInstance();
      else 
        return Instance.Instances.Add(th_id, new Impl)->GetInstance();
    }
  }
  static bool HasInstance()  {
    int th_id = AOlxThread::GetCurrentThreadId();
    if( th_id == main_thread_id )
      return true;
    else
      return Instance.Instances.HasKey(th_id);
  }
};

template <class T> ThreadSingletonMainInstance<T> ThreadSingletonBase<T>::Instance;

EndEsdlNamespace()
#endif
