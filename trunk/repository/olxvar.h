/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_olxvar_H
#define __olx_olxvar_H
#include "estlist.h"
#include "actions.h"
#include "bapp.h"
#include "wildcard.h"
#include "os_util.h"

#ifdef _PYTHON
  #include "pyext.h"
// python object types
const short
  potNone        = 0x0000,
  potBool        = 0x0001,
  potInt         = 0x0002,
  potFloat       = 0x0004,
  potString      = 0x0008,
  potComplex     = 0x0010,
  potUndefined   = 0x0080,
  potHasGetter   = 0x0100,
  potHasSetter   = 0x0200;

class TOlxPyVar {
  olxstr *Str;
  PyObject *Obj;
  short Type;
  static  char svmn[], gvmn[];
  bool HasGet() const {  return (Type&potHasGetter)!=0;  }
  bool HasSet() const {  return (Type&potHasSetter)!=0;  }
  void InitObject(PyObject* obj);
public:
  TOlxPyVar()  {
    Type = potNone;
    Str = NULL;
    Obj = NULL;
  }

  TOlxPyVar(const TOlxPyVar& oo);

  TOlxPyVar(const olxstr& val)  {
    Str = new olxstr(val);
    Type = potString;
    Obj = NULL;
  }

  TOlxPyVar(PyObject* obj);

  TOlxPyVar& operator = (const TOlxPyVar& oo)  {
    if( Str != NULL )  delete Str;
    if( Obj != NULL ) Py_DecRef(Obj);
    Type = oo.Type;
    Str = (oo.Str == NULL) ? NULL : new olxstr(*oo.Str);
    Obj = oo.Obj;
    if( Obj != NULL )  Py_INCREF(Obj);
    return *this;
  }
  
  ~TOlxPyVar();
  PyObject* GetObj()  {  return Obj;  }
  PyObject* GetObjVal();
  // returns wrapped value of the object or the object if of primitive type
  static PyObject* ObjectValue(PyObject* obj);

  olxstr* GetStr()  {  return Str;  }
  void Set(PyObject* obj);
  void Set(const olxstr& str);
};

struct TOlxVarChangeData : public IEObject  {
  const olxstr& str_val, var_name;
  PyObject* py_val;
  TOlxVarChangeData(const olxstr& v_name, const olxstr& v_val, PyObject* p_val)
    : str_val(v_val), var_name(v_name), py_val(p_val) {}
};
   
class TOlxVars : public IEObject  {
  // this object is a singleton
  static TOlxVars* Instance;
  static olx_critical_section & CS() {
    static olx_critical_section cs;
    return cs;
  }
  olxstr_dict<TOlxPyVar, true> Vars;

  template <class T>
  void _SetVar(const T& name, const olxstr& value)  {
    const size_t ind = Vars.IndexOf(name);
    try  {
      if( ind != InvalidIndex )
        Vars.GetValue(ind).Set(value);
      else
        Vars.Add(name, value);
    }
    catch( const TExceptionBase& exc)  {
      throw TFunctionFailedException(__OlxSourceInfo, exc,
        olxstr("Exception occured while setting variable '") << name <<'\'');
    }
    TOlxVarChangeData vcd(name, value, NULL);
    OnVarChange->Execute(NULL, &vcd);
  }
  template <class T>
  void _SetVar(const T& name, PyObject* value) {
    const size_t ind = Vars.IndexOf(name);
    try {
      if (ind != InvalidIndex)
        Vars.GetValue(ind).Set(value);
      else
        Vars.Add(name, value);
    }
    catch( const TExceptionBase& exc)  {
      throw TFunctionFailedException(__OlxSourceInfo, exc,
        olxstr("Exception occured while setting variable '") << name <<'\'');
    }
    TOlxVarChangeData vcd(name, EmptyString(), value);
    OnVarChange->Execute(NULL, &vcd);
  }

  const olxstr& _FindName(PyObject* value) {
    for (size_t i=0; i < Vars.Count(); i++)
      if (Vars.GetValue(i).GetObj()  == value)
        return Vars.GetKey(i);
    return EmptyString();
  }

  template <class T>
  bool _UnsetVar(const T& name) {
    const size_t ind = Vars.IndexOf(name);
    if (ind != InvalidIndex) {
      Vars.Delete(ind);
      return true;
    }
    size_t c=0;
    Wildcard w(name);
    for (size_t i=0; i < Vars.Count(); i++) {
      if (w.DoesMatch(Vars.GetKey(i))) {
        Vars.Delete(i--);
        c++;
      }
    }
    return c > 0;
  }
  TOlxVars();
  ~TOlxVars() { Instance = NULL; }
  TActionQList Actions;
public:

  TActionQueue *OnVarChange;

  static TOlxVars* GetInstance() { return Instance; }
  static TOlxVars& Init() { return *(new TOlxVars()); }
  static void Finalise() {
    if (Instance != NULL)
      delete Instance;
  }

  static size_t VarCount() {
    volatile olx_scope_cs cs(CS());
    return Instance != NULL ? Instance->Vars.Count() : 0;
  }

  static PyObject* GetVarValue(size_t index) {
    volatile olx_scope_cs cs(CS());
    return Instance->Vars.GetValue(index).GetObjVal();
  }
  static PyObject* GetVarWrapper(size_t index) {
    volatile olx_scope_cs cs(CS());
    return Instance->Vars.GetValue(index).GetObj();
  }
  static const olxstr& GetVarStr(size_t index);

  static const olxstr& GetVarName(size_t index) {
    volatile olx_scope_cs cs(CS());
    return Instance->Vars.GetKey(index);
  }

