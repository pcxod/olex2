//---------------------------------------------------------------------------

#ifndef syntaxpH
#define syntaxpH
#include "elist.h"
#include "exception.h"
#include "estlist.h"
#include "estrlist.h"
#include "tptrlist.h"
// arithmetic operators/functions
const short aofAdd  = 1,
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
            aofAbs  = 12;

template <class IC>
  class TObjectFactory  {
  public:
    virtual ~TObjectFactory()  {  ;  }
    virtual IC *NewInstance(TEList *Arguments) = 0;
  };

class TOperatorSignature  {
public:
  TEString StringValue;
  short    ShortValue;
  TOperatorSignature(const short shortVal, const TEString &strVal);

};
// an array to use in parsing

extern TOperatorSignature DefinedFunctions[];
extern short DefinedFunctionCount;

class IDataProvider  {
public:
  virtual ~IDataProvider()  {  ;  }
};

class IEvaluable  {
public:
  virtual ~IEvaluable() {  ;  }
  virtual bool Evaluate() const = 0;
};


// an abstract class for evaluation simple expressions
class IEvaluator: public IEObject  {
public:
  virtual ~IEvaluator() {  ;  }

  class TUnsupportedOperator: public TExceptionBase  {
  public:
    TUnsupportedOperator(const TEString& location):
        TExceptionBase(location, EmptyString)  {  ;  }
    virtual IEObject* Replicate()  const {  return new TUnsupportedOperator(*this);  }
  };
  virtual bool operator == (const IEvaluator &val) const {  throw TUnsupportedOperator(__OlxSourceInfo);  }
  virtual bool operator != (const IEvaluator &val) const {  throw TUnsupportedOperator(__OlxSourceInfo);  }
  virtual bool operator > (const IEvaluator &val)  const {  throw TUnsupportedOperator(__OlxSourceInfo);  }
  virtual bool operator >= (const IEvaluator &val) const {  throw TUnsupportedOperator(__OlxSourceInfo);  }
  virtual bool operator < (const IEvaluator &val)  const {  throw TUnsupportedOperator(__OlxSourceInfo);  }
  virtual bool operator <= (const IEvaluator &val) const {  throw TUnsupportedOperator(__OlxSourceInfo);  }
//  virtual bool operator <= (const IEvaluator &val) const {  throw TUnsupportedOperator(*(IEObject*)this, "Unsupported operator");  }

  virtual IEvaluator *NewInstance(IDataProvider *) = 0; // {  return NULL;  }


