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
  struct OEntry  {
    OEntry* Next;
    IEObject* Object;
  };

  static TEGC* Instance;
  static volatile bool RemovalManaged;
  // to be deleted as soon as possible
  OEntry  ASAPOHead, *ASAPOTail;
  // to be deleted at the end
  OEntry  ATEOHead, *ATEOTail;
protected:
  bool RemoveObject(OEntry& head, IEObject* obj);
  static void ManageRemoval();
  volatile bool Destructing;
protected:
  void ClearASAP();
  void ClearATE();
  void _AddASAP(IEObject* object);
  void _AddATE(IEObject* object);
  static void AtObjectDestruct(IEObject* obj) {
    Instance->_AtObjectDestruct(obj);
  }
  void _AtObjectDestruct(IEObject* obj);
public:
  TEGC();
  virtual ~TEGC();
  // add an object to be deleted ASAP
  static void Add(IEObject* object) {
    GetInstance()->_AddASAP(object);
  }
  // add an object with postponed deletion (in the destructor)
  static void AddP(IEObject* object) {
    GetInstance()->_AddATE(object);
  }
  /* call this before any API calls clike olxstr.c_str() and u_str(), once an
  instance of TBasicAPP, or derived class is created the instance gets attached
  and will be removde automatically
  */
  static void Initialise() {
    if (Instance == NULL) Instance = new TEGC;
    if (!RemovalManaged) ManageRemoval();
  }
  /* call this to manually Finalise (nothing will happen in case if TBasicApp
  instance is active), should only be called when an instanve of TBasicApp is
  never created
  */
  static void Finalise() {
    if (!RemovalManaged && Instance != NULL) {
      delete Instance;
      Instance = NULL;
    }
  }
  static TEGC* GetInstance() {
    return (Instance!=NULL ? Instance : (Instance = new TEGC));
  }

  virtual bool Execute(const IEObject*, const IEObject*, TActionQueue *) {
    ClearASAP();
    return true;
  }
//.............................................................................
  template<class T>
  static T& New() {
    T* o = new T();
    Add(o);
    return *o;
  }
//.............................................................................
  template<class T, class AC>
  static T& New(const AC& arg) {
    T* o = new T(arg);
    Add(o);
    return *o;
  }
//.............................................................................
  template<class T, class FAC, class SAC>
  static T& New( const FAC& arg1, const SAC& arg2 ) {
    T* o = new T(arg1, arg2);
    Add(o);
    return *o;
  }
//.............................................................................
  template<class T, class FAC, class SAC, class TAC>
  static T& New(const FAC& arg1, const SAC& arg2, const TAC& arg3) {
    T* o = new T(arg1, arg2, arg3);
    Add(o);
    return *o;
  }
//.............................................................................
  template<class T, class FAC, class SAC, class TAC, class FrAC>
  static T& New(const FAC& arg1, const SAC& arg2, const TAC& arg3,
    const FrAC& arg4)
  {
    T* o = new T(arg1, arg2, arg3, arg4);
    Add(o);
    return *o;
  }
//.............................................................................
  template<class T, class FAC, class SAC, class TAC, class FrAC, class FvAC>
  static T& New(const FAC& arg1, const SAC& arg2, const TAC& arg3,
    const FrAC& arg4, const FvAC& arg5)
  {
    T* o = new T(arg1, arg2, arg3, arg4, arg5);
    Add(o);
    return *o;
  }
//.............................................................................
  template<class T>
  static T& NewG()  {
    T* o = new T();
    AddP(o);
    return *o;
 }
//.............................................................................
  template<class T, class AC>
  static T& NewG(const AC& arg) {
      T* o = new T(arg);
      AddP(o);
      return *o;
  }
//.............................................................................
  template<class T, class FAC, class SAC>
  static T& NewG(const FAC& arg1, const SAC& arg2) {
    T* o = new T(arg1, arg2);
    AddP(o);
    return *o;
  }
//.............................................................................
  template<class T, class FAC, class SAC, class TAC>
  static T& NewG(const FAC& arg1, const SAC& arg2, const TAC& arg3) {
    T* o = new T(arg1, arg2, arg3);
    AddP(o);
    return *o;
  }
//.............................................................................
  template<class T, class FAC, class SAC, class TAC, class FrAC>
  static T& NewG( const FAC& arg1, const SAC& arg2, const TAC& arg3,
    const FrAC& arg4)
  {
    T* o = new T(arg1, arg2, arg3, arg4);
    AddP(o);
    return *o;
  }
//.............................................................................
  template<class T, class FAC, class SAC, class TAC, class FrAC, class FvAC>
  static T& NewG(const FAC& arg1, const SAC& arg2, const TAC& arg3,
    const FrAC& arg4, const FvAC& arg5)
  {
    T* o = new T(arg1, arg2, arg3, arg4, arg5);
    AddP(o);
    return *o;
  }
};

EndEsdlNamespace()
#endif
