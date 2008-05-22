#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include <stdlib.h>

#include "symmparser.h"
#include "unitcell.h"
#include "asymmunit.h"

#include "estack.h"
#include "estrlist.h"

#include "emath.h"
//---------------------------------------------------------------------------
// compares p with values in array SO and LO. Used in SymmToMatrix function
short TSymmParser::IsAxis(const char* axes, const olxstr& p){
  if( p.IsEmpty() )  return -1;
  for( int i=0; i < 3; i++ )
    if( axes[i] == p[0] )  return i;
  return -1;
}
//..............................................................................
// Transforms standard SYMM operation (INS, CIF files) to matrix
bool  TSymmParser::SymmToMatrix(const olxstr& S, TMatrixD &M)  {
  olxstr p, p1, opr;
  static const char Axes[3] = {'x','y','z'};
  double ratio;
  int op, index;
  bool res = true;
  TEStack<olxstr> stack;
  M.Null();
  TStrList toks(olxstr::LowerCase(S), ',');
  if( toks.Count() != 3 )
    throw TFunctionFailedException(__OlxSourceInfo,
         olxstr("Operation sign is missing or operation is incomplete while parsing \"") << S << '\"');

  for( int i=0; i < 3; i++ )  {
    stack.Clear();
    TEStack<olxstr>::LoadFromExpression(stack, toks[i]);
    p = stack.Pop();
    op = IsAxis(Axes, p);
    if( op < 0 )  {
      p1 = stack.Pop();
      if( stack.IsEmpty() )  {
        res = false;  break;
      }
      if( p1.CharAt(0) == '/' )  {
        p1 = stack.Pop();
        ratio = p1.ToDouble()/p.ToDouble();
        p1 = stack.Pop();
        if( p1.CharAt(0) == '+' )      M[i][3] = ratio;  // translation
        if( p1.CharAt(0) == '-' )      M[i][3] = -ratio;  // translation
      }
      else
        M[i][3] = p.ToDouble();  // translation

      p = stack.Pop();
    }
next_oper:
    if( abs(p.CharAt(0)-'z') > 2 )  {
      res = false;
      break;
    }
    index = 2 - 'z' + p.CharAt(0);
    M[i][index] = 1;
    if( stack.IsEmpty() )  continue;
    p = stack.Pop();
    if( p.CharAt(0) == '-' )     M[i][index] = -1;  // inversion
    if( stack.IsEmpty() )  continue;
    p = stack.Pop();
    op = IsAxis(Axes, p);
    if( op < 0 )  {
      if( stack.IsEmpty() )  {
        M[i][3] = p.ToDouble();  // translation
        continue;
      }
      if( stack.Capacity() == 1 )  {
        p = stack.Pop() + p;
        M[i][3] = p.ToDouble();  // translation
        continue;
      }
      opr = stack.Pop();  // should be '/'
      if( opr.CharAt(0) != '/' )  {
        res = false;  
        break;
      }
      p1 = stack.Pop();
      ratio = p1.ToDouble()/p.ToDouble();
      if( !stack.IsEmpty() != 0 )  {
        p = stack.Pop();
        if( p == '-' )
          ratio *= -1;
        else if( p == '+' )
          ;
        else
          stack.Push( p );
      }
      M[i][3] = ratio;  // translation
    }
    else
      goto next_oper;
  }
  if( res == false )
    throw TFunctionFailedException(__OlxSourceInfo,
         olxstr("Operation sign is missing or operation is incomplete while parsing \"") << S << '\"');
  return true;
}
//..............................................................................
// Transforms matrix to standard SYMM operation (INS, CIF files)

