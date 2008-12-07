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
bool  TSymmParser::SymmToMatrix(const olxstr& S, smatd& M)  {
  olxstr p, p1, opr;
  static const char Axes[3] = {'x','y','z'};
  double ratio;
  int op, index;
  bool res = true;
  str_stack stack;
  M.Null();
  TStrList toks(olxstr::LowerCase(S), ',');
  if( toks.Count() != 3 )
    throw TFunctionFailedException(__OlxSourceInfo,
         olxstr("Operation sign is missing or operation is incomplete while parsing \"") << S << '\"');

  for( int i=0; i < 3; i++ )  {
    stack.LoadFromExpression(toks[i]);
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
        if( p1.CharAt(0) == '+' )      M.t[i] = ratio;  // translation
        if( p1.CharAt(0) == '-' )      M.t[i] = -ratio;  // translation
      }
      else
        M.t[i] = p.ToDouble();  // translation

      p = stack.Pop();
    }
next_oper:
    if( abs(p.CharAt(0)-'z') > 2 )  {
      res = false;
      break;
    }
    index = 2 - 'z' + p.CharAt(0);
    M.r[i][index] = 1;
    if( stack.IsEmpty() )  continue;
    p = stack.Pop();
    if( p.CharAt(0) == '-' )     M.r[i][index] = -1;  // inversion
    if( stack.IsEmpty() )  continue;
    p = stack.Pop();
    op = IsAxis(Axes, p);
    if( op < 0 )  {
      if( stack.IsEmpty() )  {
        M.t[i] = p.ToDouble();  // translation
        continue;
      }
      if( stack.Count() == 1 )  {
        p = stack.Pop() << p;
        M.t[i] = p.ToDouble();  // translation
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
      M.t[i] = ratio;  // translation
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

olxstr FormatFloatEx(double f)  {
  olxstr rv;
  if( f < 0 )
    rv << '-';
  int v = abs(Round(f*12)), base = 12;
  int denom = esdl::gcd(v, base);
  if( denom != 1 )  {
    v /= denom;
    base /= denom;
  }
  if( base == 1 )
    return rv << v;
  else
    return rv << v << '/' << base;
}
olxstr TSymmParser::MatrixToSymm(const smatd& M)  {
  olxstr T, T1, T2;
  char Axis[] = {'X','Y','Z'};
  for( int j=0; j < 3; j ++ )  {
    if( j != 0 )
      T << ',';
    for( int i=0; i < 3; i ++ )  {
      if( i == j )  {
        if( M.r[j][i] != 0 )  {
          //if( M[j][3] != 0 )
          T1 << CharSign(M.r[j][i]);
          if( fabs(fabs(M.r[j][i])-1) > 1e-5 )
            T1 << (double)M.r[j][i];
          T1 << Axis[j];
        }
        continue;
      }
      if( M.r[j][i] != 0 )  {
        T1.Insert(Axis[i], 0);
        T1.Insert(CharSign(M.r[j][i]), 0);
      }
    }
    if( M.t[j] != 0 )  {
      //T2 = FormatFloatX(M[j][3]);
      T2 = olxstr::FormatFloat(3, M.t[j], false);
      T2.TrimFloat();
      T1.Insert(T2, 0);
    }
    T << T1;
    T1 = EmptyString;
  }
  return T;
}
//..............................................................................
olxstr TSymmParser::MatrixToSymmEx(const smatd& M)  {
  olxstr T, T1, T2;
  char Axis[] = {'X','Y','Z'};
  for( int j=0; j < 3; j ++ )  {
    if( j != 0 )
      T << ',';
    for( int i=0; i < 3; i ++ )  {
      if( i == j )  {
        if( M.r[j][i] != 0 )  {
          //if( M[j][3] != 0 )
          T1 << CharSign(M.r[j][i]);
          if( fabs(fabs(M.r[j][i])-1) > 1e-5 )
            T1 << (double)M.r[j][i];
          T1 << Axis[j];
        }
        continue;
      }
      if( M.r[j][i] != 0 )  {
        T1.Insert(Axis[i], 0);
        T1.Insert(CharSign(M.r[j][i]), 0);
      }
    }
    if( M.t[j] != 0 )
      T1.Insert(FormatFloatEx(M.t[j]), 0);
    T << T1;
    T1 = EmptyString;
  }
  return T;
}
//..............................................................................
olxstr TSymmParser::MatrixToSymmEx(const mat3d& M)  {
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
    T << T1;
    T1 = EmptyString;
  }
  return T;
}
//..............................................................................
smatd TSymmParser::SymmCodeToMatrixU(const TUnitCell& UC, const olxstr &Code)  {
  olxstr Tmp;
  TStrList Toks(Code, '_');
  int isymm;
  smatd mSymm;
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
    mSymm.t[0] += (int)(Toks.String(1)[0]-'5');
    mSymm.t[1] += (int)(Toks.String(1)[1]-'5');
    mSymm.t[2] += (int)(Toks.String(1)[2]-'5');
  }
  else  {
    Tmp = Toks.String(1).SubString(0, 2);
    mSymm.t[0] += (Tmp.ToInt()-55);
    Tmp = Toks.String(1).SubString(2, 2);
    mSymm.t[1] += (Tmp.ToInt()-55);
    Tmp = Toks.String(1).SubString(4, 2);
    mSymm.t[2] += (Tmp.ToInt()-55);
  }
  return mSymm;
}
//..............................................................................
smatd TSymmParser::SymmCodeToMatrixA(const TAsymmUnit& AU, const olxstr &Code)  {
  olxstr Tmp;
  TStrList Toks(Code, '_');
  int isymm;
  smatd mSymm;
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
    mSymm.t[0] += (int)(Toks.String(1)[0]-'5');
    mSymm.t[1] += (int)(Toks.String(1)[1]-'5');
    mSymm.t[2] += (int)(Toks.String(1)[2]-'5');
  }
  else  {
    Tmp = Toks.String(1).SubString(0, 2);
    mSymm.t[0] += (Tmp.ToInt()-55);
    Tmp = Toks.String(1).SubString(2, 2);
    mSymm.t[1] += (Tmp.ToInt()-55);
    Tmp = Toks.String(1).SubString(4, 2);
    mSymm.t[2] += (Tmp.ToInt()-55);
  }
  return mSymm;
}
//..............................................................................
smatd TSymmParser::SymmCodeToMatrix(const smatd_list& ml, const olxstr &Code)  {
  olxstr Tmp;
  TStrList Toks(Code, '_');
  int isymm;
  smatd mSymm;
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
    mSymm.t[0] += (int)(Toks.String(1)[0]-'5');
    mSymm.t[1] += (int)(Toks.String(1)[1]-'5');
    mSymm.t[2] += (int)(Toks.String(1)[2]-'5');
  }
  else  {
    Tmp = Toks.String(1).SubString(0, 2);
    mSymm.t[0] += (Tmp.ToInt()-55);
    Tmp = Toks.String(1).SubString(2, 2);
    mSymm.t[1] += (Tmp.ToInt()-55);
    Tmp = Toks.String(1).SubString(4, 2);
    mSymm.t[2] += (Tmp.ToInt()-55);
  }
  mSymm.SetTag( isymm );
  return mSymm;
}
//..............................................................................
// this needs to be of very high performance
olxstr TSymmParser::MatrixToSymmCode(const TUnitCell& UC, const smatd& M)  {
  const smatd& m = UC.GetMatrix( M.GetTag() );
  vec3i Trans( m.t - M.t);
  int baseVal = 5;
  if( (abs(Trans[0]) > 4) || (abs(Trans[1]) > 4) || (abs(Trans[1]) > 4) )
    baseVal = 55;

  static char bf[64];
#ifdef _MSC_VER
  sprintf_s(bf, 64, "%i_%i%i%i", M.GetTag()+1, baseVal - Trans[0], baseVal - Trans[1], baseVal - Trans[2]);
#else
  sprintf(bf, "%i_%i%i%i", M.GetTag()+1, baseVal - Trans[0], baseVal - Trans[1], baseVal - Trans[2]);
#endif
  return olxstr(bf);
}
//..............................................................................
olxstr TSymmParser::MatrixToSymmCode(const smatd_list& ml, const smatd& M)  {
  vec3i Trans( ml[M.GetTag()].t - M.t );
  int baseVal = 5;
  if( (abs(Trans[0]) > 4) || (abs(Trans[1]) > 4) || (abs(Trans[1]) > 4) )
    baseVal = 55;

  static char bf[64];
#ifdef _MSC_VER
  sprintf_s(bf, 64, "%i_%i%i%i", M.GetTag()+1, baseVal - Trans[0], baseVal - Trans[1], baseVal - Trans[2]);
#else
  sprintf(bf, "%i_%i%i%i", M.GetTag()+1, baseVal - Trans[0], baseVal - Trans[1], baseVal - Trans[2]);
#endif
  return olxstr(bf);
}
//..............................................................................

