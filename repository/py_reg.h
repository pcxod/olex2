/******************************************************************************
* Copyright (c) 2004-2025 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#pragma once

#if defined(_PYTHON) && defined(__WXWIDGETS__) && defined(_WIN32)
#include "pyext.h"
#include "hkl.h"

class py_reg {
  static PyMethodDef Methods[];
public:
  static PyObject* Exists(PyObject* self, PyObject* args);
  static PyObject* ListKeys(PyObject* self, PyObject* args);
  static PyObject* ListValues(PyObject* self, PyObject* args);
  static PyObject* Read(PyObject* self, PyObject* args);
  static olxcstr& ModuleName();
  static PyObject* PyInit();
};
#endif

