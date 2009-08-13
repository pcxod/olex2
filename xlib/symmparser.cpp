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
const char TSymmParser::Axis[] = {'X','Y','Z'};
//..............................................................................
// Transforms standard SYMM operation (INS, CIF files) to matrix
bool TSymmParser::SymmToMatrix(const olxstr& S, smatd& M)  {
  olxstr p, p1, opr;
  double ratio;
  int op, index;
  bool res = true;
  str_stack stack;
  M.Null();
  TStrList toks(olxstr::UpperCase(S), ',');
  if( toks.Count() != 3 )
    throw TFunctionFailedException(__OlxSourceInfo,
         olxstr("Operation sign is missing or operation is incomplete while parsing \"") << S << '\"');

  for( int i=0; i < 3; i++ )  {
    stack.LoadFromExpression(toks[i]);
    p = stack.Pop();
    op = IsAxis(p);
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
    if( abs(p.CharAt(0)-'Z') > 2 )  {
      res = false;
      break;
    }
    index = 2 - 'Z' + p.CharAt(0);
    M.r[i][index] = 1;
    if( stack.IsEmpty() )  continue;
    p = stack.Pop();
    if( p.CharAt(0) == '-' )     M.r[i][index] = -1;  // inversion
    if( stack.IsEmpty() )  continue;
    p = stack.Pop();
    op = IsAxis(p);
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
  // normalise the translations...
  for( int i=0; i < 3; i++ )  {
    if( M.t[i] == 0 )  continue;
    int v = olx_abs(olx_round(M.t[i]*12)), base = 12;
    int denom = esdl::gcd(v, base);
    if( denom != 1 )  {
      v /= denom;
      base /= denom;
    }
    M.t[i] = v;
    if( base != 1 )
      M.t[i] /= base;
  }
  return true;
}
//..............................................................................
smatd TSymmParser::SymmCodeToMatrixU(const class TUnitCell& UC, const olxstr& Code)  {
  return _SymmCodeToMatrix(UC, Code);
}
//..............................................................................
smatd TSymmParser::SymmCodeToMatrixA(const class TAsymmUnit& AU, const olxstr& Code)  {
  return _SymmCodeToMatrix(AU, Code);
}
//..............................................................................
olxstr TSymmParser::MatrixToSymmEx(const mat3i& M)  {
  olxstr T, T1;
  for( int j=0; j < 3; j ++ )  {
    if( j != 0 )
      T << ',';
    for( int i=0; i < 3; i ++ )  {
      if( i == j )  {
        if( M[j][i] != 0 )  {
          T1 << CharSign(M[j][i]);
          T1 << Axis[j];
        }
      }
      else if( M[j][i] != 0 )  {
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

