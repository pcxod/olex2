#ifndef __olx_exparse_expbuilder_H
#define __olx_exparse_expbuilder_H

#include "builtins.h"
#include "exptree.h"
#include "funcwrap.h"

BeginEsdlNamespace()

namespace exparse  {
  struct context  {
    LibraryRegistry functions; 
    olxdict<olxstr, IClassRegistry*, olxstrComparator<false> > classes;
    olxdict<olxstr, IEvaluable*, olxstrComparator<true> > consts;
    olxdict<olxstr, IEvaluable*, olxstrComparator<false> > vars;
    ~context()  {
      for( int i=0; i < classes.Count(); i++ )
        delete classes.GetValue(i);
      for( int i=0; i < consts.Count(); i++ )
        if( consts.GetValue(i)->dec_ref() == 0 )
          delete consts.GetValue(i);
      for( int i=0; i < vars.Count(); i++ )
        if( vars.GetValue(i)->dec_ref() == 0 )
          delete vars.GetValue(i);
    }
    void add_var(const olxstr& name, IEvaluable* val, bool replace=false)  {
      int i = vars.IndexOf(name);
      if( replace )  {
        val->inc_ref();
        if( i == -1 )
          vars.Add(name, val);
        else  {
          if( vars.GetValue(i)->dec_ref() == 0 )
            delete vars.GetValue(i);
          vars.GetEntry(i).val = val;
        }
      }
      else  {
        if( i != -1 )
          throw TInvalidArgumentException(__OlxSourceInfo, olxstr("duplicated variable: ") << name);
        val->inc_ref();
        vars.Add(name, val);
      }
    }
    void add_const(const olxstr& name, IEvaluable* val)  {
      int i = consts.IndexOf(name);
      if( i != -1 )
        throw TInvalidArgumentException(__OlxSourceInfo, olxstr("duplicated constant: ") << name);
      val->inc_ref();
      consts.Add(name, val);
    }
  };
  struct StringValue : public IEvaluable  {
    olxstr val;
    static ClassRegistry<olxstr> functions;
    static LibraryRegistry globals;
    StringValue(const olxstr& v) : val(v) {}
    virtual IEvaluable* _evaluate() const {
      throw 1;
    }
    virtual IEvaluable* find_property(const olxstr& name) {
      throw TNotImplementedException(__OlxSourceInfo);
    }
    virtual IEvaluable* find_method(const olxstr& name, const EvaluableFactory& ef, const TPtrList<IEvaluable>& args) {
      int i = functions.index_of(name, args.Count());
      if( i == -1 )  {
        i = globals.index_of(name, args.Count());
        return i == -1 ? NULL : globals.create_from_index(ef, i, args);
      }
      return functions.create_from_index(*this, ef, &val, i, args);
    }
    static cast_result str_cast(const IEvaluable* i)  {  
      return cast_result(&(IEvaluable::cast_helper<StringValue>(i))->val, false);  
    }
    virtual cast_operator get_cast_operator(const std::type_info& ti) const {  
      if( typeid(olxstr) == ti )
        return &str_cast;
      throw TCastException(__OlxSourceInfo, ti);  
    } 
    virtual IEvaluable* create_new(const void* data) const {  return new StringValue(*(olxstr*)data);  }
    virtual bool is_final() const {  return true;  }
    // globals section
    static olxstr add(const olxstr& a, const olxstr& b)  {  return a+b; }
    static bool equals(const olxstr& a, const olxstr& b)  {  return a.Equals(b); }
    static bool equalsi(const olxstr& a, const olxstr& b)  {  return a.Equalsi(b); }
    static void init_functions()  {
      functions.add("sub", &olxstr::SubString);
      functions.add<int>("len", &olxstr::Length);  // gcc...
      globals.add("+", &StringValue::add);
      globals.add("==", &StringValue::equals);
      globals.add("equals", &StringValue::equals);
      globals.add("equalsi", &StringValue::equals);
    }
  };

  struct exp_builder  {
  protected:
    static bool needs_sorting(expression_tree* root);
    static expression_tree* sort_logical(expression_tree* root);
    IEvaluable* process_const_func(IEvaluable* func, IEvaluable* left, IEvaluable* right=NULL);
    IEvaluable* evaluator_from_evator(expression_tree* root, IEvaluable* left=NULL);
    IEvaluable* create_evaluator(expression_tree* root);
    IEvaluable* locate_function(const olxstr& name, IEvaluable* left, IEvaluable* right);
  public:
    context scope;
    EvaluableFactory& factory;
    exp_builder(EvaluableFactory& _factory) : factory(_factory)  {
      scope.add_const("PI", new DoubleValue(M_PI));
      scope.add_const("true", new BoolValue(true));
      scope.add_const("false", new BoolValue(false));
      //scope.functions.add("_sin", (double (*)(double))&sin);
      factory.types.Add(&typeid(olxstr), new StringValue(""));
      StringValue::init_functions();
    }
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
