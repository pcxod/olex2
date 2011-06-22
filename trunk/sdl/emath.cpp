/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include <math.h>
#include "emath.h"
#include "exception.h"

double GlobalEsdlFunction(olx_newton_solve)(double (*f)(double), double (*df)(double), double point)  {
  double ex1 = 100,
         x = point;
  size_t count = 0;
  while( ex1 > 1e-5 )  {
    double x1 = x - f(x)/df(x);
    ex1 = olx_abs(f(x1));
    x = x1;
    count++;
    if( count >= 15000 )
      throw TFunctionFailedException(__OlxSourceInfo, "too many iterations");
  }
  return x;
}
//..............................................................................
// see wikipedia.com
unsigned int GlobalEsdlFunction(olx_gcd)(unsigned int u, unsigned int v)  {
  /* GCD(0,x) := x */
  if( u == 0 || v == 0 )
    return u | v;

  /* Let shift := lg K, where K is the greatest power of 2
  dividing both u and v. */
  int shift;
  for( shift = 0; ((u|v) & 1) == 0; ++shift) {
    u >>= 1;
    v >>= 1;
  }

  while( (u & 1) == 0 )
    u >>= 1;

  /* From here on, u is always odd. */
  do {
    while( (v&1) == 0 )  /* Loop X */
      v >>= 1;
    /* Now u and v are both odd, so diff(u, v) is even.
    Let u = min(u, v), v = diff(u, v)/2. */
    if( u < v )
      v -= u;
    else {
      int diff = u - v;
      u = v;
      v = diff;
    }
    v >>= 1;
  }
  while( v != 0 );
  return u << shift;
}
