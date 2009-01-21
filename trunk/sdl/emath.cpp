//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include <math.h>
#include "emath.h"
#include "exception.h"

UseEsdlNamespace()

//..............................................................................
double Factorial( int a)  {
  double b=1;
  for(int i=1; i<=a; i++ )   b*=i;
  return b;
}
//..............................................................................
double GlobalEsdlFunction(NewtonSolve)( double (*f)(double), double (*df)(double), double point)  {
  double ex1=100,x = point,x1;
  long count=0;
  while( ex1 > 1e-5 )  {
    x1 = x - f(x)/df(x);
    ex1 = olx_abs( f(x1));
    x = x1;
    count++;
    if( count >= 15000 )
      throw TFunctionFailedException(__OlxSourceInfo, "too many iterations");
  }
  return x;
}
//..............................................................................
// see wikipedia.com
unsigned int GlobalEsdlFunction(gcd)(unsigned int u, unsigned int v)  {
  int shift;
  /* GCD(0,x) := x */
  if (u == 0 || v == 0)
    return u | v;

  /* Let shift := lg K, where K is the greatest power of 2
     dividing both u and v. */
  for (shift = 0; ((u | v) & 1) == 0; ++shift) {
      u >>= 1;
      v >>= 1;
  }

  while ((u & 1) == 0)
    u >>= 1;

  /* From here on, u is always odd. */
  do {
      while ((v & 1) == 0)  /* Loop X */
        v >>= 1;
      /* Now u and v are both odd, so diff(u, v) is even.
         Let u = min(u, v), v = diff(u, v)/2. */
      if (u < v)
          v -= u;
      else {
          int diff = u - v;
          u = v;
          v = diff;
      }
      v >>= 1;
  }
  while (v != 0);

  return u << shift;
}

