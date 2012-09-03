#include "libmath.h"
#include "evaln.h"
#include "../integration.h"

void LibMath::Eval(const TStrObjList& Params, TMacroError& E) {
  TStrList Vars;
  math_eval::ExpEvaluator evtr;
  olxstr param = Params[0];
  evtr.build(param);
  olex::IOlexProcessor *op = olex::IOlexProcessor::GetInstance();
  for (size_t i=0; i < evtr.Evaluators.Count(); i++) {
    math_eval::Evaluator *e = evtr.Evaluators[i];
    TStrObjList Params;
    for (size_t j=0; j < e->args.Count(); j++) {
      if (EsdlInstanceOf(*e->args[j], math_eval::Variable))
        Params.Add(((math_eval::Variable*)e->args[j])->name);
      else
        Params.Add(e->args[j]->evaluate());
    }
    ABasicFunction *bf = op->GetLibrary().FindFunction(e->name);
    if (bf == NULL) {
      E.ProcessingError(__OlxSrcInfo,"Undefined function :").quote()
        << e->name;
    }
    TMacroError rv;
    bf->Run(Params, rv);
    if (!rv.IsSuccessful()) {
      E = rv;
      return;
    }
    e->set_value(rv.GetRetVal().ToDouble());
  }
  E.SetRetVal(evtr.evaluate());
}

TLibrary *LibMath::ExportLibrary(const olxstr &name) {
  TLibrary *lib = new TLibrary(name.IsEmpty() ? olxstr("math") : name);
  lib->Register(new TStaticFunction(&LibMath::Eval, "eval", fpOne,
    "Evaluates given expression")
  );
  lib->Register(new TStaticFunction(&LibMath::Abs, "abs", fpOne,
    "Evaluates absolute value of a number")
  );
  return lib;
}
