//---------------------------------------------------------------------------

#ifndef emathH
#define emathH

#include <math.h>
#include "ebase.h"
#include "exception.h"
#include "vpoint.h"

BeginEsdlNamespace()
//---------------------------------------------------------------------------
// returns corresponding character for sign
inline char CharSign(double p )  {  return (p<0) ? '-' : '+';  }
// determines the sign of a number
inline int  Sign(double a)  {  return (a<0)? -1 : 1;  }

// solves an equation by the Newton method
// f - function, df - first derivative, point - starting point
extern double NewtonSolve( double (*f)(double), double (*df)(double), double point);
  // calculates factorila of a number
double Factorial(int a);
  // rounds a floating point number
inline long Round( double a )  {
  long b = (long)a;  // |b| is always smaller than a
  return ((a < 0) ? (((b-a) >= .5) ? --b : b) : (((a-b) >= .5) ? ++b : b));
}

template <class A, class B>
  inline void SetBit( const bool Set, A &V, const B Bit )  {
    if( Set )  V |= Bit;
    else       V &= ~Bit;
  }

template <class T>
  T TetrahedronVolume(const TVPoint<T>& A, const TVPoint<T>& B,
                            const TVPoint<T>& C,const TVPoint<T>& D )  {
    TVPoint<T> a,b,n;
    T d, caS, sa;
    a = A - B;
    b = C - B;
    caS = a.CAngle(b);
    if( fabs(caS) > 0.9999 )  return 0;
    sa = sqrt( 1- caS*caS);
    caS = a.Length() * b.Length() * sa / 2;
    n = a.XProdVec(b);
    d = n[2]*A[2] + n[1]*A[1] + n[0]*A[0];
    d = n[2]*D[2] + n[1]*D[1] + n[0]*D[0] - d;
    d /= n.Length();
    return fabs( caS*d/2 );
  }

// greatest common denominator
extern unsigned int gcd(unsigned int u, unsigned int v);

//extern void SetBit( const bool Set, short &V, const short Bit );

// creates a 3D rotation matrix aroung rv vector, providin cosine of the rotation angle
template <typename MC>
void CreateRotationMatrix(TMatrix<MC>& rm, const TVector<MC>& rv, double ca)  {
  double sa;
  if( fabs(ca) > 0.001 )  sa = sqrt(1-ca*ca);
  else                    sa = 0;
  double t = 1-ca;
  rm[0][0] = t*rv[0]*rv[0] + ca;
  rm[0][1] = t*rv[0]*rv[1] + sa*rv[2];
  rm[0][2] = t*rv[0]*rv[2] - sa*rv[1];

  rm[1][0] = t*rv[0]*rv[1] - sa*rv[2];
  rm[1][1] = t*rv[1]*rv[1] + ca;
  rm[1][2] = t*rv[1]*rv[2] + sa*rv[0];

  rm[2][0] = t*rv[0]*rv[2] + sa*rv[1];
  rm[2][1] = t*rv[1]*rv[2] - sa*rv[0];
  rm[2][2] = t*rv[2]*rv[2] + ca;
}

EndEsdlNamespace()
#endif
