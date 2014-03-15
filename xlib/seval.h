/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xlib_seval_H
#define __olx_xlib_seval_H
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
class ITBasicAtomInfoDataProvider: public IDataProvider {
public:
  virtual const cm_Element *GetType() = 0;
  virtual void SetType(const cm_Element *) {
    throw TNotImplementedException(__OlxSourceInfo);
  }
};
class TTBasicAtomInfoDataProvider: public ITBasicAtomInfoDataProvider {
  const cm_Element *type;
public:
  const cm_Element *GetType() {
    if (type == NULL)
      throw TFunctionFailedException(__OlxSourceInfo, "uninitialised object");
    return type;
  }
  void SetType(const cm_Element* val)  {  type = val;  }
};
// data provider interface
class ITSAtom_DataProvider: public ITBasicAtomInfoDataProvider {
public:
  virtual TSAtom* GetTSAtom() = 0;
  virtual void SetTSAtom(TSAtom *val) {
    throw TNotImplementedException(__OlxSourceInfo);
  }
  const cm_Element *GetType() { return &GetTSAtom()->GetType(); }
};
class TTSAtom_DataProvider: public ITSAtom_DataProvider {
  TSAtom *SAtom;
public:
  virtual TSAtom* GetTSAtom() {
    if (SAtom == NULL)
      throw TFunctionFailedException(__OlxSourceInfo, "uninitialised object");
    return SAtom;
  }
  void SetTSAtom(TSAtom *val) { SAtom = val; }
};

template <class data_provider_t, class evaluator_t, class impl_t>
struct TEvaluator : public evaluator_t {
  data_provider_t *Parent;
  TEvaluator(data_provider_t* parent) : Parent(parent) {
    if (Parent == NULL)
      throw TInvalidArgumentException(__OlxSourceInfo, "null Parent");
  }
  IEvaluator *NewInstance(IDataProvider *dp) {
    return new impl_t(dynamic_cast<data_provider_t*>(
      dp->cast(typeid(data_provider_t))));
  }
  typedef TEvaluator<data_provider_t, evaluator_t, impl_t> parent_t;
};
// evaluator implementation for scalar part
class TSAtom_PartEvaluator
  : public TEvaluator<ITSAtom_DataProvider,IDoubleEvaluator,TSAtom_PartEvaluator> {
public:
  TSAtom_PartEvaluator(ITSAtom_DataProvider* parent)
    : parent_t(parent)
  {}
  double EvaluateDouble() const {
    return Parent->GetTSAtom()->CAtom().GetPart();
  }
};

// evaluator implementation for scalar uiso
class TSAtom_TypeEvaluator
  : public TEvaluator<ITSAtom_DataProvider,IStringEvaluator,TSAtom_TypeEvaluator> {
public:
  TSAtom_TypeEvaluator(ITSAtom_DataProvider* parent)
    : parent_t(parent)
  {}
  const olxstr& EvaluateString() const {
    return Parent->GetTSAtom()->GetType().symbol;
  }
};
// evaluator implementation for scalar uiso
class TSAtom_UisoEvaluator
  : public TEvaluator<ITSAtom_DataProvider,IDoubleEvaluator,TSAtom_UisoEvaluator> {
public:
  TSAtom_UisoEvaluator(ITSAtom_DataProvider* parent)
    : parent_t(parent)
  {}
  double EvaluateDouble() const {
    return Parent->GetTSAtom()->CAtom().GetUiso();
  }
};
// evaluator implementation for string label
class TSAtom_LabelEvaluator
  : public TEvaluator<ITSAtom_DataProvider,IStringEvaluator,TSAtom_LabelEvaluator> {
public:
  TSAtom_LabelEvaluator(ITSAtom_DataProvider* parent)
    : parent_t(parent)
  {}
  const olxstr& EvaluateString() const {
    return Parent->GetTSAtom()->GetLabel();
  }
};
// evaluator implementation for scalar peak
class TSAtom_PeakEvaluator
  : public TEvaluator<ITSAtom_DataProvider,IDoubleEvaluator,TSAtom_PeakEvaluator> {
public:
  TSAtom_PeakEvaluator(ITSAtom_DataProvider* parent)
    : parent_t(parent)
  {}
  double EvaluateDouble() const {
    return Parent->GetTSAtom()->CAtom().GetQPeak();
  }
};
// evaluator implementation for scalar peak
class TSAtom_OccuEvaluator
  : public TEvaluator<ITSAtom_DataProvider,IDoubleEvaluator,TSAtom_OccuEvaluator> {
public:
  TSAtom_OccuEvaluator(ITSAtom_DataProvider* parent)
    : parent_t(parent)
  {}
  double EvaluateDouble() const {
    return Parent->GetTSAtom()->CAtom().GetChemOccu();
  }
};
// evaluator implementation for scalar afix
class TSAtom_AfixEvaluator
  : public TEvaluator<ITSAtom_DataProvider,IDoubleEvaluator,TSAtom_AfixEvaluator> {
public:
  TSAtom_AfixEvaluator(ITSAtom_DataProvider* parent)
    : parent_t(parent)
  {}
  double EvaluateDouble() const {
    return Parent->GetTSAtom()->CAtom().GetAfix();
  }
};
// evaluator implementation for scalar bc
class TSAtom_BcEvaluator
  : public TEvaluator<ITSAtom_DataProvider,IDoubleEvaluator,TSAtom_BcEvaluator> {
public:
  TSAtom_BcEvaluator(ITSAtom_DataProvider* parent)
    : parent_t(parent)
  {}
  double EvaluateDouble() const {
    return (double)Parent->GetTSAtom()->BondCount();
  }
};

