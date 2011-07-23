/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_exparse_context_H
#define __olx_sdl_exparse_context_H
#include "funcwrap.h"
BeginEsdlNamespace()

namespace exparse  {
  struct context;

  struct VarProxy : public IEvaluable {
    IEvaluable* value;
    VarProxy(IEvaluable* _value) : value(_value) {  _value->inc_ref();  }
    ~VarProxy()  {
      if( value->dec_ref() == 0 )
        delete value;
    }
    void update_value(IEvaluable* _value)  {
      if( value == _value )  return;
      if( value->dec_ref() == 0 )
        delete value;
      value = _value;
      _value->inc_ref();
    }
    virtual const std::type_info& get_type_info() const {  return typeid(*value);  }
    virtual IEvaluable* _evaluate() const {  return value;  }
    virtual IEvaluable* find_property(const olxstr& name) {  return value->find_property(name);  }
    virtual IEvaluable* find_method(const olxstr& name, const struct EvaluableFactory& f,
      const TPtrList<IEvaluable>& args, IEvaluable* proxy=NULL)
    {
      return value->find_method(name, f, args, proxy == NULL ? this : proxy);
    }
    virtual cast_operator get_cast_operator(const std::type_info& ti) const {  
      return value->get_cast_operator(ti);
    } 
    virtual bool is_final() const {  return false;  }
  };
  struct context  {
    LibraryRegistry functions; 
    //olxdict<std::type_info const*, IClassRegistry*, TPointerComparator> classes;
    //olxdict<olxstr, IClassRegistry*, olxstrComparator<false> > classes;
    olxdict<olxstr, IEvaluable*, olxstrComparator<true> > consts;
    olxdict<olxstr, VarProxy*, olxstrComparator<false> > vars;
    context* parent;
    context(context* _parent = NULL) : parent(_parent)  {}
    ~context()  {
      //for( size_t i=0; i < classes.Count(); i++ )
      //  delete classes.GetValue(i);
      for( size_t i=0; i < consts.Count(); i++ )
        if( consts.GetValue(i)->dec_ref() == 0 )
          delete consts.GetValue(i);
      for( size_t i=0; i < vars.Count(); i++ )  {
        if( vars.GetValue(i)->dec_ref() == 0 )
          delete vars.GetValue(i);
      }
    }
    void add_var(const olxstr& name, IEvaluable* val, bool replace=false)  {
      const size_t ind = vars.IndexOf(name);
      if( replace )  {
        if( ind == InvalidIndex )
          vars.Add(name, new VarProxy(val))->inc_ref();
        else
          vars.GetValue(ind)->update_value(val);
      }
      else  {
        if( ind != InvalidIndex )
          throw TInvalidArgumentException(__OlxSourceInfo, olxstr("duplicated variable: ") << name);
        vars.Add(name, new VarProxy(val))->inc_ref();
      }
    }
    void add_const(const olxstr& name, IEvaluable* val)  {
      size_t i = consts.IndexOf(name);
      if( i != InvalidIndex )
        throw TInvalidArgumentException(__OlxSourceInfo, olxstr("duplicated constant: ") << name);
      val->inc_ref();
      consts.Add(name, val);
    }
    IEvaluable* find_var(const olxstr& name)  {
      size_t ind = vars.IndexOf(name);
      if( ind != InvalidIndex )  return vars.GetValue(ind);
      context* cx = this->parent;
      while( cx != NULL )  {
        IEvaluable* rv = cx->find_var(name);
        if( rv != NULL )  return rv;
        cx = cx->parent;
      }
      return NULL;
    }
    IEvaluable* find_const(const olxstr& name) const  {
      size_t ind = consts.IndexOf(name);
      if( ind != InvalidIndex )  return consts.GetValue(ind);
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