  template <class T>
  static void SetVar(const T& name, const olxstr& value)  {
    volatile olx_scope_cs cs(CS());
    if (Instance == NULL) new TOlxVars();
    Instance->_SetVar(name, value);
  }

  template <class T>
  static bool UnsetVar(const T& name)  {
    volatile olx_scope_cs cs(CS());
    if (Instance == NULL) return false;
    return Instance->_UnsetVar(name);
  }

  template <class T>
  static void SetVar(const T& name, PyObject *value)  {
    volatile olx_scope_cs cs(CS());
    if (Instance == NULL) new TOlxVars();
    Instance->_SetVar(name, value);
  }

  template <class T>
  static bool IsVar(const T& name) {
    volatile olx_scope_cs cs(CS());
    return (Instance == NULL) ? false
      : Instance->Vars.IndexOf(name) != InvalidIndex;
  }
  template <class T>
  static size_t VarIndex(const T& name) {
    volatile olx_scope_cs cs(CS());
    return (Instance == NULL) ? InvalidIndex : Instance->Vars.IndexOf(name);
  }

  template <class T>
  static olxstr FindValue(const T& name, const olxstr &def=EmptyString()) {
    volatile olx_scope_cs cs(CS());
    size_t i = VarIndex(name);
    return (i == InvalidIndex) ? def : GetVarStr(i);
  }

  static const olxstr& FindVarName(PyObject *pyObj) {
    volatile olx_scope_cs cs(CS());
    return (Instance == NULL) ? EmptyString() : Instance->_FindName(pyObj);
  }
  static TLibrary *ExportLibrary(const olxstr &name="env", TLibrary *l=NULL);
};
#else  // _PYTHON
// use a very thin implementation with no python...
class TOlxVars : public IEObject  {
  static TOlxVars* Instance;
  static olx_critical_section & CS() {
    static olx_critical_section cs;
    return cs;
  }
  olxstr_dict<olxstr, true> Vars;

  template <class T>
  void _SetVar(const T& name, const olxstr& value)  {
    const size_t ind = Vars.IndexOf(name);
    if( ind != InvalidIndex )
      Vars.GetValue(ind) = value;
    else
      Vars.Add(name, value);
  }
  template <class T>
  bool _UnsetVar(const T& name)  {
    const size_t ind = Vars.IndexOf(name);
    if( ind != InvalidIndex )  {
      Vars.Delete(ind);
      return true;
    }
    size_t c=0;
    Wildcard w(name);
    for (size_t i=0; i < Vars.Count(); i++) {
      if (w.DoesMatch(Vars.GetKey(i))) {
        Vars.Delete(i--);
        c++;
      }
    }
    return c > 0;
  }
public:
  static TOlxVars* GetInstance()  {  return Instance;  }
  static TOlxVars& Init()  {  return *(new TOlxVars());  }
  static void Finalise()  {  if( Instance != NULL )  delete Instance;  }

  static size_t VarCount()  {
    volatile olx_scope_cs cs(CS());
    return Instance != NULL ? Instance->Vars.Count() : 0;
  }

  static const olxstr& GetVarName(size_t index) {
    volatile olx_scope_cs cs(CS());
    return Instance->Vars.GetKey(index);
  }
  static const olxstr& GetVarStr(size_t index) {
    volatile olx_scope_cs cs(CS());
    return Instance->Vars.GetValue(index);
  }

  template <class T>
  static void SetVar(const T& name, const olxstr& value)  {
    volatile olx_scope_cs cs(CS());
    if (Instance == NULL) new TOlxVars();
    Instance->_SetVar(name, value);
  }
  template <class T>
  static bool UnsetVar(const T& name)  {
    volatile olx_scope_cs cs(CS());
    if (Instance == NULL) return false;
    return Instance->_UnsetVar(name);
  }
  template <class T>
  static bool IsVar(const T& name) {
    volatile olx_scope_cs cs(CS());
    return (Instance == NULL) ? false
      : Instance->Vars.IndexOf(name) != InvalidIndex;
  }
  template <class T>
  static size_t VarIndex(const T& name) {
    volatile olx_scope_cs cs(CS());
    return (Instance == NULL) ? InvalidIndex : Instance->Vars.IndexOf(name);
  }
  template <class T>
  static olxstr FindValue(const T& name, const olxstr &def=EmptyString()) {
    volatile olx_scope_cs cs(CS());
    size_t i = VarIndex(name);
    return (i == InvalidIndex) ? def : GetVarStr(i);
  }
  static TLibrary *ExportLibrary(const olxstr &name="env", TLibrary *l=NULL);
};
#endif  // _PYTHON
/* a convinience object: sets var withi given name on the creation and unsets
on destruction thread safe - synchronised
*/
class OlxStateVar  {
  olxstr name, prev_val;
  bool was_set, restored;
public:
  OlxStateVar(const olxstr& _name)
    : name(_name), was_set(false), restored(false)
  {
    volatile olx_scope_cs cs(TBasicApp::GetCriticalSection());
    size_t i = TOlxVars::VarIndex(_name);
    if( i != InvalidIndex )  {
      prev_val = TOlxVars::GetVarStr(i);
      was_set = true;
    }
    TOlxVars::SetVar(name, TrueString());
  }
  ~OlxStateVar()  {  Restore();  }
  void Restore()  {
    volatile olx_scope_cs cs(TBasicApp::GetCriticalSection());
    if( restored )  return;
    if( was_set )
      TOlxVars::SetVar(name, prev_val);
    else
      TOlxVars::UnsetVar(name);
    restored = true;
  }
};
#endif
