#ifndef IpBaseH
#define IpBaseH

//#ifndef __GNUC__ // it cannot compile this ..

#include "ebase.h"
#include "typelist.h"
#include "egc.h"
//#include "estring.h"
#include "estlist.h"
#include <typeinfo>
/*
  A base for interpreter
  (c) O. Dolomanov, 2007
*/
BeginEsdlNamespace()

template <class PT>
  class TPTWrapper : public AReferencible  {
    PT Value;
  public:
    TPTWrapper( const PT& val )  {
      Value = val;
    }
    TPTWrapper( const TPTWrapper& val ) {
      Value = val.GetValue();
    }
    TPTWrapper( TPTWrapper* val )  {
      Value = val->GetValue();
      if( val->GetRefCount() == 0 )
        delete val;
    }

    PT& Val()                    {  return Value;  }
    PT& operator ()()            {  return Value;  }
    const PT& GetValue()  const  {  return Value;  }
    void SetValue(const PT& v)   {  Value = v;  }

    TIString ToString()  const   {
      static olxstr str;
      olxstr val(Value);
      return (str = val);
    }

    virtual IEObject* Replicate() const  {  return new TPTWrapper(*this);  }
  };

template <class PT>
  class TPTWrapper<const PT&> : public AReferencible  {
    PT const* Value;
  public:
    TPTWrapper( const PT& val ) {
      Value = &val;
    }
    TPTWrapper( const TPTWrapper& val )  {
      Value = val.Value;
    }
    TPTWrapper( TPTWrapper* val )  {
      Value = val->GetValue();
      if( val->GetRefCount() == 0 )
        delete val;
    }

    const PT& operator ()()      {  return *Value;  }
    const PT& GetValue()  const  {  return *Value;  }
    void SetValue(const PT& v)   {  Value = v;  }

    virtual IEObject* Replicate() const  {  return new TPTWrapper(*this);  }
  };

template <>
  class TPTWrapper<void> : public AReferencible  {
  public:
    TPTWrapper(void) {  }
    TPTWrapper( const TPTWrapper& val )  {  }
    TPTWrapper( TPTWrapper* val )  {  }
    void Val()                    {   }
    void operator ()()            {   }
    void GetValue()  const  {    }
    void SetValue(void)   {    }

    TIString ToString()  const   {
      //TEGC::
      static olxstr str = "void";
      return str;
    }

    virtual IEObject* Replicate() const  {  return new TPTWrapper(*this);  }

  };

/* function interface */
class IFunc  {
public:
  virtual ~IFunc()  {  }
  virtual const olxstr& GetName() const = 0;
  virtual olxstr GetSignature() const = 0;
  virtual olxstr GetRunTimeSignature() const = 0;
  // note that a new instance is returned - must be deleted if called explicetely!
  virtual IEObject* Run(TPtrList<IEObject const>& params) = 0;
  virtual int GetArgc()  const  = 0;
  virtual void DoDebug() = 0;
  virtual const std::type_info& GetArgType(int i) const = 0;
  virtual const std::type_info& GetRetValType() const = 0;
};

/* abstract function */
template <class RV>
class AFunc : public IFunc {
protected:
  TPtrList<std::type_info const> ArgTypes;
  olxstr Name, RTSignature;
  bool Debug;
  void CollectDebugInfo(const TPtrList<IEObject const>& params)  {
  RTSignature = GetName();
    RTSignature << '(';
    for( int i=0; i < params.Count(); i++ )  {
      if( params[i] != NULL )  {
        RTSignature << typeid( *params[i] ).name();
        RTSignature << '[' << params[i]->ToString() << ']';
      }
      else
        RTSignature << "NULL";
      if( (i+1) < params.Count() )
        RTSignature << ", ";
    }
    RTSignature << ')';
  }
public:
  AFunc(const olxstr& name )  {
    Name = name;
    Debug = false;
  }
  virtual void DoDebug()  {  Debug = true;  }
  virtual const olxstr& GetName() const {  return Name;  }
  virtual olxstr GetSignature() const {
    olxstr rv = GetRetValType().name();
    rv << ' ' << GetName() << '(';
    for( int i=0; i < ArgTypes.Count(); i++ )  {
      rv << ArgTypes[i]->name();
      if( (i+1) < ArgTypes.Count() )
        rv << ", ";
    }
    rv << ')';
    return rv;
  }
  virtual olxstr GetRunTimeSignature() const {  return RTSignature;  }

