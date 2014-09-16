/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "actions.h"
#include "exception.h"
UseEsdlNamespace()

TActionQueue::TActionQueue(TActionQList* parent, const olxstr& name) : Name(name) {
  if (parent == NULL)
    throw TInvalidArgumentException(__OlxSourceInfo, "Parent");
  RunOnce = false;
  Parent = parent;
}
//..............................................................................
void TActionQueue::Add(AActionHandler* A) {
  if (Handlers.Contains(A))
    throw TInvalidArgumentException(__OlxSourceInfo, "handler");
  Handlers.Add(A);
}
//..............................................................................
void TActionQueue::AddFirst(AActionHandler* A) {
  if (Handlers.Contains(A))
    throw TInvalidArgumentException(__OlxSourceInfo, "handler");
  Handlers.Insert(0, A);
}
//..............................................................................
void TActionQueue::Add(AEventsDispatcher* D, int MsgId, short MsgSubId) {
  if (Contains(D))
    throw TInvalidArgumentException(__OlxSourceInfo, "dispatcher");
  Dispatchers.AddNew(MsgId, MsgSubId, D);
}
//..............................................................................
bool TActionQueue::Enter(const IEObject* Sender, const IEObject* Data,
  TActionQueue *caller)
{
  if (!IsEnabled()) return false;
  bool res = false;
  for (size_t i=0; i < Handlers.Count(); i++ ) {
    if (Handlers[i]->IsEnabled()) {
      bool r = Handlers[i]->Enter(Sender, Data, caller == NULL ? this : caller);
      if (r) {
        if (RunOnce) return true;
        res = r;
      }
    }
  }
  for (size_t i=0; i < Dispatchers.Count(); i++) {
    if (Dispatchers[i].Dispatcher->IsEnabled() &&
        (Dispatchers[i].MsgSubId & msiEnter) !=0)
    {
      bool r = Dispatchers[i].Dispatcher->Dispatch(
        Dispatchers[i].MsgId, msiEnter, Sender, Data, caller == NULL ? this : caller);
      if (r) {
        if (RunOnce) return true;
        res = r;
      }
    }
  }
  return res;
}
//..............................................................................
void TActionQueue::UpdateData(const IEObject* Sender, const IEObject* Data,
  TActionQueue *caller)
{
  if (!IsEnabled()) return;
  for (size_t i=0; i < Handlers.Count(); i++) {
    if (Handlers[i]->IsEnabled())
      Handlers[i]->UpdateData(Sender, Data, caller == NULL ? this : caller);
  }
  for (size_t i=0; i < DispatcherCount(); i++) {
    if (Dispatchers[i].Dispatcher->IsEnabled() &&
        (Dispatchers[i].MsgSubId & msiUpdateData) != 0)
    {
      Dispatchers[i].Dispatcher->Dispatch(
        Dispatchers[i].MsgId, msiUpdateData, Sender, Data, caller == NULL ? this : caller);
    }
  }
}
//..............................................................................
bool TActionQueue::Exit(const IEObject* Sender, const IEObject* Data,
  TActionQueue *caller)
{
  if (!IsEnabled()) return false;
  bool res = false;
  for (size_t i=0; i < Handlers.Count(); i++) {
    if (Handlers[i]->IsEnabled()) {
      bool r = Handlers[i]->Exit(Sender, Data, caller == NULL ? this : caller);
      if (r) {
        if (RunOnce) return true;
        res = r;
      }
    }
  }
  for (size_t i=0; i < DispatcherCount(); i++) {
    if (Dispatchers[i].Dispatcher->IsEnabled() &&
        (Dispatchers[i].MsgSubId & msiExit) != 0)
    {
      bool r = Dispatchers[i].Dispatcher->Dispatch(
        Dispatchers[i].MsgId, msiExit, Sender, Data, caller == NULL ? this : caller);
      if (r) {
        if (RunOnce) return true;
        res = r;
      }
    }
  }
  return res;
}
//..............................................................................
bool TActionQueue::Execute(const IEObject* Sender, const IEObject* Data,
  TActionQueue *caller)
{
  if (!IsEnabled()) return false;
  bool res = false;
  for (size_t i=0; i < Handlers.Count(); i++) {
    if (Handlers[i]->IsEnabled()) {
      bool r = Handlers[i]->Execute(Sender, Data, caller == NULL ? this : caller);
      if (r) {
        if (RunOnce) return true;
        res = r;
      }
    }
  }
  for (size_t i=0; i < DispatcherCount(); i++) {
    if (Dispatchers[i].Dispatcher->IsEnabled() &&
        (Dispatchers[i].MsgSubId & msiExecute) != 0)
    {
      bool r = Dispatchers[i].Dispatcher->Dispatch(
        Dispatchers[i].MsgId, msiExecute, Sender, Data, caller == NULL ? this : caller);
      if (r) {
        if (RunOnce) return true;
        res = r;
      }
    }
  }
  return res;
}
//..............................................................................
void TActionQueue::Clear() {
  Handlers.Clear();
  Dispatchers.Clear();
}
//..............................................................................
void TActionQueue::Remove(AActionHandler* A) {
  Handlers.Remove(A);
  A->OnRemove(this);
}
//..............................................................................
bool TActionQueue::Contains(const AActionHandler* A) {
  return Handlers.Contains(A);
}
//..............................................................................
void TActionQueue::Remove(const AEventsDispatcher* D) {
  for (size_t i=0; i < DispatcherCount(); i++) {
    if (Dispatchers[i].Dispatcher == D) {
      Dispatchers.Delete(i);
      break;
    }
  }
}
//..............................................................................
bool TActionQueue::Contains(const AEventsDispatcher* D) {
  for (size_t i=0; i < DispatcherCount(); i++)
    if (Dispatchers[i].Dispatcher == D)
      return true;
  return false;
}
//..............................................................................
void TActionQueue::TakeOver(TActionQueue& aq) {
  Handlers = aq.Handlers;
  Dispatchers = aq.Dispatchers;
  aq.Handlers.Clear();
  aq.Dispatchers.Clear();
  RunOnce = aq.RunOnce;
}
//..............................................................................
//------------------------------------------------------------------------------
//TActionQueue function bodies
//------------------------------------------------------------------------------
//..............................................................................
TActionQueue& TActionQList::New(const olxstr& name)  {
  if( Queues.IndexOf(name) != InvalidIndex )
    throw TFunctionFailedException(__OlxSourceInfo, "the queue already exists");
  TActionQueue* Q = new TActionQueue(this, name);
  Queues.Add(name, Q);
  return *Q;
}
//..............................................................................
TActionQueue& TActionQList::Add(TActionQueue *q)  {
  if( Queues.IndexOf(q->GetName()) != InvalidIndex )
    throw TFunctionFailedException(__OlxSourceInfo, "the queue already exists");
  Queues.Add(q->GetName(), q);
  return *q;
}
//..............................................................................
bool TActionQList::Execute(const olxstr& Name, const IEObject* Sender,
  const IEObject* Data)
{
  TActionQueue* Q = Queues[Name];
  if (Q == NULL)
    throw TFunctionFailedException(__OlxSourceInfo, "undefined action queue");
  return Q->Execute(Sender, Data);
}
//..............................................................................
void TActionQList::Clear()  {
  size_t ac=0;
  TPtrList<AActionHandler> hands;
  for (size_t i=0; i < Queues.Count(); i++)
    ac += Queues.GetObject(i)->HandlerCount();

  hands.SetCapacity(ac);
  ac = 0;
  for (size_t i=0; i < Queues.Count(); i++) {
    TActionQueue* Q = Queues.GetObject(i);
    for (size_t j=0; j < Q->HandlerCount(); j++) {
      hands.Add(Q->GetHandler(j));
      ac++;
    }
    delete Q; // do not need them any more
  }
  // ac = Handlers->Count(); - from previous loop
  hands.ForEach(ACollectionItem::IndexTagSetter());
  for (size_t i=0; i < ac; i++) {
    if ((size_t)hands[i]->GetTag() == i) {  // the object represents the last object ( out of dubs)
      hands[i]->OnRemove(NULL);
      if (hands[i]->IsToDelete())
        delete hands[i];
    }
  }
  Queues.Clear();
}
