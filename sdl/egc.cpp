#include "egc.h"
#include "bapp.h"

TEGC* TEGC::Instance = NULL;
volatile bool TEGC::RemovalManaged = false;
//..............................................................................
TEGC::TEGC()  {
  // force TBasicApp::OnIdle to delete this object
  AActionHandler::SetToDelete(true);
  ASAPOHead.Next = NULL;
  ASAPOHead.Object = NULL;
  ASAPOTail = NULL;
  ATEOHead.Next = NULL;
  ATEOHead.Object = NULL;
  ATEOTail = NULL;
  if( !TBasicApp::HasInstance() )
    RemovalManaged = false;
  else  {
    TBasicApp::GetInstance().OnIdle.Add(this);
    RemovalManaged = true;
  }
  Destructing = false;
}
//..............................................................................
TEGC::~TEGC()  {
  Destructing = true;
  ClearASAP();
  ClearATE();
  Instance = NULL;
}
//..............................................................................
void TEGC::ManageRemoval()  {
  if( TBasicApp::HasInstance() )  {
    RemovalManaged = true;
    TBasicApp::GetInstance().OnIdle.Add(Instance);
  }
}
//..............................................................................
void TEGC::ClearASAP()  {
  if( ASAPOHead.Next == NULL )  return;
  OEntry* entry = ASAPOHead.Next;
  while( entry != NULL )  {
    entry->Object->RemoveDestructionHandler(&TEGC::AtObjectDestruct);
    delete entry->Object;
    OEntry* en = entry;
    entry = entry->Next;
    delete en;
  }
  ASAPOTail = NULL;
  ASAPOHead.Next = NULL;
}
//..............................................................................
void TEGC::ClearATE()  {
  if( ATEOHead.Next == NULL )  return;
  OEntry* entry = ATEOHead.Next;
  while( entry != NULL )  {
    entry->Object->RemoveDestructionHandler(&TEGC::AtObjectDestruct);
    delete entry->Object;
    OEntry* en = entry;
    entry = entry->Next;
    delete en;
  }
  ATEOTail = NULL;
  ATEOHead.Next = NULL;
}
//..............................................................................
void TEGC::_AddASAP(IEObject* object)  {
  if( ASAPOTail == NULL )
    ASAPOTail = ASAPOHead.Next = new OEntry;
  else  {
    ASAPOTail->Next = new OEntry;
    ASAPOTail = ASAPOTail->Next;
  }
  ASAPOTail->Next = NULL;
  ASAPOTail->Object = object;
  object->AddDestructionHandler(&TEGC::AtObjectDestruct);
  // check if the object was already placed to the ATE store ...
  RemoveObject( ATEOHead, object );
}
//..............................................................................
void TEGC::_AddATE( IEObject* object )  {
  if( ATEOTail == NULL )  {
    ATEOTail = ATEOHead.Next = new OEntry;
  }
  else  {
    ATEOTail->Next = new OEntry;
    ATEOTail = ATEOTail->Next;
  }
  ATEOTail->Next = NULL;
  ATEOTail->Object = object;
  object->AddDestructionHandler(&TEGC::AtObjectDestruct);
}
//..............................................................................
void TEGC::_AtObjectDestruct(IEObject* obj)  {
  if( Destructing )  return;
  if( !RemoveObject(ASAPOHead, obj) )  {
    if( !RemoveObject(ATEOHead, obj) )
      throw TInvalidArgumentException(__OlxSourceInfo, "could not locate the object");
    else if( ATEOHead.Next == NULL )
        ATEOTail = NULL;
  }
  else
    if( ASAPOHead.Next == NULL )
      ASAPOTail = NULL;
}
//..............................................................................
bool TEGC::RemoveObject(OEntry& head, IEObject* obj)  {
  if( head.Next == NULL )  return false;
  OEntry* entry = &head;
  while( entry&& entry->Next != NULL )  {
    if( entry->Next->Object == obj )  {
      OEntry* en = entry->Next->Next;
      delete entry->Next;
      entry->Next = en;
      return true;
    }
    entry = entry->Next;
  }

  return false;
}
//..............................................................................
