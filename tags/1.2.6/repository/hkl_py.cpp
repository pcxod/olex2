/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "hkl_py.h"
#ifdef _PYTHON

PyMethodDef hkl_py::Methods[] = {
  {"Read", hkl_py::Read, METH_VARARGS, "reads an HKL file and returns a tuple of the reflections"},
  {"Write", hkl_py::Write, METH_VARARGS, "writes provided tuple to an hkl file"},
  {NULL, NULL, 0, NULL}
};

void hkl_py::PyInit()  {
  Py_InitModule( "olex_hkl", Methods );
}
//..................................................................................................
PyObject* hkl_py::Read(PyObject* self, PyObject* args)  {
  olxstr fn;
  if( !PythonExt::ParseTuple(args, "w", &fn) )
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "w");
  if( !TEFile::Exists(fn) ) {
    return PythonExt::SetErrorMsg(PyExc_IOError, __OlxSourceInfo,
      olxstr("File does not exist: ") << fn);
  }
  THklFile hkl;
  olxstr error;
  try  {  hkl.LoadFromFile(fn, false);  }
  catch (const TExceptionBase& e)  {
    return PythonExt::SetErrorMsg(PyExc_IOError, __OlxSourceInfo,
      olxstr("Invalid HKL file: ") << fn << '\n' <<
      e.GetException()->GetError());
  }
  PyObject* rv = PyTuple_New(hkl.RefCount());
  for (size_t i=0; i < hkl.RefCount(); i++) {
    PyObject* ref = PyTuple_New(7);
    PyTuple_SetItem(rv, i, ref);
    PyTuple_SetItem(ref, 0, Py_BuildValue("i", hkl[i].GetH()));
    PyTuple_SetItem(ref, 1, Py_BuildValue("i", hkl[i].GetK()));
    PyTuple_SetItem(ref, 2, Py_BuildValue("i", hkl[i].GetL()));
    PyTuple_SetItem(ref, 3, Py_BuildValue("d", hkl[i].GetI()));
    PyTuple_SetItem(ref, 4, Py_BuildValue("d", hkl[i].GetS()));
    PyTuple_SetItem(ref, 5, Py_BuildValue("i", hkl[i].GetBatch()));
    PyTuple_SetItem(ref, 6, Py_BuildValue("b", !hkl[i].IsOmitted()));
  }
  return rv;
}
//..................................................................................................
PyObject* hkl_py::Write(PyObject* self, PyObject* args)  {
  olxstr fn;
  PyObject* in;
  if( !PythonExt::ParseTuple(args, "wO", &fn, &in) )
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "wO");
  if( !PyList_CheckExact(in) ) {
    return PythonExt::SetErrorMsg(PyExc_RuntimeError, __OlxSourceInfo,
      "A list is expected");
  }
  size_t sz = PyList_Size(in);
  TRefList rf;
  int h, k, l, flag, test_flag = -1;
  double I, S;
  for( size_t i=0; i < sz; i++ )  {
    PyObject* prf = PyList_GetItem(in, i);
    if( !PyTuple_CheckExact(prf) || PyTuple_Size(prf) < 6) {
      return PythonExt::SetErrorMsg(PyExc_RuntimeError, __OlxSourceInfo,
        "A tuple of 6 items is expected");
    }
    if( !PyArg_ParseTuple(prf, "iiiddi", &h, &k, &l, &I, &S, &flag) ) {
      return PythonExt::SetErrorMsg(PyExc_RuntimeError, __OlxSourceInfo,
        "Failed to parse the (iiiddi) tuple");
    }
    if( test_flag == -1 )
       test_flag = flag;
    else if( test_flag == TReflection::NoBatchSet &&
      flag != TReflection::NoBatchSet )
    {
      return PythonExt::SetErrorMsg(PyExc_IOError, __OlxSourceInfo,
        "Error: reflections with and without batch numbers are provided");
    }
    rf.Add(new TReflection(h, k, l, I, S, flag)).SetTag(1);
  }
  bool res = false;
  olxstr error;
  try  {  res = THklFile::SaveToFile(fn, rf);  }
  catch( const TExceptionBase& e )  {  error = e.GetException()->GetError();  }
  if( !res )  {
    return PythonExt::SetErrorMsg(PyExc_IOError, __OlxSourceInfo,
      olxstr("Failed to save the HKL file: ") << fn << '\n' << error);
  }
  return PythonExt::PyNone();
}
#endif