  class TCastException: public TExceptionBase  {
  public:
    TCastException(const TEString& location, const IEObject* obj):
        TExceptionBase(location, (obj != NULL) ? EsdlObjectName(*obj) : EmptyString)  { ;  }
    virtual IEObject* Replicate()  const {  return new TCastException(*this);  }
  };
  virtual short EvaluateShort()          const  {  throw TCastException(__OlxSourceInfo, this);  }
  virtual int EvaluateInt()              const  {  throw TCastException(__OlxSourceInfo, this);  }
  virtual long EvaluateLong()            const  {  throw TCastException(__OlxSourceInfo, this);  }
  virtual float EvaluateFloat()          const  {  throw TCastException(__OlxSourceInfo, this);  }
  virtual double EvaluateDouble()        const  {  throw TCastException(__OlxSourceInfo, this);  }
  virtual unsigned short EvaluateUshort()const  {  throw TCastException(__OlxSourceInfo, this);  }
  virtual unsigned int EvaluateUint()    const  {  throw TCastException(__OlxSourceInfo, this);  }
  virtual unsigned long EvaluateUlong()  const  {  throw TCastException(__OlxSourceInfo, this);  }
  virtual bool EvaluateBool()            const  {  throw TCastException(__OlxSourceInfo, this);  }
  virtual const TEString& EvaluateString()     const  {  throw TCastException(__OlxSourceInfo, this);  }
};
/*
class IArithmetic
{
public:
  virtual ~IArithmetic()  {  ; }
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
  ~IStringEvaluator() {  ;  }
  bool operator == (const IEvaluator &val) const  {  return !EvaluateString().CompareCI( val.EvaluateString() );  }
  bool operator != (const IEvaluator &val) const  {  return EvaluateString().CompareCI( val.EvaluateString() ) != 0;  }
  bool operator > (const IEvaluator &val) const   {  return EvaluateString().CompareCI( val.EvaluateString() ) > 0;  }
  bool operator >= (const IEvaluator &val) const  {  return EvaluateString().CompareCI( val.EvaluateString() ) >= 0;  }
  bool operator < (const IEvaluator &val) const   {  return EvaluateString().CompareCI( val.EvaluateString() ) < 0;  }
  bool operator <= (const IEvaluator &val) const  {  return EvaluateString().CompareCI( val.EvaluateString() ) <= 0;  }
  const TEString& EvaluateString() const = 0;
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
  TEString Value;
public:
  TStringEvaluator(const TEString &Val)  {  Value = Val;  }
  ~TStringEvaluator() {  ;  }
  const TEString& EvaluateString() const {  return Value;  }
  void SetValue(const TEString& v)  {  Value = v;  }
  IEvaluator *NewInstance(IDataProvider *) {  return new TStringEvaluator(Value);  }
};

class TScalarEvaluator: public IDoubleEvaluator  {
  double Value;
public:
  TScalarEvaluator(double Val)  {  Value = Val;  }
  ~TScalarEvaluator() {  ;  }
  double EvaluateDouble() const {  return Value;  }
  void SetValue(double v)  {  Value = v;  }
  IEvaluator *NewInstance(IDataProvider *) {  return new TScalarEvaluator(Value);  }
};

class TBoolEvaluator: public IBoolEvaluator  {
  bool Value;
public:
  TBoolEvaluator(bool Val)  {  Value = Val;  }
  ~TBoolEvaluator() {  ;  }
  bool   EvaluateBool() const {  return Value;  }
  bool   Evaluate() const {  return Value;  }
  void SetValue( bool v )  {  Value = v;  }
  IEvaluator *NewInstance(IDataProvider *) {  return new TBoolEvaluator (Value);  }
};

class TBoolEvaluable: public IEvaluable  {
  bool Value;
public:
  TBoolEvaluable(bool Val)  {  Value = Val;  }
  ~TBoolEvaluable() {  ;  }
  bool   Evaluate() const {  return Value;  }
};

template <class PropertyProviderClass, class EvaluatorClass>
  class TPropertyEvaluator: public EvaluatorClass  {
  protected:
    PropertyProviderClass FData;
  public:
    TPropertyEvaluator()                         {  FData = NULL;  }
    TPropertyEvaluator(PropertyProviderClass data)     {  FData = data;  }
    virtual ~TPropertyEvaluator()                      {  ;  }
    void Data( PropertyProviderClass data )            {  FData = data;  }
    PropertyProviderClass Data()                 const {  return FData;  }
  };

template <class CollectionProviderClass, class PropertyProviderClass>
  class ACollection  {
  protected:
    CollectionProviderClass FData;
  public:
    ACollection()                                 {  FData = NULL;  }
    ACollection( CollectionProviderClass data)    {  FData = data;  }
    virtual ~ACollection()                        {  ;  }
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
    bool   EvaluateBool()            const
    {
      FPropertyEvaluator->Data( FCollection->Item( FIterator->EvaluateInt() ) );
      return FPropertyEvaluator->EvaluateBool();
    }
    short   EvaluateShort()          const
    {
      FPropertyEvaluator->Data( FCollection->Item( FIterator->EvaluateInt() ) );
      return FPropertyEvaluator->EvaluateShort();
    }
    int   EvaluateInt()              const
    {
      FPropertyEvaluator->Data( FCollection->Item( FIterator->EvaluateInt() ) );
      return FPropertyEvaluator->EvaluateInt();
    }
    long   EvaluateLong()            const
    {
      FPropertyEvaluator->Data( FCollection->Item( FIterator->EvaluateInt() ) );
      return FPropertyEvaluator->EvaluateLong();
    }
    float   EvaluateFloat()          const
    {
      FPropertyEvaluator->Data( FCollection->Item( FIterator->EvaluateInt() ) );
      return FPropertyEvaluator->EvaluateFloat();
    }
    double EvaluateDouble()          const
    {
      FPropertyEvaluator->Data( FCollection->Item( FIterator->EvaluateInt() ) );
      return FPropertyEvaluator->EvaluateDouble();
    }
    unsigned short   EvaluateUshort()const
    {
      FPropertyEvaluator->Data( FCollection->Item( FIterator->EvaluateInt() ) );
      return FPropertyEvaluator->EvaluateUshort();
    }
    unsigned int   EvaluateUint()    const
    {
      FPropertyEvaluator->Data( FCollection->Item( FIterator->EvaluateInt() ) );
      return FPropertyEvaluator->EvaluateUint();
    }
    unsigned long  EvaluateUlong()   const
    {
      FPropertyEvaluator->Data( FCollection->Item( FIterator->EvaluateInt() ) );
      return FPropertyEvaluator->EvaluateUlong();
    }
    const TEString& EvaluateString()     const
    {
      FPropertyEvaluator->Data( FCollection->Item( FIterator->EvaluateInt() ) );
      return FPropertyEvaluator->EvaluateString();
    }

    virtual ~TCollectionPropertyEvaluator()  {  ;  }
  };

// property types
const short mtUndefined  = -1,
            mtProperty   = 1,
            mtComplex    = 2,
            mtCollection = 3;

class IClassDefinition
{
public:
  virtual ~IClassDefinition()  {  ;  }
  // returns one of the above mtXXX constants
  virtual short GetMemberType(const TEString& propName);
  virtual TEString GetPropertyType(const TEString& propName);
  virtual TEString GetComplexType(const TEString& propName);
  virtual TEString GetCollectionType(const TEString& propName);
  virtual IEvaluator* GetPropertyEvaluator(const TEString& propName);
  virtual IDataProvider* GetComplexEvaluator(const TEString& propName);
  //virtual ICollection* GetCollectionEvaluator(const TEString& propName);

};

class TEvaluatorFactory  {
  TSStrPObjList<IClassDefinition*, true> ClassDefinitions;
public:
  TEvaluatorFactory() {  ;  }
  IEvaluator* Evaluator(const TEString &Val)  {
    TStrList toks(Val, '.');
    if( toks.Count() <= 1 )
    {
      // a special action has to be taken if operators are to be considered
      // in the future
      return NULL;
    }
    IClassDefinition * classDef = ClassDefinitions[toks.String(0)];
    //TODO: report the error
    if( !classDef )  return NULL;
    toks.Delete(0);
    return ProcessProperties( classDef, toks );
  }
protected:
  IEvaluator* ProcessProperties( IClassDefinition *classDef, TStrList props )
  {
    short memberType = classDef->GetMemberType( props.String(0) );
    switch( memberType )
    {
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
  IEvaluatorFactory()  {  ;  }
  virtual ~IEvaluatorFactory() {  ;  }
  virtual IEvaluator* Evaluator(const TEString &Val) = 0;
  virtual IEvaluable* Evaluable(const TEString &Val) = 0;
  virtual IEvaluator* Evaluator(const TEString& name, const TEString &Val) = 0;
  virtual IEvaluable* Evaluable(const TEString& name, const TEString &Val) = 0;
//  virtual ICollection* Collection(const TEString &Name) = 0;
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
  TcoGOperator(IEvaluator *left, IEvaluator *right)  {
    Left = left;
    Right = right;
    if( Left == NULL || Right == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, "NULL evaluator");
  }
  ~TcoGOperator()  {  ;  }
  bool Evaluate() const {  return *Left > *Right;  }
};
class TcoLOperator: public IEvaluable  {
  IEvaluator *Left, *Right;
public:
  TcoLOperator(IEvaluator *left, IEvaluator *right)  {
    Left = left;
    Right = right;
    if( Left == NULL || Right == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, "NULL evaluator");
  }
  ~TcoLOperator()  {  ;  }
  bool Evaluate() const {  return *Left < *Right;  }
};
class TcoNEOperator: public IEvaluable  {
  IEvaluator *Left, *Right;
public:
  TcoNEOperator(IEvaluator *left, IEvaluator *right)  {
    Left = left;
    Right = right;
    if( Left == NULL || Right == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, "NULL evaluator");
  }
  ~TcoNEOperator()  {  ;  }
  bool Evaluate() const {  return *Left != *Right;  }
};
class TcoGEOperator: public IEvaluable  {
  IEvaluator *Left, *Right;
public:
  TcoGEOperator(IEvaluator *left, IEvaluator *right)  {
    Left = left;
    Right = right;
    if( Left == NULL || Right == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, "NULL evaluator");
  }
  ~TcoGEOperator()  {  ;  }
  bool Evaluate() const {  return *Left >= *Right;  }
};
class TcoEOperator: public IEvaluable  {
  IEvaluator *Left, *Right;
public:
  TcoEOperator(IEvaluator *left, IEvaluator *right)  {
    Left = left;
    Right = right;
    if( Left == NULL || Right == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, "NULL evaluator");
  }
  ~TcoEOperator()  {  ;  }
  bool Evaluate() const {  return *Left == *Right;  }
};
class TcoLEOperator: public IEvaluable  {
  IEvaluator *Left, *Right;
public:
  TcoLEOperator(IEvaluator *left, IEvaluator *right)  {
    Left = left;
    Right = right;
    if( Left == NULL || Right == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, "NULL evaluator");
  }
  ~TcoLEOperator()  {  ;  }
  bool Evaluate() const {  return *Left <= *Right;  }
};
/* ___________________________________________________________________________*/
/* ___________________________________________________________________________*/
/* ________________LOGICAL OPERATORS IMPLEMENTATION___________________________*/
class TloAndOperator: public IEvaluable  {
  IEvaluable *OperandA, *OperandB;
public:
  TloAndOperator(IEvaluable *operandA, IEvaluable *operandB)  {
    OperandA = operandA;
    OperandB = operandB;
    if( OperandA == NULL || OperandB == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, "NULL evaluable");
  }
  ~TloAndOperator()  {  ;  }
  bool Evaluate()  const {
    if( !OperandA->Evaluate() )  return false;
    if( !OperandB->Evaluate() )  return false;
    return true;
  }
};
class TloOrOperator: public IEvaluable  {
  IEvaluable *OperandA, *OperandB;
public:
  TloOrOperator(IEvaluable *operandA, IEvaluable *operandB)  {
    OperandA = operandA;
    OperandB = operandB;
    if( OperandA == NULL || OperandB == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, "NULL evaluable");
  }
  ~TloOrOperator()  {  ;  }
  bool Evaluate() const {
    if( OperandA->Evaluate() )  return true;
    if( OperandB->Evaluate() )  return true;
    return false;
  }
};
class TloNotOperator: public IEvaluable  {
  IEvaluable *Operand;
public:
  TloNotOperator(IEvaluable *operand)  {
    Operand = operand;
    if( Operand == NULL )
      throw TInvalidArgumentException(__OlxSourceInfo, "NULL evaluable");
  }
  ~TloNotOperator()  {  ;  }
  bool Evaluate() const {  return !Operand->Evaluate();  }
};
/* ___________________________________________________________________________*/

