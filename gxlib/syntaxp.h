//---------------------------------------------------------------------------

#ifndef syntaxpH
#define syntaxpH
#include "xbond.h"
#include "xatom.h"
#include "glgroup.h"
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
  class TObjectFactory
  {
  public:
    virtual ~TObjectFactory()  {  ;  }
    virtual IC *NewInstance(TEList *Arguments) = 0;
  };

class TOperatorSignature
{
public:
  olxstr StringValue;
  short    ShortValue;
  TOperatorSignature(const short shortVal, const olxstr &strVal);

};
// an array to use in parsing

extern TOperatorSignature DefinedFunctions[];
extern short DefinedFunctionCount;

class IDataProvider
{
public:
  virtual ~IDataProvider()  {  ;  }
};

class IEvaluable
{
public:
  virtual ~IEvaluable() {  ;  }
  virtual bool Evaluate() = 0;
};


// an abstract class for evaluation simple expressions
class IEvaluator: public IEObject  {
public:
  virtual ~IEvaluator() {  ;  }

  class TUnsupportedOperator: public TBasicException  {
  public:
    TUnsupportedOperator(const olxstr& location):
        TBasicException(location, EmptyString)  {  ;  }
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


  class TCastException: public TBasicException  {
  public:
    TCastException(const olxstr& location ):
        TBasicException(location, EmptyString)  { ;  }
    virtual IEObject* Replicate()  const {  return new TCastException(*this);  }
  };
  virtual short EvaluateShort()          const  {  throw TCastException(__OlxSourceInfo);  }
  virtual int EvaluateInt()              const  {  throw TCastException(__OlxSourceInfo);  }
  virtual long EvaluateLong()            const  {  throw TCastException(__OlxSourceInfo);  }
  virtual float EvaluateFloat()          const  {  throw TCastException(__OlxSourceInfo);  }
  virtual double EvaluateDouble()        const  {  throw TCastException(__OlxSourceInfo);  }
  virtual unsigned short EvaluateUshort()const  {  throw TCastException(__OlxSourceInfo);  }
  virtual unsigned int EvaluateUint()    const  {  throw TCastException(__OlxSourceInfo);  }
  virtual unsigned long EvaluateUlong()  const  {  throw TCastException(__OlxSourceInfo);  }
  virtual bool EvaluateBool()            const  {  throw TCastException(__OlxSourceInfo);  }
  virtual const olxstr& EvaluateString()     const  {  throw TCastException(__OlxSourceInfo);  }
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
class IStringEvaluator: public IEvaluator
{
public:
  ~IStringEvaluator() {  ;  }
  bool operator == (const IEvaluator &val) const  {  return !EvaluateString().Comparei( val.EvaluateString() );  }
  bool operator != (const IEvaluator &val) const  {  return EvaluateString().Comparei( val.EvaluateString() ) != 0;  }
  bool operator > (const IEvaluator &val) const   {  return EvaluateString().Comparei( val.EvaluateString() ) > 0;  }
  bool operator >= (const IEvaluator &val) const  {  return EvaluateString().Comparei( val.EvaluateString() ) >= 0;  }
  bool operator < (const IEvaluator &val) const   {  return EvaluateString().Comparei( val.EvaluateString() ) < 0;  }
  bool operator <= (const IEvaluator &val) const  {  return EvaluateString().Comparei( val.EvaluateString() ) <= 0;  }
  const olxstr& EvaluateString() const = 0;
};

class IDoubleEvaluator: public IEvaluator
{
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

class IFloatEvaluator: public IEvaluator
{
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

class IIntEvaluator: public IEvaluator
{
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

class IShortEvaluator: public IEvaluator
{
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

class IUshortEvaluator: public IEvaluator
{
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

class IUintEvaluator: public IEvaluator
{
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

class IUlongEvaluator: public IEvaluator
{
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

class IBoolEvaluator: public IEvaluator
{
public:
  bool operator == (const IEvaluator &val) const  {  return EvaluateBool() == val.EvaluateBool();  }
  bool operator != (const IEvaluator &val) const  {  return EvaluateBool() != val.EvaluateBool();  }
  bool   EvaluateBool()            const = 0;
};

class TStringEvaluator: public IStringEvaluator
{
  olxstr Value;
public:
  TStringEvaluator(const olxstr &Val)  {  Value = Val;  }
  ~TStringEvaluator() {  ;  }
  const olxstr& EvaluateString() const {  return Value;  }
  void SetValue(const olxstr& v)  {  Value = v;  }
  IEvaluator *NewInstance(IDataProvider *) {  return new TStringEvaluator(Value);  }
};

class TScalarEvaluator: public IDoubleEvaluator
{
  double Value;
public:
  TScalarEvaluator(double Val)  {  Value = Val;  }
  ~TScalarEvaluator() {  ;  }
  double EvaluateDouble() const {  return Value;  }
  void SetValue(double v)  {  Value = v;  }
  IEvaluator *NewInstance(IDataProvider *) {  return new TScalarEvaluator(Value);  }
};

class TBoolEvaluator: public IBoolEvaluator
{
  bool Value;
public:
  TBoolEvaluator(bool Val)  {  Value = Val;  }
  ~TBoolEvaluator() {  ;  }
  bool   EvaluateBool() const {  return Value;  }
  void SetValue( bool v )  {  Value = v;  }
  IEvaluator *NewInstance(IDataProvider *) {  return new TBoolEvaluator (Value);  }
};

template <class PropertyProviderClass, class EvaluatorClass>
  class TPropertyEvaluator: public EvaluatorClass
  {
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
  class ACollection
  {
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
  class TCollectionPropertyEvaluator: public IEvaluator
  {
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
    const olxstr& EvaluateString()     const
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
  IEvaluator* Evaluator(const olxstr &Val)  {
    TStrList toks(Val, '.');
    if( toks.Count() <= 1 )
    {
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

class IEvaluatorFactory
{
public:
  IEvaluatorFactory()  {  ;  }
  virtual int EvaluatorCount() = 0;
  virtual IEvaluator *Evaluator(int index) = 0;
  virtual const olxstr& EvaluatorName(int index) = 0;
  virtual ~IEvaluatorFactory() {  ;  }
  virtual IEvaluator* Evaluator(const olxstr &Val) = 0;
//  virtual ICollection* Collection(const olxstr &Name) = 0;
};

/*
a class to peform comparison operations on one (!) or two objects
*/
class TArithmeticOperator: public IEvaluator
{
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
  bool Evaluate() {  return *Left > *Right;  }
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
  bool Evaluate() {  return *Left < *Right;  }
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
  bool Evaluate() {  return *Left != *Right;  }
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
  bool Evaluate() {  return *Left >= *Right;  }
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
  bool Evaluate() {  return *Left == *Right;  }
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
  bool Evaluate() {  return *Left <= *Right;  }
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
  bool Evaluate()  {
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
  bool Evaluate()  {
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
  bool Evaluate()  {  return !Operand->Evaluate();  }
};
/* ___________________________________________________________________________*/

// signgle argument operator factory
template <class OC, class IC, class AC>
  class TsaFactory: public TObjectFactory<OC>
  {
  public:
    ~TsaFactory()  {  ;  }
    OC *NewInstance(TEList *Args)
    {
      if( Args->Count() != 1 )  return NULL;  //TODO: throw exception
      return new IC( (AC*)Args->Item(0) );
    }
  };
// two argument operator factory
template <class OC, class IC, class AC>
  class TtaFactory: public TObjectFactory<OC>
  {
  public:
    ~TtaFactory()  {  ;  }
    OC *NewInstance(TEList *Args)
    {
      if( Args->Count() != 2 )  return NULL;  //TODO: throw exception
      return new IC( (AC*)Args->Item(0), (AC*)Args->Item(1) );
    }
  };


class TSyntaxParser
{
  IEvaluable *Root;
  IEvaluatorFactory *EvaluatorFactory;
  TEList Evaluables;
  TEList Evaluators;
  TStrList FErrors;
  TSStrPObjList<olxstr,TObjectFactory<IEvaluable>*, false > LogicalOperators, ComparisonOperators, ArithmeticFunctions;
protected:
  IEvaluable* SimpleParse(const olxstr& Expression);
public:
  TSyntaxParser(IEvaluatorFactory *FactoryInstance, const olxstr &Text);
  ~TSyntaxParser();
  bool Evaluate()  {  return Root->Evaluate();  }

  const TStrList& Errors() const  {  return FErrors;  }
};

/******************************************************************************/
// atomaticaly generted code
class TFactoryRegister;
class ITXAtom_DataProvider;
class TTXAtom_EvaluatorFactory;
class TXAtom_LabelEvaluator;
class TXAtom_TypeEvaluator;
class TXAtom_PartEvaluator;
class TXAtom_AfixEvaluator;
class TXAtom_UisoEvaluator;
class TXAtom_PeakEvaluator;
class TXAtom_BcEvaluator;
class TXAtom_SelectedEvaluator;
class TXAtom_AtomEvaluator;
class TXAtom_BaiEvaluator;
class ITSAtom_DataProvider;
class TTSAtom_EvaluatorFactory;
class TSAtom_LabelEvaluator;
class TSAtom_TypeEvaluator;
class TSAtom_PartEvaluator;
class TSAtom_AfixEvaluator;
class TSAtom_UisoEvaluator;
class TSAtom_PeakEvaluator;
class TSAtom_BcEvaluator;
class ITBasicAtomInfoDataProvider;
class TTBasicAtomInfoEvaluatorFactory;
class TBaiTypeEvaluator;
class TBaiNameEvaluator;
class TBaiMwEvaluator;
class ITGlGroupDataProvider;
class TTGlGroupEvaluatorFactory;
class TSelAEvaluator;
class TSelBEvaluator;
class ITXBond_DataProvider;
class TTXBond_EvaluatorFactory;
class TXBond_LengthEvaluator;
class TXBond_TypeEvaluator;
class TXBond_DeletedEvaluator;
class TXBond_AEvaluator;
class TXBond_BEvaluator;
// data provider interface
class ITXAtom_DataProvider: public IDataProvider
{
public:
  virtual TXAtom *GetTXAtom() = 0;
};
// data provider interface
class ITBasicAtomInfoDataProvider: public IDataProvider
{
public:
  virtual TBasicAtomInfo *GetTBasicAtomInfo() = 0;
};
// data provider interface
class ITGlGroupDataProvider: public IDataProvider
{
public:
  virtual TGlGroup *GetTGlGroup() = 0;
};
// data provider interface
class ITXBond_DataProvider: public IDataProvider
{
public:
  virtual TXBond *GetTXBond() = 0;
};
// data provider interface
class ITSAtom_DataProvider: public IDataProvider
{
public:
  virtual TSAtom* GetTSAtom() = 0;
};
// factory class implementation
class TTXBond_EvaluatorFactory: public IEvaluatorFactory, ITXBond_DataProvider
{
  TXBond *XBond;
  // the list of all evaluators
  TSStrPObjList<olxstr,IEvaluator*, true> Evaluators;
  TSStrPObjList<olxstr,IDataProvider*, true> DataProviders;
  TFactoryRegister *FactoryRegister;
public:
  IEvaluator *Evaluator(const olxstr & propertyName)  {  return Evaluators[propertyName];  }
  IEvaluator *Evaluator(int index)  {  return Evaluators.GetObject(index);  }
  const olxstr& EvaluatorName(int index)  {  return Evaluators.GetComparable(index);  }
  int EvaluatorCount()  {  return Evaluators.Count();  }
  // variable getter, to be used by evaluators
  TXBond *GetTXBond(){  return XBond;  }
  // variable setter
  void SetTXBond_(TXBond *val)  {  XBond = val;  }
  // destructor
  ~TTXBond_EvaluatorFactory()  {
    for( int i=0; i < Evaluators.Count(); i++ )
      delete Evaluators.GetObject(i);
    for( int i=0; i < DataProviders.Count(); i++ )
      delete DataProviders.GetObject(i);
  }
  // constructor to create instaces of registered evaluators
  TTXBond_EvaluatorFactory(TFactoryRegister *parent);
};
// factory class implementation
class TTXAtom_EvaluatorFactory: public IEvaluatorFactory, ITXAtom_DataProvider
{
  TXAtom *XAtom;
  // the list of all evaluators
  TSStrPObjList<olxstr,IEvaluator*, true> Evaluators;
  TSStrPObjList<olxstr,IDataProvider*, true> DataProviders;
  TFactoryRegister *FactoryRegister;
public:
  IEvaluator *Evaluator(const olxstr & propertyName)  {  return Evaluators[propertyName];  }
  IEvaluator *Evaluator(int index)  {  return Evaluators.GetObject(index);  }
  const olxstr& EvaluatorName(int index)  {  return Evaluators.GetComparable(index);  }
  int EvaluatorCount()  {  return Evaluators.Count();  }
  // variable getter, to be used by evaluators
  TXAtom *GetTXAtom(){  return XAtom;  }
  // variable setter
  void SetTXAtom(TXAtom *val)  {  XAtom = val;  }
  // destructor
  ~TTXAtom_EvaluatorFactory()
  {
    for( int i=0; i < Evaluators.Count(); i++ )
      delete Evaluators.GetObject(i);
    for( int i=0; i < DataProviders.Count(); i++ )
      delete DataProviders.GetObject(i);
  }
  // constructor to create instaces of registered evaluators
  TTXAtom_EvaluatorFactory(TFactoryRegister *parent);
};
// evaluator implementation for scalar part
class TSAtom_PartEvaluator: public IDoubleEvaluator
{
  ITSAtom_DataProvider *Parent;
public:
  // constructor
  TSAtom_PartEvaluator(ITSAtom_DataProvider* parent) { Parent = parent;  }
  // virtual method
  IEvaluator *NewInstance( IDataProvider *dp)  {  return new TSAtom_PartEvaluator( (ITSAtom_DataProvider*)dp);  }
  // destructor
  TSAtom_PartEvaluator()  {  ;  }
  // evaluator function
  double EvaluateDouble() const {  return Parent->GetTSAtom()->CAtom().GetPart();  }
};
// evaluator implementation for string name
class TBaiNameEvaluator: public IStringEvaluator
{
  ITBasicAtomInfoDataProvider *Parent;
public:
  // constructor
  TBaiNameEvaluator(ITBasicAtomInfoDataProvider* parent) { Parent = parent;  }
  // virtual method
  IEvaluator *NewInstance( IDataProvider *dp)  {  return new TBaiNameEvaluator( (ITBasicAtomInfoDataProvider*)dp);  }
  // destructor
  TBaiNameEvaluator()  {  ;  }
  // evaluator function
  const olxstr& EvaluateString() const {  return Parent->GetTBasicAtomInfo()->GetName();  }
};
// evaluator implementation for scalar uiso
class TXAtom_UisoEvaluator: public IDoubleEvaluator
{
  ITXAtom_DataProvider *Parent;
public:
  // constructor
  TXAtom_UisoEvaluator(ITXAtom_DataProvider* parent) { Parent = parent;  }
  // virtual method
  IEvaluator *NewInstance( IDataProvider *dp)  {  return new TXAtom_UisoEvaluator( (ITXAtom_DataProvider*)dp);  }
  // destructor
  TXAtom_UisoEvaluator()  {  ;  }
  // evaluator function
  double EvaluateDouble() const {  return Parent->GetTXAtom()->Atom().CAtom().GetUiso();  }
};
// evaluator implementation for string type
class TSAtom_TypeEvaluator: public IStringEvaluator
{
  ITSAtom_DataProvider *Parent;
public:
  // constructor
  TSAtom_TypeEvaluator(ITSAtom_DataProvider* parent) { Parent = parent;  }
  // virtual method
  IEvaluator *NewInstance( IDataProvider *dp)  {  return new TSAtom_TypeEvaluator( (ITSAtom_DataProvider*)dp);  }
  // destructor
  TSAtom_TypeEvaluator()  {  ;  }
  // evaluator function
  const olxstr& EvaluateString() const {  return Parent->GetTSAtom()->GetAtomInfo().GetSymbol();  }
};
// evaluator implementation for bool selected
class TXAtom_SelectedEvaluator: public IBoolEvaluator
{
  ITXAtom_DataProvider *Parent;
public:
  // constructor
  TXAtom_SelectedEvaluator(ITXAtom_DataProvider* parent) { Parent = parent;  }
  // virtual method
  IEvaluator *NewInstance( IDataProvider *dp)  {  return new TXAtom_SelectedEvaluator( (ITXAtom_DataProvider*)dp);  }
  // destructor
  TXAtom_SelectedEvaluator()  {  ;  }
  // evaluator function
  bool EvaluateBool() const {  return Parent->GetTXAtom()->Selected();  }
};
// evaluator implementation for scalar length
class TXBond_LengthEvaluator: public IDoubleEvaluator
{
  ITXBond_DataProvider *Parent;
public:
  // constructor
  TXBond_LengthEvaluator(ITXBond_DataProvider* parent) { Parent = parent;  }
  // virtual method
  IEvaluator *NewInstance( IDataProvider *dp)  {  return new TXBond_LengthEvaluator( (ITXBond_DataProvider*)dp);  }
  // destructor
  TXBond_LengthEvaluator()  {  ;  }
  // evaluator function
  double EvaluateDouble() const {  return Parent->GetTXBond()->Bond().Length();  }
};
// factory class implementation
class TTSAtom_EvaluatorFactory: public IEvaluatorFactory, ITSAtom_DataProvider
{
  TSAtom* SAtom;
  // the list of all evaluators
  TSStrPObjList<olxstr,IEvaluator*, true> Evaluators;
  TSStrPObjList<olxstr,IDataProvider*, true> DataProviders;
  TFactoryRegister *FactoryRegister;
public:
  IEvaluator *Evaluator(const olxstr & propertyName)  {  return Evaluators[propertyName];  }
  IEvaluator *Evaluator(int index)  {  return Evaluators.GetObject(index);  }
  const olxstr& EvaluatorName(int index)  {  return Evaluators.GetComparable(index);  }
  int EvaluatorCount() {  return Evaluators.Count();  }
  // variable getter, to be used by evaluators
  TSAtom* GetTSAtom(){  return SAtom;  }
  // variable setter
  void SetTSAtom_(TSAtom* val)  {  SAtom = val;  }
  // destructor
  ~TTSAtom_EvaluatorFactory()
  {
    for( int i=0; i < Evaluators.Count(); i++ )
      delete Evaluators.GetObject(i);
    for( int i=0; i < DataProviders.Count(); i++ )
      delete DataProviders.GetObject(i);
  }
  // constructor to create instaces of registered evaluators
  TTSAtom_EvaluatorFactory(TFactoryRegister *parent);
};
// evaluator implementation for complex atom
class TXAtom_AtomEvaluator: public ITSAtom_DataProvider
{
  ITXAtom_DataProvider *Parent;
public:
  // constructor
  TXAtom_AtomEvaluator(ITXAtom_DataProvider* parent) { Parent = parent;  }
  // destructor
  TXAtom_AtomEvaluator()  {  ;  }
  // evaluator function
  TSAtom* GetTSAtom()  {  return &Parent->GetTXAtom()->Atom();  }
};
// evaluator implementation for complex b
class TSelBEvaluator: public ITXBond_DataProvider
{
  ITGlGroupDataProvider *Parent;
public:
  // constructor
  TSelBEvaluator(ITGlGroupDataProvider* parent) { Parent = parent;  }
  // destructor
  TSelBEvaluator()  {  ;  }
  // evaluator function
  TXBond *GetTXBond()  {return (TXBond*)Parent->GetTGlGroup()->Object(0);  }
};
// evaluator implementation for scalar bc
class TXAtom_BcEvaluator: public IDoubleEvaluator
{
  ITXAtom_DataProvider *Parent;
public:
  // constructor
  TXAtom_BcEvaluator(ITXAtom_DataProvider* parent) { Parent = parent;  }
  // virtual method
  IEvaluator *NewInstance( IDataProvider *dp)  {  return new TXAtom_BcEvaluator( (ITXAtom_DataProvider*)dp);  }
  // destructor
  TXAtom_BcEvaluator()  {  ;  }
  // evaluator function
  double EvaluateDouble() const {  return Parent->GetTXAtom()->Atom().BondCount();  }
};
// factory class implementation
class TTBasicAtomInfoEvaluatorFactory: public IEvaluatorFactory, ITBasicAtomInfoDataProvider
{
  TBasicAtomInfo *bai;
  // the list of all evaluators
  TSStrPObjList<olxstr,IEvaluator*, true> Evaluators;
  TSStrPObjList<olxstr,IDataProvider*, true> DataProviders;
  TFactoryRegister *FactoryRegister;
public:
  IEvaluator *Evaluator(const olxstr & propertyName)  {  return Evaluators[propertyName];  }
  IEvaluator *Evaluator(int index)  {  return Evaluators.GetObject(index);  }
  const olxstr& EvaluatorName(int index)  {  return Evaluators.GetComparable(index);  }
  int EvaluatorCount()  {  return Evaluators.Count();  }
  // variable getter, to be used by evaluators
  TBasicAtomInfo *GetTBasicAtomInfo(){  return bai;  }
  // variable setter
  void SetTBasicAtomInfo(TBasicAtomInfo *val)  {  bai = val;  }
  // destructor
  ~TTBasicAtomInfoEvaluatorFactory()
  {
    for( int i=0; i < Evaluators.Count(); i++ )
      delete Evaluators.GetObject(i);
    for( int i=0; i < DataProviders.Count(); i++ )
      delete DataProviders.GetObject(i);
  }
  // constructor to create instaces of registered evaluators
  TTBasicAtomInfoEvaluatorFactory(TFactoryRegister *parent);
};
// evaluator implementation for complex bai
class TXAtom_BaiEvaluator: public ITBasicAtomInfoDataProvider
{
  ITXAtom_DataProvider *Parent;
public:
  // constructor
  TXAtom_BaiEvaluator(ITXAtom_DataProvider* parent) { Parent = parent;  }
  // destructor
  TXAtom_BaiEvaluator()  {  ;  }
  // evaluator function
  TBasicAtomInfo *GetTBasicAtomInfo()  {return &Parent->GetTXAtom()->Atom().GetAtomInfo();  }
};
// evaluator implementation for scalar peak
class TXAtom_PeakEvaluator: public IDoubleEvaluator
{
  ITXAtom_DataProvider *Parent;
public:
  // constructor
  TXAtom_PeakEvaluator(ITXAtom_DataProvider* parent) { Parent = parent;  }
  // virtual method
  IEvaluator *NewInstance( IDataProvider *dp)  {  return new TXAtom_PeakEvaluator( (ITXAtom_DataProvider*)dp);  }
  // destructor
  TXAtom_PeakEvaluator()  {  ;  }
  // evaluator function
  double EvaluateDouble() const {  return Parent->GetTXAtom()->Atom().CAtom().GetQPeak();  }
};
// evaluator implementation for scalar uiso
class TSAtom_UisoEvaluator: public IDoubleEvaluator
{
  ITSAtom_DataProvider *Parent;
public:
  // constructor
  TSAtom_UisoEvaluator(ITSAtom_DataProvider* parent) { Parent = parent;  }
  // virtual method
  IEvaluator *NewInstance( IDataProvider *dp)  {  return new TSAtom_UisoEvaluator( (ITSAtom_DataProvider*)dp);  }
  // destructor
  TSAtom_UisoEvaluator()  {  ;  }
  // evaluator function
  double EvaluateDouble() const {  return Parent->GetTSAtom()->CAtom().GetUiso();  }
};
// evaluator implementation for string label
class TSAtom_LabelEvaluator: public IStringEvaluator
{
  ITSAtom_DataProvider *Parent;
public:
  // constructor
  TSAtom_LabelEvaluator(ITSAtom_DataProvider* parent) { Parent = parent;  }
  // virtual method
  IEvaluator *NewInstance( IDataProvider *dp)  {  return new TSAtom_LabelEvaluator( (ITSAtom_DataProvider*)dp);  }
  // destructor
  TSAtom_LabelEvaluator()  {  ;  }
  // evaluator function
  const olxstr& EvaluateString() const {  return Parent->GetTSAtom()->GetLabel();  }
};
// evaluator implementation for string type
class TBaiTypeEvaluator: public IStringEvaluator
{
  ITBasicAtomInfoDataProvider *Parent;
public:
  // constructor
  TBaiTypeEvaluator(ITBasicAtomInfoDataProvider* parent) { Parent = parent;  }
  // virtual method
  IEvaluator *NewInstance( IDataProvider *dp)  {  return new TBaiTypeEvaluator( (ITBasicAtomInfoDataProvider*)dp);  }
  // destructor
  TBaiTypeEvaluator()  {  ;  }
  // evaluator function
  const olxstr& EvaluateString() const {  return Parent->GetTBasicAtomInfo()->GetSymbol();  }
};
// evaluator implementation for scalar peak
class TSAtom_PeakEvaluator: public IDoubleEvaluator
{
  ITSAtom_DataProvider *Parent;
public:
  // constructor
  TSAtom_PeakEvaluator(ITSAtom_DataProvider* parent) { Parent = parent;  }
  // virtual method
  IEvaluator *NewInstance( IDataProvider *dp)  {  return new TSAtom_PeakEvaluator( (ITSAtom_DataProvider*)dp);  }
  // destructor
  TSAtom_PeakEvaluator()  {  ;  }
  // evaluator function
  double EvaluateDouble() const {  return Parent->GetTSAtom()->CAtom().GetQPeak();  }
};
// evaluator implementation for complex a
class TSelAEvaluator: public ITXAtom_DataProvider
{
  ITGlGroupDataProvider *Parent;
public:
  // constructor
  TSelAEvaluator(ITGlGroupDataProvider* parent) { Parent = parent;  }
  // destructor
  TSelAEvaluator()  {  ;  }
  // evaluator function
  TXAtom *GetTXAtom()  {return (TXAtom*)Parent->GetTGlGroup()->Object(0);  }
};
// evaluator implementation for scalar mw
class TBaiMwEvaluator: public IDoubleEvaluator
{
  ITBasicAtomInfoDataProvider *Parent;
public:
  // constructor
  TBaiMwEvaluator(ITBasicAtomInfoDataProvider* parent) { Parent = parent;  }
  // virtual method
  IEvaluator *NewInstance( IDataProvider *dp)  {  return new TBaiMwEvaluator( (ITBasicAtomInfoDataProvider*)dp);  }
  // destructor
  TBaiMwEvaluator()  {  ;  }
  // evaluator function
  double EvaluateDouble() const {  return Parent->GetTBasicAtomInfo()->GetMr();  }
};
// evaluator implementation for scalar afix
class TXAtom_AfixEvaluator: public IDoubleEvaluator
{
  ITXAtom_DataProvider *Parent;
public:
  // constructor
  TXAtom_AfixEvaluator(ITXAtom_DataProvider* parent) { Parent = parent;  }
  // virtual method
  IEvaluator *NewInstance( IDataProvider *dp)  {  return new TXAtom_AfixEvaluator( (ITXAtom_DataProvider*)dp);  }
  // destructor
  TXAtom_AfixEvaluator()  {  ;  }
  // evaluator function
  double EvaluateDouble() const {  return Parent->GetTXAtom()->Atom().CAtom().GetAfix();  }
};
// evaluator implementation for string label
class TXAtom_LabelEvaluator: public IStringEvaluator
{
  ITXAtom_DataProvider *Parent;
public:
  // constructor
  TXAtom_LabelEvaluator(ITXAtom_DataProvider* parent) { Parent = parent;  }
  // virtual method
  IEvaluator *NewInstance( IDataProvider *dp)  {  return new TXAtom_LabelEvaluator( (ITXAtom_DataProvider*)dp);  }
  // destructor
  TXAtom_LabelEvaluator()  {  ;  }
  // evaluator function
  const olxstr& EvaluateString() const {  return Parent->GetTXAtom()->Atom().GetLabel();  }
};
// evaluator implementation for scalar type
class TXBond_TypeEvaluator: public IDoubleEvaluator
{
  ITXBond_DataProvider *Parent;
public:
  // constructor
  TXBond_TypeEvaluator(ITXBond_DataProvider* parent) { Parent = parent;  }
  // virtual method
  IEvaluator *NewInstance( IDataProvider *dp)  {  return new TXBond_TypeEvaluator( (ITXBond_DataProvider*)dp);  }
  // destructor
  TXBond_TypeEvaluator()  {  ;  }
  // evaluator function
  double EvaluateDouble() const {  return Parent->GetTXBond()->Bond().GetType();  }
};
// evaluator implementation for scalar part
class TXAtom_PartEvaluator: public IDoubleEvaluator
{
  ITXAtom_DataProvider *Parent;
public:
  // constructor
  TXAtom_PartEvaluator(ITXAtom_DataProvider* parent) { Parent = parent;  }
  // virtual method
  IEvaluator *NewInstance( IDataProvider *dp)  {  return new TXAtom_PartEvaluator( (ITXAtom_DataProvider*)dp);  }
  // destructor
  TXAtom_PartEvaluator()  {  ;  }
  // evaluator function
  double EvaluateDouble() const {  return Parent->GetTXAtom()->Atom().CAtom().GetPart();  }
};
// evaluator implementation for scalar afix
class TSAtom_AfixEvaluator: public IDoubleEvaluator
{
  ITSAtom_DataProvider *Parent;
public:
  // constructor
  TSAtom_AfixEvaluator(ITSAtom_DataProvider* parent) { Parent = parent;  }
  // virtual method
  IEvaluator *NewInstance( IDataProvider *dp)  {  return new TSAtom_AfixEvaluator( (ITSAtom_DataProvider*)dp);  }
  // destructor
  TSAtom_AfixEvaluator()  {  ;  }
  // evaluator function
  double EvaluateDouble() const {  return Parent->GetTSAtom()->CAtom().GetAfix();  }
};
// factory class implementation
class TTGlGroupEvaluatorFactory: public IEvaluatorFactory, ITGlGroupDataProvider
{
  TGlGroup *sel;
  // the list of all evaluators
  TSStrPObjList<olxstr,IEvaluator*, true> Evaluators;
  TSStrPObjList<olxstr,IDataProvider*, true> DataProviders;
  TFactoryRegister *FactoryRegister;
public:
  IEvaluator *Evaluator(const olxstr & propertyName)  {  return Evaluators[propertyName];  }
  IEvaluator *Evaluator(int index)  {  return Evaluators.GetObject(index);  }
  const olxstr& EvaluatorName(int index)  {  return Evaluators.GetComparable(index);  }
  int EvaluatorCount()  {  return Evaluators.Count();  }
  // variable getter, to be used by evaluators
  TGlGroup *GetTGlGroup(){  return sel;  }
  // variable setter
  void SetTGlGroup(TGlGroup *val)  {  sel = val;  }
  // destructor
  ~TTGlGroupEvaluatorFactory()
  {
    for( int i=0; i < Evaluators.Count(); i++ )
      delete Evaluators.GetObject(i);
    for( int i=0; i < DataProviders.Count(); i++ )
      delete DataProviders.GetObject(i);
  }
  // constructor to create instaces of registered evaluators
  TTGlGroupEvaluatorFactory(TFactoryRegister *parent);
};
// evaluator implementation for complex A
class TXBond_AEvaluator: public ITSAtom_DataProvider
{
  ITXBond_DataProvider *Parent;
public:
  // constructor
  TXBond_AEvaluator(ITXBond_DataProvider* parent) { Parent = parent;  }
  // destructor
  TXBond_AEvaluator()  {  ;  }
  // evaluator function
  TSAtom* GetTSAtom()  {return &Parent->GetTXBond()->Bond().A();  }
};
// evaluator implementation for scalar bc
class TSAtom_BcEvaluator: public IDoubleEvaluator
{
  ITSAtom_DataProvider *Parent;
public:
  // constructor
  TSAtom_BcEvaluator(ITSAtom_DataProvider* parent) { Parent = parent;  }
  // virtual method
  IEvaluator *NewInstance( IDataProvider *dp)  {  return new TSAtom_BcEvaluator( (ITSAtom_DataProvider*)dp);  }
  // destructor
  TSAtom_BcEvaluator()  {  ;  }
  // evaluator function
  double EvaluateDouble() const {  return Parent->GetTSAtom()->BondCount();  }
};
// evaluator implementation for string type
class TXAtom_TypeEvaluator: public IStringEvaluator
{
  ITXAtom_DataProvider *Parent;
public:
  // constructor
  TXAtom_TypeEvaluator(ITXAtom_DataProvider* parent) { Parent = parent;  }
  // virtual method
  IEvaluator *NewInstance( IDataProvider *dp)  {  return new TXAtom_TypeEvaluator( (ITXAtom_DataProvider*)dp);  }
  // destructor
  TXAtom_TypeEvaluator()  {  ;  }
  // evaluator function
  const olxstr& EvaluateString() const {  return Parent->GetTXAtom()->Atom().GetAtomInfo().GetSymbol();  }
};
// evaluator implementation for complex B
class TXBond_BEvaluator: public ITSAtom_DataProvider
{
  ITXBond_DataProvider *Parent;
public:
  // constructor
  TXBond_BEvaluator(ITXBond_DataProvider* parent) { Parent = parent;  }
  // destructor
  TXBond_BEvaluator()  {  ;  }
  // evaluator function
  TSAtom* GetTSAtom()  {  return &Parent->GetTXBond()->Bond().B();  }
};
// evaluator implementation for bool deleted
class TXBond_DeletedEvaluator: public IBoolEvaluator
{
  ITXBond_DataProvider *Parent;
public:
  // constructor
  TXBond_DeletedEvaluator(ITXBond_DataProvider* parent) { Parent = parent;  }
  // virtual method
  IEvaluator *NewInstance( IDataProvider *dp)  {  return new TXBond_DeletedEvaluator( (ITXBond_DataProvider*)dp);  }
  // destructor
  TXBond_DeletedEvaluator()  {  ;  }
  // evaluator function
  bool EvaluateBool() const {  return Parent->GetTXBond()->Deleted();  }
};

class TFactoryRegister: public IEvaluatorFactory
{
  TSStrPObjList<olxstr,IEvaluatorFactory*, true> Factories;
  TSStrPObjList<olxstr,IEvaluatorFactory*, true> FactoryMap;
public:
  IEvaluatorFactory *Factory(const olxstr &name) {  return Factories[name];  }
  IEvaluatorFactory *BindingFactory(const olxstr &name) {  return FactoryMap[name];  }
  TFactoryRegister();
  ~TFactoryRegister();
  int EvaluatorCount()  {  return 0;  }
  IEvaluator *Evaluator(int index)  {  return NULL;  }
  const olxstr& EvaluatorName(int index)  {  static olxstr rubbish;  return rubbish;  }
  IEvaluator *Evaluator(const olxstr& name);
};

#endif
