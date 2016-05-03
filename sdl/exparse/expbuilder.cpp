/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "expbuilder.h"

using namespace esdl::exparse;

ClassInfo<StringValue,olxstr> StringValue::info;
ClassInfo<ListValue,ListValue::list_t> ListValue::info;

//..............................................................................
bool exp_builder::needs_sorting(expression_tree* root)  {
  if( BuiltInsFactory::is_cmp(root->data) ||
      BuiltInsFactory::is_arithmetic(root->data) )
  {
    if( root->right != 0 &&
        BuiltInsFactory::is_logical(root->right->data) )
    {
      if (root->right->left == 0) { // +/- number
        return false;
      }
      return !root->right->priority;
    }
  }
  if( BuiltInsFactory::has_arithmetic_priority(root->data) )  {
    if( root->right != 0 &&
      (BuiltInsFactory::is_arithmetic(root->right->data) &&
      !BuiltInsFactory::has_arithmetic_priority(root->right->data)) )
    {
      if (root->right->left == 0) { // +/- number
        return false;
      }
      return !root->right->priority;
    }
  }
  return false;
}
//..............................................................................
expression_tree* exp_builder::sort_logical(expression_tree* root) {
  // need to test that it is not just -number... root->right->left does it?
  if (root->parent == 0 && root->left == 0 && root->right != 0 &&
    root->right->left != 0)
  {
    expression_tree* top = root->right;
    root->right = top->left;
    top->parent = root->parent;
    root->parent = top;
    top->left = root;
    return sort_logical(top);
  }
  if (needs_sorting(root)) {
    if (root->parent != 0 && root->left == 0) {
      expression_tree* top = root->right;
      root->right = top->left;
      top->parent = root->parent->parent;
      top->left = root->parent;
      if (top->right != 0) {
        top->right = sort_logical(top->right);
      }
      return top;
    }
    else {
      expression_tree* top = root->right;
      root->right = top->left;
      top->left = root;
      top->parent = root->parent;
      root->parent = top;
      if (top->right != 0) {
        top->right = sort_logical(top->right);
      }
      return top;
    }
  }
  if (root->right != 0) {
    expression_tree *original = root->right,
      *result = sort_logical(root->right);
    if (result != original) {
      return result;
    }
  }
  return root;
}
//..............................................................................
IEvaluable* exp_builder::process_const_func(IEvaluable* func, IEvaluable* left,
  IEvaluable* right)
{
  if( (dynamic_cast<BuiltInsFactory::IConstFunc*>(func) != 0 ||
    dynamic_cast<BuiltInsFactory::IConstFunc2*>(func) != 0) )
  {
    ANumberEvaluator* left_na = (left == 0 ? 0
      : dynamic_cast<ANumberEvaluator*>(left));
    ANumberEvaluator* right_na = (right == 0 ? 0
      : dynamic_cast<ANumberEvaluator*>(right));
    if( (left == 0) ||
      ((left_na != 0 && left_na->is_final()) &&
      ((right == 0) || (right_na != 0 && right_na->is_final()))) )
    {
      IEvaluable* ce = func->_evaluate();
      delete func;
      return ce;
    }
  }
  return func;
}
//..............................................................................
IEvaluable* exp_builder::evaluator_from_evator(expression_tree* root,
  IEvaluable* left)
{
  TPtrList<IEvaluable> args;
  bool all_const = true;
  for (size_t i = 0; i < root->evator->args.Count(); i++) {
    args.Add(create_evaluator(root->evator->args[i]));
    if (args.GetLast() == 0) {
      for (size_t j = 0; j < args.Count() - 2; j++) {
        delete args[j];
      }
      throw TInvalidArgumentException(__OlxSourceInfo,
        "could not find appropriate evaluable");
    }
    ANumberEvaluator* ne = dynamic_cast<ANumberEvaluator*>(args.GetLast());
    if (ne == 0 || !ne->is_final()) {
      all_const = false;
    }
  }
  IEvaluable* rv = 0;
  if (root->parent != 0 && root->parent->data == '.') {
    if (left == 0) {
      throw TFunctionFailedException(__OlxSourceInfo, "invalid indirection");
    }
    rv = left->find_method(root->evator->name, factory, args);
    if (rv == 0) {
      IClassInfo *ci = factory.classes.Find(&left->get_type_info(), 0);
      if (ci != 0) {
        rv = ci->get_functions().create_from_name(*left, factory, root->evator->name, args);
      }
    }
  }
  else {
    rv = BuiltInsFactory::create(root->evator->name, args);
    if (rv == 0) {
      IClassInfo *cr = scope.classes.Find(root->evator->name, 0);
      if (cr != 0) {
        rv = cr->get_static_functions().create_from_name(factory, "__init__", args);
      }
      if (rv == 0) {
        rv = scope.functions.create_from_name(factory, root->evator->name, args);
      }
    }
  }
  if (rv == 0) {
    throw TFunctionFailedException(__OlxSourceInfo,
      olxstr("undefined function: ").quote() << root->evator->name <<
      " for type " << left->get_type_info().name());
  }
  // need to test if the function result cannot be changed
  if (all_const) {
    return process_const_func(rv, 0);
  }
  return rv;
}
//..............................................................................
IEvaluable* exp_builder::locate_function(const olxstr& name, IEvaluable* left,
  IEvaluable* right)
{
  TPtrList<IEvaluable> args;
  args.Add(left);
  if (right != 0) {
    args.Add(right);
  }
  IEvaluable* rv = left->find_method(name, factory, args);
  if (rv == 0) {
    rv = BuiltInsFactory::create(name, left, right);
    rv = process_const_func(rv, left, right);
  }
  if (rv == 0) {
    if (left != 0) {
      delete left;
    }
    if (right != 0) {
      delete right;
    }
    throw TFunctionFailedException(__OlxSourceInfo,
      olxstr("could not create specified evaluator: ") << name);
  }
  return rv;
}
//..............................................................................
IEvaluable* exp_builder::create_evaluator(expression_tree* root,
  bool finaliseAssignment)
{
  if (root->data == '.') {
    IEvaluable* left = 0,
      *rv = 0;
    if (root->left == 0 || (left = create_evaluator(root->left)) == 0) {
      throw TFunctionFailedException(__OlxSourceInfo, "invalid indirection");
    }
    while (root->right != 0 && root->right->evator != 0) {
      rv = evaluator_from_evator(root->right, left);
      if (rv->is_final() || rv->is_function()) {
        IEvaluable* eval = rv->_evaluate();
        if (left->ref_cnt() == 0) {
          delete left;
        }
        if (rv->ref_cnt() == 0) {
          delete rv;
        }
        left = rv = eval;
      }
      else {
        if (left->ref_cnt() == 0) {
          delete left;
        }
        left = rv;
      }
      root = root->right;
    }
    if (root->right != 0) {  // anything more left?
      IEvaluable* right = create_evaluator(root->right);
      return locate_function(root->data, rv, right);
    }
    return rv;
  }
  if (root->data == '=') {  // assignment
    if (root->right == 0 || root->left == 0 || root->evator != 0) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "invalid assignment operator");
    }
    IEvaluable* val = create_evaluator(root->right);
    if (!val->is_final() && finaliseAssignment) {
      IEvaluable* res = val->_evaluate();
      if (val->ref_cnt() == 0) {
        delete val;
      }
      val = res;
    }
    scope.add_var(root->left->data, val, true);
    return val;
  }
  if (root->left != 0 || root->right != 0) {
    IEvaluable *left = 0,
      *right = 0;
    try {
      if (root->left == 0) {
        if (root->evator != 0) {
          left = evaluator_from_evator(root);
        }
      }
      else {
        left = create_evaluator(root->left);
      }
      right = (root->right != 0 ? create_evaluator(root->right) : 0);
    }
    catch (const TExceptionBase& exc) {
      if (left != 0) {
        delete left;
      }
      if (right != 0) {
        delete right;
      }
      throw TFunctionFailedException(__OlxSourceInfo, exc,
        "could not find appropriate evaluable");
    }
    if (left == 0 && right != 0) {
      left = right;
      right = 0;
    }
    return locate_function(root->data, left, right);
  }
  else {
    if (root->evator != 0) {
      return evaluator_from_evator(root);
    }
    else {
      return evaluator_from_string(root->data);
    }
  }
}
//..............................................................................
IEvaluable* exp_builder::evaluator_from_string(const olxstr &str) const {
  if ((str.StartsFrom('\'') && str.EndsWith('\'')) ||
    (str.StartsFrom('\"') && str.EndsWith('\"')))
  {
    return new StringValue(olxstr(str).Trim(str.CharAt(0)));
  }
  else if (str.StartsFrom('[') && str.EndsWith(']')) {
    ListValue *lv = new ListValue;
    TStrList toks(str.SubStringFrom(1, 1), ',');
    for (size_t ti = 0; ti < toks.Count(); ti++) {
      lv->val.Add(evaluator_from_string(toks[ti]))->inc_ref();
    }
    return lv;
  }
  if (str.IsNumber()) { // is number?
    if (str.IndexOf('.') != InvalidIndex) {
      return new DoubleValue(str.ToDouble());
    }
    else {
      return new IntValue(str.ToInt());
    }
  }
  IEvaluable* rv = scope.find_const(str);
  if (rv != 0) {
    return rv;
  }
  rv = scope.find_var(str);
  if (rv == 0) {
    throw TFunctionFailedException(__OlxSourceInfo,
      olxstr("undefined variable: ").quote() << str);
  }
  return rv->undress();
}
//..............................................................................