  virtual int GetArgc() const  {  return ArgTypes.Count();  }
  virtual const std::type_info& GetArgType(int i) const  {  return *ArgTypes[i];  }
  virtual const std::type_info& GetRetValType() const {  return typeid(RV);  }
  void ValidateParameters(TPtrList<IEObject const>& params)  {
    if( Debug )
      CollectDebugInfo(params);
    try  {
      if( params.Count() != ArgTypes.Count() ) 
        throw 10;
      for( int i=0; i < ArgTypes.Count(); i++ )  {
        if( typeid(*params[i]) != *ArgTypes[i] ) 
          throw 20;
      }
    }
    catch( int excN )  {
      if( !Debug )
        CollectDebugInfo(params);
      olxstr reason;
      if( excN == 10 )  reason = "Invalid number of arguments ";
      if( excN == 20 )  reason = "Argument type mismatch ";
      reason << "in call to " << GetSignature() << " with following arguments: " << RTSignature;

      throw TFunctionFailedException( __OlxSourceInfo, reason);
    }
  }
};

/////////////////////////////  FUNCTIONS RETURNING VOID  //////////////////////
/////////////////////////////  NO ARGUMENT FUNCTION RETURNING VOID  ///////////
// static function
class VFunc : public AFunc<void> {
  void (*func)();
public:
  VFunc(const olxstr& name, void (*f)(void) ) : AFunc<void>(name) {
    this->func = f;
  }
  virtual IEObject* Run(TPtrList<IEObject const>& params )  {
    ValidateParameters( params );
    this->func();
    return new TPTWrapper<void>();
  }
};
// class member function
template <class BaseClass>
  class CVFunc : public AFunc<void> {
    void (BaseClass::*func)();
    BaseClass* Instance;
  public:
    CVFunc(const olxstr& name, BaseClass* instance, void (BaseClass::*f)() ) : AFunc<void>(name) {
      this->func = f;
      Instance = instance;
    }
    virtual IEObject* Run(TPtrList<IEObject const>& params )  {
      ValidateParameters( params );
      (Instance->*func)();
      return NULL;
    }
  };
/////////////////////////////  SINGLE ARGUMENT FUNCTIONS RETURNING VOID  //////////////////////
// static function
template <class A1>
  class VFunc1 : public AFunc<void> {
    void (*func)(A1);
  public:
    VFunc1(const olxstr& name, void (*f)(A1) ) : AFunc<void>(name) {
      this->func = f;
      ArgTypes.Add( &typeid(TPTWrapper<A1>) );
    }
    virtual IEObject* Run(TPtrList<IEObject const>& params )  {
      ValidateParameters( params );
      this->func(  ((TPTWrapper<A1>*)params[0])->GetValue() );
      return NULL;
    }
  };
// class member function
template <class BaseClass, class A1>
  class CVFunc1 : public AFunc<void> {
    void (BaseClass::*func)(A1);
    BaseClass* Instance;
  public:
    CVFunc1(const olxstr& name, BaseClass* instance, void (BaseClass::*f)(A1) ) : AFunc<void>(name) {
      this->func = f;
      Instance = instance;
      ArgTypes.Add( &typeid(TPTWrapper<A1>) );
    }
    virtual IEObject* Run(TPtrList<IEObject const>& params )  {
      ValidateParameters( params );
      (Instance->*func)(  ((TPTWrapper<A1>*)params[0])->GetValue() );
      return NULL;
    }
  };
/////////////////////////////  TWO ARGUMENT FUNCTIONS RETURNING VOID  //////////////////////
// static function
template <class A1, class A2>
  class VFunc2 : public AFunc<void> {
    void (*func)(A1, A2);
  public:
    VFunc2(const olxstr& name, void (*f)(A1, A2) ) : AFunc<void>(name) {
      this->func = f;
      ArgTypes.Add( &typeid(TPTWrapper<A1>) );
      ArgTypes.Add( &typeid(TPTWrapper<A2>) );
    }
    virtual IEObject* Run(TPtrList<IEObject const>&params )  {
      ValidateParameters( params );
      this->func(  ((TPTWrapper<A1>*)params[0])->GetValue(),
                   ((TPTWrapper<A2>*)params[1])->GetValue() );
      return NULL;
    }
  };
