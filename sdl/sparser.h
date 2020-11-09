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
    virtual IC *NewInstance(TPtrList<IOlxObject>* Arguments) = 0;
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

class IEvaluable : public IOlxObject {
public:
  virtual ~IEvaluable() {}
  virtual bool Evaluate() = 0;
};


// an abstract class for evaluation simple expressions
class IEvaluator : public IOlxObject {
public:
  virtual ~IEvaluator() {}

  class TUnsupportedOperator : public TBasicException {
  public:
    TUnsupportedOperator(const olxstr& location) :
      TBasicException(location, EmptyString()) {}
    virtual IOlxObject* Replicate()  const { return new TUnsupportedOperator(*this); }
  };
  virtual bool operator == (const IEvaluator&) const { throw TUnsupportedOperator(__OlxSourceInfo); }
  virtual bool operator != (const IEvaluator&) const { throw TUnsupportedOperator(__OlxSourceInfo); }
  virtual bool operator > (const IEvaluator&)  const { throw TUnsupportedOperator(__OlxSourceInfo); }
  virtual bool operator >= (const IEvaluator&) const { throw TUnsupportedOperator(__OlxSourceInfo); }
  virtual bool operator < (const IEvaluator&)  const { throw TUnsupportedOperator(__OlxSourceInfo); }
  virtual bool operator <= (const IEvaluator&) const { throw TUnsupportedOperator(__OlxSourceInfo); }
  virtual IEvaluator* NewInstance(IDataProvider*) = 0;


  class TCastException : public TBasicException {
  public:
    TCastException(const olxstr& location) :
      TBasicException(location, EmptyString()) {
      ;
    }
    virtual IOlxObject* Replicate()  const { return new TCastException(*this); }
  };
  virtual long EvaluateInt()             const { throw TCastException(__OlxSourceInfo); }
  virtual double EvaluateFloat()         const { throw TCastException(__OlxSourceInfo); }
  virtual bool EvaluateBool()            const { throw TCastException(__OlxSourceInfo); }
  virtual olxstr EvaluateString()        const { throw TCastException(__OlxSourceInfo); }
};

class IStringEvaluator : public IEvaluator {
public:
  ~IStringEvaluator() {  }
  bool operator == (const IEvaluator& val)  const { return !EvaluateString().Comparei(val.EvaluateString()); }
  bool operator != (const IEvaluator& val)  const { return EvaluateString().Comparei(val.EvaluateString()) != 0; }
  bool operator > (const IEvaluator& val)   const { return EvaluateString().Comparei(val.EvaluateString()) > 0; }
  bool operator >= (const IEvaluator& val)  const { return EvaluateString().Comparei(val.EvaluateString()) >= 0; }
  bool operator < (const IEvaluator& val)   const { return EvaluateString().Comparei(val.EvaluateString()) < 0; }
  bool operator <= (const IEvaluator& val)  const { return EvaluateString().Comparei(val.EvaluateString()) <= 0; }

  long EvaluateInt()           const { return EvaluateString().RadInt<long>(); }
  double EvaluateFloat()       const { return EvaluateString().ToDouble(); }
  bool EvaluateBool()          const { return EvaluateString().ToBool(); }
  olxstr EvaluateString()       const = 0;
};

class IFloatEvaluator: public IEvaluator  {
public:
  bool operator == (const IEvaluator &val) const  {  return EvaluateFloat() == val.EvaluateFloat();  }
  bool operator != (const IEvaluator &val) const  {  return EvaluateFloat() != val.EvaluateFloat();  }
  bool operator > (const IEvaluator &val) const   {  return EvaluateFloat() > val.EvaluateFloat();  }
  bool operator >= (const IEvaluator &val) const  {  return EvaluateFloat() >= val.EvaluateFloat();  }
  bool operator < (const IEvaluator &val) const   {  return EvaluateFloat() < val.EvaluateFloat();  }
  bool operator <= (const IEvaluator &val) const  {  return EvaluateFloat() <= val.EvaluateFloat();  }

