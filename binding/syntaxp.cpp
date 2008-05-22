//---------------------------------------------------------------------------

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "syntaxp.h"
#include "typelist.h"

TOperatorSignature::TOperatorSignature(const short shortVal, const TEString &strVal)
{
  ShortValue = shortVal;
  StringValue = strVal;
}

TOperatorSignature DefinedFunctions[] = {
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
                                          TOperatorSignature(aofAbs,  "abs")
                                        };
short DefinedFunctionCount = 7;


TSyntaxParser::TSyntaxParser(IEvaluatorFactory *FactoryInstance)
{
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
}
TSyntaxParser::~TSyntaxParser()  {
  Clear();
  for( int i=0; i < LogicalOperators.Count(); i++ )
    delete LogicalOperators.Object(i);
  for( int i=0; i < ComparisonOperators.Count(); i++ )
    delete ComparisonOperators.Object(i);
}

void ParseOutExp(const TEString& exp, int& i, TEString& res)  {
  char Char = exp[i];
  while( (Char <= 'z' && Char >= 'a') ||
         (Char <= 'Z' && Char >= 'A') ||
         (Char <= '9' && Char >= '0') ||
         (Char == '_') || (Char =='.') )
  {
      res += Char;
      if( ++i >= exp.Length() )  break;
      Char = exp[i];
    }
}
void ParseOutComplexExp(const TEString& exp, int& i, TEString& res)  {
  if( exp[i] == '(' )  {
    if( ++i >= exp.Length() )
      throw TFunctionFailedException(__OlxSourceInfo, "Unclosed brackets");
    int bc = 1;
    while( bc )  {
      if( exp[i] == '(' )       bc++;
      else if( exp[i] == ')' )  bc--;

      if( bc == 0 )  break;
      res += exp[i];
      if( ++i > exp.Length() )
        throw TFunctionFailedException(__OlxSourceInfo, "Unclosed brackets");
    }
  }
}
void ParseOutStr(const TEString& exp, int& i, TEString& res)  {
  if( exp[i] == '\'' || exp[i] == '\"' )  {  // string beginning
    char StringWrappingChar = exp[i];
    while( true )  {
      if( ++i >= exp.Length() )
        throw TFunctionFailedException(__OlxSourceInfo, "unclosed quotation");
      if( exp[i] == StringWrappingChar )  {
        if( !res.IsEmpty() && res[res.Length()-1] == '\\' )  res[res.Length()-1] = exp[i];
        else  break;
      }
      res += exp[i];
    }
  }
}
IEvaluator* TSyntaxParser::CreateEvaluator(const TEString& expr, const TEString& args, const TEString& strval)  {
  IEvaluator* evl = NULL;
  if( !expr.IsEmpty() )  {
    if( args.Length() )  {
      evl = EvaluatorFactory->Evaluator( expr, args );
      if( evl == NULL )
        throw TFunctionFailedException(__OlxSourceInfo, TEString("Could not find evaluator for: ") << expr);
    }
    else if( expr.IsNumber() )  {
      evl = new TScalarEvaluator( expr.Double() );
      Evaluators.Add( evl );
    }
    else  if( expr.CompareCI("true") == 0  || expr.CompareCI("false") == 0 )  {
      evl = new TBoolEvaluator( expr.Bool() );
      Evaluators.Add( evl );
    }
    else  {
      evl = EvaluatorFactory->Evaluator( expr );
      if( evl == NULL )
        throw TFunctionFailedException(__OlxSourceInfo, TEString("Could not find evaluator for: ") << expr);
    }
  }
  else  {
    evl = new TStringEvaluator( strval );
    Evaluators.Add( evl );
  }
  return evl;
}