// class member function
template <class BaseClass, class A1, class A2>
  class CVFunc2 : public AFunc<void> {
    void (BaseClass::*func)(A1, A2);
    BaseClass* Instance;
  public:
    CVFunc2(const olxstr& name, BaseClass* instance, void (BaseClass::*f)(A1, A2) ) : AFunc<void>(name) {
      this->func = f;
      Instance = instance;
      ArgTypes.Add( &typeid(TPTWrapper<A1>) );
      ArgTypes.Add( &typeid(TPTWrapper<A2>) );
    }
    virtual IEObject* Run(TPtrList<IEObject const>&params )  {
      ValidateParameters( params );
      (Instance->*func)(  ((TPTWrapper<A1>*)params[0])->GetValue(),
                          ((TPTWrapper<A2>*)params[1])->GetValue() );
      return NULL;
    }
  };
/////////////////////////////  THREE ARGUMENT FUNCTIONS RETURNING VOID  //////////////////////
// static function
template <class A1, class A2, class A3>
  class VFunc3 : public AFunc<void> {
    void (*func)(A1, A2, A3);
  public:
    VFunc3(const olxstr& name, void (*f)(A1, A2, A3) ) : AFunc<void>(name) {
      this->func = f;
      ArgTypes.Add( &typeid(TPTWrapper<A1>) );
      ArgTypes.Add( &typeid(TPTWrapper<A2>) );
      ArgTypes.Add( &typeid(TPTWrapper<A3>) );
    }
    virtual IEObject* Run(TPtrList<IEObject const>&params )  {
      ValidateParameters( params );
      this->func(  ((TPTWrapper<A1>*)params[0])->GetValue(),
                   ((TPTWrapper<A2>*)params[1])->GetValue(),
                   ((TPTWrapper<A3>*)params[2])->GetValue() );
      return NULL;
    }
  };
// class member function
template <class BaseClass, class A1, class A2, class A3>
  class CVFunc3 : public AFunc<void> {
    void (BaseClass::*func)(A1, A2, A3);
    BaseClass* Instance;
  public:
    CVFunc3(const olxstr& name, BaseClass* instance, 
        void (BaseClass::*f)(A1, A2, A3) ) : AFunc<void>(name) {
      this->func = f;
      Instance = instance;
      ArgTypes.Add( &typeid(TPTWrapper<A1>) );
      ArgTypes.Add( &typeid(TPTWrapper<A2>) );
      ArgTypes.Add( &typeid(TPTWrapper<A3>) );
    }
    virtual IEObject* Run(TPtrList<IEObject const>&params )  {
      ValidateParameters( params );
      (Instance->*func)(  ((TPTWrapper<A1>*)params[0])->GetValue(),
                          ((TPTWrapper<A2>*)params[1])->GetValue(),
                          ((TPTWrapper<A3>*)params[2])->GetValue() );
      return NULL;
    }
  };
/////////////////////////////  FOUR ARGUMENT FUNCTIONS RETURNING VOID  //////////////////////
// static function
template <class A1, class A2, class A3, class A4>
  class VFunc4 : public AFunc<void> {
    void (*func)(A1, A2, A3, A4);
  public:
    VFunc4(const olxstr& name, void (*f)(A1, A2, A3, A4) ) : AFunc<void>(name) {
      this->func = f;
      ArgTypes.Add( &typeid(TPTWrapper<A1>) );
      ArgTypes.Add( &typeid(TPTWrapper<A2>) );
      ArgTypes.Add( &typeid(TPTWrapper<A3>) );
      ArgTypes.Add( &typeid(TPTWrapper<A4>) );
    }
    virtual IEObject* Run(TPtrList<IEObject const>&params )  {
      ValidateParameters( params );
      this->func(  ((TPTWrapper<A1>*)params[0])->GetValue(),
                   ((TPTWrapper<A2>*)params[1])->GetValue(),
                   ((TPTWrapper<A3>*)params[2])->GetValue(),
                   ((TPTWrapper<A4>*)params[3])->GetValue() );
      return NULL;
    }
  };
