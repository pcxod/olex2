/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "libmath.h"
#include "evaln.h"
#include "../integration.h"

double LibMath::eval_(const TStrObjList& Params, TMacroData& E) {
  TStrList Vars;
  math_eval::ExpEvaluator evtr;
  olxstr param = Params[0];
  evtr.build(param);
  olex2::IOlex2Processor* op = olex2::IOlex2Processor::GetInstance();
  for (size_t i = 0; i < evtr.Evaluators.Count(); i++) {
    math_eval::Evaluator* e = evtr.Evaluators[i];
    TStrObjList Params;
    for (size_t j = 0; j < e->args.Count(); j++) {
      if (e->args[j]->Is<math_eval::Variable>()) {
        Params.Add(exparse::parser_util::unquote(
          ((math_eval::Variable*)e->args[j])->name));
      }
      else
        Params.Add(e->args[j]->evaluate());
    }
    ABasicFunction* bf = op->GetLibrary().FindFunction(e->name);
    if (bf == 0) {
      E.ProcessingError(__OlxSrcInfo, "Undefined function: ").quote()
        << e->name;
      return 0;
    }
    TMacroData rv;
    bf->Run(Params, rv);
    if (!rv.IsSuccessful()) {
      E = rv;
      return 0;
    }
    e->set_value(rv.GetRetVal().ToDouble());
  }
  for (size_t i = 0; i < evtr.Variables.Count(); i++) {
    if (evtr.Variables[i]->name.Equalsi("pi")) {
      evtr.Variables[i]->set_value(M_PI);
    }
    else if (evtr.Variables[i]->name.Equalsi("e")) {
      evtr.Variables[i]->set_value(2.718281828);
    }
  }
  return evtr.evaluate();
}
//.............................................................................
void LibMath::Eval(const TStrObjList& Params, TMacroData& E) {
  double v = eval_(Params, E);
  if (E.IsProcessingError()) {
    return;
  }
  E.SetRetVal(v);
}
//.............................................................................
void LibMath::EvalBool(const TStrObjList& Params, TMacroData& E) {
  double v = eval_(Params, E);
  if (E.IsProcessingError()) {
    return;
  }
  E.SetRetVal(v != 0);
}
//.............................................................................
TLibrary *LibMath::ExportLibrary(const olxstr &name) {
  TLibrary *lib = new TLibrary(name.IsEmpty() ? olxstr("math") : name);
  lib->Register(new TStaticFunction(&LibMath::Eval, "eval", fpOne,
    "Evaluates given expression")
  );
  lib->Register(new TStaticFunction(&LibMath::EvalBool, "eval_bool", fpOne,
    "Evaluates given expression to true/false")
  );
  return lib;
}
