/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_hkl_py_H
#define __olx_hkl_py_H

#ifdef _PYTHON
#include "pyext.h"
#include "hkl.h"

class hkl_py  {
  static PyMethodDef Methods[];
public:
  static PyObject* Read(PyObject* self, PyObject* args);
  static PyObject* Write(PyObject* self, PyObject* args);
  static void PyInit();
};
#endif

#endif
