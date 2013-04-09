/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_mps_H
#define __olx_sdl_mps_H
#include "exception.h"
#include "bapp.h"
#include "actions.h"
#include "evector.h"
#include "olxthpool.h"
#include "etime.h"
BeginEsdlNamespace()

const short
  tLinearTask = 0x0000,
  tQuadraticTask = 0x0001;

template <typename> class TListIteratorManager;

class TaskBase {
protected:
  olx_critical_section *critical_section_;
public:
  TaskBase() : critical_section_(NULL) {}
  void SetCriticalSection(olx_critical_section &cs) {
    critical_section_ = &cs;
  }
  olx_critical_section *GetCriticalSection() const {
    return critical_section_;
  }
};

template <class TaskClass> class TArrayIterationItem : public ITask {
    size_t StartIndex, EndIndex;
    uint16_t Id;
    TaskClass& Task;
  public:
    TArrayIterationItem(TaskClass& task, size_t startIndex, size_t endIndex)
      : Task(task)
    {
      StartIndex = startIndex;
      EndIndex = endIndex;
    }
    virtual void Run() {
      while( StartIndex < EndIndex )
        Task.Run(StartIndex++);
    }
    DefPropP(size_t, StartIndex)
    DefPropP(size_t, EndIndex)
    DefPropP(uint16_t, Id)
  };


template <class TaskClass> class TListIteratorManager {
    TTypeList<TArrayIterationItem<TaskClass> > Items;
    TPtrList<TaskClass> Tasks;
  protected:
    void CalculateRatios(eveci& res, size_t ListSize, const short TaskType)  {
      if( TaskType == tLinearTask )  {
        const short mt = TBasicApp::GetInstance().GetMaxThreadCount();
        res.Resize(mt);  // max 4 threads to support
        res[0] = (int)(ListSize/mt);
        for( short i=1; i < mt-1; i++ ) res[i] = res[0];
        if( mt > 1 )
          res[mt-1] = (int)(ListSize - (int)((mt-1)*res[0]));
      }
      else if( TaskType == tQuadraticTask )  {
        const short mt = olx_min(4,
          TBasicApp::GetInstance().GetMaxThreadCount());
        res.Resize(mt);  // max 4 threads to support
        switch( mt )  {
          case 1:
            res[0] = (int)ListSize;
            break;
          case 2:
            res[0] = (int)(ListSize*(2-sqrt(2.0))/2);
            res[1] = (int)(ListSize - res[0]);
            break;
          case 3:
            res[0] = (int)(ListSize*(2-sqrt(8.0/3))/2);
            res[1] = (int)(ListSize*(2-sqrt(4.0/3))/2 - res[0]);
            res[2] = (int)(ListSize - (res[0]+res[1]));
            break;
          case 4:
            res[0] = (int)(ListSize*(2-sqrt(3.0))/2);
            res[1] = (int)(ListSize*(2-sqrt(2.0))/2 - res[0]);
            res[2] = (int)((ListSize - (res[0]+res[1]))*(2-sqrt(2.0))/2);
            res[3] = (int)(ListSize - (res[0]+res[1]+res[2]));
            break;
          default:
            throw TInvalidArgumentException(__OlxSourceInfo, "thread count");
        }
      }
      else {
        throw TInvalidArgumentException(__OlxSourceInfo,
          "unknown task complexity");
      }
    }
  public:
    TListIteratorManager(TaskClass& task, size_t ListSize, const short TaskType,
      size_t minSize)
    {
      Tasks.Add(&task);  // must not delete it!!!
      // should we create parallel tasks then at all?
      if( ListSize < minSize || TThreadPool::GetSlotsCount() == 1)  {
        for( size_t i=0; i < ListSize; i++ )
          task.Run(i);
        return;
      }
      eveci ratios;
      CalculateRatios(ratios, ListSize, TaskType);
      size_t startIndex = 0;
      TaskClass* taskInstance = &task;
      olx_critical_section cs;
      for( size_t i=0; i < ratios.Count(); i++ )  {
        if( i > 0 )  {
          taskInstance = task.Replicate();
          Tasks.Add(*taskInstance);
        }
        Tasks[i]->SetCriticalSection(cs);
        TArrayIterationItem<TaskClass>& item = Items.Add(
          new TArrayIterationItem<TaskClass>(*taskInstance,
            startIndex, startIndex + ratios[i]));
        startIndex += ratios[i];
        item.SetId((uint16_t)(Items.Count()-1));
        TThreadPool::AllocateTask(item);
      }
      TThreadPool::DoRun();
    }
    ~TListIteratorManager()  {
      Tasks.Delete(0);
      Tasks.DeleteItems(false);
    }
    size_t Count() {  return Tasks.Count();  }
    TaskClass& operator [] (size_t i) {  return *Tasks[i];  }
    const TaskClass& operator [] (size_t i) const {  return *Tasks[i];  }
  };

EndEsdlNamespace()
#endif
