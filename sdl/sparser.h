/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sld_sparser_H
#define __olx_sld_sparser_H
#include "tptrlist.h"
#include "estlist.h"
#include "estrlist.h"
// arithmetic operators/functions
const short
  aofAdd  = 1,
  aofSub  = 2,
  aofMul  = 3,
  aofDiv  = 4,
  aofExt  = 5,
  aofSin  = 6,
  aofCos  = 7,
  aofTan  = 8,
  aofAsin = 9,
  aofAcos = 10,
  aofAtan = 11,
  aofAbs  = 12,
  aofRemainder = 13;

template <class IC>
  class TObjectFactory  {
  public:
    virtual ~TObjectFactory() {}
    virtual IC *NewInstance(TPtrList<IEObject>* Arguments) = 0;
  };

class TOperatorSignature  {
public:
  olxstr StringValue;
  short ShortValue;
  TOperatorSignature(const short shortVal, const olxstr &strVal);
  static TOperatorSignature DefinedFunctions[];
  static short DefinedFunctionCount;
};

class IDataProvider {
public:
  virtual ~IDataProvider() {}
  virtual IDataProvider* cast(const std::type_info &ti) {
    return this;
  }
};

class IEvaluable : public IEObject {
public:
  virtual ~IEvaluable() {}
  virtual bool Evaluate() = 0;
};


// an abstract class for evaluation simple expressions
class IEvaluator: public IEObject  {
public:
  virtual ~IEvaluator() {}

  class TUnsupportedOperator: public TBasicException  {
  public:
    TUnsupportedOperator(const olxstr& location):
        TBasicException(location, EmptyString()) {}
    virtual IEObject* Replicate()  const {  return new TUnsupportedOperator(*this);  }
  };
  virtual bool operator == (const IEvaluator &) const {  throw TUnsupportedOperator(__OlxSourceInfo);  }
  virtual bool operator != (const IEvaluator &) const {  throw TUnsupportedOperator(__OlxSourceInfo);  }
  virtual bool operator > (const IEvaluator &)  const {  throw TUnsupportedOperator(__OlxSourceInfo);  }
  virtual bool operator >= (const IEvaluator &) const {  throw TUnsupportedOperator(__OlxSourceInfo);  }
  virtual bool operator < (const IEvaluator &)  const {  throw TUnsupportedOperator(__OlxSourceInfo);  }
  virtual bool operator <= (const IEvaluator &) const {  throw TUnsupportedOperator(__OlxSourceInfo);  }
//  virtual bool operator <= (const IEvaluator &val) const {  throw TUnsupportedOperator(*(IEObject*)this, "Unsupported operator");  }

  virtual IEvaluator *NewInstance(IDataProvider *) = 0; // {  return NULL;  }