IEvaluable* TSyntaxParser::CreateEvaluable(const TEString& expr, const TEString& args, const TEString& strval)  {
  IEvaluable* evl = NULL;
  if( !expr.IsEmpty() )  {
    if( args.Length() )  {
      evl = EvaluatorFactory->Evaluable( expr, args );
      if( evl == NULL )
        throw TFunctionFailedException(__OlxSourceInfo, TEString("Could not find evaluator for: ") << expr);
    }
    else if( expr.IsNumber() )  {
      evl = new TBoolEvaluable( expr.Double() != 0 );
      Evaluables.Add( evl );
    }
    else  if( expr.CompareCI("true") == 0  || expr.CompareCI("false") == 0 )  {
      evl = new TBoolEvaluable( expr.Bool() );
      Evaluables.Add( evl );
    }
    else  {
      evl = EvaluatorFactory->Evaluable( expr );
      if( evl == NULL )
        throw TFunctionFailedException(__OlxSourceInfo, TEString("Could not find evaluator for: ") << expr);
    }
  }
  else  {
    evl = new TBoolEvaluable( strval.Length() != 0 );
    Evaluables.Add( evl );
  }
  return evl;
}


IEvaluable* TSyntaxParser::SimpleParse(const TEString& Exp)  {
  TEString LeftExp, RightExp, LeftStr, RightStr, FuncArgL, FuncArgR;
  IEvaluable *LeftCondition, *RightCondition;
  IEvaluable *LogicalOperator;
  TObjectFactory<IEvaluable> *loFactory = NULL, *coFactory=NULL;

  LeftCondition = RightCondition = NULL;
  LogicalOperator = NULL;
  char Char, StringWrappingChar;
  short index, bc;
  TEString ComplexExp;
  for( int i=0; i < Exp.Length(); i++ )  {
    Char = Exp[i];

    if( Char == '(' )  {
      ComplexExp.SetLength(0);
      ParseOutComplexExp(Exp, i, ComplexExp);
      if( ComplexExp.Length() )  {
        if( loFactory )
          RightCondition = SimpleParse(ComplexExp);
        else
          LeftCondition = SimpleParse(ComplexExp);
      }
      if( ++i < Exp.Length() )  Char = Exp[i];
      else                      Char = '\0';
    }


    if( coFactory || loFactory )  {
      if( !RightStr.IsEmpty() )  ParseOutExp(Exp, i, RightStr);
      else                       ParseOutExp(Exp, i, RightExp);
    }
    else  {
      if( !LeftStr.IsEmpty() )  ParseOutExp(Exp, i, LeftStr);
      else                      ParseOutExp(Exp, i, LeftExp);
    }
    Char = Exp[i];

    // spaces are for readibility only
//    if( Char == ' ' )  continue;

    if( Char == '\'' || Char == '\"' )  {  // string begiining
      if( coFactory ) 
        ParseOutStr( Exp, i, RightStr );
      else
        ParseOutStr( Exp, i, LeftStr );
    }
    else if( Char == '(' )  {  // function ...
      if( coFactory || loFactory )
        ParseOutComplexExp( Exp, i, FuncArgR);
      else
        ParseOutComplexExp( Exp, i, FuncArgL);
    }
    Char = Exp[i];
    
    // procesing comparison operators
    if( coFactory && (LeftExp.Length() || LeftStr.Length()) && (RightExp.Length() || RightStr.Length()) )  {
      IEvaluator *LeftEvaluator = CreateEvaluator(LeftExp, FuncArgL, LeftStr),
                 *RightEvaluator = CreateEvaluator(RightExp, FuncArgR, RightStr);
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
    if( loFactory )  {
      if( RightCondition )  {
        TEList Args;
        if( LogicalOperator )  {
          Args.Add( LogicalOperator );
          Args.Add( RightCondition );
          LogicalOperator = loFactory->NewInstance(&Args);
        }
        else  {
          if( LeftCondition ) Args.Add( LeftCondition );
          Args.Add( RightCondition );
          LogicalOperator = loFactory->NewInstance(&Args);
        }
        Evaluables.Add( LogicalOperator );
        LeftCondition = NULL;
        RightCondition = NULL;
        loFactory = NULL;
      }
      else if( (LeftExp.Length() || LeftStr.Length()) && (RightExp.Length() || RightStr.Length()) )  {
        LeftCondition = CreateEvaluable(LeftExp, FuncArgL, LeftStr);
        RightCondition = CreateEvaluable(RightExp, FuncArgR, RightStr);
        TEList Args;
        Args.Add( LeftCondition );
        Args.Add( RightCondition );
        LogicalOperator = loFactory->NewInstance(&Args);
        Evaluables.Add( LogicalOperator );
        LeftCondition = NULL;
        RightCondition = NULL;
        loFactory = NULL;
      }
      else if( RightExp.Length() || RightStr.Length() )  {
        RightCondition = CreateEvaluable(RightExp, FuncArgR, RightStr);
        TEList Args;
        Args.Add( RightCondition );
        LogicalOperator = loFactory->NewInstance(&Args);
        Evaluables.Add( LogicalOperator );
        LeftCondition = NULL;
        RightCondition = NULL;
        loFactory = NULL;
      }
    }

    if( !coFactory && (i < Exp.Length()) )  {
      for( int j=0; j < ComparisonOperators.Count(); j++ )  {
        index = -1;
        while( (++index < ComparisonOperators.String(j).Length()) &&
               (ComparisonOperators.String(j)[index] == Char)  )
        {
          if( (i+index+1) >= Exp.Length() )  break;
          Char = Exp[i+index+1];
        }
        if( index == ComparisonOperators.String(j).Length() )  {
          i += (index-1);
          coFactory = (TObjectFactory<IEvaluable>*)ComparisonOperators.Object(j);
          break;
        }
        Char = Exp[i];           // roll back the character
      }
    }

    if( !loFactory && (i < Exp.Length()) )  {
      for( int j=0; j < LogicalOperators.Count(); j++ )  {
        index = -1;
        while( (++index < LogicalOperators.String(j).Length()) &&
               (LogicalOperators.String(j)[index] == Char)  )
        {
          if( (i+index+1) >= Exp.Length() )  break;
          Char = Exp[i+index+1];
        }
        if( index == LogicalOperators.String(j).Length() )  {
          i+= (index-1);
          loFactory = (TObjectFactory<IEvaluable>*)LogicalOperators.Object(j);
          break;
        }
        Char = Exp[i];           // roll back the character
      }
    }
    else if( Char == '!' )  {
      if( (i+1) >= Exp.Length() )
        throw TFunctionFailedException(__OlxSourceInfo, "wrong syntax" );
      i++;
      if( Exp[i] == '(')  {
        ComplexExp.SetLength(0);
        ParseOutComplexExp( Exp, i, ComplexExp );
      }
      else  {
        ComplexExp.SetLength(1);
        ComplexExp[0] = '!';
        ParseOutExp( Exp, i, ComplexExp );
      }
      Char = Exp[i];
      if( Char == '(' )  {
        ComplexExp << '(';
        ParseOutComplexExp( Exp, i, ComplexExp );
        ComplexExp << ')';
      }
      if( ComplexExp.Length() )  {
        if( loFactory )  {
          RightCondition = SimpleParse(ComplexExp);
          TEList Args;
          if( LogicalOperator )  {
            Args.Add( LogicalOperator );
            Args.Add( RightCondition );
            LogicalOperator = loFactory->NewInstance(&Args);
          }
          else  {
            if( LeftCondition == NULL )
              LeftCondition = CreateEvaluable(LeftExp, FuncArgL, LeftStr);
            Args.Add( LeftCondition );
            Args.Add( RightCondition );
            LogicalOperator = loFactory->NewInstance(&Args);
          }
          Evaluables.Add( LogicalOperator );
          LeftCondition = NULL;
          RightCondition = NULL;
          loFactory = NULL;
        }
        else
          LeftCondition = SimpleParse(ComplexExp);
      }
    }

  }
  if( LogicalOperator != NULL )  return LogicalOperator;
  if( LeftCondition == NULL )
    LeftCondition = CreateEvaluable(LeftExp, FuncArgL, LeftStr);
  if( LeftCondition )  return LeftCondition;

  throw TFunctionFailedException(__OlxSourceInfo, TEString("Failed to parse -" ) << Exp);
}

void TSyntaxParser::Clear()  {
  for( int i=0; i < Evaluables.Count(); i++ )
    delete Evaluables[i];
  Evaluables.Clear();
  for( int i=0; i < Evaluators.Count(); i++ )
    delete Evaluators[i];
  Evaluators.Clear();
  Root = NULL;
}

void TSyntaxParser::Parse(const TEString &Text)  {
  Clear();
  Root = SimpleParse( TEString::RemoveChars(Text, ' ') );
}