// class member function
template <class BaseClass, class A1, class A2, class A3, class A4>
  class CVFunc4 : public AFunc<void> {
    void (BaseClass::*func)(A1, A2, A3, A4);
    BaseClass* Instance;
  public:
    CVFunc4(const olxstr& name, BaseClass* instance,
        void (BaseClass::*f)(A1, A2, A3, A4) ) : AFunc<void>(name) {
      this->func = f;
      Instance = instance;
      ArgTypes.Add( &typeid(TPTWrapper<A1>) );
      ArgTypes.Add( &typeid(TPTWrapper<A2>) );
      ArgTypes.Add( &typeid(TPTWrapper<A3>) );
      ArgTypes.Add( &typeid(TPTWrapper<A4>) );
    }
    virtual IEObject* Run(TPtrList<IEObject const>&params )  {
      ValidateParameters( params );
      (Instance->*func)(  ((TPTWrapper<A1>*)params[0])->GetValue(),
                          ((TPTWrapper<A2>*)params[1])->GetValue(),
                          ((TPTWrapper<A3>*)params[2])->GetValue(),
                          ((TPTWrapper<A4>*)params[3])->GetValue() );
      return NULL;
    }
  };

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

/////////////////////////////  FUNCTIONS RETURNING A VALUE //////////////////////
/////////////////////////////  NO ARGUMENT FUNCTION RETURNING A VALUE ///////////
// static function
template <class RV>
  class PFunc : public AFunc<RV> {
    RV (*func)();
  public:
    PFunc(const olxstr& name, RV (*f)(void) ) : AFunc<RV>(name) {
      this->func = f;
    }
    virtual IEObject* Run(TPtrList<IEObject const>&params )  {
      TPTWrapper<RV>* rv;
      rv = new TPTWrapper<RV>( this->func() );
      return rv;
    }
  };
// class member function
template <class RV, class BaseClass>
  class CPFunc : public AFunc<RV> {
    RV (BaseClass::*func)();
    BaseClass* Instance;
  public:
    CPFunc(const olxstr& name, BaseClass* instance, RV (BaseClass::*f)() ) : AFunc<RV>(name) {
      this->func = f;
      Instance = instance;
    }
    virtual IEObject* Run(TPtrList<IEObject const>&params )  {
      AFunc<RV>::ValidateParameters( params );
      TPTWrapper<RV>* rv;
      rv = new TPTWrapper<RV>( (Instance->*func)() );
      return rv;
    }
}  ;
/////////////////////////////  SINGLE ARGUMENT FUNCTIONS RETURNING A VALUE  //////////////////////
// static function
template <class RV, class A1>
  class PFunc1 : public AFunc<RV> {
    RV (*func)(A1);
  public:
    PFunc1(const olxstr& name, RV (*f)(A1) ) : AFunc<RV>(name) {
      this->func = f;
      AFunc<RV>::ArgTypes.Add( &typeid(TPTWrapper<A1>) );
    }
    virtual IEObject* Run(TPtrList<IEObject const>&params )  {
      AFunc<RV>::ValidateParameters( params );
      TPTWrapper<RV>* rv;
      rv = new TPTWrapper<RV>( this->func(  ((TPTWrapper<A1>*)params[0])->GetValue() ) );
      return rv;
    }
  };
//class member function
template <class RV, class BaseClass, class A1>
  class CPFunc1 : public AFunc<RV> {
    RV (BaseClass::*func)(A1);
    BaseClass* Instance;
  public:
    CPFunc1(const olxstr& name, BaseClass* instance, RV (BaseClass::*f)(A1) ) : AFunc<RV>(name) {
      this->func = f;
      Instance = instance;
      AFunc<RV>::ArgTypes.Add( &typeid(TPTWrapper<A1>) );
    }
    virtual IEObject* Run(TPtrList<IEObject const>&params )  {
      AFunc<RV>::ValidateParameters( params );
      TPTWrapper<RV>* rv;
      rv = new TPTWrapper<RV>( (Instance->*func)(  ((TPTWrapper<A1>*)params[0])->GetValue() ) );
      return rv;
    }
  };