  class TCastException: public TBasicException  {
  public:
    TCastException(const olxstr& location ):
        TBasicException(location, EmptyString()) { ;  }
    virtual IEObject* Replicate()  const {  return new TCastException(*this);  }
  };
  virtual short EvaluateShort()         const {  throw TCastException(__OlxSourceInfo);  }
  virtual int EvaluateInt()             const {  throw TCastException(__OlxSourceInfo);  }
  virtual long EvaluateLong()           const {  throw TCastException(__OlxSourceInfo);  }
  virtual float EvaluateFloat()         const {  throw TCastException(__OlxSourceInfo);  }
  virtual double EvaluateDouble()       const {  throw TCastException(__OlxSourceInfo);  }
  virtual unsigned short EvaluateUshort()const  {  throw TCastException(__OlxSourceInfo);  }
  virtual unsigned int EvaluateUint()   const {  throw TCastException(__OlxSourceInfo);  }
  virtual unsigned long EvaluateUlong() const {  throw TCastException(__OlxSourceInfo);  }
  virtual bool EvaluateBool()           const {  throw TCastException(__OlxSourceInfo);  }
  virtual const olxstr& EvaluateString()    const {  throw TCastException(__OlxSourceInfo);  }
};
/*
class IArithmetic
{
public:
  virtual ~IArithmetic() {  ; }
  virtual IEvaluator* operator + (const IEvaluator &v) const = 0;
  virtual IEvaluator* operator - (const IEvaluator &v) const = 0;
  virtual IEvaluator* operator / (const IEvaluator &v) const = 0;
  virtual IEvaluator* operator * (const IEvaluator &v) const = 0;
  virtual IEvaluable* Ext( const Evaluator &v)  const = 0;
};

  typedef IPrimitiveWrapper<float>   IFloatWrapper;
  typedef IPrimitiveWrapper<double>  IDoubleWrapper;
  typedef IPrimitiveWrapper<short>   IShortWrapper;
  typedef IPrimitiveWrapper<int>     IIntWrapper;
  typedef IPrimitiveWrapper<long>    ILongWrapper;
  typedef IPrimitiveWrapper<unsigned short>   IUShortWrapper;
  typedef IPrimitiveWrapper<unsigned int>     IUIntWrapper;
  typedef IPrimitiveWrapper<unsigned long>    IULongWrapper;
*/
class IStringEvaluator: public IEvaluator  {
public:
  ~IStringEvaluator() {  }
  bool operator == (const IEvaluator &val) const  {  return !EvaluateString().Comparei( val.EvaluateString() );  }
  bool operator != (const IEvaluator &val) const  {  return EvaluateString().Comparei( val.EvaluateString() ) != 0;  }
  bool operator > (const IEvaluator &val) const   {  return EvaluateString().Comparei( val.EvaluateString() ) > 0;  }
  bool operator >= (const IEvaluator &val) const  {  return EvaluateString().Comparei( val.EvaluateString() ) >= 0;  }
  bool operator < (const IEvaluator &val) const   {  return EvaluateString().Comparei( val.EvaluateString() ) < 0;  }
  bool operator <= (const IEvaluator &val) const  {  return EvaluateString().Comparei( val.EvaluateString() ) <= 0;  }
  const olxstr& EvaluateString() const = 0;
};

class IDoubleEvaluator: public IEvaluator  {
public:
  bool operator == (const IEvaluator &val) const  {  return EvaluateDouble() == val.EvaluateDouble();  }
  bool operator != (const IEvaluator &val) const  {  return EvaluateDouble() != val.EvaluateDouble();  }
  bool operator > (const IEvaluator &val) const   {  return EvaluateDouble() > val.EvaluateDouble();  }
  bool operator >= (const IEvaluator &val) const  {  return EvaluateDouble() >= val.EvaluateDouble();  }
  bool operator < (const IEvaluator &val) const   {  return EvaluateDouble() < val.EvaluateDouble();  }
  bool operator <= (const IEvaluator &val) const  {  return EvaluateDouble() <= val.EvaluateDouble();  }

  bool   EvaluateBool()            const {  return (EvaluateDouble()!=0);  }
  short   EvaluateShort()          const {  return (short)EvaluateDouble();  }
  int   EvaluateInt()              const {  return (int)EvaluateDouble();  }
  long   EvaluateLong()            const {  return (long)EvaluateDouble();  }
  float   EvaluateFloat()          const {  return (float)EvaluateDouble();  }
  double EvaluateDouble()          const = 0;
  unsigned short   EvaluateUshort()const {  return (unsigned short)EvaluateDouble();  }
  unsigned int   EvaluateUint()    const {  return (unsigned int)EvaluateDouble();  }
  unsigned long   EvaluateUlong()  const {  return (unsigned long)EvaluateDouble();  }
};

class IFloatEvaluator: public IEvaluator  {
public:
  bool operator == (const IEvaluator &val) const  {  return EvaluateFloat() == val.EvaluateFloat();  }
  bool operator != (const IEvaluator &val) const  {  return EvaluateFloat() != val.EvaluateFloat();  }
  bool operator > (const IEvaluator &val) const   {  return EvaluateFloat() > val.EvaluateFloat();  }
  bool operator >= (const IEvaluator &val) const  {  return EvaluateFloat() >= val.EvaluateFloat();  }
  bool operator < (const IEvaluator &val) const   {  return EvaluateFloat() < val.EvaluateFloat();  }
  bool operator <= (const IEvaluator &val) const  {  return EvaluateFloat() <= val.EvaluateFloat();  }

