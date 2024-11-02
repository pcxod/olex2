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

TActionQueue::TActionQueue(TActionQList* parent, const olxstr& name)
: Name(name)
{
  if (parent == 0) {
    throw TInvalidArgumentException(__OlxSourceInfo, "Parent");
  }
  RunOnce = false;
  Parent = parent;
}
//..............................................................................
void TActionQueue::Add(const olx_vptr<AActionHandler> &a) {
  Handlers.Add(new olx_vptr<AActionHandler>(a));
}
//..............................................................................
void TActionQueue::InsertFirst(const olx_vptr<AActionHandler> &a) {
  Handlers.Insert(0, new olx_vptr<AActionHandler>(a));
}
//..............................................................................
void TActionQueue::Add(const olx_vptr<AEventsDispatcher> &D,
  int MsgId, short MsgSubId)
{
  Dispatchers.AddNew(MsgId, MsgSubId, D);
}
//..............................................................................
bool TActionQueue::Enter(const IOlxObject* Sender, const IOlxObject* Data,
  TActionQueue *caller)
{
  if (!IsEnabled()) {
    return false;
  }
  bool res = false;
  for (size_t i=0; i < Handlers.Count(); i++ ) {
    AActionHandler &a = Handlers[i];
    if (a.IsEnabled()) {
      bool r = a.Enter(Sender, Data, caller == 0 ? this : caller);
      if (r) {
        if (RunOnce) {
          return true;
        }
        res = r;
      }
    }
  }
  for (size_t i=0; i < Dispatchers.Count(); i++) {
    AEventsDispatcher &d = Dispatchers[i].Dispatcher;
    if (d.IsEnabled() && (Dispatchers[i].MsgSubId & msiEnter) !=0) {
      bool r = d.Dispatch(Dispatchers[i].MsgId, msiEnter, Sender, Data,
        caller == 0 ? this : caller);
      if (r) {
        if (RunOnce) {
          return true;
        }
        res = r;
      }
    }
  }
  return res;
}
//..............................................................................
void TActionQueue::UpdateData(const IOlxObject* Sender, const IOlxObject* Data,
  TActionQueue *caller)
{
  if (!IsEnabled()) {
    return;
  }
  for (size_t i=0; i < Handlers.Count(); i++) {
    AActionHandler &a = Handlers[i];
    if (a.IsEnabled()) {
      a.UpdateData(Sender, Data, caller == 0 ? this : caller);
    }
  }
  for (size_t i=0; i < DispatcherCount(); i++) {
    AEventsDispatcher &d = Dispatchers[i].Dispatcher;
    if (d.IsEnabled() && (Dispatchers[i].MsgSubId & msiUpdateData) != 0) {
      d.Dispatch(Dispatchers[i].MsgId, msiUpdateData, Sender, Data,
        caller == 0 ? this : caller);
    }
  }
}
//..............................................................................
bool TActionQueue::Exit(const IOlxObject* Sender, const IOlxObject* Data,
  TActionQueue *caller)
{
  if (!IsEnabled()) {
    return false;
  }
  bool res = false;
  for (size_t i=0; i < Handlers.Count(); i++) {
    AActionHandler &a = Handlers[i];
    if (a.IsEnabled()) {
      bool r = a.Exit(Sender, Data, caller == 0 ? this : caller);
      if (r) {
        if (RunOnce) {
          return true;
        }
        res = r;
      }
    }
  }
  for (size_t i=0; i < DispatcherCount(); i++) {
    AEventsDispatcher &d = Dispatchers[i].Dispatcher;
    if (d.IsEnabled() && (Dispatchers[i].MsgSubId & msiExit) != 0) {
      bool r = d.Dispatch(Dispatchers[i].MsgId, msiExit, Sender, Data,
        caller == 0 ? this : caller);
      if (r) {
        if (RunOnce) {
          return true;
        }
        res = r;
      }
    }
  }
  return res;
}
//..............................................................................
bool TActionQueue::Execute(const IOlxObject* Sender, const IOlxObject* Data,
  TActionQueue *caller)
{
  if (!IsEnabled()) {
    return false;
  }
  bool res = false;
  for (size_t i=0; i < Handlers.Count(); i++) {
    AActionHandler &a = Handlers[i];
    if (a.IsEnabled()) {
      bool r = a.Execute(Sender, Data, caller == 0 ? this : caller);
      if (r) {
        if (RunOnce) {
          return true;
        }
        res = r;
      }
    }
  }
  for (size_t i=0; i < DispatcherCount(); i++) {
    AEventsDispatcher &d = Dispatchers[i].Dispatcher;
    if (d.IsEnabled() && (Dispatchers[i].MsgSubId & msiExecute) != 0) {
      bool r = d.Dispatch(Dispatchers[i].MsgId, msiExecute, Sender, Data,
        caller == 0 ? this : caller);
      if (r) {
        if (RunOnce) {
          return true;
        }
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
void TActionQueue::Remove(AActionHandler* a) {
  for (size_t i = 0; i < Handlers.Count(); i++) {
    if (Handlers[i] == a) {
      a->OnRemove(this);
      Handlers.Delete(i);
      return;
    }
  }
}
//..............................................................................
bool TActionQueue::Contains(AActionHandler *a) {
  for (size_t i = 0; i < Handlers.Count(); i++) {
    if (Handlers[i] == a) {
      return true;
    }
  }
  return false;
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
bool TActionQueue::Contains(AEventsDispatcher *D) {
  for (size_t i = 0; i < DispatcherCount(); i++) {
    if (Dispatchers[i].Dispatcher == D) {
      return true;
    }
  }
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
TActionQueue& TActionQList::New(const olxstr& name) {
  if (Queues.IndexOf(name) != InvalidIndex) {
    throw TFunctionFailedException(__OlxSourceInfo, "the queue already exists");
  }
  TActionQueue* Q = new TActionQueue(this, name);
  Queues.Add(name, Q);
  return *Q;
}
//..............................................................................
TActionQueue& TActionQList::Add(TActionQueue* q) {
  if (Queues.IndexOf(q->GetName()) != InvalidIndex) {
    throw TFunctionFailedException(__OlxSourceInfo, "the queue already exists");
  }
  Queues.Add(q->GetName(), q);
  return *q;
}
//..............................................................................
bool TActionQList::Execute(const olxstr& Name, const IOlxObject* Sender,
  const IOlxObject* Data)
{
  TActionQueue* Q = Queues.Find(Name, 0);
  if (Q == 0) {
    throw TFunctionFailedException(__OlxSourceInfo, "undefined action queue");
  }
  return Q->Execute(Sender, Data);
}
//..............................................................................
void TActionQList::Clear() {
  size_t ac = 0;
  for (size_t i = 0; i < Queues.Count(); i++) {
    ac += Queues.GetValue(i)->HandlerCount();
  }

  TPtrList<AActionHandler> hands(olx_reserve(ac));
  ac = 0;
  for (size_t i = 0; i < Queues.Count(); i++) {
    TActionQueue* Q = Queues.GetValue(i);
    for (size_t j = 0; j < Q->HandlerCount(); j++) {
      hands.Add(Q->GetHandler(j))->SetTag(ac++);
    }
    delete Q; // do not need them any more
  }
  for (size_t i = 0; i < ac; i++) {
    if ((size_t)hands[i]->GetTag() == i) {
      hands[i]->OnRemove(0);
      if (hands[i]->IsToDelete()) {
        delete hands[i];
      }
    }
  }
  Queues.Clear();
}
