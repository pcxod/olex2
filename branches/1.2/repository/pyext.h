/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_python_ext_H
#define __olx_python_ext_H
#include "macrolib.h"
#include "etime.h"
#include "etbuffer.h"

#ifdef _PYTHON
//---------------------------------------------------------------------------
using namespace olex2;
typedef void (*pyRegFunc)();

class PythonExt : public IOlxObject {
  static PythonExt *&Instance() {
    static PythonExt* inst=NULL;
    return inst;
  }
  IOlex2Processor* OlexProcessor;
  TLibrary *Library, *BindLibrary;
  TTypeList<pyRegFunc> ToRegister;
//.............................................................................
  struct ProfileInfo  {
    int CallCount;
    uint64_t TotalTimeMs;
    ProfileInfo()  {  Reset();  }
    void Reset()   {  CallCount = 0;  TotalTimeMs = 0;  }
  };
//.............................................................................
public:
  class BasicWrapper : public IOlxObject  {
    PythonExt::ProfileInfo *PI;
    uint64_t StartTime;
    int Recursion;  // specifies that the function is called recursively
  protected:
    PyObject *PyFunction;
    bool IsProfilingEnabled()  const  {  return PI != NULL;  }
    void OnCallEnter()  {
      if( PI == NULL )  return;
      PI->CallCount++;
      if( ++Recursion > 1 )  return;
      StartTime = TETime::msNow();
     }
    void OnCallLeave()  {
      if( PI == NULL )  return;
      if( --Recursion > 0 )  return;
      uint64_t now = TETime::msNow();
      PI->TotalTimeMs += ((StartTime < now) ? (now - StartTime) : 0);
      Recursion = 0;
    }
    //..........................................................................
    void EnableProfiling(bool enable) {
      if( enable )  {
        if( PI == NULL )  PI = new PythonExt::ProfileInfo();
        else  PI->Reset();
      }
      else  {
        delete PI;
        PI = NULL;
      }
    }
    //..........................................................................
  public:
    BasicWrapper(bool enableProfiling)  {
      PI = NULL;
      StartTime = 0;
      Recursion = 0;
      EnableProfiling(enableProfiling);
    }
    //..........................................................................
    virtual ~BasicWrapper()  {  if( PI != NULL )  delete PI;  }
    PyObject* GetFunction() const {  return PyFunction;  }
    PythonExt::ProfileInfo *ProfileInfo() {  return PI;  }
  };
//.............................................................................
  TPtrList<BasicWrapper> ToDelete;
//.............................................................................
  void ClearToDelete()  {
    for( size_t i=0; i < ToDelete.Count(); i++ )
      delete ToDelete[i];
  }
//.............................................................................
  void macReset(TStrObjList& Cmds, const TParamList &Options, TMacroData& E);
  void macRun(TStrObjList& Cmds, const TParamList &Options, TMacroData& E);
  void funLogLevel(const TStrObjList& Params, TMacroData& E);
  void funExport(const TStrObjList& Params, TMacroData& E);
  olxstr module_name;
  PythonExt(IOlex2Processor* olexProcessor, const olxstr &module_name);
  uint16_t LogLevel;
public:
  ~PythonExt();
  // must be called only once
  static PythonExt& Init(IOlex2Processor* olexProcessor,
    const olxstr &module_name="olex")
  {
    return *(new PythonExt(olexProcessor, module_name));
  }
  static void Finilise()  {
    if (Instance() != NULL) {
      delete Instance();
      Instance() = NULL;
    }
  }
  int RunPython(const olxstr& script);
  DefPropP(uint16_t, LogLevel)
  template <class T>
    T * AddToDelete(T* td)  {  return (T*)ToDelete.Add(td);  }

  void Register(pyRegFunc regFunc)  {
    ToRegister.AddCopy(regFunc);
    if( Py_IsInitialized() )  {
      (*regFunc)();
    }
  }
  TLibrary* ExportLibrary(const olxstr& name=EmptyString());
//  static TLibrary* GetExportedLibrary()  {  return Library;  }
  TLibrary* GetBindLibrary()  {  return BindLibrary;  }
  IOlex2Processor* GetOlexProcessor()  {  return OlexProcessor;  }
  void CheckInitialised();

  static PythonExt* GetInstance() {
    if (Instance() == NULL)
      throw TFunctionFailedException(__OlxSourceInfo, "Uninitialised object");
    return Instance();
  }
  PyObject* GetProfileInfo();
// building string
  static PyObject* BuildString(const olxstr& str)  {
#ifdef _UNICODE
    if( str.IsEmpty() ) // silly Py...
      return PyUnicode_FromWideChar(L"", 0);
    return PyUnicode_FromWideChar(str.raw_str(), str.Length());
#else
    if( str.IsEmpty() ) // silly Py...
      return PyString_FromStringAndSize("", 0);
    return PyString_FromStringAndSize(str.raw_str(), str.Length());
#endif
  }
  static PyObject* BuildCString(const olxcstr& str)  {
    if( str.IsEmpty() ) // silly Py...
      return PyString_FromStringAndSize("", 0);
    return PyString_FromStringAndSize(str.raw_str(), str.Length());
  }
  static PyObject* BuildWString(const olxwstr& str) {
    if( str.IsEmpty() ) // silly Py...
      return PyUnicode_FromWideChar(L"", 0);
    return PyUnicode_FromWideChar(str.raw_str(), str.Length());
  }
// parsing string
  static olxstr ParseStr(PyObject *pobj) {
    char*  crv;
    olxstr rv;
    if( pobj->ob_type == &PyString_Type && PyArg_Parse(pobj, "s", &crv) )  {
      rv = crv;
    }
    else if( pobj->ob_type == &PyUnicode_Type )  {
      size_t sz =  PyUnicode_GetSize(pobj);
      TTBuffer<wchar_t> wc_bf(sz+1);
      sz = PyUnicode_AsWideChar((PyUnicodeObject*)pobj, wc_bf.Data(), sz);
      if( sz > 0 )
        rv.Append( wc_bf.Data(), sz);
    }
    else
      rv = PyObject_REPR(pobj);
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
    return NULL;
  }
  static PyObject* SetErrorMsg(PyObject* err_type, const olxcstr& location,
    const char* msg)
  {
    PyObject* val = Py_BuildValue("s: s", location.c_str(), msg);
    PyErr_SetObject(err_type, val);
    Py_DECREF(val);
    return NULL;
  }
  static PyObject* InvalidArgumentException(const olxcstr& location,
    const char* msg)
  {
    PyObject* val = Py_BuildValue("s: ss", location.c_str(),
      "invalid argument format for ", msg);
    PyErr_SetObject(PyExc_TypeError, val);
    Py_DECREF(val);
    return NULL;
  }
  static PyObject* PyNone()  {  Py_INCREF(Py_None);  return Py_None;  }
  static PyObject* PyTrue()  {  Py_INCREF(Py_True);  return Py_True;  }
  static PyObject* PyFalse()  {  Py_INCREF(Py_False);  return Py_False;  }
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
};

#endif
#endif