  long EvaluateInt()           const { return (long)EvaluateFloat(); }
  olxstr EvaluateString()      const { return olxstr(EvaluateFloat()); }
  double EvaluateFloat()       const = 0;
};

class IIntEvaluator : public IEvaluator {
public:
  bool operator == (const IEvaluator& val) const { return EvaluateInt() == val.EvaluateInt(); }
  bool operator != (const IEvaluator& val) const { return EvaluateInt() != val.EvaluateInt(); }
  bool operator > (const IEvaluator& val) const { return EvaluateInt() > val.EvaluateInt(); }
  bool operator >= (const IEvaluator& val) const { return EvaluateInt() >= val.EvaluateInt(); }
  bool operator < (const IEvaluator& val) const { return EvaluateInt() < val.EvaluateInt(); }
  bool operator <= (const IEvaluator& val) const { return EvaluateInt() <= val.EvaluateInt(); }

  double EvaluateFloat()       const { return (double)EvaluateInt(); }
  olxstr EvaluateString()      const { return olxstr(EvaluateInt()); }
  long EvaluateInt()           const = 0;
};

class IBoolEvaluator : public IEvaluator {
public:
  bool operator == (const IEvaluator& val) const { return EvaluateBool() == val.EvaluateBool(); }
  bool operator != (const IEvaluator& val) const { return EvaluateBool() != val.EvaluateBool(); }
  bool   EvaluateBool() const = 0;
  olxstr EvaluateString() const { return olxstr(EvaluateBool()); }
};

class TStringEvaluator : public IStringEvaluator {
  olxstr Value;
public:
  TStringEvaluator(const olxstr& Val) { Value = Val; }
  olxstr EvaluateString() const { return Value; }
  void SetValue(const olxstr& v) { Value = v; }
  IEvaluator* NewInstance(IDataProvider*) { return new TStringEvaluator(Value); }
};

class TScalarEvaluator : public IFloatEvaluator {
  double Value;
public:
  TScalarEvaluator(double Val) { Value = Val; }
  ~TScalarEvaluator() {}
  double EvaluateFloat() const { return Value; }
  void SetValue(double v) { Value = v; }
  IEvaluator* NewInstance(IDataProvider*) { return new TScalarEvaluator(Value); }
};

class TBoolEvaluator : public IBoolEvaluator {
  bool Value;
public:
  TBoolEvaluator(bool Val) { Value = Val; }
  ~TBoolEvaluator() { ; }
  bool   EvaluateBool() const { return Value; }
  void SetValue(bool v) { Value = v; }
  IEvaluator* NewInstance(IDataProvider*) { return new TBoolEvaluator(Value); }
};

template <class PropertyProviderClass, class EvaluatorClass>
class TPropertyEvaluator : public EvaluatorClass {
protected:
  PropertyProviderClass FData;
public:
  TPropertyEvaluator() { FData = 0; }
  TPropertyEvaluator(PropertyProviderClass data) { FData = data; }
  void Data(PropertyProviderClass data) { FData = data; }
  PropertyProviderClass Data() const { return FData; }
};

template <class CollectionProviderClass, class PropertyProviderClass>
class ACollection {
protected:
  CollectionProviderClass FData;
public:
  ACollection() { FData = 0; }
  ACollection(CollectionProviderClass data) { FData = data; }
  virtual ~ACollection() {}
  PropertyProviderClass Item(int index) { return 0; }
  void Data(CollectionProviderClass data) { FData = data; }
  CollectionProviderClass Data() const { return FData; }
  virtual int Size() = 0;
};

