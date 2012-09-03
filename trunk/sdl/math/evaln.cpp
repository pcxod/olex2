/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include <math.h>
#include "../log.h"
#include "evaln.h"
#include "../bapp.h"
#include "../exparse/expbuilder.h"

using namespace exparse;
using namespace math_eval;
//.............................................................................
//.............................................................................
ExpEvaluator::ExpEvaluator()
  : root(NULL), eval_root(NULL)
{
  factory('~', new FuncEvaluator1::factory(&chs));
  factory("abs", new FuncEvaluator1::factory(&olx_abs<double>));
  factory("cos", new FuncEvaluator1::factory((double (*)(double))&cos));
  factory("sin", new FuncEvaluator1::factory((double (*)(double))&sin));
  factory('^', new FuncEvaluator2::factory((double (*)(double, double))&pow));
  factory('+', new FuncEvaluator2::factory(&add));
  factory('-', new FuncEvaluator2::factory(&sub));
  factory('*', new FuncEvaluator2::factory(&mul));
  factory('/', new FuncEvaluator2::factory(&div));
  factory('%', new FuncEvaluator2::factory(&mod));
}
//.............................................................................
ExpEvaluator::~ExpEvaluator() {
  if (root != NULL)
    delete root;
  if (eval_root != NULL)
    delete eval_root;
  for (size_t i=0; i < factory.Count(); i++)
    delete factory.GetValue(i);
}
//.............................................................................
AEvaluable *ExpEvaluator::create(const olxstr &name,
    const TPtrList<AEvaluable> &args)
{
  IFactory *f = factory.Find(name, NULL);
  if (f==NULL)
    return Evaluators.Add(new Evaluator(name, args));
  AEvaluable *e = f->create(args);
  return e == NULL ? Evaluators.Add(new Evaluator(name, args)) : e;
}
//.............................................................................
AEvaluable *ExpEvaluator::create(exparse::expression_tree *t) {
  if (t->left != NULL || t->right != NULL) {
    TPtrList<AEvaluable> args;
    if (t->left == NULL) {
      if (t->evator == NULL)
        throw TFunctionFailedException(__OlxSourceInfo, "invalid expression");
      TPtrList<AEvaluable> largs;
      for (size_t i=0; i < t->evator->args.Count(); i++)
        largs.Add(create(t->evator->args[i]));
      args << create(t->evator->name, largs);
    }
    else
      args << create(t->left);
    if (t->right != NULL) args << create(t->right);
    return create(t->data, args);
  }
  if (t->evator != NULL) {
    TPtrList<AEvaluable> args;
    for (size_t i=0; i < t->evator->args.Count(); i++)
      args.Add(create(t->evator->args[i]));
    return create(t->evator->name, args);
  }
  if (t->left == NULL && t->right == NULL) {
    if (t->data.IsNumber())
      return new NumberEvaluator(t->data.ToDouble());
    else {
      AEvaluable *ae = ScopeVariables.FindObject(t->data);
      if (ae != NULL) return ae;
      return Variables.Add(new Variable(t->data));
    }
  }
  throw TFunctionFailedException(__OlxSourceInfo, "should not reach here!");
}
//.............................................................................
ConstPtrList<AEvaluable> ExpEvaluator::create_args(const olxstr &args_) {
  TPtrList<AEvaluable> rv;
  TStrList args;
  parser_util::split_args(args_, args);
  for (size_t i=0; i < args.Count(); i++) {
    if (args[i].IsNumber())
      rv.Add(new NumberEvaluator(args[i].ToDouble()));

  }
  return rv;
}
//.............................................................................
void ExpEvaluator::build(const olxstr &expr) {
  if (root != NULL) delete root;
  if (eval_root != NULL) delete eval_root;
  Variables.Clear();
  Evaluators.Clear();
  root = new expression_tree(NULL, expr);
  parser_util::operator_set os;
  os.remove_operator('.');
  root->expand(os);
  root = exparse::exp_builder::sort_logical(root);
  eval_root = create(root);
}
//.............................................................................
