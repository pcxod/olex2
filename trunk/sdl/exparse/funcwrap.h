/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef olx_func_wrap_H
#define olx_func_wrap_H
#include "../ebase.h"
#include "../typelist.h"
#include "../edict.h"
#include "../estlist.h"
#include "evaluable.h"
#include <typeinfo>
BeginEsdlNamespace()

namespace exparse  {

  using namespace std;
  /* function interface */
  struct IBasicFunction  {
    virtual ~IBasicFunction()  {}
    virtual const olxstr& get_name() const = 0;
    virtual olxstr get_signature() const = 0;
    virtual size_t get_argc() const  = 0;
    virtual const type_info& get_arg_type(size_t i) const = 0;
    virtual const type_info& get_RV_type() const = 0;
  };
  struct IMemberFunction : public IBasicFunction  {
    virtual IEvaluable* run(IOlxObject *self, const EvaluableFactory& factory,
      const TPtrList<IEvaluable>& params) = 0;
  };
  struct IStaticFunction : public IBasicFunction  {
    virtual IEvaluable* run(const EvaluableFactory& factory,
      const TPtrList<IEvaluable>& params) = 0;
  };

  /* abstract function */
  template <class base, class rv> class AFunction : public base {
  protected:
    TPtrList<type_info const> arg_types;
    olxstr name;
  public:
    AFunction(const olxstr& _name ) : name(_name)  {}
    virtual const olxstr& get_name() const {  return name;  }
    virtual olxstr get_signature() const {
      olxstr retval = get_RV_type().name();
      retval << ' ' << get_name() << '(';
      for( size_t i=0; i < arg_types.Count(); i++ )  {
        retval << arg_types[i]->name();
        if( (i+1) < arg_types.Count() )
          retval << ", ";
      }
      retval << ')';
      return retval;
    }
    virtual size_t get_argc() const  {  return arg_types.Count();  }
    virtual const type_info& get_arg_type(size_t i) const {
      return *arg_types[i];
    }
    virtual const type_info& get_RV_type() const {  return typeid(rv);  }
  };