olxstr FormatFloatX(double f)  {
  int sig = Sign(f);
  f = fabs(f);
  int w = (int)f;
  double r = f-w;
  if( fabs(r) < 0.01 )  return olxstr(w*sig);
  if( fabs(r) > 0.99 )  return olxstr((w+1)*sig);
  r = 1./r;
  int w1 = (int)r;
  double r1 = r-w1;

  if( fabs(r1) < 0.01 )
    if( sig < 0 )
      return olxstr('-') << (int)(w*r+1) << '/' << (int)r;
    else
      return olxstr( (int)(w*r+1) ) << '/' << (int)r;
  if( fabs(r1) > 0.99 )
    if( sig < 0 )
      return olxstr('-') << (int)(w*r+1) << '/' << (int)(r+1);
    else
      return olxstr( (int)(w*r+1) ) << '/' << (int)(r+1);

  r1 = 1./r1;

  if( sig < 0 )
    return olxstr('-') << (int)(w*r+r1) << '/' << (int)(r1+w1);
  else
    return olxstr( (int)(w*r+r1) ) << '/' << (int)(r1+w1);
}
olxstr TSymmParser::MatrixToSymm(const TMatrixD &M)  {
  olxstr T, T1, T2;
  char Axis[] = {'X','Y','Z'};
  for( int j=0; j < 3; j ++ )  {
    if( j != 0 )
      T << ',';
    for( int i=0; i < 3; i ++ )  {
      if( i == j )  {
        if( M[j][i] != 0 )  {
          //if( M[j][3] != 0 )
          T1 << CharSign(M[j][i]);
          if( fabs(fabs(M[j][i])-1) > 1e-5 )
            T1 << (double)M[j][i];
          T1 << Axis[j];
        }
        continue;
      }
      if( M[j][i] != 0 )  {
        T1.Insert(Axis[i], 0);
        T1.Insert(CharSign(M[j][i]), 0);
      }
    }
    if( M[j][3] != 0 )  {
      //T2 = FormatFloatX(M[j][3]);
      T2 = olxstr::FormatFloat(3, M[j][3], false);
      T2.TrimFloat();
      T1.Insert(T2, 0);
    }
    T << T1;
    T1 = EmptyString;
  }
  return T;
}
//..............................................................................
TMatrixD TSymmParser::SymmCodeToMatrixU(const TUnitCell& UC, const olxstr &Code)  {
  olxstr Tmp;
  TStrList Toks(Code, '_');
  int isymm;
  TMatrixD mSymm(3,4);
  if( Toks.Count() == 1 )  {
    isymm = Toks[0].ToInt()-1;
    if( isymm < 0 || isymm >= UC.MatrixCount() )
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("wrong matrix index: ") << isymm);
    return UC.GetMatrix(isymm);
  }
  if( Toks.Count() != 2 )
    throw TFunctionFailedException(__OlxSourceInfo, olxstr("wrong Code: ") << Code);
  isymm = Toks.String(0).ToInt()-1;
  if( isymm < 0 || isymm >= UC.MatrixCount() )
    throw TFunctionFailedException(__OlxSourceInfo, olxstr("wrong matrix index: ") << isymm);
  mSymm = UC.GetMatrix(isymm);

  if( (Toks.String(1).Length() != 3) && (Toks.String(1).Length() != 6))
    throw TFunctionFailedException(__OlxSourceInfo, olxstr("wrong matrix code: ") << Toks.String(1));
  if( Toks.String(1).Length() == 3 )  {
    mSymm[0][3] += (int)(Toks.String(1)[0]-'5');
    mSymm[1][3] += (int)(Toks.String(1)[1]-'5');
    mSymm[2][3] += (int)(Toks.String(1)[2]-'5');
  }
  else  {
    Tmp = Toks.String(1).SubString(0, 2);
    mSymm[0][3] += (Tmp.ToInt()-55);
    Tmp = Toks.String(1).SubString(2, 2);
    mSymm[1][3] += (Tmp.ToInt()-55);
    Tmp = Toks.String(1).SubString(4, 2);
    mSymm[2][3] += (Tmp.ToInt()-55);
  }
  return mSymm;
}
//..............................................................................
TMatrixD TSymmParser::SymmCodeToMatrixA(const TAsymmUnit& AU, const olxstr &Code)  {
  olxstr Tmp;
  TStrList Toks(Code, '_');
  int isymm;
  TMatrixD mSymm(3,4);
  if( Toks.Count() == 1 )  {
    isymm = Toks.String(0).ToInt()-1;
    if( isymm < 0 || isymm >= AU.MatrixCount() )
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("wrong matrix index: ") << isymm);
    return AU.GetMatrix(isymm);
  }
  if( Toks.Count() != 2 )
    throw TFunctionFailedException(__OlxSourceInfo, olxstr("wrong Code: ") << Code);
  isymm = Toks.String(0).ToInt()-1;
  if( isymm < 0 || isymm >= AU.MatrixCount() )
    throw TFunctionFailedException(__OlxSourceInfo, olxstr("wrong matrix index: ") << isymm);
  mSymm = AU.GetMatrix(isymm);

  if( (Toks.String(1).Length() != 3) && (Toks.String(1).Length() != 6))
    throw TFunctionFailedException(__OlxSourceInfo, olxstr("wrong matrix code: ") << Toks.String(1));
  if( Toks.String(1).Length() == 3 )  {
    mSymm[0][3] += (int)(Toks.String(1)[0]-'5');
    mSymm[1][3] += (int)(Toks.String(1)[1]-'5');
    mSymm[2][3] += (int)(Toks.String(1)[2]-'5');
  }
  else  {
    Tmp = Toks.String(1).SubString(0, 2);
    mSymm[0][3] += (Tmp.ToInt()-55);
    Tmp = Toks.String(1).SubString(2, 2);
    mSymm[1][3] += (Tmp.ToInt()-55);
    Tmp = Toks.String(1).SubString(4, 2);
    mSymm[2][3] += (Tmp.ToInt()-55);
  }
  return mSymm;
}
//..............................................................................
TMatrixD TSymmParser::SymmCodeToMatrix(const TMatrixDList& ml, const olxstr &Code)  {
  olxstr Tmp;
  TStrList Toks(Code, '_');
  int isymm;
  TMatrixD mSymm(3,4);
  if( Toks.Count() == 1 )  {
    isymm = Toks[0].ToInt()-1;
    if( isymm < 0 || isymm >= ml.Count() )
      throw TFunctionFailedException(__OlxSourceInfo, olxstr("wrong matrix index: ") << isymm);
    mSymm = ml[isymm];
    return mSymm;
  }
  if( Toks.Count() != 2 )
    throw TFunctionFailedException(__OlxSourceInfo, olxstr("wrong Code: ") << Code);
  isymm = Toks[0].ToInt()-1;
  if( isymm < 0 || isymm >= ml.Count() )
    throw TFunctionFailedException(__OlxSourceInfo, olxstr("wrong matrix index: ") << isymm);
  mSymm = ml[isymm];

  if( (Toks.String(1).Length() != 3) && (Toks.String(1).Length() != 6))
    throw TFunctionFailedException(__OlxSourceInfo, olxstr("wrong matrix code: ") << Toks.String(1));
  if( Toks.String(1).Length() == 3 )  {
    mSymm[0][3] += (int)(Toks.String(1)[0]-'5');
    mSymm[1][3] += (int)(Toks.String(1)[1]-'5');
    mSymm[2][3] += (int)(Toks.String(1)[2]-'5');
  }
  else  {
    Tmp = Toks.String(1).SubString(0, 2);
    mSymm[0][3] += (Tmp.ToInt()-55);
    Tmp = Toks.String(1).SubString(2, 2);
    mSymm[1][3] += (Tmp.ToInt()-55);
    Tmp = Toks.String(1).SubString(4, 2);
    mSymm[2][3] += (Tmp.ToInt()-55);
  }
  mSymm.SetTag( isymm );
  return mSymm;
}
//..............................................................................
// this needs to be of very high performance
olxstr TSymmParser::MatrixToSymmCode(const TUnitCell& UC, const TMatrixD &M)  {
  const TMatrixD& m = UC.GetMatrix( M.GetTag() );
  TVector<int> Trans(3);
  int baseVal = 5;
  Trans[0] = (int)(m[0][3] - M[0][3]);
  Trans[1] = (int)(m[1][3] - M[1][3]);
  Trans[2] = (int)(m[2][3] - M[2][3]);
  if( (abs(Trans[0]) > 4) || (abs(Trans[1]) > 4) || (abs(Trans[1]) > 4) )
    baseVal = 55;

  char bf[32];
#ifdef _MSC_VER
  sprintf_s(bf, 32, "%i_%i%i%i", M.GetTag()+1, baseVal - Trans[0], baseVal - Trans[1], baseVal - Trans[2]);
#else
  sprintf(bf, "%i_%i%i%i", M.GetTag()+1, baseVal - Trans[0], baseVal - Trans[1], baseVal - Trans[2]);
#endif
  return olxstr(bf);
}
//..............................................................................
olxstr TSymmParser::MatrixToSymmCode(const TMatrixDList& ml, const TMatrixD &M)  {
  TVector<int> Trans(3);
  int baseVal = 5;
  Trans[0] = (int)(ml[M.GetTag()][0][3] - M[0][3]);
  Trans[1] = (int)(ml[M.GetTag()][1][3] - M[1][3]);
  Trans[2] = (int)(ml[M.GetTag()][2][3] - M[2][3]);
  if( (abs(Trans[0]) > 4) || (abs(Trans[1]) > 4) || (abs(Trans[1]) > 4) )
    baseVal = 55;

  char bf[32];
#ifdef _MSC_VER
  sprintf_s(bf, 32, "%i_%i%i%i", M.GetTag()+1, baseVal - Trans[0], baseVal - Trans[1], baseVal - Trans[2]);
#else
  sprintf(bf, "%i_%i%i%i", M.GetTag()+1, baseVal - Trans[0], baseVal - Trans[1], baseVal - Trans[2]);
#endif
  return olxstr(bf);
}
//..............................................................................

