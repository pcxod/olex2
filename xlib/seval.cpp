/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "seval.h"
#include "asymmunit.h"
#include "residue.h"
#include "atomref.h"
#include "lattice.h"

IEvaluator* TSFactoryRegister::Evaluator(const olxstr& name) {
  TStrList toks(name, '.');
  IEvaluatorFactory* factory = FactoryMap.Find(toks[0], 0);
  if (factory == 0) {
    olxstr en = toks.Text('.');
    for (size_t i = 0; i < FactoryMap.Count(); i++) {
      IEvaluator* e = FactoryMap.GetValue(i)->Evaluator(en);
      if (e != 0) {
        return e;
      }
    }
    return 0;
  }
  else {
    toks.Delete(0);
    return factory->Evaluator(toks.Text('.'));
  }
}
// constructor to create instaces of registered evaluators
TTSAtom_EvaluatorFactory::TTSAtom_EvaluatorFactory(
  IEvaluatorFactory *factoryRegister, ITSAtom_DataProvider *provider_)
  : provider(provider_)
{
  FactoryRegister = factoryRegister;
  // register new instance of TSAtom_LabelEvaluator
  Evaluators.Add("label", new TSAtom_LabelEvaluator(provider));
  // register new instance of TSAtom_TypeEvaluator
  Evaluators.Add("type", new TSAtom_TypeEvaluator(provider));
  // register new instance of TSAtom_PartEvaluator
  Evaluators.Add("part", new TSAtom_PartEvaluator(provider));
  // register new instance of TSAtom_AfixEvaluator
  Evaluators.Add("afix", new TSAtom_AfixEvaluator(provider));
  // register new instance of TSAtom_UisoEvaluator
  Evaluators.Add("uiso", new TSAtom_UisoEvaluator(provider));
  // register new instance of TSAtom_PeakEvaluator
  Evaluators.Add("peak", new TSAtom_PeakEvaluator(provider));
  Evaluators.Add("occu", new TSAtom_OccuEvaluator(provider));
  // register new instance of TSAtom_BcEvaluator
  Evaluators.Add("bc", new TSAtom_BcEvaluator(provider));
  Evaluators.Add("rc", new TSAtom_ResiClassEvaluator(provider));
  Evaluators.Add("rn", new TSAtom_ResiNumEvaluator(provider));

  IEvaluatorFactory *tSAtomBaiEvaluatorFactory =
    FactoryRegister->FindFactory("TTBasicAtomInfoEvaluatorFactory");
  for( size_t i=0; i < tSAtomBaiEvaluatorFactory->EvaluatorCount(); i++ ) {
    Evaluators.Add(olxstr("bai.") << tSAtomBaiEvaluatorFactory->EvaluatorName(i),
      tSAtomBaiEvaluatorFactory->Evaluator(i)->NewInstance(provider));
  }
}
// constructor to create instaces of registered evaluators
TTBasicAtomInfoEvaluatorFactory::TTBasicAtomInfoEvaluatorFactory(
  IEvaluatorFactory *factoryRegister, ITBasicAtomInfoDataProvider *provider_)
  : provider(provider_)
{
  FactoryRegister = factoryRegister;
  // register new instance of TBaiTypeEvaluator
  Evaluators.Add("type", new TBaiTypeEvaluator(provider));
  // register new instance of TBaiNameEvaluator
  Evaluators.Add("name", new TBaiNameEvaluator(provider));
  // register new instance of TBaiMwEvaluator
  Evaluators.Add("mw", new TBaiMwEvaluator(provider));
  // register new instance of TBaiZEvaluator
  Evaluators.Add("z", new TBaiZEvaluator(provider));
}
//.............................................................................
TSFactoryRegister::TSFactoryRegister() {
  TTBasicAtomInfoEvaluatorFactory* tTBasicAtomInfoEvaluatorFactory =
    new TTBasicAtomInfoEvaluatorFactory(this, new TTBasicAtomInfoDataProvider);
  Factories.Add("TTBasicAtomInfoEvaluatorFactory",
    tTBasicAtomInfoEvaluatorFactory);
  FactoryMap.Add("bai", tTBasicAtomInfoEvaluatorFactory);
  TTSAtom_EvaluatorFactory* tTSAtom_EvaluatorFactory =
    new TTSAtom_EvaluatorFactory(this, new TTSAtom_DataProvider);
  Factories.Add("TTSAtom_EvaluatorFactory", tTSAtom_EvaluatorFactory);
  FactoryMap.Add("Atom", tTSAtom_EvaluatorFactory);
}
//.............................................................................
TSFactoryRegister::~TSFactoryRegister() {
  for (size_t i = 0; i < Factories.Count(); i++) {
    delete Factories.GetValue(i);
  }
}
//.............................................................................
//.............................................................................
//.............................................................................
const olxstr& TSAtom_ResiClassEvaluator::EvaluateString() const {
  const TCAtom& a = Parent->GetTSAtom()->CAtom();
  return a.GetResiId() == 0 ? EmptyString()
    : a.GetParent()->GetResidue(a.GetResiId()).GetClassName();
}
//.............................................................................
double TSAtom_ResiNumEvaluator::EvaluateDouble() const {
  const TCAtom& a = Parent->GetTSAtom()->CAtom();
  return a.GetResiId() == 0 ? 0
    : a.GetParent()->GetResidue(a.GetResiId()).GetNumber();
}
//.............................................................................
bool TSAtom_TypeEvaluator::operator == (const IEvaluator& val) const {
  olxstr v = val.EvaluateString();
  if (v.StartsFrom('$')) {
    SortedElementPList elms = TAtomReference::DecodeTypes(
      v.SubStringFrom(1), Parent->GetTSAtom()->GetParent().GetAsymmUnit());
    return elms.Contains(Parent->GetTSAtom()->GetType());
  }
  return !EvaluateString().Comparei(val.EvaluateString());
}
//.............................................................................
