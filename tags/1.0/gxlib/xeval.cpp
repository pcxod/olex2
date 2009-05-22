#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "xeval.h"

/* automaticaly generated code  */
// constructor to create instaces of registered evaluators
TTXBond_EvaluatorFactory::TTXBond_EvaluatorFactory(IEvaluatorFactory *factoryRegister)  {
  FactoryRegister = factoryRegister;
  // register new instance of TXBond_LengthEvaluator
  Evaluators.Add("length", (IEvaluator*const&)new TXBond_LengthEvaluator(this));
  // register new instance of TXBond_TypeEvaluator
  Evaluators.Add("type", (IEvaluator*const&)new TXBond_TypeEvaluator(this));
  // register new instance of TXBond_DeletedEvaluator
  Evaluators.Add("deleted", (IEvaluator*const&)new TXBond_DeletedEvaluator(this));
  // add new instance of data provider TXBond_AEvaluator
  TXBond_AEvaluator *tXBondAEvaluator = new TXBond_AEvaluator(this);
  DataProviders.Add("TXBond_AEvaluator", tXBondAEvaluator);
  IEvaluatorFactory *tXBondAEvaluatorFactory = FactoryRegister->Factory("TTSAtom_EvaluatorFactory");
  for( int i=0; i < tXBondAEvaluatorFactory->EvaluatorCount(); i++ )
  {
    Evaluators.Add(olxstr("A.") << tXBondAEvaluatorFactory->EvaluatorName(i), tXBondAEvaluatorFactory->Evaluator(i)->NewInstance(tXBondAEvaluator));
  }
  // add new instance of data provider TXBond_BEvaluator
  TXBond_BEvaluator *tXBondBEvaluator = new TXBond_BEvaluator(this);
  DataProviders.Add("TXBond_BEvaluator", tXBondBEvaluator);
  IEvaluatorFactory *tXBondBEvaluatorFactory = FactoryRegister->Factory("TTSAtom_EvaluatorFactory");
  for( int i=0; i < tXBondBEvaluatorFactory->EvaluatorCount(); i++ )
  {
    Evaluators.Add(olxstr("B.") << tXBondBEvaluatorFactory->EvaluatorName(i), tXBondBEvaluatorFactory->Evaluator(i)->NewInstance(tXBondBEvaluator));
  }
}
// constructor to create instaces of registered evaluators
TTXAtom_EvaluatorFactory::TTXAtom_EvaluatorFactory(IEvaluatorFactory *factoryRegister)  {
  FactoryRegister = factoryRegister;
  // register new instance of TXAtomLabelEvaluator
  Evaluators.Add("label", (IEvaluator*const&)new TXAtom_LabelEvaluator(this));
  // register new instance of TXAtomTypeEvaluator
  Evaluators.Add("type", (IEvaluator*const&)new TXAtom_TypeEvaluator(this));
  // register new instance of TXAtomPartEvaluator
  Evaluators.Add("part", (IEvaluator*const&)new TXAtom_PartEvaluator(this));
  // register new instance of TXAtomAfixEvaluator
  Evaluators.Add("afix", (IEvaluator*const&)new TXAtom_AfixEvaluator(this));
  // register new instance of TXAtomUisoEvaluator
  Evaluators.Add("uiso", (IEvaluator*const&)new TXAtom_UisoEvaluator(this));
  // register new instance of TXAtomPeakEvaluator
  Evaluators.Add("peak", (IEvaluator*const&)new TXAtom_PeakEvaluator(this));
  // register new instance of TXAtomBcEvaluator
  Evaluators.Add("bc", (IEvaluator*const&)new TXAtom_BcEvaluator(this));
  // register new instance of TXAtomSelectedEvaluator
  Evaluators.Add("selected", (IEvaluator*const&)new TXAtom_SelectedEvaluator(this));
  // add new instance of data provider TXAtomAtomEvaluator
  TXAtom_AtomEvaluator *tXAtomAtomEvaluator = new TXAtom_AtomEvaluator(this);
  DataProviders.Add("TXAtomAtomEvaluator", tXAtomAtomEvaluator);
  IEvaluatorFactory *tXAtomAtomEvaluatorFactory = FactoryRegister->Factory("TTSAtom_EvaluatorFactory");
  for( int i=0; i < tXAtomAtomEvaluatorFactory->EvaluatorCount(); i++ )
  {
    Evaluators.Add(olxstr("atom.") << tXAtomAtomEvaluatorFactory->EvaluatorName(i), tXAtomAtomEvaluatorFactory->Evaluator(i)->NewInstance(tXAtomAtomEvaluator));
  }
  // add new instance of data provider TXAtomBaiEvaluator
  TXAtom_BaiEvaluator *tXAtomBaiEvaluator = new TXAtom_BaiEvaluator(this);
  DataProviders.Add("TXAtomBaiEvaluator", tXAtomBaiEvaluator);
  IEvaluatorFactory *tXAtomBaiEvaluatorFactory = FactoryRegister->Factory("TTBasicAtomInfoEvaluatorFactory");
  for( int i=0; i < tXAtomBaiEvaluatorFactory->EvaluatorCount(); i++ )
  {
    Evaluators.Add(olxstr("bai.") << tXAtomBaiEvaluatorFactory->EvaluatorName(i), tXAtomBaiEvaluatorFactory->Evaluator(i)->NewInstance(tXAtomBaiEvaluator));
  }
}
// constructor to create instaces of registered evaluators
TTGlGroupEvaluatorFactory::TTGlGroupEvaluatorFactory(IEvaluatorFactory *factoryRegister)
{
  FactoryRegister = factoryRegister;
  // add new instance of data provider TSelAEvaluator
  TSelAEvaluator *tSelAEvaluator = new TSelAEvaluator(this);
  DataProviders.Add("TSelAEvaluator", tSelAEvaluator);
  IEvaluatorFactory *tSelAEvaluatorFactory = FactoryRegister->Factory("TTXAtomEvaluatorFactory");
  for( int i=0; i < tSelAEvaluatorFactory->EvaluatorCount(); i++ )
  {
    Evaluators.Add(olxstr("a.") << tSelAEvaluatorFactory->EvaluatorName(i), tSelAEvaluatorFactory->Evaluator(i)->NewInstance(tSelAEvaluator));
  }
  // add new instance of data provider TSelBEvaluator
  TSelBEvaluator *tSelBEvaluator = new TSelBEvaluator(this);
  DataProviders.Add("TSelBEvaluator", tSelBEvaluator);
  IEvaluatorFactory *tSelBEvaluatorFactory = FactoryRegister->Factory("TTXBond_EvaluatorFactory");
  for( int i=0; i < tSelBEvaluatorFactory->EvaluatorCount(); i++ )
  {
    Evaluators.Add(olxstr("b.") << tSelBEvaluatorFactory->EvaluatorName(i), tSelBEvaluatorFactory->Evaluator(i)->NewInstance(tSelBEvaluator));
  }
}

