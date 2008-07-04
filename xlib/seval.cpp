#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "seval.h"

/* automaticaly generated code  */
TSFactoryRegister::~TSFactoryRegister()  {
  for( int i=0; i < Factories.Count(); i++ )
    delete (IEvaluatorFactory*)Factories.Object(i);
}
IEvaluator *TSFactoryRegister::Evaluator(const olxstr& name)  {
  TStrList toks(name, '.');
  IEvaluatorFactory* factory = FactoryMap[toks.String(0)];
  toks.Delete(0);
  return factory?factory->Evaluator(toks.Text('.')):NULL;
}
// constructor to create instaces of registered evaluators
TTSAtom_EvaluatorFactory::TTSAtom_EvaluatorFactory(IEvaluatorFactory *factoryRegister)
{
  FactoryRegister = factoryRegister;
  // register new instance of TSAtom_LabelEvaluator
  Evaluators.Add("label", (IEvaluator*const&)new TSAtom_LabelEvaluator(this));
  // register new instance of TSAtom_TypeEvaluator
  Evaluators.Add("type", (IEvaluator*const&)new TSAtom_TypeEvaluator(this));
  // register new instance of TSAtom_PartEvaluator
  Evaluators.Add("part", (IEvaluator*const&)new TSAtom_PartEvaluator(this));
  // register new instance of TSAtom_AfixEvaluator
  Evaluators.Add("afix", (IEvaluator*const&)new TSAtom_AfixEvaluator(this));
  // register new instance of TSAtom_UisoEvaluator
  Evaluators.Add("uiso", (IEvaluator*const&)new TSAtom_UisoEvaluator(this));
  // register new instance of TSAtom_PeakEvaluator
  Evaluators.Add("peak", (IEvaluator*const&)new TSAtom_PeakEvaluator(this));
  // register new instance of TSAtom_BcEvaluator
  Evaluators.Add("bc", (IEvaluator*const&)new TSAtom_BcEvaluator(this));
}
// constructor to create instaces of registered evaluators
TTBasicAtomInfoEvaluatorFactory::TTBasicAtomInfoEvaluatorFactory(IEvaluatorFactory *factoryRegister)
{
  FactoryRegister = factoryRegister;
  // register new instance of TBaiTypeEvaluator
  Evaluators.Add("type", (IEvaluator*const&)new TBaiTypeEvaluator(this));
  // register new instance of TBaiNameEvaluator
  Evaluators.Add("name", (IEvaluator*const&)new TBaiNameEvaluator(this));
  // register new instance of TBaiMwEvaluator
  Evaluators.Add("mw", (IEvaluator*const&)new TBaiMwEvaluator(this));
}

