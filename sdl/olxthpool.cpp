#include "olxthpool.h"
#include "bapp.h"

TTypeList<TThreadSlot> TThreadPool::tasks;
olx_critical_section TThreadPool::crit_sect;
size_t TThreadPool::current_task = 0;

bool TThreadSlot::IsSuspended() const {
  volatile olx_scope_cs _cs(TThreadPool::GetCriticalSection());
  return suspended;
}
void TThreadSlot::Suspend()  {
  volatile olx_scope_cs _cs(TThreadPool::GetCriticalSection());
  suspended = true;
}
void TThreadSlot::Resume()  {
  volatile olx_scope_cs _cs(TThreadPool::GetCriticalSection());
  suspended = false;
}
void TThreadSlot::SetTask(ITask& _task)  {
  volatile olx_scope_cs _cs(TThreadPool::GetCriticalSection());
  if( task != NULL )  // should never happen...
    throw TFunctionFailedException(__OlxSourceInfo, "Slot is occupied");
  task = &_task;
}
int TThreadSlot::Run() {
  while( true )  {
    if( Terminate )  break;
    if( suspended )  {
      Yield();
      olx_sleep(5);
      continue;
    }
    if( task != NULL )  {
      task->Run();
      task = NULL;
      suspended = true;
    }
  }
  return 0;
}

void TThreadPool::_checkThreadCount()  {
  int max_th = TBasicApp::GetInstance().GetMaxThreadCount();
  if( max_th <= 0 )
    throw TInvalidArgumentException(__OlxSourceInfo, "undefined number of possible threads");
  while( tasks.Count() < (size_t)max_th )
    tasks.AddNew();
  while( tasks.Count() > (size_t)max_th )  {
    if( tasks.Last().IsRunning() )
      tasks.Last().Join(true);
    tasks.Delete(tasks.Count()-1);
  }
}

void TThreadPool::AllocateTask(ITask& task) {
  if( current_task == 0 )
    _checkThreadCount();
  if( current_task >= tasks.Count() )
    throw TFunctionFailedException(__OlxSourceInfo, "Number of requested and available slots mismatch");
  tasks[current_task++].SetTask(task);
}

void TThreadPool::DoRun()  {
  if( current_task == 0 )
    throw TFunctionFailedException(__OlxSourceInfo, "No slots were alloceted");
  for( size_t i=0; i < current_task; i++ )  {
    if( tasks[i].IsRunning() )
      tasks[i].Resume();
    else if( !tasks[i].Start() )  {
      throw TFunctionFailedException(__OlxSourceInfo, "Failed to start thread");
    }
  }
  bool running = true;
  while( running )  {
    running = false;
    for( size_t i=0; i < current_task; i++ )  {
      if( !tasks[i].IsSuspended() )  {
        running = true;
        break;
      }
    }
    AOlxThread::Yield();
    //olx_sleep(50);
  }
  current_task = 0;
}

