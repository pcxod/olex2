#ifndef __olx_hkl_py_H
#define __olx_hkl_py_H

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