  /////////////////////////////  FUNCTIONS RETURNING VOID  //////////////////////
  /////////////////////////////  NO ARGUMENT FUNCTION RETURNING VOID  ///////////
  // static function
  class VoidFunction : public AFunction<IStaticFunction,void> {
    void (*func)();
  public:
    VoidFunction(const olxstr& name, void (*f)(void) )
      : AFunction<IStaticFunction,void>(name), func(f) {}
    virtual IEvaluable* run(const EvaluableFactory&,
      const TPtrList<IEvaluable>& params)
    {
      this->func();
      return new VoidValue;
    }
  };
  // class member function
  template <class base_class> class VoidMemberFunction
    : public AFunction<IMemberFunction,void>
  {
    void (base_class::*func)();
  public:
    VoidMemberFunction(const olxstr& name, void (base_class::*f)() )
      : AFunction<IMemberFunction,void>(name), func(f)  {}
    virtual IEvaluable* run(IOlxObject *self, const EvaluableFactory&,
      const TPtrList<IEvaluable>& params )
    {
      IEvaluable *b = dynamic_cast<IEvaluable *>(self);
      if (b != NULL)
        (b->cast<base_class>().val->*func)();
      else
        (dynamic_cast<base_class*>(self)->*func)();
      return new VoidValue;
    }
  };
  /////////////////////////////  SINGLE ARGUMENT FUNCTIONS RETURNING VOID  ////
  // static function
  template <class argt_1> class VoidFunction1
    : public AFunction<IStaticFunction,void>
  {
    void (*func)(argt_1);
  public:
    VoidFunction1(const olxstr& name, void (*f)(argt_1))
      : AFunction<IStaticFunction,void>(name), func(f)
    {
      arg_types.Add(typeid(argt_1));
    }
    virtual IEvaluable* run(const EvaluableFactory&,
      const TPtrList<IEvaluable>& params)
    {
      this->func(params[0]->cast<argt_1>());
      return new VoidValue;
    }
  };
  // class member function
  template <class base_class, typename argt_1>
  class VoidMemberFunction1 : public AFunction<IMemberFunction, void> {
    void (base_class::*func)(argt_1);
  public:
    VoidMemberFunction1(const olxstr& name, void (base_class::*f)(argt_1))
      : AFunction<IMemberFunction,void>(name), func(f)
    {
      arg_types.Add(typeid(argt_1));
    }
    virtual IEvaluable* run(IOlxObject *self, const EvaluableFactory&,
      const TPtrList<IEvaluable>& params )
    {
      IEvaluable *b = dynamic_cast<IEvaluable *>(self);
      if (b != NULL)
        (b->cast<base_class>().val->*func)(params[0]->cast<argt_1>());
      else
        (dynamic_cast<base_class*>(self)->*func)(params[0]->cast<argt_1>());
      return new VoidValue;
    }
  };
  /////////////////////////////  TWO ARGUMENT FUNCTIONS RETURNING VOID  ///////
  // static function
  template <class argt_1, class argt_2>
  class VoidFunction2 : public AFunction<IStaticFunction,void> {
    void (*func)(argt_1, argt_2);
  public:
    VoidFunction2(const olxstr& name, void (*f)(argt_1, argt_2) )
      : AFunction<IStaticFunction,void>(name), func(f)
    {
      arg_types.Add(typeid(argt_1));
      arg_types.Add(typeid(argt_2));
    }
    virtual IEvaluable* run(const EvaluableFactory&,
      const TPtrList<IEvaluable>&params)
    {
      this->func(params[0]->cast<argt_1>(), params[1]->cast<argt_2>());
      return new VoidValue;
    }
  };
  // class member function
  template <class base_class, class argt_1, class argt_2>
  class VoidMemberFunction2 : public AFunction<IMemberFunction,void> {
    void (base_class::*func)(argt_1, argt_2);
  public:
    VoidMemberFunction2(const olxstr& name,
      void (base_class::*f)(argt_1, argt_2) )
      : AFunction<IMemberFunction,void>(name), func(f)
    {
      arg_types.Add(typeid(argt_1));
      arg_types.Add(typeid(argt_2));
    }
    virtual IEvaluable* run(IOlxObject *self, const EvaluableFactory&,
      const TPtrList<IEvaluable>&params )
    {
      IEvaluable *b = dynamic_cast<IEvaluable *>(self);
      if (b != NULL)
        (b->cast<base_class>().val->*func)(
          params[0]->cast<argt_1>(),
          params[1]->cast<argt_2>());
      else
        (dynamic_cast<base_class*>(self)->*func)(
          params[0]->cast<argt_1>(),
          params[1]->cast<argt_2>());
      return new VoidValue;
    }
  };

