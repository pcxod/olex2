/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_exparse_syntax_H
#define __olx_sdl_exparse_syntax_H
#include "funcwrap.h"
BeginEsdlNamespace()

namespace exparse  {

  struct iloop  {  };
  struct exe_block  {
    TPtrList<IEvaluable> lines;
  };
  struct conditional : public exe_block {
    IEvaluable *condition;
  };
  struct function : public exe_code  {

  };
  struct syn_if : public conditional  {
    exe_block else_body;
    TPtrList<conditional> elifs;
  };
  struct syn_for : public conditional  {
    IEvaluable *pre, *post;
  };
  struct syn_while : public conditional  {
    IEvaluable *condition;
  };

};  // namespace exparse

EndEsdlNamespace()
#endif