/////////////////////////////  TWO ARGUMENT FUNCTIONS RETURNING A VALUE  //////////////////////
// static function
template <class RV, class A1, class A2>
  class PFunc2 : public AFunc<RV> {
    RV (*func)(A1, A2 );
  public:
    PFunc2(const olxstr& name,  RV (*f)(A1,A2) ) : AFunc<RV>(name) {
      this->func = f;
      AFunc<RV>::ArgTypes.Add( &typeid(TPTWrapper<A1>) );
      AFunc<RV>::ArgTypes.Add( &typeid(TPTWrapper<A2>) );
    }
    virtual IEObject* Run(TPtrList<IEObject const>&params )  {
      AFunc<RV>::ValidateParameters( params );
      TPTWrapper<RV>* rv;
        rv = new TPTWrapper<RV>(
          this->func(  ((TPTWrapper<A1>*)params[0])->GetValue(),
                     ((TPTWrapper<A2>*)params[1])->GetValue() ) );
    return rv;
  }
};
//class member function
template <class RV, class BaseClass, class A1, class A2>
  class CPFunc2 : public AFunc<RV> {
    RV (BaseClass::*func)(A1, A2);
    BaseClass* Instance;
  public:
    CPFunc2(const olxstr& name, BaseClass* instance, 
        RV (BaseClass::*f)(A1, A2) ) : AFunc<RV>(name) {
      this->func = f;
      Instance = instance;
      AFunc<RV>::ArgTypes.Add( &typeid(TPTWrapper<A1>) );
      AFunc<RV>::ArgTypes.Add( &typeid(TPTWrapper<A2>) );
    }
    virtual IEObject* Run(TPtrList<IEObject const>&params )  {
      AFunc<RV>::ValidateParameters( params );
      TPTWrapper<RV>* rv;
      rv = new TPTWrapper<RV>( (Instance->*func)(  ((TPTWrapper<A1>*)params[0])->GetValue(),
                                                ((TPTWrapper<A2>*)params[1])->GetValue() ) );
      return rv;
    }
  };
/////////////////////////////  THREE ARGUMENT FUNCTIONS RETURNING A VALUE  //////////////////////
// static function
template <class RV, class A1, class A2, class A3>
  class PFunc3 : public AFunc<RV> {
    RV (*func)(A1, A2, A3 );
  public:
    PFunc3(const olxstr& name,  RV (*f)(A1,A2,A3) ) : AFunc<RV>(name) {
      this->func = f;
      AFunc<RV>::ArgTypes.Add( &typeid(TPTWrapper<A1>) );
      AFunc<RV>::ArgTypes.Add( &typeid(TPTWrapper<A2>) );
      AFunc<RV>::ArgTypes.Add( &typeid(TPTWrapper<A3>) );
    }
    virtual IEObject* Run(TPtrList<IEObject const>&params )  {
      AFunc<RV>::ValidateParameters( params );
      TPTWrapper<RV>* rv;
        rv = new TPTWrapper<RV>(
          this->func(  ((TPTWrapper<A1>*)params[0])->GetValue(),
                       ((TPTWrapper<A2>*)params[1])->GetValue(),
                       ((TPTWrapper<A3>*)params[2])->GetValue() ) );
    return rv;
  }
};
//class member function
template <class RV, class BaseClass, class A1, class A2, class A3>
  class CPFunc3 : public AFunc<RV> {
    RV (BaseClass::*func)(A1, A2, A3);
    BaseClass* Instance;
  public:
    CPFunc3(const olxstr& name, BaseClass* instance,
        RV (BaseClass::*f)(A1, A2, A3) ) : AFunc<RV>(name) {
      this->func = f;
      Instance = instance;
      AFunc<RV>::ArgTypes.Add( &typeid(TPTWrapper<A1>) );
      AFunc<RV>::ArgTypes.Add( &typeid(TPTWrapper<A2>) );
      AFunc<RV>::ArgTypes.Add( &typeid(TPTWrapper<A3>) );
    }
    virtual IEObject* Run(TPtrList<IEObject const>&params )  {
      AFunc<RV>::ValidateParameters( params );
      TPTWrapper<RV>* rv;
      rv = new TPTWrapper<RV>( (Instance->*func)(  ((TPTWrapper<A1>*)params[0])->GetValue(),
                                                   ((TPTWrapper<A2>*)params[1])->GetValue(),
                                                   ((TPTWrapper<A3>*)params[2])->GetValue() ) );
      return rv;
    }
  };
