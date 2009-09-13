#ifndef __olx_exparse_expbuilder_H
#define __olx_exparse_expbuilder_H

#include "builtins.h"
#include "exptree.h"
#include "funcwrap.h"

BeginEsdlNamespace()

namespace exparse  {
  struct context  {
    TPtrList<IEvaluable> Vars;
    TStrList VarNames;
    olxdict<olxstr, IEvaluable*, olxstrComparator<false> > UserFunctions;
    LibraryRegistry functions; 
    //olxdict<olxstr, ClassRegistry*, olxstrComparator<false> > classes;
    ~context()  {
//      for( int i=0; i < classes.Count(); i++ )
//        delete classes.GetValue(i);
    }
  };
  struct Variable : public ANumberEvaluator  {
    size_t index;
    const context& scope;
    Variable(const context& _scope, size_t _index) : scope(_scope), index(_index) {}
    virtual IEvaluable* _evaluate() const {  return scope.Vars[index]; }
  };

  struct exp_builder  {
  protected:
    static bool needs_sorting(expression_tree* root);
    static expression_tree* sort_logical(expression_tree* root);
    IEvaluable* process_const_func(IEvaluable* func, IEvaluable* left, IEvaluable* right=NULL);
    IEvaluable* evaluator_from_evator(expression_tree* root);
    IEvaluable* create_evaluator(expression_tree* root);
  public:
    context scope;
    EvaluableFactory& factory;
    exp_builder(EvaluableFactory& _factory) : factory(_factory)  {}
    IEvaluable* build(const olxstr& exp)  {
      expression_parser expp(exp);
      expp.expand();
      expp.root = sort_logical(expp.root);
      return create_evaluator(expp.root);
    }
  };
};  // end exparse namespace

EndEsdlNamespace()

#endif
