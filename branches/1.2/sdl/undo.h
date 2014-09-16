/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_undo_H
#define __olx_sdl_undo_H
#include "tptrlist.h"
BeginEsdlNamespace()

class IUndoAction;
class TUndoData;

class TUndoStack  {
  TPtrList<TUndoData> UndoList;
public:
  TUndoStack() {}
  virtual ~TUndoStack() {  Clear();  }
  // for convinience
  const TPtrList<TUndoData>& GetList() const {  return UndoList;  }
  void Clear();
  bool isEmpty() const {  return UndoList.Count() == 0;  }
  /* the function will not do anything for a NULL UndoObject, which might
  appear when a function cannot be undone or undone safely (graphical objects)
  */
  inline TUndoData* Push(TUndoData* UndoObject)  {
    if (UndoObject!= NULL)
      UndoList.Add(UndoObject);
    return UndoObject;
  }
  TUndoData* Pop();
};

class IUndoAction  {
public:
  IUndoAction() {}
  virtual ~IUndoAction() {}
  virtual void Execute(TUndoData* data) = 0;
};

class TUndoData : public IEObject {
  TPtrList<TUndoData> UndoList;
  IUndoAction* UndoAction;
public:
  TUndoData(IUndoAction* a) { UndoAction = a; }
  virtual ~TUndoData();
  virtual void Undo() {
    if (UndoAction)
      UndoAction->Execute(this);
    for (size_t i=0; i < UndoList.Count(); i++)
      UndoList[i]->Undo();
  }
  void AddAction(TUndoData* data) { UndoList.Add(data); }
};


template <class BaseClass> class TUndoActionImplMF
  : public IUndoAction
{
  BaseClass *BC;
  void (BaseClass::*MFunc)(TUndoData*);
public:
  TUndoActionImplMF(BaseClass* base, void (BaseClass::*f)(TUndoData*))
    : BC(base), MFunc(f)
  {}
  virtual void Execute(TUndoData *Data) { (BC->*MFunc)(Data); }
};

class TUndoActionImplSF: public IUndoAction {
  void (*SFunc)(TUndoData*);
public:
  TUndoActionImplSF(void (*f)(TUndoData*))
    : SFunc(f)
  {}
  virtual void Execute(TUndoData *Data) { (*SFunc)(Data); }
};

struct UndoAction {
  template <class base_t>
  static TUndoActionImplMF<base_t> *
  New(base_t *base, void (base_t::*f)(TUndoData*)) {
    return new TUndoActionImplMF<base_t>(base, f);
  }

  static TUndoActionImplSF *New(void (*f)(TUndoData*)) {
    return new TUndoActionImplSF(f);
  }
};

EndEsdlNamespace()
#endif