  bool   EvaluateBool()            const {  return (EvaluateFloat()!=0);  }
  short   EvaluateShort()          const {  return (short)EvaluateFloat();  }
  int   EvaluateInt()              const {  return (int)EvaluateFloat();  }
  long   EvaluateLong()            const {  return (long)EvaluateFloat();  }
  float   EvaluateFloat()          const = 0;
  double EvaluateDouble()          const {  return (double)EvaluateFloat();  }
  unsigned short   EvaluateUshort()const {  return (unsigned short)EvaluateFloat();  }
  unsigned int   EvaluateUint()    const {  return (unsigned int)EvaluateFloat();  }
  unsigned long   EvaluateUlong()  const {  return (unsigned long)EvaluateFloat();  }
};

class IIntEvaluator: public IEvaluator  {
public:
  bool operator == (const IEvaluator &val) const  {  return EvaluateInt() == val.EvaluateInt();  }
  bool operator != (const IEvaluator &val) const  {  return EvaluateInt() != val.EvaluateInt();  }
  bool operator > (const IEvaluator &val) const   {  return EvaluateInt() > val.EvaluateInt();  }
  bool operator >= (const IEvaluator &val) const  {  return EvaluateInt() >= val.EvaluateInt();  }
  bool operator < (const IEvaluator &val) const   {  return EvaluateInt() < val.EvaluateInt();  }
  bool operator <= (const IEvaluator &val) const  {  return EvaluateInt() <= val.EvaluateInt();  }

  bool   EvaluateBool()            const {  return (EvaluateInt()!=0);  }
  short   EvaluateShort()          const {  return (short)EvaluateInt();  }
  int   EvaluateInt()              const = 0;
  long   EvaluateLong()            const {  return (long)EvaluateInt();  }
  float   EvaluateFloat()          const {  return (float)EvaluateInt();  }
  double EvaluateDouble()          const {  return (double)EvaluateInt();  }
  unsigned short   EvaluateUshort()const {  return (unsigned short)EvaluateInt();  }
  unsigned int   EvaluateUint()    const {  return (unsigned int)EvaluateInt();  }
  unsigned long   EvaluateUlong()  const {  return (unsigned long)EvaluateInt();  }
};

class IShortEvaluator: public IEvaluator  {
public:
  bool operator == (const IEvaluator &val) const  {  return EvaluateShort() == val.EvaluateShort();  }
  bool operator != (const IEvaluator &val) const  {  return EvaluateShort() != val.EvaluateShort();  }
  bool operator > (const IEvaluator &val) const   {  return EvaluateShort() > val.EvaluateShort();  }
  bool operator >= (const IEvaluator &val) const  {  return EvaluateShort() >= val.EvaluateShort();  }
  bool operator < (const IEvaluator &val) const   {  return EvaluateShort() < val.EvaluateShort();  }
  bool operator <= (const IEvaluator &val) const  {  return EvaluateShort() <= val.EvaluateShort();  }

  bool   EvaluateBool()            const {  return (EvaluateShort()!=0);  }
  short   EvaluateShort()          const = 0;
  int   EvaluateInt()              const {  return (int)EvaluateShort();  }
  long   EvaluateLong()            const {  return (long)EvaluateShort();  }
  float   EvaluateFloat()          const {  return (float)EvaluateShort();  }
  double EvaluateDouble()          const {  return (double)EvaluateShort();  }
  unsigned short   EvaluateUshort()const {  return (unsigned short)EvaluateShort();  }
  unsigned int   EvaluateUint()    const {  return (unsigned int)EvaluateShort();  }
  unsigned long   EvaluateUlong()  const {  return (unsigned long)EvaluateShort();  }
};

class IUshortEvaluator: public IEvaluator  {
public:
  bool operator == (const IEvaluator &val) const  {  return EvaluateUshort() == val.EvaluateUshort();  }
  bool operator != (const IEvaluator &val) const  {  return EvaluateUshort() != val.EvaluateUshort();  }
  bool operator > (const IEvaluator &val) const   {  return EvaluateUshort() > val.EvaluateUshort();  }
  bool operator >= (const IEvaluator &val) const  {  return EvaluateUshort() >= val.EvaluateUshort();  }
  bool operator < (const IEvaluator &val) const   {  return EvaluateUshort() < val.EvaluateUshort();  }
  bool operator <= (const IEvaluator &val) const  {  return EvaluateUshort() <= val.EvaluateUshort();  }

