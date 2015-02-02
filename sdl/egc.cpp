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

TEGC* TEGC::Instance = NULL;
volatile bool TEGC::RemovalManaged = false;
//.............................................................................
TEGC::TEGC() {
  // force TBasicApp::OnIdle to delete this object
  AActionHandler::SetToDelete(true);
  ASAPOHead.Next = NULL;
  ASAPOHead.Object = NULL;
  ASAPOTail = NULL;
  ATEOHead.Next = NULL;
  ATEOHead.Object = NULL;
  ATEOTail = NULL;
  if (!TBasicApp::HasInstance())
    RemovalManaged = false;
  else  {
    TBasicApp::GetInstance().OnIdle.Add(this);
    RemovalManaged = true;
  }
  Destructing = false;
}
//.............................................................................
TEGC::~TEGC() {
  Destructing = true;
  ClearASAP();
  ClearATE();
  Instance = NULL;
}
//.............................................................................
void TEGC::ManageRemoval() {
  if (TBasicApp::HasInstance()) {
    RemovalManaged = true;
    TBasicApp::GetInstance().OnIdle.Add(Instance);
  }
}
//.............................................................................
void TEGC::ClearASAP() {
  if (ASAPOHead.Next == NULL)  return;
  OEntry* entry = ASAPOHead.Next;
  while (entry != NULL) {
    entry->Object->RemoveDestructionHandler(&TEGC::AtObjectDestruct);
    delete entry->Object;
    OEntry* en = entry;
    entry = entry->Next;
    delete en;
  }
  ASAPOTail = NULL;
  ASAPOHead.Next = NULL;
}
//.............................................................................
void TEGC::ClearATE() {
  if (ATEOHead.Next == NULL)  return;
  OEntry* entry = ATEOHead.Next;
  while (entry != NULL) {
    entry->Object->RemoveDestructionHandler(&TEGC::AtObjectDestruct);
    delete entry->Object;
    OEntry* en = entry;
    entry = entry->Next;
    delete en;
  }
  ATEOTail = NULL;
  ATEOHead.Next = NULL;
}
//.............................................................................
void TEGC::_AddASAP(IEObject* object) {
  if (!object->AddDestructionHandler(&TEGC::AtObjectDestruct)) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "object is already managed");
  }
  if (ASAPOTail == NULL)
    ASAPOTail = ASAPOHead.Next = new OEntry;
  else {
    ASAPOTail->Next = new OEntry;
    ASAPOTail = ASAPOTail->Next;
  }
  ASAPOTail->Next = NULL;
  ASAPOTail->Object = object;
  // check if the object was already placed to the ATE store ...
  RemoveObject(ATEOHead, object);
}
//.............................................................................
void TEGC::_AddATE(IEObject* object) {
  if (!object->AddDestructionHandler(&TEGC::AtObjectDestruct)) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "object is already managed");
  }
  if (ATEOTail == NULL) {
    ATEOTail = ATEOHead.Next = new OEntry;
  }
  else {
    ATEOTail->Next = new OEntry;
    ATEOTail = ATEOTail->Next;
  }
  ATEOTail->Next = NULL;
  ATEOTail->Object = object;
}
//.............................................................................
void TEGC::_AtObjectDestruct(IEObject* obj) {
  if (Destructing)  return;
  if (!RemoveObject(ASAPOHead, obj)) {
    if (!RemoveObject(ATEOHead, obj)) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        "could not locate the object");
    }
  }
}
//.............................................................................
bool TEGC::RemoveObject(OEntry& head, IEObject* obj) {
  if (head.Next == NULL)  return false;
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
