//---------------------------------------------------------------------------

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "syntaxp.h"
#include "typelist.h"

TOperatorSignature::TOperatorSignature(const short shortVal, const olxstr &strVal) {
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


TSyntaxParser::TSyntaxParser(IEvaluatorFactory *FactoryInstance, const olxstr &Expression)
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

  Root = SimpleParse( olxstr::DeleteChars(Expression, ' ') );
}
TSyntaxParser::~TSyntaxParser()
{
  for( int i=0; i < Evaluables.Count(); i++ )
    delete (IEvaluable*)Evaluables[i];
  for( int i=0; i < Evaluators.Count(); i++ )
    delete (IEvaluator*)Evaluators[i];
  for( int i=0; i < LogicalOperators.Count(); i++ )
    delete LogicalOperators.Object(i);
  for( int i=0; i < ComparisonOperators.Count(); i++ )
    delete ComparisonOperators.Object(i);
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
      if( ComplexExp.Length() )  {
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

    if( Char == '\'' || Char == '\"' )  {  // string begiining
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

    // procesing comparison operators
    if( coFactory && (LeftExp.Length() || LeftStr.Length()) && (RightExp.Length() || RightStr.Length()) )  {
      IEvaluator *LeftEvaluator = NULL, *RightEvaluator = NULL;
      if( LeftExp.Length() )  {
        if( LeftExp.IsNumber() )  {
          LeftEvaluator = new TScalarEvaluator( LeftExp.ToDouble() );
          Evaluators.Add( LeftEvaluator );
        }
        else  if( !LeftExp.Comparei("true") || !LeftExp.Comparei("false") )  {
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
        else  if( !RightExp.Comparei("true") || !RightExp.Comparei("false") )  {
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

  }
  if( LogicalOperator )  return LogicalOperator;
  if( LeftCondition )  return LeftCondition;

  FErrors.Add( olxstr("Could not parse: ") << Exp);
  return NULL;
}

/* automaticaly generated code  */
// constructor to create instaces of registered evaluators
TTXBond_EvaluatorFactory::TTXBond_EvaluatorFactory(TFactoryRegister *factoryRegister)  {
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
TTXAtom_EvaluatorFactory::TTXAtom_EvaluatorFactory(TFactoryRegister *factoryRegister)  {
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
TFactoryRegister::TFactoryRegister()
{
  TTBasicAtomInfoEvaluatorFactory *tTBasicAtomInfoEvaluatorFactory = new TTBasicAtomInfoEvaluatorFactory(this);
  Factories.Add("TTBasicAtomInfoEvaluatorFactory", tTBasicAtomInfoEvaluatorFactory);
  FactoryMap.Add("bai", tTBasicAtomInfoEvaluatorFactory);
  TTSAtom_EvaluatorFactory *tTSAtom_EvaluatorFactory = new TTSAtom_EvaluatorFactory(this);
  Factories.Add("TTSAtom_EvaluatorFactory", tTSAtom_EvaluatorFactory);
  FactoryMap.Add("SAtom", tTSAtom_EvaluatorFactory);
  TTXAtom_EvaluatorFactory *tTXAtomEvaluatorFactory = new TTXAtom_EvaluatorFactory(this);
  Factories.Add("TTXAtomEvaluatorFactory", tTXAtomEvaluatorFactory);
  FactoryMap.Add("XAtom", tTXAtomEvaluatorFactory);
  TTXBond_EvaluatorFactory *tTXBond_EvaluatorFactory = new TTXBond_EvaluatorFactory(this);
  Factories.Add("TTXBond_EvaluatorFactory", tTXBond_EvaluatorFactory);
  FactoryMap.Add("XBond", tTXBond_EvaluatorFactory);
  TTGlGroupEvaluatorFactory *tTGlGroupEvaluatorFactory = new TTGlGroupEvaluatorFactory(this);
  Factories.Add("TTGlGroupEvaluatorFactory", tTGlGroupEvaluatorFactory);
  FactoryMap.Add("sel", tTGlGroupEvaluatorFactory);
}
TFactoryRegister::~TFactoryRegister()  {
  for( int i=0; i < Factories.Count(); i++ )
    delete (IEvaluatorFactory*)Factories.Object(i);
}
IEvaluator *TFactoryRegister::Evaluator(const olxstr& name)  {
  TStrList toks(name, '.');
  IEvaluatorFactory* factory = FactoryMap[toks.String(0)];
  toks.Delete(0);
  return factory?factory->Evaluator(toks.Text('.')):NULL;
}
// constructor to create instaces of registered evaluators
TTSAtom_EvaluatorFactory::TTSAtom_EvaluatorFactory(TFactoryRegister *factoryRegister)
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
TTGlGroupEvaluatorFactory::TTGlGroupEvaluatorFactory(TFactoryRegister *factoryRegister)
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
// constructor to create instaces of registered evaluators
TTBasicAtomInfoEvaluatorFactory::TTBasicAtomInfoEvaluatorFactory(TFactoryRegister *factoryRegister)
{
  FactoryRegister = factoryRegister;
  // register new instance of TBaiTypeEvaluator
  Evaluators.Add("type", (IEvaluator*const&)new TBaiTypeEvaluator(this));
  // register new instance of TBaiNameEvaluator
  Evaluators.Add("name", (IEvaluator*const&)new TBaiNameEvaluator(this));
  // register new instance of TBaiMwEvaluator
  Evaluators.Add("mw", (IEvaluator*const&)new TBaiMwEvaluator(this));
}

