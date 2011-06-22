/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_py_core_h
#define __olx_py_core_h
#include "pyext.h"

/* This class exports olex_core module to the embedded python */

class OlexPyCore {
public:
  static void PyInit();
};

#endif