/////////////////////////////  FOUR ARGUMENT FUNCTIONS RETURNING A VALUE  //////////////////////
// static function
template <class RV, class A1, class A2, class A3, class A4>
  class PFunc4 : public AFunc<RV> {
    RV (*func)(A1, A2, A3, A4);
  public:
    PFunc4(const olxstr& name,  RV (*f)(A1,A2,A3,A4) ) : AFunc<RV>(name) {
      this->func = f;
      AFunc<RV>::ArgTypes.Add( &typeid(TPTWrapper<A1>) );
      AFunc<RV>::ArgTypes.Add( &typeid(TPTWrapper<A2>) );
      AFunc<RV>::ArgTypes.Add( &typeid(TPTWrapper<A3>) );
      AFunc<RV>::ArgTypes.Add( &typeid(TPTWrapper<A4>) );
    }
    virtual IEObject* Run(TPtrList<IEObject const>&params )  {
      AFunc<RV>::ValidateParameters( params );
      TPTWrapper<RV>* rv;
        rv = new TPTWrapper<RV>(
          this->func(  ((TPTWrapper<A1>*)params[0])->GetValue(),
                       ((TPTWrapper<A2>*)params[1])->GetValue(),
                       ((TPTWrapper<A3>*)params[2])->GetValue(),
                       ((TPTWrapper<A4>*)params[3])->GetValue() ) );
    return rv;
  }
};
//class member function
template <class RV, class BaseClass, class A1, class A2, class A3, class A4>
  class CPFunc4 : public AFunc<RV> {
    RV (BaseClass::*func)(A1, A2, A3, A4);
    BaseClass* Instance;
  public:
    CPFunc4(const olxstr& name, BaseClass* instance,
        RV (BaseClass::*f)(A1, A2, A3, A4) ) : AFunc<RV>(name) {
      this->func = f;
      Instance = instance;
      AFunc<RV>::ArgTypes.Add( &typeid(TPTWrapper<A1>) );
      AFunc<RV>::ArgTypes.Add( &typeid(TPTWrapper<A2>) );
      AFunc<RV>::ArgTypes.Add( &typeid(TPTWrapper<A3>) );
      AFunc<RV>::ArgTypes.Add( &typeid(TPTWrapper<A4>) );
    }
    virtual IEObject* Run(TPtrList<IEObject const>&params )  {
      AFunc<RV>::ValidateParameters( params );
      TPTWrapper<RV>* rv;
      rv = new TPTWrapper<RV>( (Instance->*func)(  ((TPTWrapper<A1>*)params[0])->GetValue(),
                                                   ((TPTWrapper<A2>*)params[1])->GetValue(),
                                                   ((TPTWrapper<A3>*)params[2])->GetValue(),
                                                   ((TPTWrapper<A4>*)params[3])->GetValue() ) );
      return rv;
    }
  };