template <class CollectionProviderClass, class PropertyProviderClass, class EvaluatorClass>
class TCollectionPropertyEvaluator : public IEvaluator {
  ACollection<CollectionProviderClass, PropertyProviderClass>* FCollection;
  TPropertyEvaluator<PropertyProviderClass, EvaluatorClass>* FPropertyEvaluator;
  IEvaluator* FIterator;
public:
  TCollectionPropertyEvaluator(ACollection<CollectionProviderClass, PropertyProviderClass>* C,
    IEvaluator* itr, TPropertyEvaluator<PropertyProviderClass, EvaluatorClass>* PE)
  {
    FCollection = C;
    FIterator = itr;
    FPropertyEvaluator = PE;
  }
  bool EvaluateBool() const {
    FPropertyEvaluator->Data(FCollection->Item(FIterator->EvaluateInt()));
    return FPropertyEvaluator->EvaluateBool();
  }
  long EvaluateInt()  const {
    FPropertyEvaluator->Data(FCollection->Item(FIterator->EvaluateInt()));
    return FPropertyEvaluator->EvaluateInt();
  }
  double EvaluateFloat() const {
    FPropertyEvaluator->Data(FCollection->Item(FIterator->EvaluateInt()));
    return FPropertyEvaluator->EvaluateDouble();
  }
  olxstr EvaluateString() const {
    FPropertyEvaluator->Data(FCollection->Item(FIterator->EvaluateInt()));
    return FPropertyEvaluator->EvaluateString();
  }
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
  olxstr_dict<IClassDefinition*, true> ClassDefinitions;
public:
  TEvaluatorFactory() {}
  IEvaluator* Evaluator(const olxstr &Val) {
    TStrList toks(Val, '.');
    if( toks.Count() <= 1 ) {
      // a special action has to be taken if operators are to be considered
      // in the future
      return 0;
    }
    IClassDefinition * classDef = ClassDefinitions.Find(toks[0], 0);
    //TODO: report the error
    if (classDef == 0) {
      return 0;
    }
    toks.Delete(0);
    return ProcessProperties(classDef, toks);
  }
protected:
  IEvaluator* ProcessProperties(IClassDefinition *classDef, const TStrList &props) {
    short memberType = classDef->GetMemberType(props[0]);
    switch( memberType ) {
      case mtProperty:
        break;
      case mtComplex:
        break;
      case mtCollection:
        break;
      // TODO: report error
      default:
        return 0;
    }
    return 0;
  }
};

class IEvaluatorFactory {
public:
  virtual size_t EvaluatorCount() = 0;
  virtual IEvaluator* Evaluator(size_t index) = 0;
  virtual IEvaluatorFactory* FindFactory(const olxstr&) {
    throw TNotImplementedException(__OlxSourceInfo);
  }
  virtual const olxstr& EvaluatorName(size_t index) = 0;
  virtual ~IEvaluatorFactory() {}
  virtual IEvaluator* Evaluator(const olxstr& Val) = 0;
};

/*
a class to peform comparison operations on one (!) or two objects
*/
class TArithmeticOperator: public IEvaluator  {
  IEvaluator *Left, *Right;
  short Type;
public:
  TArithmeticOperator(const short type, IEvaluator *left, IEvaluator *Right = 0);
};
/* ___________________________________________________________________________*/
/* ________________COMPARISON OPERATORS IMPLEMENTATION________________________*/

class TcoGOperator : public IEvaluable {
  IEvaluator* Left, * Right;
public:
  TcoGOperator(IEvaluator* left, IEvaluator* right) {
    Left = left;
    Right = right;
    if (Left == 0 || Right == 0) {
      throw TInvalidArgumentException(__OlxSourceInfo, "NULL evaluator");
    }
  }
  bool Evaluate() { return *Left > * Right; }
};

class TcoLOperator : public IEvaluable {
  IEvaluator* Left, * Right;
public:
  TcoLOperator(IEvaluator* left, IEvaluator* right) {
    Left = left;
    Right = right;
    if (Left == 0 || Right == 0) {
      throw TInvalidArgumentException(__OlxSourceInfo, "NULL evaluator");
    }
  }
  bool Evaluate() { return *Left < *Right; }
};

class TcoNEOperator : public IEvaluable {
  IEvaluator* Left, * Right;
public:
  TcoNEOperator(IEvaluator* left, IEvaluator* right) {
    Left = left;
    Right = right;
    if (Left == 0 || Right == 0) {
      throw TInvalidArgumentException(__OlxSourceInfo, "NULL evaluator");
    }
  }
  bool Evaluate() { return *Left != *Right; }
};