// signgle argument operator factory
template <class OC, class IC, class AC>
  class TsaFactory: public TObjectFactory<OC>  {
  public:
    ~TsaFactory()  {  ;  }
    OC *NewInstance(TEList *Args)  {
      if( Args->Count() != 1 )  return NULL;  //TODO: throw exception
      return new IC( (AC*)Args->Item(0) );
    }
  };
// two argument operator factory
template <class OC, class IC, class AC>
  class TtaFactory: public TObjectFactory<OC>  {
  public:
    ~TtaFactory()  {  ;  }
    OC *NewInstance(TEList *Args)  {
      if( Args->Count() != 2 )  return NULL;  //TODO: throw exception
      return new IC( (AC*)Args->Item(0), (AC*)Args->Item(1) );
    }
  };


class TSyntaxParser  {
  IEvaluable *Root;
  IEvaluatorFactory *EvaluatorFactory;
  TPtrList<IEvaluable> Evaluables;
  TPtrList<IEvaluator> Evaluators;
  TSStrPObjList< TObjectFactory<IEvaluable>*, false > LogicalOperators, ComparisonOperators, ArithmeticFunctions;
  IEvaluator* CreateEvaluator(const TEString& expr, const TEString& args, const TEString& strval);
  IEvaluable* CreateEvaluable(const TEString& expr, const TEString& args, const TEString& strval);
protected:
  IEvaluable* SimpleParse(const TEString& Expression);
  void Clear();
public:
  TSyntaxParser(IEvaluatorFactory *FactoryInstance);
  ~TSyntaxParser();
  inline bool Evaluate()  {  return Root->Evaluate();  }
  inline IEvaluable *GetRoot()  {  return Root;  }
  void Parse(const TEString &Text);
};

/******************************************************************************/

#endif
