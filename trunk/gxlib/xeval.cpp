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
// constructor to create instaces of registered evaluators
TTXBond_EvaluatorFactory::TTXBond_EvaluatorFactory(IEvaluatorFactory *factoryRegister)  {
  FactoryRegister = factoryRegister;
  // register new instance of TXBond_LengthEvaluator
  Evaluators.Add("length", new TXBond_LengthEvaluator(this));
  // register new instance of TXBond_TypeEvaluator
  Evaluators.Add("type", new TXBond_TypeEvaluator(this));
  // register new instance of TXBond_DeletedEvaluator
  Evaluators.Add("deleted", new TXBond_DeletedEvaluator(this));
  // add new instance of data provider TXBond_AEvaluator
  TXBond_AEvaluator *tXBondAEvaluator = new TXBond_AEvaluator(this);
  DataProviders.Add("TXBond_AEvaluator", tXBondAEvaluator);
  IEvaluatorFactory *tXBondAEvaluatorFactory = FactoryRegister->Factory("TTSAtom_EvaluatorFactory");
  for( size_t i=0; i < tXBondAEvaluatorFactory->EvaluatorCount(); i++ )
    Evaluators.Add(olxstr("A.") << tXBondAEvaluatorFactory->EvaluatorName(i), tXBondAEvaluatorFactory->Evaluator(i)->NewInstance(tXBondAEvaluator));
  // add new instance of data provider TXBond_BEvaluator
  TXBond_BEvaluator *tXBondBEvaluator = new TXBond_BEvaluator(this);
  DataProviders.Add("TXBond_BEvaluator", tXBondBEvaluator);
  IEvaluatorFactory *tXBondBEvaluatorFactory = FactoryRegister->Factory("TTSAtom_EvaluatorFactory");
  for( size_t i=0; i < tXBondBEvaluatorFactory->EvaluatorCount(); i++ )
    Evaluators.Add(olxstr("B.") << tXBondBEvaluatorFactory->EvaluatorName(i), tXBondBEvaluatorFactory->Evaluator(i)->NewInstance(tXBondBEvaluator));
}
// constructor to create instaces of registered evaluators
TTXAtom_EvaluatorFactory::TTXAtom_EvaluatorFactory(IEvaluatorFactory *factoryRegister)  {
  FactoryRegister = factoryRegister;
  // register new instance of TXAtomLabelEvaluator
  // register new instance of TXAtomSelectedEvaluator
  Evaluators.Add("selected", new TXAtom_SelectedEvaluator(this));
  // add new instance of data provider TXAtomAtomEvaluator
  IEvaluatorFactory *tXAtomAtomEvaluatorFactory = FactoryRegister->Factory("TTSAtom_EvaluatorFactory");
  for( size_t i=0; i < tXAtomAtomEvaluatorFactory->EvaluatorCount(); i++ )
    Evaluators.Add(tXAtomAtomEvaluatorFactory->EvaluatorName(i), tXAtomAtomEvaluatorFactory->Evaluator(i)->NewInstance(this));
  // add new instance of data provider TXAtomBaiEvaluator
  TXAtom_BaiEvaluator *tXAtomBaiEvaluator = new TXAtom_BaiEvaluator(this);
  DataProviders.Add("TXAtomBaiEvaluator", tXAtomBaiEvaluator);
  IEvaluatorFactory *tXAtomBaiEvaluatorFactory = FactoryRegister->Factory("TTBasicAtomInfoEvaluatorFactory");
  for( size_t i=0; i < tXAtomBaiEvaluatorFactory->EvaluatorCount(); i++ )
    Evaluators.Add(olxstr("bai.") <<tXAtomBaiEvaluatorFactory->EvaluatorName(i), tXAtomBaiEvaluatorFactory->Evaluator(i)->NewInstance(tXAtomBaiEvaluator));
}
// constructor to create instaces of registered evaluators
TTGlGroupEvaluatorFactory::TTGlGroupEvaluatorFactory(IEvaluatorFactory *factoryRegister)
{
  FactoryRegister = factoryRegister;
  // add new instance of data provider TSelAEvaluator
  TSelAEvaluator *tSelAEvaluator = new TSelAEvaluator(this);
  DataProviders.Add("TSelAEvaluator", tSelAEvaluator);
  IEvaluatorFactory *tSelAEvaluatorFactory = FactoryRegister->Factory("TTXAtomEvaluatorFactory");
  for( size_t i=0; i < tSelAEvaluatorFactory->EvaluatorCount(); i++ )
    Evaluators.Add(olxstr("a.") << tSelAEvaluatorFactory->EvaluatorName(i), tSelAEvaluatorFactory->Evaluator(i)->NewInstance(tSelAEvaluator));
  // add new instance of data provider TSelBEvaluator
  TSelBEvaluator *tSelBEvaluator = new TSelBEvaluator(this);
  DataProviders.Add("TSelBEvaluator", tSelBEvaluator);
  IEvaluatorFactory *tSelBEvaluatorFactory = FactoryRegister->Factory("TTXBond_EvaluatorFactory");
  for( size_t i=0; i < tSelBEvaluatorFactory->EvaluatorCount(); i++ )
    Evaluators.Add(olxstr("b.") << tSelBEvaluatorFactory->EvaluatorName(i), tSelBEvaluatorFactory->Evaluator(i)->NewInstance(tSelBEvaluator));
}
