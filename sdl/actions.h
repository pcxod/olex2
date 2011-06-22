/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_actions_H
#define __olx_actions_H
#include "ebase.h"
#include "estlist.h"
#include "tptrlist.h"
#include "typelist.h"
#undef GetObject
BeginEsdlNamespace()

const short
  msiEnter      = 0x0001,
  msiExecute    = 0x0002,
  msiExit       = 0x0004,
  msiUpdateData = 0x0008,
  msiAll        = 0x000F;

class AActionHandler;
class AEventsDispatcher;
class TActionQueue;
class TActionQList;

struct TDispatchInfo  {
  int MsgId;
  int MsgSubId;
  AEventsDispatcher* Dispatcher;
  TDispatchInfo(int msgid, int subid, AEventsDispatcher* dispatcher) : 
    MsgId(msgid), MsgSubId(subid), Dispatcher(dispatcher) {}
};
// abstract class;  an item of the actionqueue
class AActionHandler : public ACollectionItem  {
  bool  Enabled, // not executed if false
        ToDelete;  // default is true - the object will be deleted by the collection!!
public:
  AActionHandler() : Enabled(true), ToDelete(true)  {}
  virtual ~AActionHandler()  {}
  // handler before the event
  virtual bool Enter(const IEObject* Sender, const IEObject* Data=NULL)  {  return false;  }
  // handler after the event
  virtual bool Exit(const IEObject* Sender, const IEObject* Data=NULL)  {  return false;  }
  /* handler during the event, does some operation when Obj is changed;
     the action depends on the type of queue
  */
  virtual void UpdateData(const IEObject* Sender, const IEObject* Data=NULL)  {}
  virtual bool Execute(const IEObject* Sender, const IEObject* Data=NULL)  {  return false;  }
  // the function is called the object is removed from the queue
  virtual void OnRemove()  {}
  DefPropBIsSet(Enabled)
  /* beware that it is set to true by default, which means that the object will
    be automatically deleted by the parent queue
  */
  DefPropBIsSet(ToDelete)
};
/*
 AEventsDispatcher allows to do events/actions hanling within one loop
 (AActionHandler is a single object ususally created  for one event)
 simply derive your class from this one and write the Dispartch method
 Register it in an ActionQueue object (see below)
 */
class AEventsDispatcher: public ACollectionItem  {
  bool Enabled;
public:
  AEventsDispatcher() : Enabled(true)  {}
  virtual ~AEventsDispatcher()  {}
  /* dispatch is called all time an event occures, MsgId is the Id passed when the
     object is registered in a queue, MsgSubId is one msiEnter, msiExecute, msiExit or
     msiUpdateData (see the discription of cirtain events). Sender is the object sending the message,
     Data -  is the object being changed or some data object
  */
  virtual bool Dispatch(int MsgId, short MsgSubId, const IEObject* Sender, const IEObject* Data=NULL)=0;
  DefPropBIsSet(Enabled)
};
class TActionQueue: public AActionHandler  {
  TPtrList<AActionHandler> Handlers;
  TTypeList<TDispatchInfo> Dispatchers;
  olxstr Name;
  TActionQList* Parent;
  bool ExecuteOnce; // set to true to stop of the first successful handler, false by default
public:
  TActionQueue(TActionQList* Parent, const olxstr& Name);
  virtual ~TActionQueue()  {}
  
  // copies handlers and dispatchers etc and clears the aq
  void TakeOver(TActionQueue& aq);

  size_t HandlerCount() const {  return Handlers.Count(); };
  AActionHandler& GetHandler(size_t i) const {  return *Handlers[i]; }

