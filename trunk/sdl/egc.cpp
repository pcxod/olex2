/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "egc.h"
#include "bapp.h"
#include "eset.h"

//.............................................................................
TEGC::TEGC() {
  Instance_() = this;
  // force TBasicApp::OnIdle to delete this object
  AActionHandler::SetToDelete(true);
  ASAPOHead.Next = NULL;
  ASAPOHead.Object = NULL;
  ASAPOTail = NULL;
  ATEOHead.Next = NULL;
  ATEOHead.Object = NULL;
  ATEOTail = NULL;
  if (!TBasicApp::HasInstance())
    RemovalManaged_() = false;
  else {
    TBasicApp::GetInstance().OnIdle.Add(this);
    RemovalManaged_() = true;
  }
  Destructing = false;
}
//.............................................................................
TEGC::~TEGC() {
  Destructing = true;
  ClearASAP();
  ClearATE();
  Instance_() = NULL;
}
//.............................................................................
void TEGC::ManageRemoval() {
  if (TBasicApp::HasInstance()) {
    RemovalManaged_() = true;
    TBasicApp::GetInstance().OnIdle.Add(Instance_());
  }
}
//.............................................................................
void TEGC::Clear(OEntry *entry) {
  while (entry != NULL) {
    ADestructionOservable *o =
      dynamic_cast<ADestructionOservable *>(entry->Object);
    if (o != 0) {
      o->RemoveDestructionObserver(
        DestructionObserver::Make(&TEGC::AtObjectDestruct));
    }
    AReferencible *ar = dynamic_cast<AReferencible *>(entry->Object);
    if (ar != 0) {
      if (ar->DecRef() == 0) {
        delete entry->Object;
      }
    }
    else {
      delete entry->Object;
    }
    OEntry* en = entry;
    entry = entry->Next;
    delete en;
  }
}
//.............................................................................
void TEGC::ClearASAP() {
  Clear(ASAPOHead.Next);
  ASAPOTail = NULL;
  ASAPOHead.Next = NULL;
}
//.............................................................................
void TEGC::ClearATE() {
  Clear(ATEOHead.Next);
  ATEOTail = NULL;
  ATEOHead.Next = NULL;
}
//.............................................................................
void TEGC::Add(IOlxObject* object, OEntry &head, OEntry *&tail) {
  {
    AReferencible *ar = dynamic_cast<AReferencible *>(object);
    if (ar != 0) {
      ar->IncRef();
    }
  }
  {
    ADestructionOservable *o = dynamic_cast<ADestructionOservable *>(object);
    if (o != 0) {
      o->AddDestructionObserver(
        DestructionObserver::Make(&TEGC::AtObjectDestruct));
    }
  }
  if (tail == NULL) {
    tail = head.Next = new OEntry;
  }
  else {
    tail->Next = new OEntry;
    tail = tail->Next;
  }
  tail->Next = NULL;
  tail->Object = object;
}
//.............................................................................
void TEGC::_AddASAP(IOlxObject* object) {
  Add(object, ASAPOHead, ASAPOTail);
  // check if the object was already placed to the ATE store ...
  RemoveObject(ATEOHead, object);
}
//.............................................................................
void TEGC::_AddATE(IOlxObject* object) {
  Add(object, ATEOHead, ATEOTail);
}
//.............................................................................
void TEGC::_AtObjectDestruct(IOlxObject* obj) {
  if (Destructing)  return;
  if (!RemoveObject(ASAPOHead, obj)) {
    if (!RemoveObject(ATEOHead, obj)) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        "could not locate the object");
    }
  }
}
//.............................................................................
bool TEGC::RemoveObject(OEntry& head, IOlxObject* obj) {
  OEntry* entry = &head;
  while (entry != NULL && entry->Next != NULL) {
    if (entry->Next->Object == obj) {
      OEntry* en = entry->Next->Next;
      if (entry->Next == ATEOTail)
        ATEOTail = (entry == &ATEOHead ? NULL : entry);
      else if (entry->Next == ASAPOTail)
        ASAPOTail = (entry == &ASAPOHead ? NULL : entry);
      delete entry->Next;
      entry->Next = en;
      return true;
    }
    entry = entry->Next;
  }
  return false;
}
//.............................................................................
