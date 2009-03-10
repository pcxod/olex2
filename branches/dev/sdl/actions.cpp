//----------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "actions.h"
#include "exception.h"

UseEsdlNamespace()
//------------------------------------------------------------------------------
//TActionHandler function bodies
//------------------------------------------------------------------------------
AActionHandler::AActionHandler()  {
  SetToDelete(true);
  SetEnabled(true);
};
//..............................................................................
AActionHandler::~AActionHandler()  {  return;  }
//------------------------------------------------------------------------------
//TEventsDispatcher function bodies
//------------------------------------------------------------------------------
AEventsDispatcher::AEventsDispatcher()  {  SetEnabled(true); }
//..............................................................................
AEventsDispatcher::~AEventsDispatcher() {  return; }
//..............................................................................
//------------------------------------------------------------------------------
//TActionQueue function bodies
//------------------------------------------------------------------------------
TActionQueue::TActionQueue(TActionQList *Parent, const olxstr & Nm)  {
  if( Parent == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "Please, use TActionQList to create actions");
  FName = Nm;
  FExecuteOnce = false;
  FParent = Parent;
}
//..............................................................................
TActionQueue::~TActionQueue()  {
}
//..............................................................................
void TActionQueue::Add(AActionHandler *A)  {
  if( Handlers.IndexOf(A) != -1 )
    throw TFunctionFailedException(__OlxSourceInfo, "the handler is already in the list");
  Handlers.Add(A);
}
//..............................................................................
void TActionQueue::AddFirst(AActionHandler *A)  {
  if( Handlers.IndexOf(A) != -1 )
    throw TFunctionFailedException(__OlxSourceInfo, "the handler is already in the list");
  Handlers.Insert(0, A);
}
//..............................................................................
void TActionQueue::Add(AEventsDispatcher *D, int MsgId, short MsgSubId)  {
  if( Contains(D) )
    throw TFunctionFailedException(__OlxSourceInfo, "the dispatcher is already in the list");
  Dispatchers.AddNew(MsgId, MsgSubId, D);
}
//..............................................................................
bool TActionQueue::Enter(const IEObject *Sender, const IEObject *Data)  {
  if( (!HandlerCount() && !DispatcherCount()) || !IsEnabled() ) return false;
  for( int i=0; i < HandlerCount(); i++ )  {
    if( Handlers[i]->IsEnabled() )
      if( Handlers[i]->Enter(Sender, Data) && FExecuteOnce )
        return true;
  }
  for( int i=0; i < DispatcherCount(); i++ )  {
    if( Dispatchers[i].Dispatcher->IsEnabled() )  {
      if( Dispatchers[i].MsgSubId & msiEnter )
        if( Dispatchers[i].Dispatcher->Dispatch(Dispatchers[i].MsgId, msiEnter, Sender, Data) && FExecuteOnce )
          return true;
    }
  }
  return false;
}
//..............................................................................
void TActionQueue::UpdateData(const IEObject *Sender, const IEObject *Data)  {
  if( (!HandlerCount() && !DispatcherCount()) || !IsEnabled() ) return;
  for( int i=0; i < Handlers.Count(); i++ )  {
    if( Handlers[i]->IsEnabled() )
      Handlers[i]->UpdateData(Sender, Data);
  }
  for( int i=0; i < DispatcherCount(); i++ )  {
    if( Dispatchers[i].Dispatcher->IsEnabled() )  {
      if( Dispatchers[i].MsgSubId & msiUpdateData )
        Dispatchers[i].Dispatcher->Dispatch(Dispatchers[i].MsgId, msiUpdateData, Sender, Data);
    }
  }
}
//..............................................................................
bool TActionQueue::Exit(const IEObject *Sender, const IEObject *Data)  {
  if( (!HandlerCount() && !DispatcherCount()) || !IsEnabled() ) return false;
  for( int i=0; i < Handlers.Count(); i++ )  {
    if( Handlers[i]->IsEnabled() )
      if( Handlers[i]->Exit(Sender, Data) && FExecuteOnce )
        return true;
  }
  for( int i=0; i < DispatcherCount(); i++ )  {
    if( Dispatchers[i].Dispatcher->IsEnabled() )  {
      if( Dispatchers[i].MsgSubId & msiExit )
        if( Dispatchers[i].Dispatcher->Dispatch(Dispatchers[i].MsgId, msiExit, Sender, Data) && FExecuteOnce )
          return true;
    }
  }
  return false;
}
//..............................................................................
bool TActionQueue::Execute(const IEObject *Sender, const IEObject *Data)  {
  if( (!HandlerCount() && !DispatcherCount()) || !IsEnabled() ) return false;
  bool res = false, lres;
  for( int i=0; i < Handlers.Count(); i++ )  {
    if( Handlers[i]->IsEnabled() )  {
      lres = Handlers[i]->Execute(Sender, Data);
      if( lres )  res = lres;
    }
  }
  for( int i=0; i < DispatcherCount(); i++ )  {
    if( Dispatchers[i].Dispatcher->IsEnabled() )  {
      if( Dispatchers[i].MsgSubId & msiExecute )  {
        lres = Dispatchers[i].Dispatcher->Dispatch(Dispatchers[i].MsgId, msiExecute, Sender, Data);
        if( FExecuteOnce ) return true;
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
void TActionQueue::Remove(AActionHandler *A)  {
  Handlers.Remove(A);
  A->OnRemove();
}
//..............................................................................
bool TActionQueue::Contains(AActionHandler *A)  {
  if( Handlers.IndexOf(A) != -1 )
    return true;
  return false;
}
//..............................................................................
void TActionQueue::Remove(AEventsDispatcher *D)  {
  for(int i=0; i < DispatcherCount(); i++ )  {
    if( Dispatchers[i].Dispatcher == D )  {
      Dispatchers.Delete(i);
      break;
    }
  }
}
//..............................................................................
bool TActionQueue::Contains(AEventsDispatcher *D)  {
  for(int i=0; i < DispatcherCount(); i++ )
    if( Dispatchers[i].Dispatcher == D )  return true;
  return false;
}
//..............................................................................
//------------------------------------------------------------------------------
//TActionQueue function bodies
//------------------------------------------------------------------------------
//..............................................................................
TActionQueue& TActionQList::NewQueue(const olxstr &Name)  {
  if( Queues.IndexOfComparable(Name) != -1 )
    throw TFunctionFailedException(__OlxSourceInfo, "the queue already exists");
  TActionQueue *Q = new TActionQueue(this, Name);
  Queues.Add(Name, Q);
  return *Q;
}
//..............................................................................
bool TActionQList::Execute(const olxstr &Name, const IEObject *Sender, const IEObject *Data)  {
  TActionQueue *Q = Queues[Name];
  if( Q == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "undefined action queue");
  return Q->Execute(Sender, Data);
}
//..............................................................................
void TActionQList::Clear()  {
  int ac=0;
  TPtrList<AActionHandler> hands;;
  TActionQueue* Q;
  for( int i = 0; i < Queues.Count(); i++ )
    ac += Queues.GetObject(i)->HandlerCount();

  hands.SetCapacity(ac);
  ac = 0;
  for( int i = 0; i < Queues.Count(); i++ )  {
    Q = Queues.GetObject(i);
    for( int j = 0; j < Q->HandlerCount(); j++ )  {
      hands.Add( Q->Handler(j) );
      ac++;
    }
    delete Q; // do not need them anymore
  }
  // ac = Handlers->Count(); - from previous loop
  for( int i =0; i < ac; i++ )
    hands[i]->SetTag(i);

  for( int i =0; i < ac; i++ )  {
    hands[i]->OnRemove();
    if( hands[i]->GetTag() == i )  {  // the object represents the last object ( out of dubs)
      if( hands[i]->IsToDelete() )
        delete hands[i];
    }
  }
  Queues.Clear();
}