  size_t DispatcherCount() const {  return Dispatchers.Count(); };
  AEventsDispatcher& GetDispatcher(size_t i) const {  return *Dispatchers[i].Dispatcher;  }
  // adds new handler
  void Add(AActionHandler* handler);
  /* inserts a new handler at the beginning of the list so that it will be executed first of all */
  void AddFirst(AActionHandler* handler);
  // adds new dispatcher, no AddFirst - Handlers are executed before the dispatchers...
  void Add(AEventsDispatcher* dispatcher, int MsgId, short MsgSubId = msiAll);
  //makes subsequent calls to the handlers stored in the list
  bool Execute(const IEObject* Sender, const IEObject* Data=NULL);
  bool Enter(const IEObject* Sender, const IEObject* Data=NULL);
  bool Exit(const IEObject* Sender, const IEObject* Data=NULL);
  void UpdateData(const IEObject* Sender, const IEObject* Data);
  virtual void OnRemove()  {}
  // empties the queue
  void Clear();
  // removes specified handler from the queue
  void Remove(AActionHandler* handler);
  bool Contains(AActionHandler* handler);

  // removes specified dispatcher from the queue
  void Remove(AEventsDispatcher* dispatcher);
  // returns true if a specified handler belongs to the queue
  bool Contains(AEventsDispatcher* dispatcher);
  // returns the name of the queue
  const olxstr& GetName() const {  return Name;  }
};

class TActionQList: public IEObject  {
private:
  TCSTypeList<olxstr,TActionQueue*> Queues;
public:
  TActionQList()  {}
  virtual ~TActionQList() {  Clear();  }
  /* is used to create a new named actions queue
    throws an exception if the queue already exists: use Exists to check
  */
  TActionQueue& New(const olxstr& Name);
  // executes a named queue
  bool Execute(const olxstr& Name, const IEObject* Sender, const IEObject* Data=NULL);

  bool Exists(const olxstr& Name) const {  return Queues.IndexOf(Name) != InvalidIndex;  }
  /* throws exception if the queue does not exist */
  TActionQueue* Find(const olxstr& Name) const {  return Queues[Name];  }
  // queue by index
  TActionQueue& Get(size_t index) const {  return *Queues.GetObject(index);  }
  TActionQueue& operator [](size_t index) const {  return *Queues.GetObject(index);  }
  // returns the number of queues
  size_t Count() const {  return Queues.Count();  }
  // empties the list
  void Clear();
};

// action proxy object, allows synchronisation of two action queues
class TActionProxy : public AActionHandler  {
  TActionQueue& queue;
public:
  TActionProxy(TActionQueue& q) : queue(q) {}
  virtual bool Enter(const IEObject* Sender, const IEObject* Data=NULL) {  
	  return queue.Enter(Sender, Data);
	}
  virtual bool Exit(const IEObject* Sender, const IEObject* Data=NULL)  {
	  return queue.Exit(Sender, Data);
	}
  virtual void UpdateData(const IEObject* Sender, const IEObject* Data=NULL)  {
	  queue.UpdateData(Sender, Data);
	}
  virtual bool Execute(const IEObject* Sender, const IEObject* Data=NULL)  {
	  return queue.Execute(Sender, Data);
	} 
};
// disabled the queue when created and restores the state (if was enabled!) when destroyed
class TActionQueueLock  {
  TActionQueue* queue;
  bool changed;
public:
  TActionQueueLock(TActionQueue* _queue) : queue(_queue), changed(false)  {
    if( queue != NULL )  {
      if( queue->IsEnabled() )  {
        changed = true;
        queue->SetEnabled(false);
      }
    }
  }
  ~TActionQueueLock()  {
    if( changed && queue != NULL )
      queue->SetEnabled(true);
  }
  void Unlock()  {
    if( changed )  {
      if( queue != NULL )
        queue->SetEnabled(true);
      changed = false;
    }
  }
};

class TOnProgress: public IEObject {  // generic on progress data
  uint64_t Max, Pos;
  olxstr Action;
public:
  TOnProgress() : Max(0), Pos(0)  {}
  virtual ~TOnProgress()  {}
  // set Max to a valid number before OnProgress is executed!
  DefPropP(uint64_t, Max)
  DefPropP(uint64_t, Pos)
  inline void IncPos(uint64_t v=1)   {  Pos += v;  }
  DefPropC(olxstr, Action)
	
	TOnProgress& operator = (const TOnProgress& pg)  {
	  Max = pg.Max;
		Pos = pg.Pos;
		Action = pg.Action;
		return *this;
	}
};

EndEsdlNamespace()
#endif