// evaluator implementation for bool selected
class TTSAtom_EvaluatorFactory: public IEvaluatorFactory {
  // the list of all evaluators
  olxstr_dict<IEvaluator*, true> Evaluators;
  olxstr_dict<IDataProvider*, true> DataProviders;
  IEvaluatorFactory *FactoryRegister;
public:
  IEvaluator *Evaluator(const olxstr &propertyName) {
    return Evaluators.Find(propertyName, NULL);
  }
  IEvaluator *Evaluator(size_t index) {
    return Evaluators.GetValue(index);
  }
  const olxstr& EvaluatorName(size_t index) {
    return Evaluators.GetKey(index);
  }
  size_t EvaluatorCount() {  return Evaluators.Count();  }
  ~TTSAtom_EvaluatorFactory()  {
    for( size_t i=0; i < Evaluators.Count(); i++ )
      delete Evaluators.GetValue(i);
    for( size_t i=0; i < DataProviders.Count(); i++ )
      delete DataProviders.GetValue(i);
    delete provider;
  }
  // constructor to create instances of registered evaluators
  TTSAtom_EvaluatorFactory(IEvaluatorFactory *parent,
    ITSAtom_DataProvider *provider);
  ITSAtom_DataProvider *provider;
};

// evaluator implementation for scalar mw
class TBaiMwEvaluator
  : public TEvaluator<ITBasicAtomInfoDataProvider,IDoubleEvaluator,TBaiMwEvaluator> {
public:
  TBaiMwEvaluator(ITBasicAtomInfoDataProvider* parent)
    : parent_t(parent)
  {}
  double EvaluateDouble() const {  return Parent->GetType()->GetMr();  }
};
// evaluator implementation for scalar z
class TBaiZEvaluator
  : public TEvaluator<ITBasicAtomInfoDataProvider,IDoubleEvaluator,TBaiZEvaluator> {
public:
  TBaiZEvaluator(ITBasicAtomInfoDataProvider* parent)
    : parent_t(parent)
  {}
  double EvaluateDouble() const {  return Parent->GetType()->z;  }
};
// evaluator implementation for string name
class TBaiNameEvaluator
  : public TEvaluator<ITBasicAtomInfoDataProvider,IStringEvaluator,TBaiNameEvaluator> {
public:
  TBaiNameEvaluator(ITBasicAtomInfoDataProvider* parent)
    : parent_t(parent)
  {}
  const olxstr& EvaluateString() const {  return Parent->GetType()->name;  }
};
// evaluator implementation for string type
class TBaiTypeEvaluator
  : public TEvaluator<ITBasicAtomInfoDataProvider,IStringEvaluator,TBaiTypeEvaluator> {
public:
  TBaiTypeEvaluator(ITBasicAtomInfoDataProvider* parent)
    : parent_t(parent)
  {}
  const olxstr& EvaluateString() const {  return Parent->GetType()->symbol;  }
};

// factory class implementation
class TTBasicAtomInfoEvaluatorFactory : public IEvaluatorFactory {
  // the list of all evaluators
  olxstr_dict<IEvaluator*, true> Evaluators;
  olxstr_dict<IDataProvider*, true> DataProviders;
  IEvaluatorFactory *FactoryRegister;
public:
  IEvaluator *Evaluator(const olxstr & propertyName)  {
    return Evaluators.Find(propertyName, NULL);
  }
  IEvaluator *Evaluator(size_t index)  { return Evaluators.GetValue(index); }
  const olxstr& EvaluatorName(size_t index)  {  return Evaluators.GetKey(index);  }
  size_t EvaluatorCount()  {  return Evaluators.Count();  }
  ~TTBasicAtomInfoEvaluatorFactory()  {
    for( size_t i=0; i < Evaluators.Count(); i++ )
      delete Evaluators.GetValue(i);
    for( size_t i=0; i < DataProviders.Count(); i++ )
      delete DataProviders.GetValue(i);
    delete provider;
  }
  // constructor to create instaces of registered evaluators
  TTBasicAtomInfoEvaluatorFactory(IEvaluatorFactory *parent,
    ITBasicAtomInfoDataProvider *provider);
  ITBasicAtomInfoDataProvider *provider;
};

class TSFactoryRegister: public IEvaluatorFactory  {
protected:
  olxstr_dict<IEvaluatorFactory*, true> Factories;
  olxstr_dict<IEvaluatorFactory*, true> FactoryMap;
public:
  TSFactoryRegister();
  ~TSFactoryRegister();
  IEvaluatorFactory *Factory(const olxstr &name) {
    return Factories.Find(name, NULL);
  }
  IEvaluatorFactory *BindingFactory(const olxstr &name) {
    return FactoryMap.Find(name, NULL);
  }
  size_t EvaluatorCount()  {  return 0;  }
  IEvaluator *Evaluator(size_t index)  {
    throw TNotImplementedException(__OlxSourceInfo);
  }
  const olxstr& EvaluatorName(size_t index)  {
    throw TNotImplementedException(__OlxSourceInfo);
  }
  IEvaluator *Evaluator(const olxstr& name);
};

#endif
