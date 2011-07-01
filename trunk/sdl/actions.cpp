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

TActionQueue::TActionQueue(TActionQList* parent, const olxstr& name) : Name(name)  {
  if( parent == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "Please, use TActionQList to create actions");
  ExecuteOnce = false;
  Parent = parent;
}
//..............................................................................
void TActionQueue::Add(AActionHandler* A)  {
  if( Handlers.IndexOf(A) != InvalidIndex )
    throw TFunctionFailedException(__OlxSourceInfo, "the handler is already in the list");
  Handlers.Add(A);
}
//..............................................................................
void TActionQueue::AddFirst(AActionHandler* A)  {
  if( Handlers.IndexOf(A) != InvalidIndex )
    throw TFunctionFailedException(__OlxSourceInfo, "the handler is already in the list");
  Handlers.Insert(0, A);
}
//..............................................................................
void TActionQueue::Add(AEventsDispatcher* D, int MsgId, short MsgSubId)  {
  if( Contains(D) )
    throw TFunctionFailedException(__OlxSourceInfo, "the dispatcher is already in the list");
  Dispatchers.AddNew(MsgId, MsgSubId, D);
}
//..............................................................................
bool TActionQueue::Enter(const IEObject* Sender, const IEObject* Data)  {
  if( !IsEnabled() ) return false;
  for( size_t i=0; i < Handlers.Count(); i++ )  {
    if( Handlers[i]->IsEnabled() )
      if( Handlers[i]->Enter(Sender, Data) && ExecuteOnce )
        return true;
  }
  for( size_t i=0; i < Dispatchers.Count(); i++ )  {
    if( Dispatchers[i].Dispatcher->IsEnabled() )  {
      if( Dispatchers[i].MsgSubId & msiEnter )
        if( Dispatchers[i].Dispatcher->Dispatch(Dispatchers[i].MsgId, msiEnter, Sender, Data) && ExecuteOnce )
          return true;
    }
  }
  return false;
}
//..............................................................................
void TActionQueue::UpdateData(const IEObject* Sender, const IEObject* Data)  {
  if( !IsEnabled() ) return;
  for( size_t i=0; i < Handlers.Count(); i++ )  {
    if( Handlers[i]->IsEnabled() )
      Handlers[i]->UpdateData(Sender, Data);
  }
  for( size_t i=0; i < DispatcherCount(); i++ )  {
    if( Dispatchers[i].Dispatcher->IsEnabled() )  {
      if( Dispatchers[i].MsgSubId & msiUpdateData )
        Dispatchers[i].Dispatcher->Dispatch(Dispatchers[i].MsgId, msiUpdateData, Sender, Data);
    }
  }
}
//..............................................................................
bool TActionQueue::Exit(const IEObject* Sender, const IEObject* Data)  {
  if( !IsEnabled() ) return false;
  for( size_t i=0; i < Handlers.Count(); i++ )  {
    if( Handlers[i]->IsEnabled() )
      if( Handlers[i]->Exit(Sender, Data) && ExecuteOnce )
        return true;
  }
  for( size_t i=0; i < DispatcherCount(); i++ )  {
    if( Dispatchers[i].Dispatcher->IsEnabled() )  {
      if( Dispatchers[i].MsgSubId & msiExit )
        if( Dispatchers[i].Dispatcher->Dispatch(Dispatchers[i].MsgId, msiExit, Sender, Data) && ExecuteOnce )
          return true;
    }
  }
  return false;
}
//..............................................................................
bool TActionQueue::Execute(const IEObject* Sender, const IEObject* Data)  {
  if( !IsEnabled() ) return false;
  bool res = false;
  for( size_t i=0; i < Handlers.Count(); i++ )  {
    if( Handlers[i]->IsEnabled() )  {
      bool lres = Handlers[i]->Execute(Sender, Data);
      if( lres )  res = lres;
    }
  }
  for( size_t i=0; i < DispatcherCount(); i++ )  {
    if( Dispatchers[i].Dispatcher->IsEnabled() )  {
      if( Dispatchers[i].MsgSubId & msiExecute )  {
        bool lres = Dispatchers[i].Dispatcher->Dispatch(Dispatchers[i].MsgId, msiExecute, Sender, Data);
        if( ExecuteOnce ) return true;
        if( lres )  res = lres;
      }
    }
  }
  return res;
}
//..............................................................................
void TActionQueue::Clear()  {
  Handlers.Clear();
  Dispatchers.Clear();
}
//..............................................................................
void TActionQueue::Remove(AActionHandler* A)  {
  Handlers.Remove(A);
  A->OnRemove();
}
//..............................................................................
bool TActionQueue::Contains(AActionHandler* A)  {
  if( Handlers.IndexOf(A) != InvalidIndex )
    return true;
  return false;
}
//..............................................................................
void TActionQueue::Remove(AEventsDispatcher* D)  {
  for( size_t i=0; i < DispatcherCount(); i++ )  {
    if( Dispatchers[i].Dispatcher == D )  {
      Dispatchers.Delete(i);
      break;
    }
  }
}
//..............................................................................
bool TActionQueue::Contains(AEventsDispatcher* D)  {
  for( size_t i=0; i < DispatcherCount(); i++ )
    if( Dispatchers[i].Dispatcher == D )  return true;
  return false;
}
//..............................................................................
void TActionQueue::TakeOver(TActionQueue& aq)  {
  Handlers = aq.Handlers;
  Dispatchers = aq.Dispatchers;
  aq.Handlers.Clear();
  aq.Dispatchers.Clear();
  ExecuteOnce = aq.ExecuteOnce;
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
bool TActionQList::Execute(const olxstr& Name, const IEObject* Sender, const IEObject* Data)  {
  TActionQueue* Q = Queues[Name];
  if( Q == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "undefined action queue");
  return Q->Execute(Sender, Data);
}
//..............................................................................
void TActionQList::Clear()  {
  size_t ac=0;
  TPtrList<AActionHandler> hands;
  for( size_t i = 0; i < Queues.Count(); i++ )
    ac += Queues.GetObject(i)->HandlerCount();

  hands.SetCapacity(ac);
  ac = 0;
  for( size_t i = 0; i < Queues.Count(); i++ )  {
    TActionQueue* Q = Queues.GetObject(i);
    for( size_t j = 0; j < Q->HandlerCount(); j++ )  {
      hands.Add(Q->GetHandler(j));
      ac++;
    }
    delete Q; // do not need them anymore
  }
  // ac = Handlers->Count(); - from previous loop
  for( size_t i =0; i < ac; i++ )
    hands[i]->SetTag(i);

  for( size_t i =0; i < ac; i++ )  {
    hands[i]->OnRemove();
    if( hands[i]->GetTag() == i )  {  // the object represents the last object ( out of dubs)
      if( hands[i]->IsToDelete() )
        delete hands[i];
    }
  }
  Queues.Clear();
}