  bool   EvaluateBool()            const {  return (EvaluateUshort()!=0);  }
  short   EvaluateShort()          const {  return (short)EvaluateUshort();  }
  int   EvaluateInt()              const {  return (int)EvaluateUshort();  }
  long   EvaluateLong()            const {  return (long)EvaluateUshort();  }
  float   EvaluateFloat()          const {  return (float)EvaluateUshort();  }
  double EvaluateDouble()          const {  return (double)EvaluateUshort();  }
  unsigned short   EvaluateUshort()const = 0;
  unsigned int   EvaluateUint()    const {  return (unsigned int)EvaluateUshort();  }
  unsigned long   EvaluateUlong()  const {  return (unsigned long)EvaluateUshort();  }
};

class IUintEvaluator: public IEvaluator  {
public:
  bool operator == (const IEvaluator &val) const  {  return EvaluateUint() == val.EvaluateUint();  }
  bool operator != (const IEvaluator &val) const  {  return EvaluateUint() != val.EvaluateUint();  }
  bool operator > (const IEvaluator &val) const   {  return EvaluateUint() > val.EvaluateUint();  }
  bool operator >= (const IEvaluator &val) const  {  return EvaluateUint() >= val.EvaluateUint();  }
  bool operator < (const IEvaluator &val) const   {  return EvaluateUint() < val.EvaluateUint();  }
  bool operator <= (const IEvaluator &val) const  {  return EvaluateUint() <= val.EvaluateUint();  }

  bool   EvaluateBool()            const {  return (EvaluateUint()!=0);  }
  short   EvaluateShort()          const {  return (short)EvaluateUint();  }
  int   EvaluateInt()              const {  return (int)EvaluateUint();  }
  long   EvaluateLong()            const {  return (long)EvaluateUint();  }
  float   EvaluateFloat()          const {  return (float)EvaluateUint();  }
  double EvaluateDouble()          const {  return (double)EvaluateUint();  }
  unsigned short   EvaluateUshort()const {  return (unsigned short)EvaluateUint();  }
  unsigned int   EvaluateUint()    const = 0;
  unsigned long   EvaluateUlong()  const {  return (unsigned long)EvaluateUint();  }
};

class IUlongEvaluator: public IEvaluator  {
public:
  bool operator == (const IEvaluator &val) const  {  return EvaluateUlong() == val.EvaluateUlong();  }
  bool operator != (const IEvaluator &val) const  {  return EvaluateUlong() != val.EvaluateUlong();  }
  bool operator > (const IEvaluator &val) const   {  return EvaluateUlong() > val.EvaluateUlong();  }
  bool operator >= (const IEvaluator &val) const  {  return EvaluateUlong() >= val.EvaluateUlong();  }
  bool operator < (const IEvaluator &val) const   {  return EvaluateUlong() < val.EvaluateUlong();  }
  bool operator <= (const IEvaluator &val) const  {  return EvaluateUlong() <= val.EvaluateUlong();  }

  bool   EvaluateBool()            const {  return (EvaluateUlong()!=0);  }
  short   EvaluateShort()          const {  return (short)EvaluateUlong();  }
  int   EvaluateInt()              const {  return (int)EvaluateUlong();  }
  long   EvaluateLong()            const {  return (long)EvaluateUlong();  }
  float   EvaluateFloat()          const {  return (float)EvaluateUlong();  }
  double EvaluateDouble()          const {  return (double)EvaluateUlong();  }
  unsigned short   EvaluateUshort()const {  return (unsigned short)EvaluateUlong();  }
  unsigned int   EvaluateUint()    const {  return (unsigned int)EvaluateUlong();  }
  unsigned long   EvaluateUlong()  const = 0;
};

class IBoolEvaluator: public IEvaluator  {
public:
  bool operator == (const IEvaluator &val) const  {  return EvaluateBool() == val.EvaluateBool();  }
  bool operator != (const IEvaluator &val) const  {  return EvaluateBool() != val.EvaluateBool();  }
  bool   EvaluateBool()            const = 0;
};

class TStringEvaluator: public IStringEvaluator  {
  olxstr Value;
public:
  TStringEvaluator(const olxstr &Val) {  Value = Val;  }
  ~TStringEvaluator() {  ;  }
  const olxstr& EvaluateString() const {  return Value;  }
  void SetValue(const olxstr& v) {  Value = v;  }
  IEvaluator *NewInstance(IDataProvider *) {  return new TStringEvaluator(Value);  }
};

