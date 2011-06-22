/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xgl_xeval_H
#define __olx_xgl_xeval_H
#include "xbond.h"
#include "xatom.h"
#include "glgroup.h"
#include "seval.h"

// data provider interface
class ITXAtom_DataProvider: public IDataProvider  {
public:
  virtual TXAtom *GetTXAtom() = 0;
};
// data provider interface
class ITGlGroupDataProvider: public IDataProvider  {
public:
  virtual TGlGroup *GetTGlGroup() = 0;
};
// data provider interface
class ITXBond_DataProvider: public IDataProvider  {
public:
  virtual TXBond *GetTXBond() = 0;
  virtual TSBond *GetTSBond()  {  return GetTXBond();  }
};
// factory class implementation
class TTXBond_EvaluatorFactory: public IEvaluatorFactory, ITXBond_DataProvider  {
  TXBond *XBond;
  // the list of all evaluators
  TSStrPObjList<olxstr,IEvaluator*, true> Evaluators;
  TSStrPObjList<olxstr,IDataProvider*, true> DataProviders;
  IEvaluatorFactory *FactoryRegister;
public:
  IEvaluator *Evaluator(const olxstr & propertyName)  {  return Evaluators[propertyName];  }
  IEvaluator *Evaluator(size_t index)  {  return Evaluators.GetObject(index);  }
  const olxstr& EvaluatorName(size_t index)  {  return Evaluators.GetKey(index);  }
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
class TTXAtom_EvaluatorFactory: public IEvaluatorFactory, ITXAtom_DataProvider  {
  TXAtom *XAtom;
  // the list of all evaluators
  TSStrPObjList<olxstr,IEvaluator*, true> Evaluators;
  TSStrPObjList<olxstr,IDataProvider*, true> DataProviders;
  IEvaluatorFactory *FactoryRegister;
public:
  IEvaluator *Evaluator(const olxstr & propertyName)  {  return Evaluators[propertyName];  }
  IEvaluator *Evaluator(size_t index)  {  return Evaluators.GetObject(index);  }
  const olxstr& EvaluatorName(size_t index)  {  return Evaluators.GetKey(index);  }
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
// evaluator implementation for bool selected
class TXAtom_SelectedEvaluator: public IBoolEvaluator  {
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

class TXAtom_BaiEvaluator: public ITBasicAtomInfoDataProvider  {
  ITXAtom_DataProvider *Parent;
public:
  TXAtom_BaiEvaluator(ITXAtom_DataProvider* parent) { Parent = parent;  }
  TXAtom_BaiEvaluator()  {  ;  }
  const cm_Element *GetType()  {return &Parent->GetTXAtom()->GetType();  }
};

// evaluator implementation for complex b
class TSelBEvaluator: public ITXBond_DataProvider  {
  ITGlGroupDataProvider *Parent;
public:
  // constructor
  TSelBEvaluator(ITGlGroupDataProvider* parent) { Parent = parent;  }
  // destructor
  TSelBEvaluator()  {  ;  }
  // evaluator function
  TXBond *GetTXBond()  {return &(TXBond&)Parent->GetTGlGroup()->GetObject(0);  }
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
  double EvaluateDouble() const {  return Parent->GetTXBond()->GetType();  }
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
  const olxstr& EvaluatorName(size_t index)  {  return Evaluators.GetKey(index);  }
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
  TSAtom* GetTSAtom()  {return &Parent->GetTXBond()->A();  }
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
  TSAtom* GetTSAtom()  {  return &Parent->GetTXBond()->B();  }
};
// evaluator implementation for bool deleted
class TXBond_DeletedEvaluator: public IBoolEvaluator
{
  ITXBond_DataProvider *Parent;
public:
  // constructor
  TXBond_DeletedEvaluator(ITXBond_DataProvider* parent) { Parent = parent;  }
  // virtual method
  IEvaluator *NewInstance(IDataProvider *dp)  {  return new TXBond_DeletedEvaluator( (ITXBond_DataProvider*)dp);  }
  // destructor
  TXBond_DeletedEvaluator()  {  ;  }
  // evaluator function
  bool EvaluateBool() const {  return Parent->GetTXBond()->IsDeleted();  }
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
  double EvaluateDouble() const {  return Parent->GetTXBond()->Length();  }
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
