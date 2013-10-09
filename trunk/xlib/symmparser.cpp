/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "symmparser.h"
#include "estack.h"
#include "estrlist.h"
#include "emath.h"
#include "exparse/expbuilder.h"
//---------------------------------------------------------------------------
//..............................................................................
// Transforms standard SYMM operation (INS, CIF files) to matrix
smatdd TSymmParser::SymmToMatrix(const olxstr& S)  {
  using namespace exparse;
  smatdd M;
  bool res = true;
  try {
    M.Null();
    TStrList toks(S.ToUpperCase().DeleteChars(' '), ',');
    if (toks.Count() != 3) {
      res = false;
      throw TFunctionFailedException(__OlxSourceInfo,
        olxstr("Invalid number of tokens ").quote() << S);
    }
    str_stack stack;
    try {
      for (int i=0; i < 3; i++) {
        stack.LoadFromExpression(toks[i]);
        while (!stack.IsEmpty()) {
          olxstr p = stack.Pop();
          const short op = IsAxis(p);
          if (op < 0) {
            if (stack.IsEmpty()) {
              M.t[i] = p.ToDouble();
              continue;
            }
            olxstr p1 = stack.Pop();
            if (p1.CharAt(0) == '/') {
              p1 = stack.Pop();
              double ratio = p1.ToDouble()/p.ToDouble(), sig=1;
              if (!stack.IsEmpty()) {
                p1 = stack.Pop();
                if (p1 == '-') sig = -1;
                else if (p1 != '+') {
                  throw TInvalidArgumentException(__OlxSourceInfo, "expression");
                }
              }
              M.t[i] = ratio*sig;
            }
            else {
              M.t[i] = p.ToDouble();
              if (p1 == '-')
                M.t[i] = -M.t[i];
              else if (p1 != '+')
                throw TInvalidArgumentException(__OlxSourceInfo, "expression");
            }
          }
          else {
            M.r[i][op] = 1;
            if (stack.IsEmpty()) continue;
            p = stack.Pop();
            if (p == '-')
              M.r[i][op] = -1;  // inversion
            else if (p == '*') {
              p = stack.Pop();
              M.r[i][op] = p.ToDouble();
              if (!stack.IsEmpty()) {
                p = stack.Pop();
                if (p == '-') {
                  M.r[i][op] = -M.r[i][op];
                }
              }
            }
            else if (p != '+')
              throw TInvalidArgumentException(__OlxSourceInfo, "expression");
          }
        }
      }
    }
    catch (const TExceptionBase &e) {
      res = false;
    }
    if( res == false )  {
      throw TFunctionFailedException(__OlxSourceInfo,
        olxstr("due to operation sign is missing or operation is incomplete "
        "while parsing ").quote() << S);
    }
    // normalise the translations...
    for( int i=0; i < 3; i++ )  {
      if( M.t[i] == 0 )  continue;
      int v = olx_abs(olx_round(M.t[i]*12)), base = 12;
      int denom = olx_gcd(v, base);
      if( denom != 1 )  {
        v /= denom;
        base /= denom;
      }
      M.t[i] = (M.t[i] < 0 ? -v : v);
      if( base != 1 )
        M.t[i] /= base;
    }
    return M;
  }
  catch(const TExceptionBase&)  {
    throw TFunctionFailedException(__OlxSourceInfo, olxstr("to parse SYMM card ").quote() << S);
  }
}
//..............................................................................
bool TSymmParser::IsAbsSymm(const olxstr& s)  {
  if( s.Length() < 5 )  return false;
  if( s.IndexOf(',') != InvalidIndex )  { // asbolute representation
    try  {
      SymmToMatrix(s);
      return true;
    }
    catch(...)  {  return false;  }
  }
  return false;
}
//..............................................................................
bool TSymmParser::IsRelSymm(const olxstr& s)  {
  if( s.Length() < 4 )  return false;
  const size_t ui = s.IndexOf('_');
  if( ui != InvalidIndex )  {
    const size_t r = s.Length()-ui-1;
    if( !(r == 3 || r == 6) )
      return false;
    for( size_t i=0; i < s.Length(); i++ )  {
      if( i == ui )  continue;
      if( !olxstr::o_isdigit(s.CharAt(i)) )
        return false;
    }
  }
  else  {
    if( s.Length() < 4 || s.Length() > 9 )
      return false;
    for( size_t i=0; i < s.Length(); i++ )  {
      if( !olxstr::o_isdigit(s.CharAt(i)) )
        return false;
    }
  }
  return true;
}
//..............................................................................
bool TSymmParser::IsSymm(const olxstr& s)  {
  if( !IsRelSymm(s) )
    return IsAbsSymm(s);
  return true;
}
//..............................................................................
