#ifndef __olx_py_core_h
#define __olx_py_core_h
#include "pyext.h"

/* This class exports olex_core module to the embedded python */

class OlexPyCore {
public:
  static void PyInit();
};

#endif
