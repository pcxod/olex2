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
    void run(exp_builder& eb) const {
      for (size_t i = 0; i < lines.Count(); i++) {
        IEvaluable* v = eb.create_evaluator(lines[i]);
        if (v->ref_cnt() == 0) delete v;   // discard unless a var bound to it
      }
    }
  };
  struct conditional : public exe_block {
    expression_tree* condition_src;
    conditional() : condition_src(0) {}

    bool test(exp_builder& eb) {
      IEvaluable* c = eb.create_evaluator(condition_src);
      bool r = c->cast<bool>().val;
      if (c->ref_cnt() == 0) delete c;
      return r;
    }
  };
  struct function : public exe_code  {

  };
  struct syn_if : public conditional  {
    exe_block else_body;
    TPtrList<conditional> elifs;
    void run(exp_builder& eb) {
      {
        IEvaluable* v = eb.create_evaluator(pre_src);
        if (v->ref_cnt() == 0) delete v;
      }
      const size_t max_iter = 1000000;
      for (size_t n = 0; test(eb); n++) {
        if (n >= max_iter) {
          throw TFunctionFailedException(__OlxSourceInfo,
            "loop iteration limit exceeded");
        }
        exe_block::run(eb);
        IEvaluable* v = eb.create_evaluator(post_src);
        if (v->ref_cnt() == 0) delete v;
      }
    }
  };

  struct syn_for : public conditional  {
    IEvaluable *pre, *post;
  };
  struct syn_while : public conditional  {
    IEvaluable *condition;
    void run(exp_builder& eb) {
      const size_t max_iter = 1000000;
      for (size_t n = 0; test(eb); n++) {
        if (n >= max_iter) {
          throw TFunctionFailedException(__OlxSourceInfo,
            "loop iteration limit exceeded");
        }
        exe_block::run(eb);
      }
    }
  };

};  // namespace exparse

EndEsdlNamespace()
#endif