  /////////////////////////////  FUNCTIONS RETURNING A VALUE //////////////////
  /////////////////////////////  NO ARGUMENT FUNCTION RETURNING A VALUE ///////
  // static function
  template <class rvt> class Function : public AFunction<IStaticFunction,rvt> {
    rvt (*func)();
  public:
    Function(const olxstr& name, rvt (*f)(void) )
      : AFunction<IStaticFunction,rvt>(name), func(f) {}
    virtual IEvaluable* run(const EvaluableFactory& factory,
      const TPtrList<IEvaluable>& params)
    {
      return factory.create<rvt>(this->func());
    }
  };
  // class member function
  template <class rvt, class base_class> class MemberFunction
    : public AFunction<IMemberFunction,rvt>
  {
    rvt (base_class::*func)();
  public:
    MemberFunction(const olxstr& name, rvt (base_class::*f)() )
      : AFunction<IMemberFunction,rvt>(name), func(f) {}
    virtual IEvaluable* run(IOlxObject *self, const EvaluableFactory& factory,
      const TPtrList<IEvaluable>& params)
    {
      IEvaluable *b = dynamic_cast<IEvaluable *>(self);
      if (b != NULL)
        return factory.create<rvt>((b->cast<base_class>().val->*func)());
      return factory.create<rvt>((dynamic_cast<base_class*>(self)->*func)());
    }
  };
  /////////////////////////////  SINGLE ARGUMENT FUNCTIONS RETURNING A VALUE  /
  // static function
  template <class rvt, class argt_1> class Function1
    : public AFunction<IStaticFunction,rvt>
  {
    rvt (*func)(argt_1);
  public:
    Function1(const olxstr& name, rvt (*f)(argt_1))
      : AFunction<IStaticFunction,rvt>(name), func(f)
    {
      AFunction<IStaticFunction,rvt>::arg_types.Add(typeid(argt_1));
    }
    virtual IEvaluable* run(const EvaluableFactory& factory,
      const TPtrList<IEvaluable>&params)
    {
      return factory.create<rvt>(this->func(params[0]->cast<argt_1>()));
    }
  };
  //class member function
  template <class rvt, class base_class, class argt_1>
  class MemberFunction1 : public AFunction<IMemberFunction,rvt> {
    rvt (base_class::*func)(argt_1);
  public:
    MemberFunction1(const olxstr& name, rvt (base_class::*f)(argt_1) )
      : AFunction<IMemberFunction,rvt>(name), func(f)
    {
      AFunction<IMemberFunction,rvt>::arg_types.Add(typeid(argt_1));
    }
    virtual IEvaluable* run(IOlxObject *self, const EvaluableFactory& factory,
      const TPtrList<IEvaluable>& params)
    {
      IEvaluable *b = dynamic_cast<IEvaluable *>(self);
      if (b != NULL)
        return factory.create<rvt>((b->cast<base_class>().val->*func)(
          params[0]->cast<argt_1>()));
      return factory.create<rvt>((dynamic_cast<base_class*>(self)->*func)(
        params[0]->cast<argt_1>()));
    }
  };
  /////////////////////////////  TWO ARGUMENT FUNCTIONS RETURNING A VALUE  ////
  // static function
  template <class rvt, class argt_1, class argt_2>
  class Function2 : public AFunction<IStaticFunction,rvt> {
    rvt (*func)(argt_1, argt_2);
  public:
    Function2(const olxstr& name,  rvt(*f)(argt_1,argt_2) ) :
      AFunction<IStaticFunction,rvt>(name), func(f)
    {
      AFunction<IStaticFunction,rvt>::arg_types.Add(typeid(argt_1));
      AFunction<IStaticFunction,rvt>::arg_types.Add(typeid(argt_2));
    }
    virtual IEvaluable* run(const EvaluableFactory& factory,
      const TPtrList<IEvaluable>&params )
    {
      return factory.create<rvt>(this->func(params[0]->cast<argt_1>(),
        params[1]->cast<argt_2>()));
    }
  };
  //class member function
  template <class rvt, class base_class, class argt_1, class argt_2>
  class MemberFunction2 : public AFunction<IMemberFunction,rvt> {
    rvt (base_class::*func)(argt_1, argt_2);
  public:
    MemberFunction2(const olxstr& name, rvt (base_class::*f)(argt_1, argt_2))
      : AFunction<IMemberFunction,rvt>(name), func(f)
    {
      AFunction<IMemberFunction,rvt>::arg_types.Add(typeid(argt_1));
      AFunction<IMemberFunction,rvt>::arg_types.Add(typeid(argt_2));
    }
    virtual IEvaluable* run(IOlxObject *self, const EvaluableFactory& factory,
      const TPtrList<IEvaluable>& params)
    {
      IEvaluable *b = dynamic_cast<IEvaluable *>(self);
      if (b != NULL)
        return factory.create((b->cast<base_class>().val->*func)(
            params[0]->cast<argt_1>(), params[1]->cast<argt_2>()));
      return factory.create( (dynamic_cast<base_class*>(self)->*func)(
          params[0]->cast<argt_1>(), params[1]->cast<argt_2>()));
    }
  };
  /////////////////////////////////////////////////////////////////////////////
  class LibraryRegistry  {
    olxstr_dict<IStaticFunction*> funcs;
    void _add(const olxstr& name, IStaticFunction* f)  {
      if( funcs.IndexOf(name) != InvalidIndex )  {
        delete f;
        throw TFunctionFailedException(__OlxSourceInfo,
          olxstr("Duplicate function instance: ").quote() << name);
      }
      funcs.Add(name, f);
    }
  public:
    ~LibraryRegistry()  {
      for( size_t i=0; i < funcs.Count(); i++ )
        delete funcs.GetValue(i);
    }
    inline bool is_empty() const {  return funcs.IsEmpty();  }
    inline IStaticFunction* find(const olxstr& name, size_t argc) const {
      size_t ind = funcs.IndexOf( olxstr(name) << '#' << argc);
      return ind == InvalidIndex ? NULL : funcs.GetValue(ind);
    }
    inline size_t index_of(const olxstr& name, size_t argc) const {
      return funcs.IndexOf( olxstr(name) << '#' << argc);
    }
    void add(const olxstr& name, void (*f)(void))  {
      _add(name + "#0", new VoidFunction(name, f));
    }
    template<class argt_1>
    void add(const olxstr& name, void (*f)(argt_1) )  {
      _add(name + "#1", new VoidFunction1<argt_1>(name, f));
    }
    template<class argt_1, class argt_2>
    void add(const olxstr& name, void (*f)(argt_1,argt_2) )  {
      _add(name + "#2", new VoidFunction2<argt_1,argt_2>(name, f));
    }
    template <class rvt>
    void add(const olxstr& name, rvt (*f)(void))  {
      _add(name + "#0", new Function<rvt>(name, f));
    }
    template<class rvt, class argt_1>
    void add(const olxstr& name, rvt (*f)(argt_1) )  {
      _add(name + "#1", new Function1<rvt,argt_1>(name, f));
    }
    template<class rvt, class argt_1, class argt_2>
    void add(const olxstr& name, rvt (*f)(argt_1,argt_2) )  {
      _add(name + "#2", new Function2<rvt,argt_1,argt_2>(name, f));
    }

