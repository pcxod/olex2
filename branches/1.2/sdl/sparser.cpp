/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "sparser.h"
#include "typelist.h"
#include "exparse/exptree.h"

using namespace exparse;

TOperatorSignature::TOperatorSignature(const short shortVal,
  const olxstr &strVal)
{
  ShortValue = shortVal;
  StringValue = strVal;
}

TOperatorSignature TOperatorSignature::DefinedFunctions[] = {
                                          TOperatorSignature(aofAdd,  "+"),
                                          TOperatorSignature(aofSub,  "-"),
                                          TOperatorSignature(aofMul,  "*"),
                                          TOperatorSignature(aofDiv,  "/"),
                                          TOperatorSignature(aofExt,  "^"),
                                          TOperatorSignature(aofSin,  "sin"),
                                          TOperatorSignature(aofCos,  "cos"),
                                          TOperatorSignature(aofTan,  "tan"),
                                          TOperatorSignature(aofAsin, "asin"),
                                          TOperatorSignature(aofAcos, "acos"),
                                          TOperatorSignature(aofAtan, "atan"),
                                          TOperatorSignature(aofAbs,  "abs"),
                                          TOperatorSignature(aofAbs,  "%")
                                        };
short TOperatorSignature::DefinedFunctionCount = 13;


TSyntaxParser::TSyntaxParser(IEvaluatorFactory* FactoryInstance, const olxstr& Expression)  {
  EvaluatorFactory = FactoryInstance;
  LogicalOperators.Add("&&", new TtaFactory<IEvaluable, TloAndOperator,IEvaluable>());
  LogicalOperators.Add("||", new TtaFactory<IEvaluable, TloOrOperator, IEvaluable>());
  LogicalOperators.Add("!", new TsaFactory<IEvaluable, TloNotOperator, IEvaluable>());

  ComparisonOperators.Add("==", new TtaFactory<IEvaluable, TcoEOperator, IEvaluator>());
  ComparisonOperators.Add("!=", new TtaFactory<IEvaluable, TcoNEOperator, IEvaluator>());
  ComparisonOperators.Add("<=", new TtaFactory<IEvaluable, TcoLEOperator, IEvaluator>());
  ComparisonOperators.Add(">=", new TtaFactory<IEvaluable, TcoGEOperator, IEvaluator>());
  ComparisonOperators.Add("<", new TtaFactory<IEvaluable, TcoLOperator, IEvaluator>());
  ComparisonOperators.Add(">", new TtaFactory<IEvaluable, TcoGOperator, IEvaluator>());

  Root = SimpleParse( olxstr::DeleteChars(Expression, ' ') );
}
TSyntaxParser::~TSyntaxParser()  {
  Evaluables.DeleteItems(false);
  Evaluators.DeleteItems(false);
  for( size_t i=0; i < LogicalOperators.Count(); i++ )
    delete LogicalOperators.GetObject(i);
  for( size_t i=0; i < ComparisonOperators.Count(); i++ )
    delete ComparisonOperators.GetObject(i);
}

