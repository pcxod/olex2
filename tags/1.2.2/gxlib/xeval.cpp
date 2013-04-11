/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "xeval.h"

/* automaticaly generated code  */
// constructor to create instances of registered evaluators
TTXBond_EvaluatorFactory::TTXBond_EvaluatorFactory(
  IEvaluatorFactory *factoryRegister, ITXBond_DataProvider *provider_)
  : provider(provider_)
{
  FactoryRegister = factoryRegister;
  // register new instance of TXBond_LengthEvaluator
  Evaluators.Add("length", new TXBond_LengthEvaluator(provider));
  // register new instance of TXBond_TypeEvaluator
  Evaluators.Add("type", new TXBond_TypeEvaluator(provider));
  // register new instance of TXBond_DeletedEvaluator
  Evaluators.Add("deleted", new TXBond_DeletedEvaluator(provider));
  // add new instance of data provider TXBond_AEvaluator
  TXBond_AtomEvaluator<1> *tXBondAEvaluator = new TXBond_AtomEvaluator<1>(
    provider);
  DataProviders.Add("TXBond_AEvaluator", tXBondAEvaluator);
  IEvaluatorFactory *tXBondAEvaluatorFactory =
    FactoryRegister->Factory("TTXAtomEvaluatorFactory");
  for( size_t i=0; i < tXBondAEvaluatorFactory->EvaluatorCount(); i++ ) {
    Evaluators.Add(olxstr("A.") << tXBondAEvaluatorFactory->EvaluatorName(i),
      tXBondAEvaluatorFactory->Evaluator(i)->NewInstance(tXBondAEvaluator));
  }
  TXBond_AtomEvaluator<2> *tXBondBEvaluator = new TXBond_AtomEvaluator<2>(provider);
  DataProviders.Add("TXBond_BEvaluator", tXBondBEvaluator);
  IEvaluatorFactory *tXBondBEvaluatorFactory =
    FactoryRegister->Factory("TTXAtomEvaluatorFactory");
  for( size_t i=0; i < tXBondBEvaluatorFactory->EvaluatorCount(); i++ ) {
    Evaluators.Add(olxstr("B.") << tXBondBEvaluatorFactory->EvaluatorName(i),
      tXBondBEvaluatorFactory->Evaluator(i)->NewInstance(tXBondBEvaluator));
  }
}
// constructor to create instances of registered evaluators
TTXAtom_EvaluatorFactory::TTXAtom_EvaluatorFactory(IEvaluatorFactory *factoryRegister,
  ITXAtom_DataProvider *provider_)
  : provider(provider_)
{
  FactoryRegister = factoryRegister;
  Evaluators.Add("selected", new TXAtom_SelectedEvaluator(provider));
  Evaluators.Add("visible", new TXAtom_VisibleEvaluator(provider));
  IEvaluatorFactory *tXAtomAtomEvaluatorFactory =
    FactoryRegister->Factory("TTSAtom_EvaluatorFactory");
  for( size_t i=0; i < tXAtomAtomEvaluatorFactory->EvaluatorCount(); i++ ) {
    Evaluators.Add(tXAtomAtomEvaluatorFactory->EvaluatorName(i),
      tXAtomAtomEvaluatorFactory->Evaluator(i)->NewInstance(provider));
  }
}
TTGlGroupEvaluatorFactory::TTGlGroupEvaluatorFactory(
  IEvaluatorFactory *factoryRegister)
{
  FactoryRegister = factoryRegister;
  TTXAtom_EvaluatorFactory *aef =
    new TTXAtom_EvaluatorFactory(factoryRegister,
    providers.Add(new TTGlGroupDataProvider));
  TTXBond_EvaluatorFactory *bef =
    new TTXBond_EvaluatorFactory(factoryRegister,
      providers.Add(new TTGlGroupDataProvider));
  Factories.Add("atom", aef);
  Factories.Add("bond", bef);
  for (size_t i=0; i < aef->EvaluatorCount(); i++)
    Evaluators.Add(olxstr("a.") << aef->EvaluatorName(i), aef->Evaluator(i));
  for (size_t i=0; i < bef->EvaluatorCount(); i++)
    Evaluators.Add(olxstr("b.") << bef->EvaluatorName(i), bef->Evaluator(i));
}
//.............................................................................
TXFactoryRegister::TXFactoryRegister()  {
  TTBasicAtomInfoEvaluatorFactory *tTBasicAtomInfoEvaluatorFactory =
    new TTBasicAtomInfoEvaluatorFactory(this, new TTBasicAtomInfoDataProvider);
  Factories.Add("TTBasicAtomInfoEvaluatorFactory", tTBasicAtomInfoEvaluatorFactory);
  FactoryMap.Add("bai", tTBasicAtomInfoEvaluatorFactory);
  TTSAtom_EvaluatorFactory *tTSAtom_EvaluatorFactory =
    new TTSAtom_EvaluatorFactory(this, new TTSAtom_DataProvider);
  Factories.Add("TTSAtom_EvaluatorFactory", tTSAtom_EvaluatorFactory);
  FactoryMap.Add("SAtom", tTSAtom_EvaluatorFactory);
  TTXAtom_EvaluatorFactory *tTXAtomEvaluatorFactory =
    new TTXAtom_EvaluatorFactory(this, new TTXAtom_DataProvider);
  Factories.Add("TTXAtomEvaluatorFactory", tTXAtomEvaluatorFactory);
  FactoryMap.Add("XAtom", tTXAtomEvaluatorFactory);
  TTXBond_EvaluatorFactory *tTXBond_EvaluatorFactory =
    new TTXBond_EvaluatorFactory(this, new TTXBond_DataProvider);
  Factories.Add("TTXBond_EvaluatorFactory", tTXBond_EvaluatorFactory);
  FactoryMap.Add("XBond", tTXBond_EvaluatorFactory);
  TTGlGroupEvaluatorFactory *tTGlGroupEvaluatorFactory =
    new TTGlGroupEvaluatorFactory(this);
  Factories.Add("TTGlGroupEvaluatorFactory", tTGlGroupEvaluatorFactory);
  FactoryMap.Add("sel", tTGlGroupEvaluatorFactory);
}