    struct FuncEvaluator : public IEvaluable  {
      IStaticFunction* func;
      TPtrList<IEvaluable> args;
      const EvaluableFactory& factory;
      FuncEvaluator(const EvaluableFactory& fc, IStaticFunction* f,
        const TPtrList<IEvaluable>& a) : func(f), args(a), factory(fc)
      {
        for( size_t i=0; i < args.Count(); i++ )
          args[i]->inc_ref();
      }
      ~FuncEvaluator()  {
        for( size_t i=0; i < args.Count(); i++ )
          if( args[i]->dec_ref() == 0 )
            delete args[i];
      }
      virtual IEvaluable* _evaluate() const {
        return func->run(factory, args);
      }
      virtual bool is_function() const {  return true;  }
    };
    IEvaluable* create_from_name(const EvaluableFactory& factory,
      const olxstr& name, const TPtrList<IEvaluable>& args)
    {
      IStaticFunction* gf = find(name, args.Count());
      if( gf == NULL ) {
        throw TInvalidArgumentException(__OlxSourceInfo,
          olxstr("could not locate specified function: ").quote() << name);
      }
      return new FuncEvaluator(factory, gf, args);
    }
    IEvaluable* create_from_index(const EvaluableFactory& factory,
      size_t index, const TPtrList<IEvaluable>& args)
    {
      return new FuncEvaluator(factory, funcs.GetValue(index), args);
    }
    IEvaluable* call(const EvaluableFactory& factory, const olxstr& name,
      const TPtrList<IEvaluable>& args)
    {
      IStaticFunction* gf = find(name, args.Count());
      if( gf == NULL ) {
        throw TInvalidArgumentException(__OlxSourceInfo,
          olxstr("could not locate specified function: ").quote() << name);
      }
      return gf->run(factory, args);
    }
    static void CompileTest();
  };

  struct IClassInfo  {
    virtual IStaticFunction* find_static_function(const olxstr& name,
      size_t argc) const = 0;
    virtual IMemberFunction* find_member_function(const olxstr& name,
      size_t argc) const = 0;
  };

  struct IClassRegistry  {
    virtual ~IClassRegistry() {}
    virtual IEvaluable* create_from_name(IEvaluable& self,
      const EvaluableFactory& factory,
      const olxstr& name,
      const TPtrList<IEvaluable>& args) const = 0;
    virtual IEvaluable* create_from_index(IEvaluable& self,
      const EvaluableFactory& factory,
      size_t index,
      const TPtrList<IEvaluable>& args) const = 0;
    virtual bool is_empty() const = 0;
    virtual IMemberFunction* find(const olxstr& name, size_t argc) const = 0;
  };

