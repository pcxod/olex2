#ifndef sevalH
#define sevalH
#include "sbond.h"
#include "satom.h"
#include "sparser.h"

// atomaticaly generted code
class TSFactoryRegister;
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
// data provider interface
class ITBasicAtomInfoDataProvider: public IDataProvider  {
public:
  virtual TBasicAtomInfo *GetTBasicAtomInfo() = 0;
};
// data provider interface
class ITSAtom_DataProvider: public IDataProvider  {
public:
  virtual TSAtom* GetTSAtom() = 0;
};
// evaluator implementation for scalar part
class TSAtom_PartEvaluator: public IDoubleEvaluator  {
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
class TTSAtom_EvaluatorFactory: public IEvaluatorFactory, ITSAtom_DataProvider
{
  TSAtom* SAtom;
  // the list of all evaluators
  TSStrPObjList<olxstr,IEvaluator*, true> Evaluators;
  TSStrPObjList<olxstr,IDataProvider*, true> DataProviders;
  IEvaluatorFactory *FactoryRegister;
public:
  IEvaluator *Evaluator(const olxstr & propertyName)  {  return Evaluators[propertyName];  }
  IEvaluator *Evaluator(size_t index)  {  return Evaluators.GetObject(index);  }
  const olxstr& EvaluatorName(size_t index)  {  return Evaluators.GetComparable(index);  }
  size_t EvaluatorCount() {  return Evaluators.Count();  }
  // variable getter, to be used by evaluators
  TSAtom* GetTSAtom(){  return SAtom;  }
  // variable setter
  void SetTSAtom_(TSAtom* val)  {  SAtom = val;  }
  // destructor
  ~TTSAtom_EvaluatorFactory()  {
    for( size_t i=0; i < Evaluators.Count(); i++ )
      delete Evaluators.GetObject(i);
    for( size_t i=0; i < DataProviders.Count(); i++ )
      delete DataProviders.GetObject(i);
  }
  // constructor to create instaces of registered evaluators
  TTSAtom_EvaluatorFactory(IEvaluatorFactory *parent);
};
// factory class implementation
class TTBasicAtomInfoEvaluatorFactory: public IEvaluatorFactory, ITBasicAtomInfoDataProvider
{
  TBasicAtomInfo *bai;
  // the list of all evaluators
  TSStrPObjList<olxstr,IEvaluator*, true> Evaluators;
  TSStrPObjList<olxstr,IDataProvider*, true> DataProviders;
  IEvaluatorFactory *FactoryRegister;
public:
  IEvaluator *Evaluator(const olxstr & propertyName)  {  return Evaluators[propertyName];  }
  IEvaluator *Evaluator(size_t index)  {  return Evaluators.GetObject(index);  }
  const olxstr& EvaluatorName(size_t index)  {  return Evaluators.GetComparable(index);  }
  size_t EvaluatorCount()  {  return Evaluators.Count();  }
  // variable getter, to be used by evaluators
  TBasicAtomInfo *GetTBasicAtomInfo(){  return bai;  }
  // variable setter
  void SetTBasicAtomInfo(TBasicAtomInfo *val)  {  bai = val;  }
  // destructor
  ~TTBasicAtomInfoEvaluatorFactory()  {
    for( size_t i=0; i < Evaluators.Count(); i++ )
      delete Evaluators.GetObject(i);
    for( size_t i=0; i < DataProviders.Count(); i++ )
      delete DataProviders.GetObject(i);
  }
  // constructor to create instaces of registered evaluators
  TTBasicAtomInfoEvaluatorFactory(IEvaluatorFactory *parent);
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
  double EvaluateDouble() const {  return (double)Parent->GetTSAtom()->BondCount();  }
};

class TSFactoryRegister: public IEvaluatorFactory  {
protected:
  TSStrPObjList<olxstr,IEvaluatorFactory*, true> Factories;
  TSStrPObjList<olxstr,IEvaluatorFactory*, true> FactoryMap;
public:
  TSFactoryRegister()  {
    TTBasicAtomInfoEvaluatorFactory *tTBasicAtomInfoEvaluatorFactory = new TTBasicAtomInfoEvaluatorFactory(this);
    Factories.Add("TTBasicAtomInfoEvaluatorFactory", tTBasicAtomInfoEvaluatorFactory);
    FactoryMap.Add("bai", tTBasicAtomInfoEvaluatorFactory);
    TTSAtom_EvaluatorFactory *tTSAtom_EvaluatorFactory = new TTSAtom_EvaluatorFactory(this);
    Factories.Add("TTSAtom_EvaluatorFactory", tTSAtom_EvaluatorFactory);
    FactoryMap.Add("SAtom", tTSAtom_EvaluatorFactory);
  }
  ~TSFactoryRegister();
  IEvaluatorFactory *Factory(const olxstr &name) {  return Factories[name];  }
  IEvaluatorFactory *BindingFactory(const olxstr &name) {  return FactoryMap[name];  }
  size_t EvaluatorCount()  {  return 0;  }
  IEvaluator *Evaluator(size_t index)  {  return NULL;  }
  const olxstr& EvaluatorName(size_t index)  {  static olxstr rubbish;  return rubbish;  }
  IEvaluator *Evaluator(const olxstr& name);
};

#endif