class TScalarEvaluator: public IDoubleEvaluator  {
  double Value;
public:
  TScalarEvaluator(double Val) {  Value = Val;  }
  ~TScalarEvaluator() {  ;  }
  double EvaluateDouble() const {  return Value;  }
  void SetValue(double v) {  Value = v;  }
  IEvaluator *NewInstance(IDataProvider *) {  return new TScalarEvaluator(Value);  }
};

class TBoolEvaluator: public IBoolEvaluator  {
  bool Value;
public:
  TBoolEvaluator(bool Val) {  Value = Val;  }
  ~TBoolEvaluator() {  ;  }
  bool   EvaluateBool() const {  return Value;  }
  void SetValue( bool v ) {  Value = v;  }
  IEvaluator *NewInstance(IDataProvider *) {  return new TBoolEvaluator (Value);  }
};

template <class PropertyProviderClass, class EvaluatorClass>
  class TPropertyEvaluator: public EvaluatorClass  {
  protected:
    PropertyProviderClass FData;
  public:
    TPropertyEvaluator()                           {  FData = NULL;  }
    TPropertyEvaluator(PropertyProviderClass data) {  FData = data;  }
    virtual ~TPropertyEvaluator()                 {}
    void Data( PropertyProviderClass data )        {  FData = data;  }
    PropertyProviderClass Data()             const {  return FData;  }
  };

template <class CollectionProviderClass, class PropertyProviderClass>
  class ACollection  {
  protected:
    CollectionProviderClass FData;
  public:
    ACollection()                                 {  FData = NULL;  }
    ACollection(CollectionProviderClass data)     {  FData = data;  }
    virtual ~ACollection()                       {}
    PropertyProviderClass Item(int index)         {  return NULL;  }
    void Data( CollectionProviderClass data )     {  FData = data;  }
    CollectionProviderClass Data()          const {  return FData;  }
    virtual int Size() = 0;
  };

template <class CollectionProviderClass, class PropertyProviderClass, class EvaluatorClass>
  class TCollectionPropertyEvaluator: public IEvaluator  {
    ACollection<CollectionProviderClass, PropertyProviderClass> *FCollection;
    TPropertyEvaluator<PropertyProviderClass, EvaluatorClass> *FPropertyEvaluator;
    IEvaluator *FIterator;
  public:
    TCollectionPropertyEvaluator(ACollection<CollectionProviderClass, PropertyProviderClass> *C,
    IEvaluator *itr, TPropertyEvaluator<PropertyProviderClass, EvaluatorClass> *PE)
    {
      FCollection = C;
      FIterator = itr;
      FPropertyEvaluator = PE;
    }
    bool   EvaluateBool()           const {
      FPropertyEvaluator->Data( FCollection->Item( FIterator->EvaluateInt() ) );
      return FPropertyEvaluator->EvaluateBool();
    }
    short   EvaluateShort()         const {
      FPropertyEvaluator->Data( FCollection->Item( FIterator->EvaluateInt() ) );
      return FPropertyEvaluator->EvaluateShort();
    }
    int   EvaluateInt()             const {
      FPropertyEvaluator->Data( FCollection->Item( FIterator->EvaluateInt() ) );
      return FPropertyEvaluator->EvaluateInt();
    }
    long   EvaluateLong()           const {
      FPropertyEvaluator->Data( FCollection->Item( FIterator->EvaluateInt() ) );
      return FPropertyEvaluator->EvaluateLong();
    }
    float   EvaluateFloat()         const {
      FPropertyEvaluator->Data( FCollection->Item( FIterator->EvaluateInt() ) );
      return FPropertyEvaluator->EvaluateFloat();
    }
    double EvaluateDouble()         const {
      FPropertyEvaluator->Data( FCollection->Item( FIterator->EvaluateInt() ) );
      return FPropertyEvaluator->EvaluateDouble();
    }
    unsigned short   EvaluateUshort()const  {
      FPropertyEvaluator->Data( FCollection->Item( FIterator->EvaluateInt() ) );
      return FPropertyEvaluator->EvaluateUshort();
    }
    unsigned int   EvaluateUint()   const {
      FPropertyEvaluator->Data( FCollection->Item( FIterator->EvaluateInt() ) );
      return FPropertyEvaluator->EvaluateUint();
    }
    unsigned long  EvaluateUlong()  const {
      FPropertyEvaluator->Data( FCollection->Item( FIterator->EvaluateInt() ) );
      return FPropertyEvaluator->EvaluateUlong();
    }
    const olxstr& EvaluateString()    const {
      FPropertyEvaluator->Data( FCollection->Item( FIterator->EvaluateInt() ) );
      return FPropertyEvaluator->EvaluateString();
    }
    virtual ~TCollectionPropertyEvaluator() {}
  };

