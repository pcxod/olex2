#ifndef __olx_exparse_explib_H
#define __olx_exparse_explib_H

#include "funcwrap.h"

BeginEsdlNamespace()

namespace exparse  {
  struct StringValue : public IEvaluable  {
    olxstr val;
    static ClassInfo<olxstr> info;
    StringValue(const olxstr& v) : val(v) {}
    virtual IEvaluable* _evaluate() const {  throw 1;  }
    virtual IEvaluable* find_property(const olxstr& name) {
      throw TNotImplementedException(__OlxSourceInfo);
    }
    virtual IEvaluable* find_method(const olxstr& name, const EvaluableFactory& ef, 
      const TPtrList<IEvaluable>& args, IEvaluable* proxy=NULL)
    {
      size_t i = info.functions.index_of(name, args.Count());
      if( i == InvalidIndex )  {
        i = info.globals.index_of(name, args.Count());
        return i == InvalidIndex ? NULL : info.globals.create_from_index(ef, i, args);
      }
      return info.functions.create_from_index(proxy == NULL ? *this : *proxy, ef, i, args);
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
    static bool nequals(const olxstr& a, const olxstr& b)  {  return !a.Equals(b); }
    static bool equalsi(const olxstr& a, const olxstr& b)  {  return a.Equalsi(b); }
    static void init_library()  {
      if( !info.is_empty() )  return;
      info.functions.add("sub", &olxstr::SubString);
      info.functions.add<size_t>("len", &olxstr::Length);  // gcc...
      info.globals.add("+", &StringValue::add);
      info.globals.add("==", &StringValue::equals);
      info.globals.add("!=", &StringValue::nequals);
      info.globals.add("equals", &StringValue::equals);
      info.globals.add("equalsi", &StringValue::equals);
    }
  };
};  // namespace exparse

EndEsdlNamespace()

#endif
