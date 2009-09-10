#ifndef __olx_evaluable_H
#define __olx_evaluable_H
#include "../ebase.h"
#include "../edict.h"

BeginEsdlNamespace()

namespace exparse  {
  class TCastException : public TBasicException  {
    const std::type_info& type;
  public:
    TCastException(const olxstr& src, const std::type_info& ti) : type(ti),
      TBasicException(src, olxstr("Invaild cast to ") << type.name()) {};
    TCastException(const TCastException& ce) : type(ce.type), TBasicException(ce) {}
    const std::type_info& GetTypeInfo() const {  return type;  }
    virtual IEObject* Replicate() const {  return new TCastException(*this);  }
  };

  struct IEvaluable {
    virtual ~IEvaluable() {}
    virtual IEvaluable* _evaluate() const = 0;
    typedef void* (*cast_operator)(const IEvaluable*);
    typedef olxdict<std::type_info const*, cast_operator, TPointerPtrComparator> operator_dict;
    virtual cast_operator get_cast_operator(const std::type_info&) const = 0;
    virtual IEvaluable* create_new(const void*)  {
      throw TNotImplementedException(__OlxSourceInfo);
    }
    template <class T> T cast() const {
      const std::type_info& ti = typeid(T);
      try  {  
        cast_operator co = get_cast_operator(ti);
        if( co != NULL )  {
          T* cast_result = (T*)(*co)(this);
          T result(*cast_result);
          delete cast_result;
          return result;
        }
      }
      catch(...)  {}
      throw TCastException(__OlxSourceInfo, ti);
    }
    template <class T> static const T* cast_helper(const IEvaluable* i)  {
      const T* ci = dynamic_cast<const T*>(i);
      if( ci == NULL )  
        throw TCastException(__OlxSourceInfo, typeid(i));
      return ci;
    }
  };

  struct ANumberEvaluator : public IEvaluable  {
  protected:
    static IEvaluable::operator_dict cast_operators;
    static const IEvaluable::operator_dict::Entry cast_operators_table[];
    static void* bool_cast(const IEvaluable* i)  {  
      return new bool(IEvaluable::cast_helper<ANumberEvaluator>(i)->Evaluate() != 0);  
    }
    static void* str_cast(const IEvaluable* i)  {  return new olxstr(IEvaluable::cast_helper<ANumberEvaluator>(i)->Evaluate());  }
    template<class T> static void* primitive_cast(const IEvaluable* i)  {  return new T((T)IEvaluable::cast_helper<ANumberEvaluator>(i)->Evaluate());  }
    template<class T> static void register_cast()  {  cast_operators.Add(&typeid(T), &ANumberEvaluator::primitive_cast<T>);  }
    template<class T> static IEvaluable::operator_dict::Entry create_operator_entry()  {  
      return IEvaluable::operator_dict::Entry(&typeid(T), &ANumberEvaluator::primitive_cast<T>);  
    }
    virtual cast_operator get_cast_operator(const std::type_info& ti) const {  return cast_operators[&ti];  } 
    virtual double Evaluate() const {
      IEvaluable* ev = _evaluate();
      const double rv = ev->cast<double>();
      return rv;
    }
  public:
    ANumberEvaluator()  {}
  };
  template <class BC, typename Type>
  struct TPrimitiveEvaluator : public ANumberEvaluator, public BC  {
    TPrimitiveEvaluator(const Type& val) : BC(val)  {}
    virtual double Evaluate() const {
      return (double)val;
    }
    virtual IEvaluable* _evaluate() const {  
      throw 1;
    }
  };

  template <class T>
  struct TPrimitiveInstance  {
  public:
    T val;
    TPrimitiveInstance(const T& _val) : val(_val)  {}
    const T& get_value() const {  return val;  }
  };

  struct VoidValue : public IEvaluable {
    virtual IEvaluable* _evaluate() const {
      throw TFunctionFailedException(__OlxSourceInfo, "cannot evaluate void type");
    }
    virtual cast_operator get_cast_operator(const std::type_info&) const {
      throw TFunctionFailedException(__OlxSourceInfo, "no casting is avilable for the void type");
    }
  };
  typedef TPrimitiveEvaluator<TPrimitiveInstance<bool>,bool> BoolValue;
  typedef TPrimitiveEvaluator<TPrimitiveInstance<int>,int> IntValue;
  typedef TPrimitiveEvaluator<TPrimitiveInstance<double>,double> DoubleValue;

  struct IEvaluableFactory  {
  
  };
  struct EvaluableFactory  {
    static olxdict<std::type_info const*, IEvaluable*, TPointerPtrComparator> types;
    template <class T> void add_ptype()  {
      types.Add(&typeid(bool), new TPrimitiveEvaluator<TPrimitiveInstance<T>,T>(0));
    }
    EvaluableFactory()  {
      if( types.IsEmpty() )  {
        add_ptype<bool>();
        add_ptype<char>();
        add_ptype<unsigned char>();
        add_ptype<short int>();
        add_ptype<unsigned short int>();
        add_ptype<int>();
        add_ptype<unsigned int>();
        add_ptype<long int>();
        add_ptype<unsigned long int>();
        add_ptype<long long int>();
        add_ptype<unsigned long long int>();
        add_ptype<float>();
        add_ptype<double>();
        add_ptype<long double>();
      }
    }
    template <class T> static IEvaluable* create(const T& val)  {
      const std::type_info& ti = typeid(T);
      int i = types.IndexOf(&ti);
      if( i == -1 )
        throw TFunctionFailedException(__OlxSourceInfo, olxstr("Could not locate object factory for ") << ti.name());
      return types.GetValue(i)->create_new(&val);
    };
  };
};  // end namespace exparse

EndEsdlNamespace()

#endif