////////////////////////////////////////////////////////////////////////////////
class TFuncRegistry  {
  TCSTypeList<olxstr, IFunc*> funcs;
public:
  ~TFuncRegistry()  {
    for( int i=0; i < funcs.Count(); i++ )
      delete funcs.GetObject(i);
  }
  template <class T>
  IFunc* FindFunction(const T& name)  const  {
    int ind = funcs.IndexOfComparable(name);
    return ind == -1 ? NULL : funcs.GetObject(ind);
  }
  //void (*F)(void)
  void Reg(const olxstr& name, void (*F)(void) )  {
    VFunc* gf = new VFunc( name, F);
    funcs.Add( name, gf);
  }
  // void (BC::*F)(), non standsrd - MSVC gets confused here with RV (BC::*F)()...
  template<class BC, void (BC::*F)()>
    void Reg(const olxstr& name, BC* instance)  {  
      CVFunc<BC>* gf = new CVFunc<BC>( name, instance, F);
      funcs.Add( name, gf);
    }
  //void (*F)(A1)
  template<class A1 > 
    void Reg(const olxstr& name, void (*F)(A1) )  {  
      VFunc1<A1>* gf = new VFunc1<A1>( name, F);
      funcs.Add( name, gf);
    }
  // void (BC::*F)(A1)
  template<class BC, class A1> 
    void Reg(const olxstr& name, BC* instance, void (BC::*F)(A1) )  {  
      CVFunc1<BC,A1>* gf = new CVFunc1<BC,A1>( name, instance, F);
      funcs.Add( name, gf);
    }
  //void (*F)(A1,A2)
  template<class A1, class A2 > 
    void Reg(const olxstr& name, void (*F)(A1,A2) )  {  
      VFunc2<A1,A2>* gf = new VFunc2<A1,A2>( name, F);
      funcs.Add( name, gf);
    }
  // void (BC::*F)(A1,A2)
  template<class BC, class A1, class A2> 
    void Reg(const olxstr& name, BC* instance, void (BC::*F)(A1,A2) )  {  
      CVFunc2<BC,A1,A2>* gf = new CVFunc2<BC,A1,A2>( name, instance, F);
      funcs.Add( name, gf);
    }
  //void (*F)(A1,A2,A3)
  template<class A1, class A2, class A3>
    void Reg(const olxstr& name, void (*F)(A1,A2,A3) )  {
      VFunc3<A1,A2,A3>* gf = new VFunc3<A1,A2,A3>( name, F);
      funcs.Add( name, gf);
    }
  // void (BC::*F)(A1,A2,A3)
  template<class BC, class A1, class A2, class A3>
    void Reg(const olxstr& name, BC* instance, void (BC::*F)(A1,A2,A3) )  {
      CVFunc3<BC,A1,A2,A3>* gf = new CVFunc3<BC,A1,A2,A3>( name, instance, F);
      funcs.Add( name, gf);
    }
  //void (*F)(A1,A2,A3,A4)
  template<class A1, class A2, class A3, class A4>
    void Reg(const olxstr& name, void (*F)(A1,A2,A3,A4) )  {
      VFunc4<A1,A2,A3,A4>* gf = new VFunc4<A1,A2,A3,A4>( name, F);
      funcs.Add( name, gf);
    }
  // void (BC::*F)(A1,A2,A3,A4)
  template<class BC, class A1, class A2, class A3, class A4>
    void Reg(const olxstr& name, BC* instance, void (BC::*F)(A1,A2,A3,A4) )  {
      CVFunc4<BC,A1,A2,A3,A4>* gf = new CVFunc4<BC,A1,A2,A3,A4>( name, instance, F);
      funcs.Add( name, gf);
    }
  // A1 (*F)(void)
  template<class A1 > 
    void Reg(const olxstr& name, A1 (*F)(void) )  {  
      PFunc<A1>* gf = new PFunc<A1>( name, F);
      funcs.Add( name, gf);
    }
  // A1 (BC::*F)(void)
  template<class BC, class A1 > 
    void Reg(const olxstr& name, BC* instance, A1 (BC::*F)(void) )  {  
      CPFunc<BC, A1>* gf = new CPFunc<BC,A1>( name, instance, F);
      funcs.Add( name, gf);
    }
  // RV (*F)(A1)
  template< class RV, class A1 > 
    void Reg(const olxstr& name, RV (*F)(A1) )  {  
      PFunc1<RV,A1>* gf = new PFunc1<RV,A1>( name, F);
      funcs.Add( name, gf);
    }
  // RV (BC::*F)(A1)
  template< class RV, class BC, class A1 > 
    void Reg(const olxstr& name, BC* instance, RV (BC::*F)(A1) )  {  
      CPFunc1<RV,BC,A1>* gf = new CPFunc1<RV,BC,A1>( name, instance, F);
      funcs.Add( name, gf);
    }
  // RV (*F)(A1, A2)
  template< class RV, class A1, class A2 > 
    void Reg(const olxstr& name, RV (*F)(A1, A2) )  {  
      PFunc2<RV, A1,A2>* gf = new PFunc2<RV,A1,A2>( name, F );
      funcs.Add( name, gf);
    }
  // RV (BC::*F)(A1,A2)
  template< class RV, class BC, class A1, class A2 > 
    void Reg(const olxstr& name, BC* instance, RV (BC::*F)(A1, A2) )  {  
      CPFunc2<RV,BC,A1,A2>* gf = new CPFunc2<RV,BC,A1,A2>( name, instance, F);
      funcs.Add( name, gf);
    }
  // RV (*F)(A1, A2, A3)
  template< class RV, class A1, class A2, class A3>
    void Reg(const olxstr& name, RV (*F)(A1, A2, A3) )  {
      PFunc3<RV, A1,A2,A3>* gf = new PFunc3<RV,A1,A2,A3>( name, F );
      funcs.Add( name, gf);
    }
  // RV (BC::*F)(A1,A2,A3)
  template< class RV, class BC, class A1, class A2, class A3>
    void Reg(const olxstr& name, BC* instance, RV (BC::*F)(A1, A2,A3) )  {
      CPFunc3<RV,BC,A1,A2,A3>* gf = new CPFunc3<RV,BC,A1,A2,A3>( name, instance, F);
      funcs.Add( name, gf);
    }
  // RV (*F)(A1, A2, A3,A4)
  template< class RV, class A1, class A2, class A3, class A4>
    void Reg(const olxstr& name, RV (*F)(A1, A2, A3, A4) )  {
      PFunc4<RV, A1,A2,A3,A4>* gf = new PFunc4<RV,A1,A2,A3,A4>( name, F );
      funcs.Add( name, gf);
    }
  // RV (BC::*F)(A1,A2,A3,A4)
  template< class RV, class BC, class A1, class A2, class A3, class A4>
    void Reg(const olxstr& name, BC* instance, RV (BC::*F)(A1, A2,A3,A4) )  {
      CPFunc4<RV,BC,A1,A2,A3,A4>* gf = new CPFunc4<RV,BC,A1,A2,A3,A4>( name, instance, F);
      funcs.Add( name, gf);
    }


