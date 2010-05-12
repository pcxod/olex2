#ifndef __olx_sdl_thread_pool_H
#define __olx_sdl_thread_pool_H
#include "olxth.h"

BeginEsdlNamespace()

// a Task interface
class ITask {
public:
  ITask()  {}
  virtual ~ITask()  {}
  virtual void Run() = 0;
};

/* Instacnes of this class are supposed to be in a 'pool' and can be reused without the 
need to create new threads. This allows to vectorise little tasks, where thread creation time 
is longer that the actual task execution */
class TThreadSlot : public AOlxThread  {
  ITask* task;
  volatile bool suspended;
public:
  TThreadSlot() : suspended(false), task(NULL)  {}
  /* the thread automatically becomes suspended when the task is completed,
  and even if a mnew task is set, it will remain suspended until Resume is called */
  virtual int Run();
  void SetTask(ITask& _task);
  bool IsSuspended() const;
  // has no effect on already suspended task
  void Suspend();
  // has no effect on running thread
  void Resume();
};

// a singleton class
class TThreadPool {
  static TTypeList<TThreadSlot> tasks;
  static void _checkThreadCount();
  static olx_critical_section crit_sect;
  static size_t current_task;
public:
  static olx_critical_section& GetCriticalSection() {  return crit_sect;  }
  static size_t GetSlotsCount()   {
    _checkThreadCount();
    return tasks.Count();
  }
  static void AllocateTask(ITask& task);
  static void DoRun();
  //static void AllocateTasks(
};
EndEsdlNamespace()
#endif
