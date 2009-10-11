#ifndef __olx_exparse_context_H
#define __olx_exparse_context_H

#include "funcwrap.h"

BeginEsdlNamespace()

namespace exparse  {
  struct context  {
    LibraryRegistry functions; 
    olxdict<olxstr, IClassRegistry*, olxstrComparator<false> > classes;
    olxdict<olxstr, IEvaluable*, olxstrComparator<true> > consts;
    olxdict<olxstr, IEvaluable*, olxstrComparator<false> > vars;
    context* parent;
    context(context* _parent = NULL) : parent(_parent)  {}
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
    IEvaluable* find_var(const olxstr& name) const  {
      int ind = vars.IndexOf(name);
      if( ind != -1 )  return vars.GetValue(ind);
      context* cx = this->parent;
      while( cx != NULL )  {
        IEvaluable* rv = cx->find_var(name);
        if( rv != NULL )  return rv;
        cx = cx->parent;
      }
      return NULL;
    }
    IEvaluable* find_const(const olxstr& name) const  {
      int ind = consts.IndexOf(name);
      if( ind != -1 )  return consts.GetValue(ind);
      context* cx = this->parent;
      while( cx != NULL )  {
        IEvaluable* rv = cx->find_const(name);
        if( rv != NULL )  return rv;
        cx = cx->parent;
      }
      return NULL;
    }
    static void init_global(context& cx)  {
      cx.add_const("PI", new DoubleValue(M_PI));
      cx.add_const("true", new BoolValue(true));
      cx.add_const("false", new BoolValue(false));
    }
  };
};  // end exparse namespace

EndEsdlNamespace()

#endif