  struct AClassFuncEvaluator : public IEvaluable  {
    const EvaluableFactory& factory;
    IEvaluable& self;
    const std::type_info& self_rtti;
    IMemberFunction* func;
    TPtrList<IEvaluable> args;
    AClassFuncEvaluator(const EvaluableFactory& fc, IEvaluable& s,
      IMemberFunction* f, const TPtrList<IEvaluable>& a)
      : factory(fc), self(s), self_rtti(s.get_type_info()), func(f), args(a)
    {
      s.inc_ref();
      for( size_t i=0; i < args.Count(); i++ )
        args[i]->inc_ref();
    }
    ~AClassFuncEvaluator()  {
      if( self.dec_ref() == 0 )
        delete &self;
      for( size_t i=0; i < args.Count(); i++ )
        if( args[i]->dec_ref() == 0 )
          delete args[i];
    }
    virtual bool is_function() const {  return true;  }
  };

  template <class base_class> class ClassRegistry
    : public IClassRegistry
  {
    olxstr_dict<IMemberFunction*> funcs;
    void _add(const olxstr& name, IMemberFunction* f)  {
      if( funcs.IndexOf(name) != InvalidIndex )  {
        delete f;
        throw TFunctionFailedException(__OlxSourceInfo,
          olxstr("Duplicate function instance: ").quote() << name);
      }
      funcs.Add(name, f);
    }
  public:
    ~ClassRegistry()  {
      for( size_t i=0; i < funcs.Count(); i++ )
        delete funcs.GetValue(i);
    }
    virtual inline bool is_empty() const {  return funcs.IsEmpty();  }
    inline IMemberFunction* find(const olxstr& name, size_t argc) const {
      size_t ind = funcs.IndexOf(olxstr(name) << '#' << argc);
      return ind == InvalidIndex ? NULL : funcs.GetValue(ind);
    }
    inline size_t index_of(const olxstr& name, size_t argc) const {
      return funcs.IndexOf( olxstr(name) << '#' << argc);
    }
    void add(const olxstr& name, void (base_class::*f)())  {
      _add(name + "#0", new VoidMemberFunction<base_class>(name, f));
    }
    void add(const olxstr& name, void (base_class::*f)() const)  {
      _add(name + "#0", new VoidMemberFunction<base_class>(name,
        (void (base_class::*)())f));
    }
    template<class argt_1>
    void add(const olxstr& name, void (base_class::*f)(argt_1) )  {
      _add(name + "#1", new VoidMemberFunction1<base_class,argt_1>(
        name, f));
    }
    template<class argt_1>
    void add(const olxstr& name, void (base_class::*f)(argt_1) const)  {
      _add(name + "#1", new VoidMemberFunction1<base_class,argt_1>(name,
        (void (base_class::*)(argt_1))f));
    }
    template<class argt_1, class argt_2>
    void add(const olxstr& name, void (base_class::*f)(argt_1,argt_2) )  {
      _add(name + "#2", new VoidMemberFunction2<base_class,argt_1,argt_2>(
        name, f));
    }
    template<class argt_1, class argt_2>
    void add(const olxstr& name, void (base_class::*f)(argt_1,argt_2) const)  {
      _add(name + "#2", new VoidMemberFunction2<base_class,argt_1,argt_2>(name,
        (void (base_class::*)(argt_1,argt_2))f));
    }
    template <class rvt>
    void add(const olxstr& name, rvt (base_class::*f)())  {
      _add(name + "#0", new MemberFunction<rvt,base_class>(name, f));
    }
    template <class rvt>
    void add(const olxstr& name, rvt (base_class::*f)() const)  {
      _add(name + "#0", new MemberFunction<rvt,base_class>(name,
        (rvt (base_class::*)())f));
    }
    template<class rvt, class argt_1>
    void add(const olxstr& name, rvt (base_class::*f)(argt_1) )  {
      _add(name + "#1", new MemberFunction1<rvt,base_class,argt_1>(name, f));
    }
    template<class rvt, class argt_1>
    void add(const olxstr& name, rvt (base_class::*f)(argt_1) const)  {
      _add(name + "#1", new MemberFunction1<rvt,base_class,argt_1>(name,
        (rvt (base_class::*)(argt_1))f));
    }
    template<class rvt, class argt_1, class argt_2>
    void add(const olxstr& name, rvt (base_class::*f)(argt_1,argt_2) )  {
      _add(name + "#2", new MemberFunction2<rvt,base_class,argt_1,argt_2>(
        name, f));
    }
    template<class rvt, class argt_1, class argt_2>
    void add(const olxstr& name, rvt (base_class::*f)(argt_1,argt_2) const)  {
      _add(name + "#2", new MemberFunction2<rvt,base_class,argt_1,argt_2>(
        name, (rvt (base_class::*)(argt_1,argt_2))f));
    }

    struct FuncEvaluator : public AClassFuncEvaluator {
      FuncEvaluator(const EvaluableFactory& fc, IEvaluable& s,
        IMemberFunction* f, const TPtrList<IEvaluable>& a)
        : AClassFuncEvaluator(fc, s, f, a)
      {}
      virtual IEvaluable* find_method(const olxstr& name,
        const EvaluableFactory& f,
        const TPtrList<IEvaluable>& args,
        IEvaluable* proxy=NULL)
      {
        size_t i = f.classes.IndexOf(&func->get_RV_type());
        if( i == InvalidIndex )  return NULL;
        IMemberFunction* mf = f.classes.GetValue(i)->find_member_function(
          name, args.Count());
        if( mf == NULL )  {
          IStaticFunction* sf = f.classes.GetValue(i)->find_static_function(
            name, args.Count());
          return sf == NULL ? NULL : new LibraryRegistry::FuncEvaluator(
            f, sf, args);
        }
        else
          return new FuncEvaluator(f, *this, mf, args);
      }
      virtual IEvaluable* _evaluate() const {
        if( self_rtti != self.get_type_info() ) {
          throw TInvalidArgumentException(__OlxSourceInfo,
            "the underlying object type has changed");
        }
        return func->run(self.cast<base_class>().val, factory, args);
      }
    };
    virtual IEvaluable* create_from_name(IEvaluable& self,
      const EvaluableFactory& factory,
      const olxstr& name, const TPtrList<IEvaluable>& args) const
    {
      IMemberFunction* gf = find(name, args.Count());
      if( gf == NULL ) {
        throw TInvalidArgumentException(__OlxSourceInfo,
          olxstr("could not locate specified function: ").quote() << name);
      }
      return new FuncEvaluator(factory, self, gf, args);
    }
    virtual IEvaluable* create_from_index(IEvaluable& self,
      const EvaluableFactory& factory,
      size_t index, const TPtrList<IEvaluable>& args) const
    {
      return new FuncEvaluator(factory, self, funcs.GetValue(index), args);
    }
    IEvaluable* call(const EvaluableFactory& factory, base_class& self,
      const olxstr& name, const TPtrList<IEvaluable>& args)
    {
      IMemberFunction* gf = find(name, args.Count());
      if( gf == NULL ) {
        throw TInvalidArgumentException(__OlxSourceInfo,
          olxstr("could not locate specified function: ").quote() << name);
      }
      return gf->run(&self, factory, args);
    }
  };

  template <class wrapper_class, class base_class> struct ClassInfo
    : public IClassInfo
  {
    ClassRegistry<base_class> functions;
    ClassRegistry<wrapper_class> wrap_functions;
    LibraryRegistry globals;
    virtual IStaticFunction* find_static_function(const olxstr& name,
      size_t argc) const
    {
      return globals.find(name, argc);
    }
    virtual IMemberFunction* find_member_function(const olxstr& name,
      size_t argc) const
    {
      IMemberFunction* mf = functions.find(name, argc);
      if( mf != NULL )  return mf;
      return wrap_functions.find(name, argc);
    }
    bool is_empty() const {
      return functions.is_empty() && globals.is_empty() &&
        wrap_functions.is_empty();
    }
  };
};  // end exparse namespace

EndEsdlNamespace()
#endif
