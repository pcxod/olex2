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
    virtual IEvaluable* run(void* self, const EvaluableFactory& factory, const TPtrList<IEvaluable>& params) = 0;
  };
  struct IStaticFunction : public IBasicFunction  {
    virtual IEvaluable* run(const EvaluableFactory& factory, const TPtrList<IEvaluable>& params) = 0;
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
      for( int i=0; i < arg_types.Count(); i++ )  {
        retval << arg_types[i]->name();
        if( (i+1) < arg_types.Count() )
          retval << ", ";
      }
      retval << ')';
      return retval;
    }
    virtual size_t get_argc() const  {  return arg_types.Count();  }
    virtual const type_info& get_arg_type(size_t i) const  {  return *arg_types[i];  }
    virtual const type_info& get_RV_type() const {  return typeid(rv);  }
  };

  /////////////////////////////  FUNCTIONS RETURNING VOID  //////////////////////
  /////////////////////////////  NO ARGUMENT FUNCTION RETURNING VOID  ///////////
  // static function
  class VoidFunction : public AFunction<IStaticFunction,void> {
    void (*func)();
  public:
    VoidFunction(const olxstr& name, void (*f)(void) ) : AFunction<IStaticFunction,void>(name), func(f) {}
    virtual IEvaluable* run(const EvaluableFactory&, const TPtrList<IEvaluable>& params)  {
      this->func();
      return new VoidValue;
    }
  };
  // class member function
  template <class base_class> class VoidMemberFunction : public AFunction<IMemberFunction,void> {
    void (base_class::*func)();
  public:
    VoidMemberFunction(const olxstr& name, void (base_class::*f)() ) : 
      AFunction<IMemberFunction,void>(name), func(f)  {}
    virtual IEvaluable* run(void* self, const EvaluableFactory&, const TPtrList<IEvaluable>& params )  {
      (static_cast<base_class*>(self)->*func)();
      return new VoidValue;
    }
  };
  /////////////////////////////  SINGLE ARGUMENT FUNCTIONS RETURNING VOID  //////////////////////
  // static function
  template <class argt_1> class VoidFunction1 : public AFunction<IStaticFunction,void> {
    void (*func)(argt_1);
  public:
    VoidFunction1(const olxstr& name, void (*f)(argt_1)) : 
      AFunction<IStaticFunction,void>(name), func(f) 
    {
      arg_types.Add(typeid(argt_1));
    }
    virtual IEvaluable* run(const EvaluableFactory&, const TPtrList<IEvaluable>& params)  {
      this->func(params[0]->cast<argt_1>());
      return new VoidValue;
    }
  };
  // class member function
  template <class base_class, typename argt_1>
  class VoidMemberFunction1 : public AFunction<IMemberFunction, void> {
    void (base_class::*func)(argt_1);
  public:
    VoidMemberFunction1(const olxstr& name, void (base_class::*f)(argt_1) ) : 
      AFunction<IMemberFunction,void>(name), func(f) 
    {
      arg_types.Add(typeid(argt_1));
    }
    virtual IEvaluable* run(void* self, const EvaluableFactory&, const TPtrList<IEvaluable>& params )  {
      (static_cast<base_class*>(self)->*func)(params[0]->cast<argt_1>() );
      return new VoidValue;
    }
  };
  /////////////////////////////  TWO ARGUMENT FUNCTIONS RETURNING VOID  //////////////////////
  // static function
  template <class argt_1, class argt_2>
  class VoidFunction2 : public AFunction<IStaticFunction,void> {
    void (*func)(argt_1, argt_2);
  public:
    VoidFunction2(const olxstr& name, void (*f)(argt_1, argt_2) ) : 
      AFunction<IStaticFunction,void>(name), func(f) 
    {
      arg_types.Add(typeid(argt_1));
      arg_types.Add(typeid(argt_2));
    }
    virtual IEvaluable* run(const EvaluableFactory&, const TPtrList<IEvaluable>&params)  {
      this->func(params[0]->cast<argt_1>(), params[1]->cast<argt_2>());
      return new VoidValue;
    }
  };
  // class member function
  template <class base_class, class argt_1, class argt_2>
  class VoidMemberFunction2 : public AFunction<IMemberFunction,void> {
    void (base_class::*func)(argt_1, argt_2);
  public:
    VoidMemberFunction2(const olxstr& name, void (base_class::*f)(argt_1, argt_2) ) : 
      AFunction<IMemberFunction,void>(name), func(f) 
    {
      arg_types.Add(typeid(argt_1));
      arg_types.Add(typeid(argt_2));
    }
    virtual IEvaluable* run(void* self, const EvaluableFactory&, const TPtrList<IEvaluable>&params )  {
      (static_cast<base_class*>(self)->*func)(
        params[0]->cast<argt_1>(),
        params[1]->cast<argt_2>());
      return new VoidValue;
    }
  };
  
  /////////////////////////////  FUNCTIONS RETURNING A VALUE //////////////////////
  /////////////////////////////  NO ARGUMENT FUNCTION RETURNING A VALUE ///////////
  // static function
  template <class rvt> class Function : public AFunction<IStaticFunction,rvt> {
    rvt (*func)();
  public:
    Function(const olxstr& name, rvt (*f)(void) ) : AFunction<IStaticFunction,rvt>(name), func(f) {}
    virtual IEvaluable* run(const EvaluableFactory& factory, const TPtrList<IEvaluable>& params)  {
      return factory.create<rvt>(this->func());
    }
  };
  // class member function
  template <class rvt, class base_class> class MemberFunction : public AFunction<IMemberFunction,rvt> {
    rvt (base_class::*func)();
  public:
    MemberFunction(const olxstr& name, rvt (base_class::*f)() ) : 
        AFunction<IMemberFunction,rvt>(name), func(f) {}
    virtual IEvaluable* run(void* self, const EvaluableFactory& factory, const TPtrList<IEvaluable>& params)  {
      return factory.create<rvt>((static_cast<base_class*>(self)->*func)());
    }
  };
  /////////////////////////////  SINGLE ARGUMENT FUNCTIONS RETURNING A VALUE  //////////////////////
  // static function
  template <class rvt, class argt_1> class Function1 : public AFunction<IStaticFunction,rvt> {
    rvt (*func)(argt_1);
  public:
    Function1(const olxstr& name, rvt (*f)(argt_1) ) : 
      AFunction<IStaticFunction,rvt>(name), func(f) 
    {
      AFunction<IStaticFunction,rvt>::arg_types.Add(typeid(argt_1));
    }
    virtual IEvaluable* run(const EvaluableFactory& factory, const TPtrList<IEvaluable>&params)  {
      return factory.create<argt_1>(this->func(params[0]->cast<argt_1>()));
    }
  };
  //class member function
  template <class rvt, class base_class, class argt_1>
  class MemberFunction1 : public AFunction<IMemberFunction,rvt> {
    rvt (base_class::*func)(argt_1);
  public:
    MemberFunction1(const olxstr& name, rvt (base_class::*f)(argt_1) ) : 
      AFunction<IMemberFunction,rvt>(name), func(f) 
    {
      AFunction<IMemberFunction,rvt>::arg_types.Add(typeid(argt_1));
    }
    virtual IEvaluable* run(void* self, const EvaluableFactory& factory, const TPtrList<IEvaluable>& params)  {
      return factory.create<rvt>((static_cast<base_class*>(self)->*func)(params[0]->cast<argt_1>()));
    }
  };
  /////////////////////////////  TWO ARGUMENT FUNCTIONS RETURNING A VALUE  //////////////////////
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
    virtual IEvaluable* run(const EvaluableFactory& factory, const TPtrList<IEvaluable>&params )  {
      return factory.create<rvt>(this->func(params[0]->cast<argt_1>(), params[1]->cast<argt_2>()));
    }
  };
  //class member function
  template <class rvt, class base_class, class argt_1, class argt_2>
  class MemberFunction2 : public AFunction<IMemberFunction,rvt> {
    rvt (base_class::*func)(argt_1, argt_2);
  public:
    MemberFunction2(const olxstr& name, rvt (base_class::*f)(argt_1, argt_2) ) : 
      AFunction<IMemberFunction,rvt>(name), func(f) 
    {
      AFunction<IMemberFunction,rvt>::arg_types.Add(typeid(argt_1));
      AFunction<IMemberFunction,rvt>::arg_types.Add(typeid(argt_2));
    }
    virtual IEvaluable* run(void* self, const EvaluableFactory& factory, const TPtrList<IEvaluable>& params)  {
      return factory.create( 
        (static_cast<base_class*>(self)->*func)(params[0]->cast<argt_1>(), params[1]->cast<argt_2>()));
    }
  };
  ////////////////////////////////////////////////////////////////////////////////
  class LibraryRegistry  {
    olxdict<olxstr, IStaticFunction*, olxstrComparator<false> > funcs;
    void _add(const olxstr& name, IStaticFunction* f)  {
      if( funcs.IndexOf(name) != -1 )  {
        delete f;
        throw TFunctionFailedException(__OlxSourceInfo, olxstr("Dulicate function instance: ") << name);
      }
      funcs.Add(name, f);
    }
  public:
    ~LibraryRegistry()  {
      for( int i=0; i < funcs.Count(); i++ )
        delete funcs.GetValue(i);
    }
    IStaticFunction* find(const olxstr& name, size_t argc)  const  {
      int ind = funcs.IndexOf( olxstr(name) << '#' << argc);
      return ind == -1 ? NULL : funcs.GetValue(ind);
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
      FuncEvaluator(const EvaluableFactory& fc, IStaticFunction* f, const TPtrList<IEvaluable>& a) : factory(fc), func(f), args(a) {}
      virtual IEvaluable* _evaluate() const {  return func->run(factory, args);  }
    };
    IEvaluable* create(const EvaluableFactory& factory, const olxstr& name, const TPtrList<IEvaluable>& args)  {
      IStaticFunction* gf = find(name, args.Count());
      if( gf == NULL )
        throw TInvalidArgumentException(__OlxSourceInfo, "could not locate specified function");
      return new FuncEvaluator(factory, gf, args);
    }
    IEvaluable* call(const EvaluableFactory& factory, const olxstr& name, const TPtrList<IEvaluable>& args)  {
      IStaticFunction* gf = find(name, args.Count());
      if( gf == NULL )
        throw TInvalidArgumentException(__OlxSourceInfo, "could not locate specified function");
      return gf->run(factory, args);
    }
    static void CompileTest();
  };
  template <class base_class> class ClassRegistry  {
    olxdict<olxstr, IMemberFunction*, olxstrComparator<false> > funcs;
    void _add(const olxstr& name, IMemberFunction* f)  {
      if( funcs.IndexOf(name) != -1 )  {
        delete f;
        throw TFunctionFailedException(__OlxSourceInfo, olxstr("Dulicate function instance: ") << name);
      }
      funcs.Add(name, f);
    }
  public:
    ~ClassRegistry()  {
      for( int i=0; i < funcs.Count(); i++ )
        delete funcs.GetValue(i);
    }
    IMemberFunction* find(const olxstr& name, size_t argc)  const  {
      int ind = funcs.IndexOf( olxstr(name) << '#' << argc);
      return ind == -1 ? NULL : funcs.GetValue(ind);
    }
    void add(const olxstr& name, void (base_class::*f)(void))  {
      _add(name + "#0", new VoidMemberFunction<base_class>(name, f));
    }
    template<class argt_1>
    void add(const olxstr& name, void (base_class::*f)(argt_1) )  {  
      _add(name + "#1", new VoidMemberFunction1<base_class,argt_1>(name, f));
    }
    template<class argt_1, class argt_2>
    void add(const olxstr& name, void (base_class::*f)(argt_1,argt_2) )  {  
      _add(name + "#2", new VoidMemberFunction2<base_class,argt_1,argt_2>(name, f));
    }
    template <class rvt>
    void add(const olxstr& name, rvt (base_class::*f)(void))  {
      _add(name + "#0", new MemberFunction<rvt,base_class>(name, f));
    }
    template<class rvt, class argt_1>
    void add(const olxstr& name, rvt (base_class::*f)(argt_1) )  {  
      _add(name + "#1", new MemberFunction1<rvt,base_class,argt_1>(name, f));
    }
    template<class rvt, class argt_1, class argt_2>
    void add(const olxstr& name, rvt (base_class::*f)(argt_1,argt_2) )  {  
      _add(name + "#2", new MemberFunction2<rvt,base_class,argt_1,argt_2>(name, f));
    }

    struct FuncEvaluator : public IEvaluable  {
      IMemberFunction* func;
      base_class& self;
      const EvaluableFactory& factory;
      TPtrList<IEvaluable> args;
      FuncEvaluator(const EvaluableFactory& fc, base_class& s, IMemberFunction* f, 
        const TPtrList<IEvaluable>& a) : factory(fc), self(s), func(f), args(a) {}
      virtual IEvaluable* _evaluate() const {  return func->run(factory, &self, args);  }
    };
    IEvaluable* create(const EvaluableFactory& factory, base_class& self, const olxstr& name, const TPtrList<IEvaluable>& args)  {
      IStaticFunction* gf = find(name, args.Count());
      if( gf == NULL )
        throw TInvalidArgumentException(__OlxSourceInfo, "could not locate specified function");
      return new FuncEvaluator(factory, self, gf, args);
    }
    IEvaluable* call(const EvaluableFactory& factory, base_class& self, const olxstr& name, const TPtrList<IEvaluable>& args)  {
      IMemberFunction* gf = find(name, args.Count());
      if( gf == NULL )
        throw TInvalidArgumentException(__OlxSourceInfo, "could not locate specified function");
      return gf->run(&self, factory, args);
    }
    static void CompileTest();
  };

};  // end exparse namespace
EndEsdlNamespace()

#endif
