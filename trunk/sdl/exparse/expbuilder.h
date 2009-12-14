#ifndef __olx_exparse_expbuilder_H
#define __olx_exparse_expbuilder_H

#include "builtins.h"
#include "exptree.h"
#include "explib.h"
#include "context.h"

BeginEsdlNamespace()

namespace exparse  {

  struct exp_builder  {
  protected:
    static bool needs_sorting(expression_tree* root);
    static expression_tree* sort_logical(expression_tree* root);
    IEvaluable* process_const_func(IEvaluable* func, IEvaluable* left, IEvaluable* right=NULL);
    IEvaluable* evaluator_from_evator(expression_tree* root, IEvaluable* left=NULL);
    IEvaluable* create_evaluator(expression_tree* root, bool finaliseAssignment=true);
    IEvaluable* locate_function(const olxstr& name, IEvaluable* left, IEvaluable* right);
  public:
    context& scope;
    EvaluableFactory& factory;
    exp_builder(EvaluableFactory& _factory, context& _scope) : factory(_factory), scope(_scope)  {
    }
    IEvaluable* build(const olxstr& exp, bool finaliseAssignment=true)  {
      expression_parser expp(exp);
      expp.expand();
      expp.root = sort_logical(expp.root);
      return create_evaluator(expp.root, finaliseAssignment);
    }
  };
};  // end exparse namespace

EndEsdlNamespace()

#endif
