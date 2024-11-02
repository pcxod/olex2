/******************************************************************************
* Copyright (c) 2004-2024 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "olxthpool.h"
#include "bapp.h"

TTypeList<TThreadSlot> TThreadPool::tasks;
size_t TThreadPool::current_task = 0;

TThreadSlot::TThreadSlot()
  : task(NULL),
  suspended(false)
{
  Detached = false;
}
TThreadSlot::~TThreadSlot() {}
bool TThreadSlot::IsSuspended() const {
  return suspended;
}
void TThreadSlot::Suspend()  {
  suspended = true;
}
void TThreadSlot::Resume()  {
  suspended = false;
}
void TThreadSlot::SetTask(ITask& _task)  {
  if (task != 0) { // should never happen...
    throw TFunctionFailedException(__OlxSourceInfo, "Slot is occupied");
  }
  task = &_task;
}
int TThreadSlot::Run() {
  while (true) {
    if (Terminate) {
      Running = false;
      break;
    }
    if (suspended) {
      Yield();
      olx_sleep(1);
      continue;
    }
    if (task != 0) {
      try {
        task->Run();
      }
      catch (const TExceptionBase& e) {
        exception = new TFunctionFailedException(__OlxSrcInfo, e);
      }
      task = 0;
      suspended = true;
    }
  }
  Running = false;
  return 0;
}

void TThreadPool::_checkThreadCount()  {
  int max_th = TBasicApp::GetInstance().GetMaxThreadCount();
  if( max_th <= 0 ) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      "undefined number of possible threads");
  }
  while (tasks.Count() < (size_t)max_th) {
    tasks.Add(new TThreadSlot);
  }
  while( tasks.Count() > (size_t)max_th )  {
    if (tasks.GetLast().IsRunning()) {
      tasks.GetLast().Join(true);
    }
    tasks.Delete(tasks.Count()-1);
  }
}

void TThreadPool::AllocateTask(ITask& task) {
  if (current_task == 0) {
    _checkThreadCount();
  }
  if( current_task >= tasks.Count() ) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "Number of requested and available slots mismatch");
  }
  tasks[current_task++].SetTask(task);
}

void TThreadPool::DoRun() {
  if (current_task == 0) {
    throw TFunctionFailedException(__OlxSourceInfo, "No slots were allocated");
  }
  for (size_t i = 0; i < current_task; i++) {
    if (tasks[i].IsRunning()) {
      tasks[i].Resume();
    }
    else if (!tasks[i].Start()) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "Failed to start thread");
    }
  }
  bool running = true;
  while (running) {
    running = false;
    for (size_t i = 0; i < current_task; i++) {
      if (!tasks[i].IsSuspended()) {
        running = true;
        break;
      }
    }
    AOlxThread::Yield();
    //olx_sleep(50);
  }
  current_task = 0;
  for (size_t i = 0; i < current_task; i++) {
    if (tasks[i].exception.ok()) {
      throw TFunctionFailedException(__OlxSourceInfo, *tasks[i].exception);
    }
  }
}
