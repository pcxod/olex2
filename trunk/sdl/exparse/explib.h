/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_exparse_explib_H
#define __olx_sdl_exparse_explib_H
#include "funcwrap.h"
BeginEsdlNamespace()

namespace exparse  {
  struct StringValue : public IEvaluable, public IEObject  {
    olxstr val;
    static ClassInfo<StringValue, olxstr> info;
    StringValue() {}
    StringValue(const olxstr& v) : val(v) {}
    virtual IEvaluable* _evaluate() const {  throw 1;  }
    virtual IEvaluable* find_property(const olxstr& name) {
      throw TNotImplementedException(__OlxSourceInfo);
    }
    virtual IEvaluable* find_method(const olxstr& name,
      const EvaluableFactory& ef,
      const TPtrList<IEvaluable>& args, IEvaluable* proxy=NULL)
    {
      size_t i = info.functions.index_of(name, args.Count());
      if( i == InvalidIndex )  {
        i = info.wrap_functions.index_of(name, args.Count());
        if( i == InvalidIndex )  {
          i = info.globals.index_of(name, args.Count());
          return i == InvalidIndex ? NULL
            : info.globals.create_from_index(ef, i, args);
        }
        return info.wrap_functions.create_from_index(proxy == NULL ? *this
          : *proxy, ef, i, args);
      }
      return info.functions.create_from_index(proxy == NULL ? *this
        : *proxy, ef, i, args);
    }
    static cast_result str_cast(const IEvaluable* i)  {
      return cast_result(&(IEvaluable::cast_helper<StringValue>(i))->val, false);
    }
    virtual cast_operator get_cast_operator(const std::type_info& ti) const {
      if( typeid(olxstr) == ti )
        return &str_cast;
      throw TCastException(__OlxSourceInfo, ti);
    }
    virtual IEvaluable* create_new(const void *data) const {
      return new StringValue(*static_cast<const olxstr*>(data));
    }
    virtual bool is_final() const {  return true;  }
    // wrapper functions
    int atoi() const {  return val.SafeInt<int>(); }
    float atof() const {  return val.ToFloat(); }
    olxstr idx_op(size_t i) const { return val.CharAt(i); }
    // globals section
    static olxstr add(const olxstr& a, const olxstr& b)  {  return a+b; }
    static bool equals(const olxstr& a, const olxstr& b)  {  return a.Equals(b); }
    static bool nequals(const olxstr& a, const olxstr& b)  {  return !a.Equals(b); }
    static bool equalsi(const olxstr& a, const olxstr& b)  {  return a.Equalsi(b); }
    static void init_library()  {
      if( !info.is_empty() )  return;
      info.functions.add("sub", &olxstr::SubString);
      info.functions.add<size_t>("len", &olxstr::Length);  // gcc...
      info.functions.add<olxch,size_t>("charAt", &olxstr::CharAt);
      info.functions.add("toUpper", &olxstr::ToUpperCase);
      info.functions.add("toLower", &olxstr::ToLowerCase);

      info.wrap_functions.add("atoi", &StringValue::atoi);
      info.wrap_functions.add("atof", &StringValue::atof);
      info.wrap_functions.add("_idx_", &StringValue::idx_op);

      info.globals.add("+", &StringValue::add);
      info.globals.add("==", &StringValue::equals);
      info.globals.add("!=", &StringValue::nequals);
      info.globals.add("equals", &StringValue::equals);
      info.globals.add("equalsi", &StringValue::equals);
    }
  };

  struct ListValue : public IEvaluable, public IEObject  {
    typedef TPtrList<IEvaluable> list_t;
    list_t val;
    static ClassInfo<ListValue, list_t> info;
    ListValue() {}
    ListValue(const TPtrList<IEvaluable>& v) : val(v) {
      for (size_t i=0; i < val.Count(); i++)
        val[i]->inc_ref();
    }
    ~ListValue() {
      for (size_t i=0; i < val.Count(); i++)
        if (val[i]->dec_ref() == 0)
          delete val[i];
    }
    virtual IEvaluable* _evaluate() const {  throw 1;  }
    virtual IEvaluable* find_property(const olxstr& name) {
      throw TNotImplementedException(__OlxSourceInfo);
    }
    virtual IEvaluable* find_method(const olxstr& name,
      const EvaluableFactory& ef,
      const TPtrList<IEvaluable>& args, IEvaluable* proxy=NULL)
    {
      size_t i = info.functions.index_of(name, args.Count());
      if( i == InvalidIndex )  {
        i = info.wrap_functions.index_of(name, args.Count());
        if( i == InvalidIndex )  {
          i = info.globals.index_of(name, args.Count());
          return i == InvalidIndex ? NULL
            : info.globals.create_from_index(ef, i, args);
        }
        return info.wrap_functions.create_from_index(proxy == NULL ? *this
          : *proxy, ef, i, args);
      }
      return info.functions.create_from_index(proxy == NULL ? *this
        : *proxy, ef, i, args);
    }
    static cast_result list_cast(const IEvaluable* i)  {
      return cast_result(&(IEvaluable::cast_helper<ListValue>(i))->val, false);
    }
    virtual cast_operator get_cast_operator(const std::type_info& ti) const {
      if( typeid(list_t) == ti )
        return &list_cast;
      throw TCastException(__OlxSourceInfo, ti);
    }
    virtual IEvaluable* create_new(const void *data) const {
      return new ListValue(*static_cast<const list_t*>(data));
    }
    virtual bool is_final() const {  return true;  }
    // wrapper functions
    IEvaluable& idx_op(size_t i) const { return *val[i]; }
    void add(IEvaluable &v_)  {
      IEvaluable *v = v_.undress();
      v->inc_ref();
      val.Add(v);
    }
    // globals section
    static list_t add(const list_t& a, const list_t& b)  {  return list_t(a) << b; }
    static void init_library()  {
      if( !info.is_empty() )  return;
      info.functions.add("sub", &list_t::SubList);
      info.functions.add<size_t>("size", &list_t::Count);  // gcc...

      info.wrap_functions.add("_idx_", &ListValue::idx_op);
      info.wrap_functions.add("add", &ListValue::add);

      info.globals.add("+", &ListValue::add);
      //info.globals.add("==", &StringValue::equals);
      //info.globals.add("!=", &StringValue::nequals);
      //info.globals.add("equals", &StringValue::equals);
      //info.globals.add("equalsi", &StringValue::equals);
    }
  };
};  // namespace exparse

EndEsdlNamespace()
#endif