  template <class T, class RV>
    RV CallFunction(const T& name)  {
      TPtrList<IEObject const> args;
      TPTWrapper<RV> retVal( (TPTWrapper<RV>*)FindFunction(name)->Run(args));
      return retVal.GetValue();
    }
  template <class T, class RV, class A1>
    RV CallFunction(const T& name, A1 a1)  {
      TPtrList<IEObject const> args;
      TPTWrapper<A1> warg1(a1);
      args.Add( &warg1 );
      TPTWrapper<RV> retVal( (TPTWrapper<RV>*)FindFunction(name)->Run(args));
      return retVal.GetValue();
    }
  template <class T, class RV, class A1, class A2>
    RV CallFunction(const T& name, A1 a1, A2 a2)  {
      TPtrList<IEObject const> args;
      TPTWrapper<A1> warg1(a1);
      args.Add( &warg1 );
      TPTWrapper<A2> warg2(a2);
      args.Add( &warg2 );
      TPTWrapper<RV> retVal( (TPTWrapper<RV>*)FindFunction(name)->Run(args) );
      return retVal.GetValue();
    }
  template <class T, class RV, class A1, class A2, class A3>
    RV CallFunction(const T& name, A1 a1, A2 a2, A3 a3)  {
      TPtrList<IEObject const> args;
      TPTWrapper<A1> warg1(a1);
      args.Add( &warg1 );
      TPTWrapper<A2> warg2(a2);
      args.Add( &warg2 );
      TPTWrapper<A3> warg3(a3);
      args.Add( &warg3 );
      TPTWrapper<RV> retVal( (TPTWrapper<RV>*)FindFunction(name)->Run(args) );
      return retVal.GetValue();
    }
  template <class T, class RV, class A1, class A2, class A3, class A4>
    RV CallFunction(const T& name, A1 a1, A2 a2, A3 a3, A4 a4)  {
      TPtrList<IEObject const> args;
      TPTWrapper<A1> warg1(a1);
      args.Add( &warg1 );
      TPTWrapper<A2> warg2(a2);
      args.Add( &warg2 );
      TPTWrapper<A3> warg3(a3);
      args.Add( &warg3 );
      TPTWrapper<A4> warg4(a4);
      args.Add( &warg4 );
      TPTWrapper<RV> retVal( (TPTWrapper<RV>*)FindFunction(name)->Run(args) );
      return retVal.GetValue();
    }
  static void CompileTest();
};

EndEsdlNamespace()
//#endif // GNUC
#endif
