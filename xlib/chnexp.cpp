#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "chnexp.h"
#include "atominfo.h"

//---------------------------------------------------------------------------
TCHNExp::TCHNExp(TAtomsInfo *AI)  {
  FMult = 1;
  FAtomsInfo = AI;
}
//..............................................................................
TCHNExp::~TCHNExp()  {
  Clear();
}
//..............................................................................
void TCHNExp::Clear()  {
  Exp.Clear();
  Dependencies.Clear();
  FMult = 1;
}
//..............................................................................
olxstr TCHNExp::SummFormula(const olxstr &Separator)  {
  TStrPObjList<olxstr,double> E1;
  CalcSummFormula(E1);
  olxstr Res;
  double v;
  for( int i=0; i < E1.Count(); i++ )  {
    Res << E1.String(i);
    v = E1.Object(i);
    if( v != 1 )
      Res << v;
    Res << Separator;
  }
  return Res;
}
//..............................................................................
double TCHNExp::MolWeight()  {
  TStrPObjList<olxstr,double> E1;
  CalcSummFormula(E1);
  double w = 0;
  TBasicAtomInfo *I;
  for( int i=0; i < E1.Count(); i++ )  {
    I = AtomsInfo()->FindAtomInfoBySymbol( E1.String(i) );
    w += (E1.Object(i) * I->GetMr());
  }
  return w;
}
//..............................................................................
void TCHNExp::CHN(double &C, double &H, double &N, double &Mr)  {
  TStrPObjList<olxstr,double> E1;
  CalcSummFormula(E1);
  double w = 0;
  TBasicAtomInfo *I;
  for( int i=0; i < E1.Count(); i++ )  {
    I = AtomsInfo()->FindAtomInfoBySymbol( E1.String(i) );
    w += (E1.Object(i) * I->GetMr() );
  }
  if( w == 0 )  w = 1;  // if w == 0 then all components are zero, so ... why not?
  for( int i=0; i < E1.Count(); i++ )  {
    I = AtomsInfo()->FindAtomInfoBySymbol(E1.String(i));
    if( I->GetIndex() == iCarbonIndex )
      C = E1.Object(i) * I->GetMr();
    else if( I->GetIndex() == iHydrogenIndex )
      H = E1.Object(i) * I->GetMr();
    else if( I->GetIndex() == iNitrogenIndex )
      N = E1.Object(i) * I->GetMr();
  }
  Mr = w;
}
//..............................................................................
olxstr TCHNExp::Composition()  {
  TStrPObjList<olxstr,double> E1;
  CalcSummFormula(E1);
  double w = 0, v;
  olxstr Res("Calculated ("), SF;
  TBasicAtomInfo *I;
  for( int i=0; i < E1.Count(); i++ )  {
    I = AtomsInfo()->FindAtomInfoBySymbol( E1.String(i) );
    SF << E1.String(i) << E1.Object(i) <<  ' ';
    w += (E1.Object(i) * I->GetMr() );
  }
  Res << SF << "): ";
  if( w == 0 )  w = 1;  // if w == 0 then all components are zero, so ... why not?
  for( int i=0; i < E1.Count(); i++ )  {
    I = AtomsInfo()->FindAtomInfoBySymbol( E1.String(i) );
    v = (E1.Object(i) * I->GetMr());
    Res << E1[i] <<  ": " << olxstr::FormatFloat(2, v/w*100) << "; ";
  }
  return Res;
}
//..............................................................................
void TCHNExp::CalcSummFormula(TStrPObjList<olxstr,double>& E)  {
  bool Added;
  TStrPObjList<olxstr,double> E1;
  for( int i=0; i < Exp.Count(); i++ )  {
    Added = false;
    for( int j=0; j < E1.Count(); j++ )  {
      if( E1.String(j) == Exp.String(i) )  {
        E1.Object(j) += Exp.Object(i);
        Added = true;
      }
    }
    if( !Added )
      E1.Add(Exp.String(i), Exp.Object(i));
  }
  for( int i=0; i < Dependencies.Count(); i++ )
    Dependencies[i].CalcSummFormula(E1);
  for( int i=0; i < E1.Count(); i++ )  {
    Added = false;
    for( int j=0; j < E.Count(); j++ )  {
      if( E.String(j) == E1.String(i) )  {
        Added = true;
        E.Object(j) += (E1.Object(i)*FMult);
        break;
      }
    }
    if( !Added )
      E.Add(E1.String(i), E1.Object(i)*FMult);
  }
}
//..............................................................................
void TCHNExp::LoadFromExpression(const olxstr &E1)  {
  olxstr NExp, ECount, Element, SMult,
           E( olxstr::DeleteChars(E1, ' '));
  bool ElementDefined = false;
  short ob, cb; // open and close brackets
  Clear();
  for( int i=0; i < E.Length(); i++ )  {
    if( E[i] == '(' )   {
      NExp = EmptyString;
      SMult = EmptyString;
      ob = 1;
      cb = 0;
      i++;
      while( cb != ob )  {
        if( E[i] == ')' )      cb++;
        if( E[i] == '(' )      ob++;
        if( cb == ob )        break;
        NExp << E[i];
        i++;
        if( i >= E.Length() )
          throw TFunctionFailedException(__OlxSourceInfo, "brackets count mismatch");
      }
      if( i < (E.Length()-1) )  {
        i++;
        while( (E[i] <= '9' && E[i] >= '0') || E[i] == '.' )  {
          SMult << E[i];
          i++;
          if( i >= E.Length() )  break;
        }
        i--;  // reset to previous
      }

      if( !NExp.IsEmpty() )  {
        TCHNExp& Exp = Dependencies.AddNew(AtomsInfo());
        Exp.LoadFromExpression(NExp);
        if( !SMult.IsEmpty() )  Exp.SetMult(SMult);
      }
      continue;
    }
    if( (E[i] <= '9' && E[i] >= '0') || E[i] == '.' )  {
      if( ElementDefined )
        ECount << E[i];
      else
        throw TFunctionFailedException(__OlxSourceInfo, "Number of fragments should follow the fragment");
    }
    else  {
      if( ElementDefined )  {
        if( Element.IsEmpty() )  return;
        if( !AtomsInfo()->IsElement(Element) )
          throw TFunctionFailedException(__OlxSourceInfo, olxstr("Unknown element: '") << Element << '\'');
        if( ECount.Length() != 0 )
          Exp.Add(Element, ECount.ToDouble());
        else
          Exp.Add(Element, 1.0);
      }
      Element = EmptyString;
      while( (E[i] <= 'Z' && E[i] >= 'A') || (E[i] <= 'z' && E[i] >= 'a'))  {
        Element << E[i];
        i++;
        if( Element.Length() == 2 )  {  // CD
          if( !AtomsInfo()->IsElement(Element) )  {  // CC-D
            Element.SetLength(1);
            i--;
            break;  // the firts letter should be an element labels
          }
          break;
        }
        if( i >= E.Length() )  {
          if( !AtomsInfo()->IsElement(Element) )
            throw TFunctionFailedException(__OlxSourceInfo, olxstr("Unknown element: '") << Element << '\'');
          Exp.Add(Element, 1);
          return;
        }
      }
      i--;  // reset to previous
      ECount = EmptyString;
      ElementDefined = true;
    }
  }
  if( ElementDefined && !Element.IsEmpty() )  { // add lst element
    if( !AtomsInfo()->IsElement(Element) )
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("Unknown element: '") << Element << '\'');
    if( !ECount.IsEmpty() )
      Exp.Add(Element, ECount.ToDouble());
    else
      Exp.Add(Element, 1);
  }
}
