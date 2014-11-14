/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_egc_H
#define __olx_sdl_egc_H
// atexit prototype
#include <stdlib.h>
#include "actions.h"

/* a very simple garbage collector class
simple objects are deleted when oOnIdle is called next time, whereas the
referencible objects are deleted only when reference count is zero

there are two options for how to delete the object: atexit function can be
used or Delete property of the AActionHandler. In the latter case there is a
danger for asynchronous execution - the object will be deleted when TBasicApp
is deleted...

    if( atexit( &AtExit ) != 0 )
      throw TFunctionFailedException(__OlxSourceInfo, "atexit failed");

  static void AtExit()  {
    if( Instance )  {
      delete Instance;
      Instance = NULL;
    }
  }
*/


BeginEsdlNamespace()

class TEGC : public AActionHandler {
  struct OEntry {
    OEntry* Next;
    IOlxObject* Object;
  };

  static TEGC *&Instance_() {
    static TEGC *i = 0;
    return i;
  }
  static bool& RemovalManaged_() {
    static bool v = false;
    return v;
  }
  // to be deleted as soon as possible
  OEntry  ASAPOHead, *ASAPOTail;
  // to be deleted at the end
  OEntry  ATEOHead, *ATEOTail;
protected:
  bool RemoveObject(OEntry& head, IOlxObject* obj);
  static void ManageRemoval();
  volatile bool Destructing;
protected:
  static void Clear(OEntry *entry);
  static void Add(IOlxObject* object, OEntry &head, OEntry *&tail);
  void ClearASAP();
  void ClearATE();
  void _AddASAP(IOlxObject* object);
  void _AddATE(IOlxObject* object);
  static void AtObjectDestruct(IOlxObject* obj) {
    Instance_()->_AtObjectDestruct(obj);
  }
  void _AtObjectDestruct(IOlxObject* obj);
public:
  TEGC();
  virtual ~TEGC();
  // add an object to be deleted ASAP
  template <class T>
  static T &Add(T* object) {
    GetInstance()._AddASAP(object);
    return *object;
  }
  // add an object with postponed deletion (in the destructor)
  template <class T>
  static T &AddP(T* object) {
    GetInstance()._AddATE(object);
    return *object;
  }
  /* call this before any API calls clike olxstr.c_str() and u_str(), once an
  instance of TBasicAPP, or derived class is created the instance gets attached
  and will be removde automatically
  */
  static TEGC &Initialise() {
    if (Instance_() == NULL) new TEGC();
    if (!RemovalManaged_())
      ManageRemoval();
    return *Instance_();
  }
  /* call this to manually Finalise (nothing will happen in case if TBasicApp
  instance is active), should only be called when an instanve of TBasicApp is
  never created
  */
  static void Finalise() {
    if (!RemovalManaged_() && Instance_() != NULL) {
      delete Instance_();
      Instance_() = NULL;
    }
  }
  static TEGC& GetInstance() {
    return *(Instance_() !=NULL ? Instance_() : new TEGC);
  }

  virtual bool Execute(const IOlxObject*, const IOlxObject*, TActionQueue *) {
    ClearASAP();
    return true;
  }
//.............................................................................
};

EndEsdlNamespace()
#endif
