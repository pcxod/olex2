/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/
#pragma once
#include "macrolib.h"
#include "etime.h"
#include "etbuffer.h"

#ifdef _PYTHON
#if !defined OLX_CCAT
#define OLX_CCAT(A, B) A ## B
#endif

inline size_t olx_PyUnicode_Length(PyObject* pobj) { return PyUnicode_GetLength(pobj); }
#define OlxInitPyModule(m_name, m_def) PyMODINIT_FUNC OLX_CCAT(PyInit_, m_name)() \
 { return PythonExt::init_module(#m_name, m_def); }

#ifdef _LIBFFI
#include <ffi.h>
class LibFFI_Func_Closure {
  ABasicFunction* func;
  olxcstr name, description;
  olx_object_ptr<ffi_cif> cif;
  ffi_closure* closure;
  void* executable_func_ptr;
  static ffi_type** arg_types() {
    static ffi_type* args[] = {
      &ffi_type_pointer, &ffi_type_pointer,
      &ffi_type_pointer, &ffi_type_pointer };
    return &args[0];
  }
  static PyObject* processor(ABasicFunction* f,
    PyObject* self, PyObject* args, PyObject* kwds);
public:
  typedef PyObject* (*closure_fn_t)(PyObject*, PyObject*);

  LibFFI_Func_Closure(ABasicFunction* f);
  ~LibFFI_Func_Closure() {
    if (closure != 0) {
      ffi_closure_free(closure);
    }
  }

  closure_fn_t get_ptr() const {
    return (closure_fn_t)executable_func_ptr;
  }

  const ABasicFunction& get_func() const { return *func; }
  const olxcstr& get_name() const { return name; }
  const olxcstr& get_description() const { return description; }
  static void closure_handler(ffi_cif* cif, void* ret, void* args_[], void* func);
};

class LibFFI_Lib_Closure {
  TLibrary* lib;
  olxcstr name;
  olx_object_ptr<ffi_cif> cif;
  ffi_closure* closure;
  void* executable_func_ptr;
  static PyObject* do_export(LibFFI_Lib_Closure* cl);
  TTypeList<LibFFI_Func_Closure> closures;
  olx_array_ptr<PyMethodDef> funcs;
public:
  typedef PyObject* (*closure_fn_t)();

  LibFFI_Lib_Closure(TLibrary* lib, const olxcstr &name_prefix);
  ~LibFFI_Lib_Closure() {
    if (closure != 0) {
      ffi_closure_free(closure);
    }
  }

  closure_fn_t get_ptr() const {
    return (closure_fn_t)executable_func_ptr;
  }

  PyObject* do_register();
  const TLibrary& get_lib() const { return *lib; }
  const olxcstr& get_name() const { return name; }
  static void closure_handler(ffi_cif* cif, void* ret, void* args_[], void* lib);
};
#endif

//---------------------------------------------------------------------------
using namespace olex2;
typedef PyObject *(*pyRegFunc)();

class PythonExt : public IOlxObject {
  static PythonExt *&Instance() {
    static PythonExt* inst = 0;
    return inst;
  }
  IOlex2Processor* OlexProcessor;
  TLibrary *Library, *BindLibrary;
  //.............................................................................
  struct ProfileInfo {
    int CallCount;
    uint64_t TotalTimeMs;
    ProfileInfo() :
      CallCount(0),
      TotalTimeMs(0)
    {}
    void Reset() {
      CallCount = 0;
      TotalTimeMs = 0;
    }
  };
  //.............................................................................
  static size_t Olx_PyUnicode_AsWideChar(PyObject *unic, wchar_t *w,
    Py_ssize_t sz)
  {
    return PyUnicode_AsWideChar(unic, w, sz);
  }
  static PyObject *pyInit();

#ifdef _LIBFFI
  TTypeList<LibFFI_Lib_Closure> lib_closures;
#endif
public:
  class BasicWrapper : public IOlxObject {
    PythonExt::ProfileInfo *PI;
    uint64_t StartTime;
    int Recursion;  // specifies that the function is called recursively
  protected:
    PyObject *PyFunction;
    bool IsProfilingEnabled()  const {
      return PI != 0;
    }
    void OnCallEnter() {
      if (PI == 0) {
        return;
      }
      PI->CallCount++;
      if (++Recursion > 1) {
        return;
      }
      StartTime = TETime::msNow();
    }
    void OnCallLeave() {
      if (PI == 0) {
        return;
      }
      if (--Recursion > 0) {
        return;
      }
      uint64_t now = TETime::msNow();
      PI->TotalTimeMs += ((StartTime < now) ? (now - StartTime) : 0);
      Recursion = 0;
    }
    //..........................................................................
  public:
    BasicWrapper(bool enableProfiling) {
      PI = 0;
      StartTime = 0;
      Recursion = 0;
      EnableProfiling(enableProfiling);
    }
    //..........................................................................
    virtual ~BasicWrapper() {
      olx_del_obj(PI);
    }
    PyObject* GetFunction() const { return PyFunction; }
    PythonExt::ProfileInfo *ProfileInfo() { return PI; }
    void EnableProfiling(bool enable) {
      if (enable) {
        if (PI == 0) {
          PI = new PythonExt::ProfileInfo();
        }
        else  PI->Reset();
      }
      else {
        delete PI;
        PI = 0;
      }
    }
  };
  //.............................................................................
  TPtrList<BasicWrapper> ToDelete;
  //.............................................................................
  void macReset(TStrObjList& Cmds, const TParamList &Options, TMacroData& E);
  void macRun(TStrObjList& Cmds, const TParamList &Options, TMacroData& E);
  void funLogLevel(const TStrObjList& Params, TMacroData& E);
  void funExport(const TStrObjList& Params, TMacroData& E);
  PythonExt(IOlex2Processor* olexProcessor);
  uint16_t LogLevel;
public:
  ~PythonExt();
  // must be called only once
  static PythonExt& Init(IOlex2Processor* olexProcessor) {
    return *(new PythonExt(olexProcessor));
  }
  static void Finilise() {
    if (Instance() != 0) {
      delete Instance();
      Instance() = 0;
    }
  }
  int RunPython(const olxstr& script);
  DefPropP(uint16_t, LogLevel);
  template <class T>
  T * AddToDelete(T* td) { return (T*)ToDelete.Add(td); }

  void Register(const olxcstr &name, pyRegFunc regFunc);
  TLibrary* ExportLibrary(const olxstr& name = EmptyString());
  //  static TLibrary* GetExportedLibrary()  {  return Library;  }
  TLibrary* GetBindLibrary() { return BindLibrary; }
  IOlex2Processor* GetOlexProcessor() { return OlexProcessor; }
  void CheckInitialised();

#ifdef _LIBFFI
  void ExportDirect(const olxcstr& name);
#endif

  static PythonExt* GetInstance() {
    if (Instance() == 0) {
      throw TFunctionFailedException(__OlxSourceInfo, "Uninitialised object");
    }
    return Instance();
  }

  void ProfileAll();

  PyObject* GetProfileInfo();
  // building string
  static PyObject* BuildString(const olxstr& str) {
#ifdef _UNICODE
    if (str.IsEmpty()) {
      return PyUnicode_FromWideChar(L"", 0);
    }
    return PyUnicode_FromWideChar(str.raw_str(), str.Length());
#else
    if (str.IsEmpty()) { // silly Py...
      return PyString_FromStringAndSize("", 0);
    }
    return PyString_FromStringAndSize(str.raw_str(), str.Length());
#endif
  }
  static PyObject* BuildCString(const olxcstr& str) {
    if (str.IsEmpty()) {
      return PyBytes_FromStringAndSize("", 0);
    }
    return PyBytes_FromStringAndSize(str.raw_str(), str.Length());
  }
  static PyObject* BuildWString(const olxwstr& str) {
    if (str.IsEmpty()) {
      return PyUnicode_FromWideChar(L"", 0);
    }
    return PyUnicode_FromWideChar(str.raw_str(), str.Length());
  }
  // parsing string
  static olxstr ParseStr(PyObject *pobj) {
    char* crv;
    olxstr rv;
    if (pobj->ob_type == &PyBytes_Type && PyArg_Parse(pobj, "s", &crv)) {
      rv = crv;
    }
    else if (pobj->ob_type == &PyUnicode_Type) {
      size_t sz = olx_PyUnicode_Length(pobj);
      TTBuffer<wchar_t> wc_bf(sz + 1);
      sz = Olx_PyUnicode_AsWideChar(pobj, wc_bf.Data(), sz);
      if (sz > 0) {
        rv.Append(wc_bf.Data(), sz);
      }
    }
    else {
      pobj = PyObject_Repr(pobj);
      if (pobj != 0) {
        return ParseStr(pobj);
      }
    }
    return rv;
  }
  /* to tackle the difference with list and tuple... it steals the reference
  now
  */
  static void SetDictItem(PyObject* dict, const char* field_name,
    PyObject* val)
  {
    PyDict_SetItemString(dict, field_name, val);
    Py_DECREF(val);
  }
  static void SetDictItem(PyObject* dict, PyObject* field_name,
    PyObject* val)
  {
    PyDict_SetItem(dict, field_name, val);
    Py_DECREF(field_name);
    Py_DECREF(val);
  }
  static void SetDictItem(PyObject* dict, const olxstr& field_name,
    PyObject* val)
  {
    SetDictItem(dict, BuildString(field_name), val);
  }
  static PyObject* SetErrorMsg(PyObject* err_type, const olxcstr& location,
    const olxstr& msg)
  {
    PyObject* val = BuildString(olxstr(location) << ": " << msg);
    PyErr_SetObject(err_type, val);
    Py_DECREF(val);
    return 0;
  }
  static PyObject* SetErrorMsg(PyObject* err_type, const olxcstr& location,
    const char* msg)
  {
    PyObject* val = Py_BuildValue("s: s", location.c_str(), msg);
    PyErr_SetObject(err_type, val);
    Py_DECREF(val);
    return 0;
  }
  static PyObject* InvalidArgumentException(const olxcstr& location,
    const char* msg)
  {
    PyObject* val = Py_BuildValue("s: ss", location.c_str(),
      "invalid argument format for ", msg);
    PyErr_SetObject(PyExc_TypeError, val);
    Py_DECREF(val);
    return 0;
  }

  static olxcstr UpdateBinaryFormat(const char *f) {
    return olxcstr(f).Replace("s#", "y#");
  }
  static PyObject* PyNone() { Py_INCREF(Py_None);  return Py_None; }
  static PyObject* PyTrue() { Py_INCREF(Py_True);  return Py_True; }
  static PyObject* PyFalse() { Py_INCREF(Py_False);  return Py_False; }
  static PyObject* PyBool(bool v) { return v ? PyTrue() : PyFalse(); }
  /* tuple parsing to process unicode and string in the same way...
    s# - char*, len
    s - char*
    w - olxstr
    i - int
    f - float
    O - PyObject
    | - optional params after column
  */
  static bool ParseTuple(PyObject* tuple, const char* format, ...);

  static PyObject* ToPython(const TDataItem& i, PyObject* to=0);

  static olxcstr &ModuleName();

  static PyObject *init_module(const olxcstr &name, PyMethodDef *m_def);
};

#endif // _PYTHON