// property types
const short mtUndefined  = -1,
            mtProperty   = 1,
            mtComplex    = 2,
            mtCollection = 3;

class IClassDefinition  {
public:
  virtual ~IClassDefinition() {}
  // returns one of the above mtXXX constants
  virtual short GetMemberType(const olxstr& propName);
  virtual olxstr GetPropertyType(const olxstr& propName);
  virtual olxstr GetComplexType(const olxstr& propName);
  virtual olxstr GetCollectionType(const olxstr& propName);
  virtual IEvaluator* GetPropertyEvaluator(const olxstr& propName);
  virtual IDataProvider* GetComplexEvaluator(const olxstr& propName);
  //virtual ICollection* GetCollectionEvaluator(const olxstr& propName);

};

class TEvaluatorFactory  {
  TSStrPObjList<olxstr,IClassDefinition*, true> ClassDefinitions;
public:
  TEvaluatorFactory() {  ;  }
  IEvaluator* Evaluator(const olxstr &Val) {
    TStrList toks(Val, '.');
    if( toks.Count() <= 1 ) {
      // a special action has to be taken if operators are to be considered
      // in the future
      return NULL;
    }
    IClassDefinition * classDef = ClassDefinitions[toks[0]];
    //TODO: report the error
    if( !classDef )  return NULL;
    toks.Delete(0);
    return ProcessProperties( classDef, toks );
  }
protected:
  IEvaluator* ProcessProperties(IClassDefinition *classDef, TStrList props) {
    short memberType = classDef->GetMemberType( props[0] );
    switch( memberType ) {
      case mtProperty:
        break;
      case mtComplex:
        break;
      case mtCollection:
        break;
      // TODO: report error
      default:
        return NULL;
    }
    return NULL;
  }
};

class IEvaluatorFactory  {
public:
  virtual size_t EvaluatorCount() = 0;
  virtual IEvaluator *Evaluator(size_t index) = 0;
  virtual IEvaluatorFactory* Factory(const olxstr&) {
    throw TNotImplementedException(__OlxSourceInfo);
  }
  virtual const olxstr& EvaluatorName(size_t index) = 0;
  virtual ~IEvaluatorFactory() {}
  virtual IEvaluator* Evaluator(const olxstr &Val) = 0;
};

/*
a class to peform comparison operations on one (!) or two objects
*/
class TArithmeticOperator: public IEvaluator  {
  IEvaluator *Left, *Right;
  short Type;
public:
  TArithmeticOperator(const short type, IEvaluator *left, IEvaluator *Right = NULL);
};
/* ___________________________________________________________________________*/
/* ________________COMPARISON OPERATORS IMPLEMENTATION________________________*/

