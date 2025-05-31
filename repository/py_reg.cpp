/******************************************************************************
* Copyright (c) 2004-2025 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#if defined(_PYTHON) && defined(__WXWIDGETS__) && defined(_WIN32)
#include "py_reg.h"
#include <wx/msw/registry.h>

PyMethodDef py_reg::Methods[] = {
  {"Exists", py_reg::Exists, METH_VARARGS, "checks if specified key exists"},
  {"ListKeys", py_reg::ListKeys, METH_VARARGS, "lists specified key sub-keys"},
  {"ListValues", py_reg::ListValues, METH_VARARGS, "lists specified key values"},
  {"Read", py_reg::Read, METH_VARARGS, "reads specified key value"},
  {NULL, NULL, 0, NULL}
};

olxcstr& py_reg::ModuleName() {
  static olxcstr mn = "olex_reg";
  return mn;
}

PyObject* py_reg::PyInit() {
  return PythonExt::init_module(ModuleName(), Methods);
}
//..................................................................................................
int RootNameToConst(const olxstr& n, int def = wxRegKey::HKCU) {
  if (n.Equalsi("HKCR")) {
    return wxRegKey::HKCR;
  }
  if (n.Equalsi("HKCU")) {
    return wxRegKey::HKCU;
  }
  if (n.Equalsi("HKLM")) {
    return wxRegKey::HKLM;
  }
  if (n.Equalsi("HKCC")) {
    return wxRegKey::HKCC;
  }
  if (n.Equalsi("HKU")) {
    return wxRegKey::HKUSR;
  }
  return def;
}

//..................................................................................................
PyObject* py_reg::Exists(PyObject* self, PyObject* args) {
  olxstr key_name, root;
  if (!PythonExt::ParseTuple(args, "w|w", &key_name, &root)) {
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "w|w");
  }
  wxRegKey k((wxRegKey::StdKey)RootNameToConst(root), key_name.u_str());
  return k.Exists() ? PythonExt::PyTrue() : PythonExt::PyFalse();
}
//..................................................................................................
PyObject* py_reg::ListKeys(PyObject* self, PyObject* args) {
  olxstr key_name, root;
  if (!PythonExt::ParseTuple(args, "w|w", &key_name, &root)) {
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "w|w");
  }
  wxRegKey key((wxRegKey::StdKey)RootNameToConst(root), key_name.u_str());
  if (!key.Exists()) {
    return PythonExt::PyNone();
  }
  wxString skn;
  long index = 0;
  PyObject* rv = PyList_New(0);
  for (bool cont = key.GetFirstKey(skn, index); cont;
    cont = key.GetNextKey(skn, index))
  {
    PyList_Append(rv, PythonExt::BuildString(skn));
  }
  return rv;
}
//..................................................................................................
PyObject* py_reg::ListValues(PyObject* self, PyObject* args) {
  olxstr key_name, root;
  if (!PythonExt::ParseTuple(args, "w|w", &key_name, &root)) {
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "w|w");
  }
  wxRegKey key((wxRegKey::StdKey)RootNameToConst(root), key_name.u_str());
  if (!key.Exists()) {
    return PythonExt::PyNone();
  }
  wxString svn;
  long index = 0;
  PyObject* rv = PyList_New(0);

  for (bool cont = key.GetFirstValue(svn, index); cont;
    cont = key.GetNextValue(svn, index))
  {
    PyList_Append(rv, PythonExt::BuildString(svn));
  }
  return rv;
}
//..................................................................................................
PyObject* py_reg::Read(PyObject* self, PyObject* args) {
  olxstr key_name, value_name, root;
  if (!PythonExt::ParseTuple(args, "ww|w", &key_name, &value_name, &root)) {
    return PythonExt::InvalidArgumentException(__OlxSourceInfo, "ww|w");
  }
  wxRegKey key((wxRegKey::StdKey)RootNameToConst(root), key_name.u_str());
  if (!key.Exists()) {
    return PythonExt::PyNone();
  }
  wxString v;
  if (key.QueryRawValue(value_name.u_str(), v)) {
    return PythonExt::BuildString(v);
  }
  return PythonExt::PyNone();
}
//..................................................................................................
#endif // _PYTHON && __WXWIDGETS__ && _WIN32