#ifndef xevalH
#define xevalH
#include "xbond.h"
#include "xatom.h"
#include "glgroup.h"
#include "seval.h"

// atomaticaly generted code
class TXFactoryRegister;
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
// factory class implementation
class TTXBond_EvaluatorFactory: public IEvaluatorFactory, ITXBond_DataProvider
{
  TXBond *XBond;
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
  TXBond *GetTXBond(){  return XBond;  }
  // variable setter
  void SetTXBond_(TXBond *val)  {  XBond = val;  }
  // destructor
  ~TTXBond_EvaluatorFactory()  {
    for( size_t i=0; i < Evaluators.Count(); i++ )
      delete Evaluators.GetObject(i);
    for( size_t i=0; i < DataProviders.Count(); i++ )
      delete DataProviders.GetObject(i);
  }
  // constructor to create instaces of registered evaluators
  TTXBond_EvaluatorFactory(IEvaluatorFactory *parent);
};
// factory class implementation
class TTXAtom_EvaluatorFactory: public IEvaluatorFactory, ITXAtom_DataProvider
{
  TXAtom *XAtom;
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
  TXAtom *GetTXAtom(){  return XAtom;  }
  // variable setter
  void SetTXAtom(TXAtom *val)  {  XAtom = val;  }
  // destructor
  ~TTXAtom_EvaluatorFactory()  {
    for( size_t i=0; i < Evaluators.Count(); i++ )
      delete Evaluators.GetObject(i);
    for( size_t i=0; i < DataProviders.Count(); i++ )
      delete DataProviders.GetObject(i);
  }
  // constructor to create instaces of registered evaluators
  TTXAtom_EvaluatorFactory(IEvaluatorFactory *parent);
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
  bool EvaluateBool() const {  return Parent->GetTXAtom()->IsSelected();  }
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
  TXBond *GetTXBond()  {return &(TXBond&)Parent->GetTGlGroup()->GetObject(0);  }
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
  double EvaluateDouble() const {  return (double)Parent->GetTXAtom()->Atom().BondCount();  }
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
  TXAtom *GetTXAtom()  {return &(TXAtom&)Parent->GetTGlGroup()->GetObject(0);  }
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
// factory class implementation
class TTGlGroupEvaluatorFactory: public IEvaluatorFactory, ITGlGroupDataProvider
{
  TGlGroup *sel;
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
  TGlGroup *GetTGlGroup(){  return sel;  }
  // variable setter
  void SetTGlGroup(TGlGroup *val)  {  sel = val;  }
  // destructor
  ~TTGlGroupEvaluatorFactory()  {
    for( size_t i=0; i < Evaluators.Count(); i++ )
      delete Evaluators.GetObject(i);
    for( size_t i=0; i < DataProviders.Count(); i++ )
      delete DataProviders.GetObject(i);
  }
  // constructor to create instaces of registered evaluators
  TTGlGroupEvaluatorFactory(IEvaluatorFactory *parent);
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
  bool EvaluateBool() const {  return Parent->GetTXBond()->IsDeleted();  }
};

class TXFactoryRegister : public TSFactoryRegister {
public:
  TXFactoryRegister()  {
    TTBasicAtomInfoEvaluatorFactory *tTBasicAtomInfoEvaluatorFactory = new TTBasicAtomInfoEvaluatorFactory(this);
    Factories.Add("TTBasicAtomInfoEvaluatorFactory", tTBasicAtomInfoEvaluatorFactory);
    FactoryMap.Add("bai", tTBasicAtomInfoEvaluatorFactory);
    TTSAtom_EvaluatorFactory *tTSAtom_EvaluatorFactory = new TTSAtom_EvaluatorFactory(this);
    Factories.Add("TTSAtom_EvaluatorFactory", tTSAtom_EvaluatorFactory);
    FactoryMap.Add("SAtom", tTSAtom_EvaluatorFactory);
    TTXAtom_EvaluatorFactory *tTXAtomEvaluatorFactory = new TTXAtom_EvaluatorFactory(this);
    Factories.Add("TTXAtomEvaluatorFactory", tTXAtomEvaluatorFactory);
    FactoryMap.Add("XAtom", tTXAtomEvaluatorFactory);
    TTXBond_EvaluatorFactory *tTXBond_EvaluatorFactory = new TTXBond_EvaluatorFactory(this);
    Factories.Add("TTXBond_EvaluatorFactory", tTXBond_EvaluatorFactory);
    FactoryMap.Add("XBond", tTXBond_EvaluatorFactory);
    TTGlGroupEvaluatorFactory *tTGlGroupEvaluatorFactory = new TTGlGroupEvaluatorFactory(this);
    Factories.Add("TTGlGroupEvaluatorFactory", tTGlGroupEvaluatorFactory);
    FactoryMap.Add("sel", tTGlGroupEvaluatorFactory);
  }
};

#endif