IEvaluable* TSyntaxParser::SimpleParse(const olxstr& Exp)  {
  olxstr LeftExp, RightExp, LeftStr, RightStr;
  IEvaluable *LeftCondition = NULL, *RightCondition = NULL;
  IEvaluable *LogicalOperator = NULL;
  TObjectFactory<IEvaluable> *loFactory = NULL, *coFactory=NULL;
  for (size_t i = 0; i < Exp.Length(); i++) {
    if (Exp.CharAt(i) == '(') {
      olxstr ComplexExp;
      if (!parser_util::parse_brackets(Exp, ComplexExp, i)) {
        FErrors.Add("Unclosed brackets");
        break;
      }
      if (!ComplexExp.IsEmpty()) {
        if (loFactory)
          RightCondition = SimpleParse(ComplexExp);
        else
          LeftCondition = SimpleParse(ComplexExp);
      }
      i++;
    }
    if (i < Exp.Length()) {
      size_t st = i;
      while (i < Exp.Length() && (olxstr::o_isalphanumeric(Exp.CharAt(i)) ||
        olxstr::o_isoneof(Exp.CharAt(i), "_.-+",4)))
      {
        i++;
      }
      olxstr expr = Exp.SubString(st, i - st);
      if (coFactory != NULL) {
        if (!RightStr.IsEmpty())
          RightStr << expr;
        else
          RightExp << expr;
      }
      else {
        if (!LeftStr.IsEmpty())
          LeftStr << expr;
        else
          LeftExp << expr;
      }
    }
    if (parser_util::skip_whitechars(Exp, i) < Exp.Length()) {
      if (parser_util::is_quote(Exp.CharAt(i))) {  // string beginning
        olxstr expr;
        if (!parser_util::parse_string(Exp, expr, i)) {
          FErrors.Add(olxstr("Unclose quotation"));
          break;
        }
        if (coFactory != NULL)
          RightStr << expr;
        else
          LeftStr << expr;
      }
    }
    // processing comparison operators
    if (coFactory != NULL && (!LeftExp.IsEmpty() || !LeftStr.IsEmpty()) &&
                             (!RightExp.IsEmpty() || !RightStr.IsEmpty()))
    {
      IEvaluator *LeftEvaluator = NULL, *RightEvaluator = NULL;
      if (!LeftExp.IsEmpty()) {
        if (LeftExp.IsNumber())
          LeftEvaluator = Evaluators.Add(new TScalarEvaluator(LeftExp.ToDouble()));
        else if (LeftExp.IsBool())
          LeftEvaluator = Evaluators.Add(new TBoolEvaluator(LeftExp.ToBool()));
        else {
          LeftEvaluator = EvaluatorFactory->Evaluator(LeftExp);
          if (LeftEvaluator == NULL)
            FErrors.Add("Could not find evaluator for: ") << LeftExp;
        }
      }
      else {
        LeftEvaluator = Evaluators.Add(new TStringEvaluator(LeftStr));
      }
      if (!RightExp.IsEmpty()) {
        if (RightExp.IsNumber()) {
          RightEvaluator = Evaluators.Add(new TScalarEvaluator(RightExp.ToDouble()));
        }
        else if (RightExp.IsBool()) {
          RightEvaluator = Evaluators.Add(new TBoolEvaluator(RightExp.ToBool()));
        }
        else {
          RightEvaluator = EvaluatorFactory->Evaluator(RightExp);
          if (RightEvaluator == NULL)
            FErrors.Add("Could not find evaluator for: ") << RightExp;
        }
      }
      else {
        RightEvaluator = new TStringEvaluator(RightStr);
        Evaluators.Add(RightEvaluator);
      }
      //RightEvaluator = EvaluatorFactory->NewEvaluator( RightExp.Length() ? RightExp : RightStr );
      TPtrList<IEObject> Args(2);
      Args[0] = LeftEvaluator;
      Args[1] = RightEvaluator;
      try {
        if (loFactory != NULL)
          RightCondition = Evaluables.Add(coFactory->NewInstance(&Args));
        else
          LeftCondition = Evaluables.Add(coFactory->NewInstance(&Args));
      }
      catch(const TExceptionBase &e) {
        throw TFunctionFailedException(__OlxSourceInfo, e);
      }
      // clean up the values for the next loop
      RightExp.SetLength(0);
      LeftExp.SetLength(0);
      LeftStr.SetLength(0);
      RightStr.SetLength(0);
      coFactory = NULL;
    }

    // process logical operators
    // do not check the left condition - for '!' operator it might be empty or
    // if there is a logical operator on the left (on the right it can be only
    // in the case of brackets)
    if (loFactory && RightCondition) {
      TPtrList<IEObject> Args;
      if (LogicalOperator != NULL) {
        Args.Add(LogicalOperator);
        Args.Add(RightCondition);
        LogicalOperator = loFactory->NewInstance(&Args);
      }
      else {
        Args.Add(LeftCondition);
        Args.Add(RightCondition);
        LogicalOperator = loFactory->NewInstance(&Args);
      }
      Evaluables.Add(LogicalOperator);
      LeftCondition = NULL;
      RightCondition = NULL;
      loFactory = NULL;
    }

    if (coFactory == NULL && (i < Exp.Length())) {
      olxch Char = Exp.CharAt(i);
      for (size_t j=0; j < ComparisonOperators.Count(); j++) {
        size_t index = 0;
        while ((index < ComparisonOperators.GetString(j).Length()) &&
               (ComparisonOperators.GetString(j).CharAt(index) == Char))
        {
          if ((i+index+1) >= Exp.Length()) break;
          Char = Exp.CharAt(i+index+1);
          index++;
        }
        if (index == ComparisonOperators.GetString(j).Length()) {
          i += (index-1);
          coFactory = ComparisonOperators.GetObject(j);
          break;
        }
        Char = Exp.CharAt(i);  // roll back the character
      }
      if (coFactory != NULL) continue;
    }

    if (loFactory == NULL && (i < Exp.Length())) {
      olxch Char = Exp.CharAt(i);
      for (size_t j = 0; j < LogicalOperators.Count(); j++) {
        size_t index = 0;
        while ((index < LogicalOperators.GetString(j).Length()) &&
               (LogicalOperators.GetString(j).CharAt(index) == Char))
        {
          if ((i+index+1) >= Exp.Length()) break;
          Char = Exp.CharAt(i+index+1);
          index++;
        }
        if (index == LogicalOperators.GetString(j).Length()) {
          i+= (index-1);
          loFactory = LogicalOperators.GetObject(j);
          break;
        }
        Char = Exp.CharAt(i);  // roll back the character
      }
    }
  }
  if (LogicalOperator != NULL)  return LogicalOperator;
  if (LeftCondition != NULL)  return LeftCondition;

  FErrors.Add("Could not parse: ").quote() << Exp;
  return NULL;
}
