#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "seval.h"

/* automaticaly generated code  */
TSFactoryRegister::~TSFactoryRegister()  {
  for( size_t i=0; i < Factories.Count(); i++ )
    delete Factories.GetObject(i);
}
IEvaluator *TSFactoryRegister::Evaluator(const olxstr& name)  {
  TStrList toks(name, '.');
  IEvaluatorFactory* factory = FactoryMap[toks[0]];
  toks.Delete(0);
  return factory?factory->Evaluator(toks.Text('.')):NULL;
}
// constructor to create instaces of registered evaluators
TTSAtom_EvaluatorFactory::TTSAtom_EvaluatorFactory(IEvaluatorFactory *factoryRegister)
{
  FactoryRegister = factoryRegister;
  // register new instance of TSAtom_LabelEvaluator
  Evaluators.Add("label", new TSAtom_LabelEvaluator(this));
  // register new instance of TSAtom_TypeEvaluator
  Evaluators.Add("type", new TSAtom_TypeEvaluator(this));
  // register new instance of TSAtom_PartEvaluator
  Evaluators.Add("part", new TSAtom_PartEvaluator(this));
  // register new instance of TSAtom_AfixEvaluator
  Evaluators.Add("afix", new TSAtom_AfixEvaluator(this));
  // register new instance of TSAtom_UisoEvaluator
  Evaluators.Add("uiso", new TSAtom_UisoEvaluator(this));
  // register new instance of TSAtom_PeakEvaluator
  Evaluators.Add("peak", new TSAtom_PeakEvaluator(this));
  Evaluators.Add("occu", new TSAtom_OccuEvaluator(this));
  // register new instance of TSAtom_BcEvaluator
  Evaluators.Add("bc", new TSAtom_BcEvaluator(this));
}
// constructor to create instaces of registered evaluators
TTBasicAtomInfoEvaluatorFactory::TTBasicAtomInfoEvaluatorFactory(IEvaluatorFactory *factoryRegister)
{
  FactoryRegister = factoryRegister;
  // register new instance of TBaiTypeEvaluator
  Evaluators.Add("type", new TBaiTypeEvaluator(this));
  // register new instance of TBaiNameEvaluator
  Evaluators.Add("name", new TBaiNameEvaluator(this));
  // register new instance of TBaiMwEvaluator
  Evaluators.Add("mw", new TBaiMwEvaluator(this));
  // register new instance of TBaiZEvaluator
  Evaluators.Add("z", new TBaiZEvaluator(this));
}

