#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "sparser.h"
#include "typelist.h"

TOperatorSignature::TOperatorSignature(const short shortVal, const olxstr &strVal) {
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
  LogicalOperators.Add("&&", (TObjectFactory<IEvaluable>*const&)new TtaFactory<IEvaluable, TloAndOperator,IEvaluable>());
  LogicalOperators.Add("||", (TObjectFactory<IEvaluable>*const&)new TtaFactory<IEvaluable, TloOrOperator, IEvaluable>());
  LogicalOperators.Add("!", (TObjectFactory<IEvaluable>*const&)new TsaFactory<IEvaluable, TloNotOperator, IEvaluable>());

  ComparisonOperators.Add("==", (TObjectFactory<IEvaluable>*const&)new TtaFactory<IEvaluable, TcoEOperator, IEvaluator>());
  ComparisonOperators.Add("!=", (TObjectFactory<IEvaluable>*const&)new TtaFactory<IEvaluable, TcoNEOperator, IEvaluator>());
  ComparisonOperators.Add("<=", (TObjectFactory<IEvaluable>*const&)new TtaFactory<IEvaluable, TcoLEOperator, IEvaluator>());
  ComparisonOperators.Add(">=", (TObjectFactory<IEvaluable>*const&)new TtaFactory<IEvaluable, TcoGEOperator, IEvaluator>());
  ComparisonOperators.Add("<", (TObjectFactory<IEvaluable>*const&)new TtaFactory<IEvaluable, TcoLOperator, IEvaluator>());
  ComparisonOperators.Add(">", (TObjectFactory<IEvaluable>*const&)new TtaFactory<IEvaluable, TcoGOperator, IEvaluator>());

  Root = SimpleParse( olxstr::DeleteChars(Expression, ' ') );
}
TSyntaxParser::~TSyntaxParser()  {
  for( int i=0; i < Evaluables.Count(); i++ )
    delete (IEvaluable*)Evaluables[i];
  for( int i=0; i < Evaluators.Count(); i++ )
    delete (IEvaluator*)Evaluators[i];
  for( int i=0; i < LogicalOperators.Count(); i++ )
    delete LogicalOperators.GetObject(i);
  for( int i=0; i < ComparisonOperators.Count(); i++ )
    delete ComparisonOperators.GetObject(i);
}
/*
olxstr Anlalyse(const olxstr& Expression)
{
  olxstr Exp = olxstr::RemoveChars(Expression, ' ');
  if( Exp.FirstIndexOf('(') == -1 )  return Exp;

  TIntList OpenBracketPos;
  short bc = 0;
  for( int i=0; i < Exp.Length(); i++ )
  {
    if( Exp[i] == '(' )
    {
      OpenBracketPos.AddCopy(i);
      continue;
    }
    if( Exp[i] == ')' )
    {

      continue;
    }

  }

}
*/
IEvaluable* TSyntaxParser::SimpleParse(const olxstr& Exp)  {
  olxstr LeftExp, RightExp, LeftStr, RightStr;
  IEvaluable *LeftCondition, *RightCondition;
  IEvaluable *LogicalOperator;
  TObjectFactory<IEvaluable> *loFactory = NULL, *coFactory=NULL;

  LeftCondition = RightCondition = NULL;
  LogicalOperator = NULL;
  olxch Char, StringWrappingChar;
  short index, bc;
  olxstr ComplexExp;
  for( int i=0; i < Exp.Length(); i++ )  {
    Char = Exp[i];

    if( Char == '(' )  {
      if( ++i >= Exp.Length() )  {
        FErrors.Add( olxstr("Unclosed brackets") );
        break;
      }
      bc = 1;
      Char = Exp[i];
      ComplexExp = EmptyString;
      while( bc )  {
        if( Char == '(' )  bc++;
        if( Char == ')' )  bc--;

        if( bc )           ComplexExp << Char;
        else               break;
        if( ++i > Exp.Length() )  {
          FErrors.Add( olxstr("Unclosed brackets") );
          break;
        }
        Char = Exp[i];
      }
      if( !ComplexExp.IsEmpty() )  {
        if( loFactory )
          RightCondition = SimpleParse(ComplexExp);
        else
          LeftCondition = SimpleParse(ComplexExp);
      }
      if( ++i < Exp.Length() )  Char = Exp[i];
      else                      Char = '\0';
    }

    while( (Char <= 'z' && Char >= 'a') ||
           (Char <= 'Z' && Char >= 'A') ||
           (Char <= '9' && Char >= '0') ||
           (Char == '_') || (Char =='.') )
    {
      // put values to different strings
      if( coFactory )  {
        if( RightStr.Length() )  RightStr << Char;
        else                     RightExp << Char;
      }
      else  {
        if( LeftStr.Length() )  LeftStr << Char;
        else                    LeftExp << Char;
      }
      if( ++i >= Exp.Length() )  break;
      Char= Exp[i];
    }

    // spaces are for readibility only
//    if( Char == ' ' )  continue;

    if( Char == '\'' || Char == '\"' )  {  // string beginning
      StringWrappingChar = Char;
      while( true )  {
        if( ++i >= Exp.Length() )  {
          FErrors.Add( olxstr("Unclose quotation") );
          break;
        }
        Char = Exp[i];
        if( Char == StringWrappingChar )  {
          if( coFactory )  {
            if( RightStr.Length() && RightStr[RightStr.Length()-1] == '\\' )  RightStr[RightStr.Length()-1] = Char;
            else  break;
          }
          else  {
            if( LeftStr.Length() && LeftStr[LeftStr.Length()-1] == '\\' )  LeftStr[LeftStr.Length()-1] = Char;
            else  break;
          }
        }
        if( coFactory )  RightStr << Char;
        else             LeftStr << Char;
      }
      if( ++i < Exp.Length() )  Char = Exp[i];
      else                      Char = '\0';
    }

    // processing comparison operators
    if( coFactory && (LeftExp.Length() || LeftStr.Length()) && (RightExp.Length() || RightStr.Length()) )  {
      IEvaluator *LeftEvaluator = NULL, *RightEvaluator = NULL;
      if( LeftExp.Length() )  {
        if( LeftExp.IsNumber() )  {
          LeftEvaluator = new TScalarEvaluator( LeftExp.ToDouble() );
          Evaluators.Add( LeftEvaluator );
        }
        else  if( LeftExp.Equalsi("true") || LeftExp.Equalsi("false") )  {
          LeftEvaluator = new TBoolEvaluator( LeftExp.ToBool() );
          Evaluators.Add( LeftEvaluator );
        }
        else  {
          LeftEvaluator = EvaluatorFactory->Evaluator( LeftExp );
          if( !LeftEvaluator )  FErrors.Add( olxstr("Could not find evaluator for: ") << LeftExp);
        }
      }
      else  {
        LeftEvaluator = new TStringEvaluator( LeftStr );
        Evaluators.Add( LeftEvaluator );
      }
      if( RightExp.Length() )  {
        if( RightExp.IsNumber() )  {
          RightEvaluator = new TScalarEvaluator( RightExp.ToDouble() );
          Evaluators.Add( RightEvaluator );
        }
        else  if( RightExp.Equalsi("true") || !RightExp.Equalsi("false") )  {
          RightEvaluator = new TBoolEvaluator( RightExp.ToBool() );
          Evaluators.Add( RightEvaluator );
        }
        else  {
          RightEvaluator = EvaluatorFactory->Evaluator( RightExp );
          if( !RightEvaluator )  FErrors.Add( olxstr("Could not find evaluator for: ") << RightExp);
        }
      }
      else  {
        RightEvaluator = new TStringEvaluator( RightStr );
        Evaluators.Add( RightEvaluator );
      }
      //RightEvaluator = EvaluatorFactory->NewEvaluator( RightExp.Length() ? RightExp : RightStr );
      TEList Args;
      Args.Add( LeftEvaluator );
      Args.Add( RightEvaluator );
      if( loFactory )  {
        RightCondition = coFactory->NewInstance(&Args);
        Evaluables.Add( RightCondition );
      }
      else  {
        LeftCondition = coFactory->NewInstance(&Args);
        Evaluables.Add( LeftCondition );
      }
      // clean up the values for the next loop
      RightExp  = EmptyString;
      LeftExp  = EmptyString;
      LeftStr  = EmptyString;
      RightStr  = EmptyString;
      coFactory = NULL;
    }

    // process logical operators
    // do not check the left condition - for '!' operator it might be empty or
    // if there is a logical operator on the left (on the right it can be only
    // in the case of brackets)
    if( loFactory && RightCondition )  {
      TEList Args;
      if( LogicalOperator )  {
        Args.Add( LogicalOperator );
        Args.Add( RightCondition );
        LogicalOperator = loFactory->NewInstance(&Args);
      }
      else  {
        Args.Add( LeftCondition );
        Args.Add( RightCondition );
        LogicalOperator = loFactory->NewInstance(&Args);
      }
      Evaluables.Add( LogicalOperator );
      LeftCondition = NULL;
      RightCondition = NULL;
      loFactory = NULL;
    }

    if( !coFactory && (i < Exp.Length()) )  {
      for( int j=0; j < ComparisonOperators.Count(); j++ )  {
        index = -1;
        while( (++index < ComparisonOperators.GetString(j).Length()) &&
               (ComparisonOperators.GetString(j).CharAt(index) == Char)  )
        {
          if( (i+index+1) >= Exp.Length() )  break;
          Char = Exp[i+index+1];
        }
        if( index == ComparisonOperators.GetString(j).Length() )  {
          i += (index-1);
          coFactory = ComparisonOperators.GetObject(j);
          break;
        }
        Char = Exp[i];           // roll back the character
      }
    }

    if( !loFactory && (i < Exp.Length()) )  {
      for( int j=0; j < LogicalOperators.Count(); j++ )  {
        index = -1;
        while( (++index < LogicalOperators.GetString(j).Length()) &&
               (LogicalOperators.GetString(j).CharAt(index) == Char)  )
        {
          if( (i+index+1) >= Exp.Length() )  break;
          Char = Exp[i+index+1];
        }
        if( index == LogicalOperators.GetString(j).Length() )  {
          i+= (index-1);
          loFactory = LogicalOperators.GetObject(j);
          break;
        }
        Char = Exp[i];           // roll back the character
      }
    }

  }
  if( LogicalOperator )  return LogicalOperator;
  if( LeftCondition )  return LeftCondition;

  FErrors.Add( olxstr("Could not parse: ") << Exp);
  return NULL;
}