class TcoGEOperator : public IEvaluable {
  IEvaluator* Left, * Right;
public:
  TcoGEOperator(IEvaluator* left, IEvaluator* right) {
    Left = left;
    Right = right;
    if (Left == 0 || Right == 0) {
      throw TInvalidArgumentException(__OlxSourceInfo, "NULL evaluator");
    }
  }
  bool Evaluate() { return *Left >= *Right; }
};

class TcoEOperator : public IEvaluable {
  IEvaluator* Left, * Right;
public:
  TcoEOperator(IEvaluator* left, IEvaluator* right) {
    Left = left;
    Right = right;
    if (Left == 0 || Right == 0) {
      throw TInvalidArgumentException(__OlxSourceInfo, "NULL evaluator");
    }
  }
  bool Evaluate() { return *Left == *Right; }
};

class TcoLEOperator : public IEvaluable {
  IEvaluator* Left, * Right;
public:
  TcoLEOperator(IEvaluator* left, IEvaluator* right) {
    Left = left;
    Right = right;
    if (Left == 0 || Right == 0) {
      throw TInvalidArgumentException(__OlxSourceInfo, "NULL evaluator");
    }
  }
  bool Evaluate() { return *Left <= *Right; }
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
    if (OperandA == 0 || OperandB == 0) {
      throw TInvalidArgumentException(__OlxSourceInfo, "NULL evaluable");
    }
  }
  bool Evaluate() {
    return OperandA->Evaluate() && OperandB->Evaluate();
  }
};

class TloOrOperator : public IEvaluable {
  IEvaluable* OperandA, * OperandB;
public:
  TloOrOperator(IEvaluable* operandA, IEvaluable* operandB) {
    OperandA = operandA;
    OperandB = operandB;
    if (OperandA == 0 || OperandB == 0) {
      throw TInvalidArgumentException(__OlxSourceInfo, "NULL evaluable");
    }
  }
  bool Evaluate() {
    return OperandA->Evaluate() || OperandB->Evaluate();
  }
};

class TloNotOperator : public IEvaluable {
  IEvaluable* Operand;
public:
  TloNotOperator(IEvaluable* operand) {
    Operand = operand;
    if (Operand == 0) {
      throw TInvalidArgumentException(__OlxSourceInfo, "NULL evaluable");
    }
  }
  bool Evaluate() { return !Operand->Evaluate(); }
};
/* ___________________________________________________________________________*/

// signgle argument operator factory
template <class OC, class IC, class AC>
class TsaFactory : public TObjectFactory<OC> {
public:
  ~TsaFactory() {}
  OC* NewInstance(TPtrList<IOlxObject>* Args) {
    if (Args->Count() != 1) {
      throw TInvalidArgumentException(__OlxSourceInfo, "number of operands");
    }
    return new IC((AC*)Args->GetItem(0));
  }
};
// two argument operator factory
template <class OC, class IC, class AC>
class TtaFactory : public TObjectFactory<OC> {
public:
  ~TtaFactory() {}
  OC* NewInstance(TPtrList<IOlxObject>* Args) {
    if (Args->Count() != 2) {
      throw TInvalidArgumentException(__OlxSourceInfo, "number of operands");
    }
    return new IC((AC*)Args->GetItem(0), (AC*)Args->GetItem(1));
  }
};


class TExpressionParser {
  IEvaluable* Root;
  IEvaluatorFactory* EvaluatorFactory;
  TPtrList<IEvaluable> Evaluables;
  TPtrList<IEvaluator> Evaluators;
  TStrList FErrors;
  olxstr_dict<TObjectFactory<IEvaluable>*, false> LogicalOperators,
    ComparisonOperators, ArithmeticFunctions;
protected:
  IEvaluable* SimpleParse(const olxstr& Expression);
public:
  TExpressionParser(IEvaluatorFactory* FactoryInstance, const olxstr& Text);
  ~TExpressionParser();
  bool Evaluate() { return Root->Evaluate(); }
  const TStrList& Errors() const { return FErrors; }
};

#endif
