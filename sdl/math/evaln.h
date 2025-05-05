/******************************************************************************
* Copyright (c) 2004-2025 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_evailn_H
#define __olx_sdl_evailn_H
#include <math.h>
#include "../emath.h"
#include "../estrlist.h"
#include "../talist.h"
#include "../edict.h"
#include "../paramlist.h"
/*
How to use:
when an expression is passed to LoadFromEpression the lits of variales is being
initialised with variable names. Before the Evaluate is called the array of
IVariables has to beinitialised with the values of the variables. Use
Variables->IndexOf(VarName) to identify the index of variables.
The Functions have to be predefined. If no variables is expected when the
constructur can be caled as TSOperatio(NULL, NULL, Function, NULL). If the
functions are not in the expression call TSOperatio(NULL, NULL, NULL, NULL).
Note that the variables list is in the upper case.
*/

namespace math_eval {

struct AEvaluable : public AReferencible {
  virtual ~AEvaluable() {}
  virtual double evaluate() const = 0;
};

struct IFactory {
  virtual ~IFactory() {}
  virtual AEvaluable *create(const TPtrList<AEvaluable> &args) = 0;
};

struct NumberEvaluator : public AEvaluable {
  double value;
  NumberEvaluator(double value)
    : value(value)
  {}
  double evaluate() const { return value; }
};

struct Variable : public NumberEvaluator {
  bool value_set;
public:
  olxstr name;
  Variable(const olxstr &name)
    : NumberEvaluator(0), value_set(false), name(name)
  {}
  double evaluate() const {
    if (!value_set) {
      throw TFunctionFailedException(__OlxSourceInfo,
        olxstr("Uninitialised object: ").quote() << name);
    }
    return value;
  }
  void set_value(double v) {
    NumberEvaluator::value = v;
    value_set = true;
  }
};

struct Evaluator : public Variable {
public:
  TPtrList<AEvaluable> args;
  Evaluator(const olxstr &name, const TPtrList<AEvaluable> &args)
    : Variable(name), args(args)
  {
    for (size_t i=0; i < args.Count(); i++) args[i]->IncRef();
  }
  ~Evaluator() {
    for (size_t i=0; i < args.Count(); i++)
      if (args[i]->DecRef() == 0)
        delete args[i];
  }
};

struct FuncEvaluator1 : public AEvaluable {
  double (*func)(double);
  AEvaluable &arg;
  FuncEvaluator1(double (*func)(double), AEvaluable &arg)
    : func(func), arg(arg)
  {
    arg.IncRef();
  }
  ~FuncEvaluator1() {
    if (arg.DecRef() == 0) delete &arg;
  }
  double evaluate() const { return (*func)(arg.evaluate()); }
  struct factory : public IFactory {
    double (*func)(double);
    factory(double (*func)(double))
      : func(func)
    {}
    AEvaluable *create(const TPtrList<AEvaluable> &args) {
      if (args.Count() != 1) {
        return 0;
      }
      return new FuncEvaluator1(func, *args[0]);
    }
  };
};

struct FuncEvaluator2 : public AEvaluable {
  double (*func)(double, double);
  AEvaluable &arg1, &arg2;
  FuncEvaluator2(double (*func)(double, double),
    AEvaluable &arg1, AEvaluable &arg2)
    : func(func), arg1(arg1), arg2(arg2)
  {
    arg1.IncRef();
    arg2.IncRef();
  }
  ~FuncEvaluator2() {
    if (arg1.DecRef() == 0) {
      delete& arg1;
    }
    if (arg2.DecRef() == 0) {
      delete& arg2;
    }
  }
  double evaluate() const { return (*func)(arg1.evaluate(), arg2.evaluate()); }
  struct factory : public IFactory {
    double (*func)(double, double);
    factory(double (*func)(double, double))
      : func(func)
    {}
    AEvaluable *create(const TPtrList<AEvaluable> &args) {
      if (args.Count() != 2) {
        return 0;
      }
      return new FuncEvaluator2(func, *args[0], *args[1]);
    }
  };
};

struct ExpEvaluator {
  olxstr_dict<IFactory *, true> factory;
  static double sin_(double a) { return sin(a*M_PI/180); }
  static double cos_(double a) { return cos(a*M_PI/180); }
  static double tan_(double a) { return tan(a*M_PI/180); }
  static double asin_(double a) { return asin(a)*180/M_PI; }
  static double acos_(double a) { return acos(a)*180/M_PI; }
  static double atan_(double a) { return atan(a)*180/M_PI; }
  static double nop(double a) { return a; }
  static double chs(double a) { return -a; }
  static double add(double a, double b) { return a+b; }
  static double sub(double a, double b) { return a-b; }
  static double mul(double a, double b) { return a*b; }
  static double div(double a, double b) { return a/b; }
  static double mod(double a, double b) { return (int)a%(int)b; }
  // boolean ops
  static double eq(double a, double b) { return a == b; }
  static double neq(double a, double b) { return a != b; }
  static double gt(double a, double b) { return a > b; }
  static double ge(double a, double b) { return a >= b; }
  static double lt(double a, double b) { return a < b; }
  static double le(double a, double b) { return a <= b; }
  static double or_(double a, double b) { return a != 0 || b != 0; }
  static double and_(double a, double b) { return a != 0 && b != 0; }

  TStringToList<olxstr, AEvaluable *> ScopeVariables;
  // these are filled in by the build function
  TPtrList<Variable> Variables;
  TPtrList<Evaluator> Evaluators;

  ConstPtrList<AEvaluable> create_args(const olxstr &args);
  ExpEvaluator();
  ~ExpEvaluator();
  void build(const olxstr &expr);
  double evaluate() {
    if (eval_root == 0) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "parse an expression first");
    }
    return eval_root->evaluate();
  }
protected:
  exparse::expression_tree *root;
  AEvaluable *eval_root;
  ConstPtrList<exparse::expression_tree> build_tree(
    exparse::expression_tree * root);
  AEvaluable *create(const olxstr &name,
    const TPtrList<AEvaluable> &args);
  AEvaluable *create(exparse::expression_tree *t);
};

} // namespace math_eval
#endif