class TcoGOperator: public IEvaluable  {
  IEvaluator *Left, *Right;
public:
  TcoGOperator(IEvaluator *left, IEvaluator *right) {
    Left = left;
    Right = right;
    if( Left == NULL || Right == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, "NULL evaluator");
  }
  ~TcoGOperator() {}
  bool Evaluate() {  return *Left > *Right;  }
};
class TcoLOperator: public IEvaluable  {
  IEvaluator *Left, *Right;
public:
  TcoLOperator(IEvaluator *left, IEvaluator *right) {
    Left = left;
    Right = right;
    if( Left == NULL || Right == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, "NULL evaluator");
  }
  ~TcoLOperator() {}
  bool Evaluate() {  return *Left < *Right;  }
};
class TcoNEOperator: public IEvaluable  {
  IEvaluator *Left, *Right;
public:
  TcoNEOperator(IEvaluator *left, IEvaluator *right) {
    Left = left;
    Right = right;
    if( Left == NULL || Right == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, "NULL evaluator");
  }
  ~TcoNEOperator() {}
  bool Evaluate() {  return *Left != *Right;  }
};
class TcoGEOperator: public IEvaluable  {
  IEvaluator *Left, *Right;
public:
  TcoGEOperator(IEvaluator *left, IEvaluator *right) {
    Left = left;
    Right = right;
    if( Left == NULL || Right == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, "NULL evaluator");
  }
  ~TcoGEOperator() {}
  bool Evaluate() {  return *Left >= *Right;  }
};
class TcoEOperator: public IEvaluable  {
  IEvaluator *Left, *Right;
public:
  TcoEOperator(IEvaluator *left, IEvaluator *right) {
    Left = left;
    Right = right;
    if( Left == NULL || Right == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, "NULL evaluator");
  }
  ~TcoEOperator() {}
  bool Evaluate() {  return *Left == *Right;  }
};
class TcoLEOperator: public IEvaluable  {
  IEvaluator *Left, *Right;
public:
  TcoLEOperator(IEvaluator *left, IEvaluator *right) {
    Left = left;
    Right = right;
    if( Left == NULL || Right == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, "NULL evaluator");
  }
  ~TcoLEOperator() {}
  bool Evaluate() {  return *Left <= *Right;  }
};
/* ___________________________________________________________________________*/
/* ___________________________________________________________________________*/
/* ________________LOGICAL OPERATORS IMPLEMENTATION___________________________*/
class TloAndOperator: public IEvaluable  {
  IEvaluable *OperandA, *OperandB;
public:
  TloAndOperator(IEvaluable *operandA, IEvaluable *operandB) {
    OperandA = operandA;
    OperandB = operandB;
    if( OperandA == NULL || OperandB == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, "NULL evaluable");
  }
  ~TloAndOperator() {}
  bool Evaluate() {
    if( !OperandA->Evaluate() )  return false;
    if( !OperandB->Evaluate() )  return false;
    return true;
  }
};
class TloOrOperator: public IEvaluable  {
  IEvaluable *OperandA, *OperandB;
public:
  TloOrOperator(IEvaluable *operandA, IEvaluable *operandB) {
    OperandA = operandA;
    OperandB = operandB;
    if( OperandA == NULL || OperandB == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, "NULL evaluable");
  }
  ~TloOrOperator() {}
  bool Evaluate() {
    if( OperandA->Evaluate() )  return true;
    if( OperandB->Evaluate() )  return true;
    return false;
  }
};
class TloNotOperator: public IEvaluable  {
  IEvaluable *Operand;
public:
  TloNotOperator(IEvaluable *operand) {
    Operand = operand;
    if( Operand == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, "NULL evaluable");
  }
  ~TloNotOperator() {}
  bool Evaluate() {  return !Operand->Evaluate();  }
};
/* ___________________________________________________________________________*/

// signgle argument operator factory
template <class OC, class IC, class AC>
  class TsaFactory: public TObjectFactory<OC> {
  public:
    ~TsaFactory() {}
    OC *NewInstance(TPtrList<IEObject>* Args) {
      if( Args->Count() != 1 )
        throw TInvalidArgumentException(__OlxSourceInfo, "number of operands");
      return new IC((AC*)Args->GetItem(0));
    }
  };
// two argument operator factory
template <class OC, class IC, class AC>
  class TtaFactory: public TObjectFactory<OC> {
  public:
    ~TtaFactory() {}
    OC *NewInstance(TPtrList<IEObject>* Args) {
      if( Args->Count() != 2 )  
        throw TInvalidArgumentException(__OlxSourceInfo, "number of operands");
      return new IC((AC*)Args->GetItem(0), (AC*)Args->GetItem(1));
    }
  };


class TSyntaxParser  {
  IEvaluable *Root;
  IEvaluatorFactory *EvaluatorFactory;
  TPtrList<IEvaluable> Evaluables;
  TPtrList<IEvaluator> Evaluators;
  TStrList FErrors;
  TSStrPObjList<olxstr,TObjectFactory<IEvaluable>*, false > LogicalOperators,
    ComparisonOperators, ArithmeticFunctions;
protected:
  IEvaluable* SimpleParse(const olxstr& Expression);
public:
  TSyntaxParser(IEvaluatorFactory *FactoryInstance, const olxstr &Text);
  ~TSyntaxParser();
  bool Evaluate() {  return Root->Evaluate();  }
  const TStrList& Errors() const  {  return FErrors;  }
};

#endif
