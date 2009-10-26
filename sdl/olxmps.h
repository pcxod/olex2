//---------------------------------------------------------------------------

#ifndef olxmpsH
#define olxmpsH
#include "ebase.h"
#include "exception.h"
#include "bapp.h"
#include "log.h"
#include "actions.h"
#include "evector.h"

#include "etime.h"

#ifdef __WIN32__
  #include <process.h>
#else
  #include <pthread.h>
#endif

//---------------------------------------------------------------------------
BeginEsdlNamespace()

const short tLinearTask = 0x0000,
            tQuadraticTask = 0x0001;

template <typename> class TListIteratorManager;

class TIterationTask {
public:
  TIterationTask();
  ~TIterationTask() {  ;  }
  void Run(long index)  {  throw TNotImplementedException(__OlxSourceInfo);  }
  TIterationTask* Replicate()  {  throw TNotImplementedException(__OlxSourceInfo);  }
};

template <class TaskClass>
  class TArrayIterationItem : public IEObject {
    size_t StartIndex, EndIndex, CurrentIndex;
    uint16_t Id;
    TaskClass& Task;
    TActionQList Actions;
  public:
    TArrayIterationItem(TaskClass& task, size_t startIndex, size_t endIndex) : Task(task)  {
      StartIndex = startIndex;
      EndIndex = endIndex;
      CurrentIndex = InvalidIndex;
      OnCompletion = &Actions.NewQueue("ONCOMPLETION");
    }
#ifdef __WIN32__
    static unsigned long _stdcall Run(void* _instance) {
//    static void Run(void* _instance) {
#else
    static void* Run(void* _instance) {
#endif
      TArrayIterationItem<TaskClass>& inst = *(TArrayIterationItem<TaskClass>*)_instance;
      for( inst.CurrentIndex = inst.StartIndex; inst.CurrentIndex < inst.EndIndex; inst.CurrentIndex++ )
        inst.Task.Run( inst.CurrentIndex );
      inst.OnCompletion->Execute(&inst, NULL);
      delete &inst;
      return 0;
    }

    size_t GetCurrentIndex() const  {  return CurrentIndex;  }
    DefPropP(size_t, StartIndex)
    DefPropP(size_t, EndIndex)
    DefPropP(uint16_t, Id)
    TActionQueue *OnCompletion;
  };


template <class TaskClass>
  class TListIteratorManager : public AActionHandler {
    TTypeList< TArrayIterationItem<TaskClass>* > Items;
    TTypeList< TaskClass > Tasks;
    static bool start_thread(TArrayIterationItem<TaskClass>* code)  {
#if defined(__WIN32__)
      unsigned long thread_id;
      HANDLE th = CreateThread(NULL, 0, code->Run, code, 0, &thread_id);
      return (th != NULL);
      //thread_id = _beginthread( code->Run, 4096, code );
      //return (thread_id != -1);
#else
      pthread_t thread_id;
      if( pthread_create(&thread_id,NULL, code->Run,(void*)code) != 0)
#endif
        return false;
      return true;
    }                                                
  protected:
    void CalculateRatios(eveci& res, size_t ListSize, const short TaskType)  {
      const short mt = olx_min(4, TBasicApp::GetInstance().GetMaxThreadCount());
      res.Resize( mt );  // max 4 threads to support
      if( TaskType == tLinearTask )  {
        res[0] = (int)(ListSize/mt);
        for( short i=1; i < mt-1; i++ ) res[i] = res[0];
        if( mt > 1 )
          res[mt-1] = (int)(ListSize - (int)((mt-1)*res[0]));
      }
      else if( TaskType == tQuadraticTask )  {
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
      else
        throw TInvalidArgumentException(__OlxSourceInfo, "unknow task complexity");
    }
  public:
    TListIteratorManager( TaskClass& task, size_t ListSize, const short TaskType, size_t minSize)  {
      SetToDelete(false);
      if( ListSize < minSize || TBasicApp::GetInstance().GetMaxThreadCount() == 1)  {  // should we create parallel tasks then at all?
        for( size_t i=0; i < ListSize; i++ )
          task.Run(i);
        return;
      }
      eveci ratios;
      CalculateRatios(ratios, ListSize, TaskType);
      size_t startIndex = 0;
      TaskClass* taskInstance = &task;
      for( size_t i=0; i < ratios.Count(); i++ )  {
        if( i > 0 )  {
          taskInstance = task.Replicate();
          Tasks.Add( *taskInstance );
        }

        TArrayIterationItem<TaskClass>* item = new TArrayIterationItem<TaskClass>( *taskInstance, startIndex, startIndex + ratios[i] );
        startIndex += ratios[i];
        Items.AddACopy( item ) ;
        item->SetId((short)(Items.Count()-1));
        item->OnCompletion->Add( this );
        start_thread( item );
      }
      while( !IsCompleted() )
        olx_sleep(25);
    }
    virtual bool Execute(const IEObject *Sender, const IEObject *Data=NULL) {
      ((TArrayIterationItem<TaskClass>*)Sender)->OnCompletion->Remove(this);
      Items.NullItem( ((TArrayIterationItem<TaskClass>*)Sender)->GetId() );
      return true;
    }

    bool IsCompleted()  const {
      for( size_t i=0; i < Items.Count(); i++ )  {
        if( !Items.IsNull(i) )  return false;
      }
      return true;
    }
    TTypeList<TaskClass>& GetTasks()  {  return Tasks;  }
  };

EndEsdlNamespace()
#endif
