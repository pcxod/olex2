//---------------------------------------------------------------------------

#ifndef emathH
#define emathH

#include <math.h>
#include "exception.h"

BeginEsdlNamespace()
//---------------------------------------------------------------------------
// returns corresponding character for sign
inline char CharSign(double p )  {  return (p<0) ? '-' : '+';  }
// determines the sign of a number
inline int Sign(double a)  {  return (a<0) ? -1 : 1;  }

// solves an equation by the Newton method
// f - function, df - first derivative, point - starting point
extern double NewtonSolve( double (*f)(double), double (*df)(double), double point);
  // calculates factorila of a number
double Factorial(int a);
// rounds a floating point number
template <typename float_t> inline long Round(const float_t a)  {
  long b = (long)a;  // |b| is always smaller than a
  return ((a < 0) ? (((b-a) >= .5) ? --b : b) : (((a-b) >= .5) ? ++b : b));
}
// returns absolute value of a number
template <typename num> inline num olx_abs(num n)  {
  return n < 0 ? -n : n;
}
/* swaps two objects using a temporary variable (copy constructor must be 
 available for complex types) */
template <typename obj> inline void olx_swap(obj& o1, obj& o2)  {
  obj tmp = o1;
  o1 = o2;
  o2 = tmp;
}
// return pow2
template <typename num> inline num sqr(num n) {  return n*n;  } 

template <typename A, typename B>
  inline void SetBit( const bool Set, A &V, const B Bit )  {
    if( Set )  V |= Bit;
    else       V &= ~Bit;
  }
template <typename A, typename B>
  inline void SetBitTrue(A &V, const B Bit )  {
    V |= Bit;
  }
template <typename A, typename B>
  inline void SetBitFalse(A &V, const B Bit )  {
    V &= ~Bit;
  }

template <class VC>
  double TetrahedronVolume(const VC& A, const VC& B, const VC& C,const VC& D )  {
    VC a(A-B),b(C-B),n;
    double d, caS, sa;
    caS = a.CAngle(b);
    if( olx_abs(caS) > 0.9999 )  return 0;
    sa = sqrt( 1- caS*caS);
    caS = a.Length() * b.Length() * sa / 2;
    n = a.XProdVec(b);
    d = n[2]*A[2] + n[1]*A[1] + n[0]*A[0];
    d = n[2]*D[2] + n[1]*D[1] + n[0]*D[0] - d;
    d /= n.Length();
    return olx_abs( caS*d/3 );
  }
  // torsion angle in degrees
template <class VC>
  double TorsionAngle(const VC& v1, const VC& v2, const VC& v3, const VC& v4)  {
    VC V1(v1 - v2), 
      V2(v3 - v2), 
      V3(v4 - v3), 
      V4(v2 - v3);
    V1 = V1.XProdVec(V2);
    V3 = V3.XProdVec(V4);
    return acos(V1.CAngle(V3))*180/M_PI;
  }
  //angle in degrees for three coordinates A-B C-B angle
template <class VC>
  double Angle(const VC& v1, const VC& v2, const VC& v3)  {
    return acos( (v1-v2).CAngle(v3-v2) )*180.0/M_PI;
  }
  //angle in degrees for four coordinates A-B D-C angle
template <class VC>
  double Angle(const VC& v1, const VC& v2, const VC& v3, const VC& v4)  {
    return acos( (v1-v2).CAngle(v4-v3) )*180.0/M_PI;
  }

// greatest common denominator
extern unsigned int gcd(unsigned int u, unsigned int v);

//extern void SetBit( const bool Set, short &V, const short Bit );
//returns volume of a sphere of radius r
inline double SphereVol(double r)  {  return 4.*M_PI/3.0*r*r*r;  }
//returns radius of a sphere of volume v
inline double SphereRad(double v)   {  return pow(v*3.0/(4.0*M_PI), 1./3.);  }
// creates a 3D rotation matrix aroung rv vector, providin cosine of the rotation angle
template <typename MC, typename VC>
void CreateRotationMatrix(MC& rm, const VC& rv, double ca)  {
  double sa;
  if( olx_abs(ca) > 0.001 )  sa = sqrt(1-ca*ca);
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
template <typename MC, typename VC>
void CreateRotationMatrix(MC& rm, const VC& rv, double ca, double sa)  {
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
template <class MC, class VC> MC& QuaternionToMatrix(const VC& qt, MC& matr)  {
  matr[0][0] = qt[0]*qt[0] + qt[1]*qt[1] - qt[2]*qt[2] - qt[3]*qt[3];
  matr[0][1] = 2*(qt[1]*qt[2] - qt[0]*qt[3]);
  matr[0][2] = 2*(qt[1]*qt[3] + qt[0]*qt[2]);

  matr[1][0] = 2*(qt[1]*qt[2] + qt[0]*qt[3]);
  matr[1][1] = qt[0]*qt[0] + qt[2]*qt[2] - qt[1]*qt[1] - qt[3]*qt[3];
  matr[1][2] = 2*(qt[2]*qt[3] - qt[0]*qt[1]);

  matr[2][0] = 2*(qt[1]*qt[3] - qt[0]*qt[2]);
  matr[2][1] = 2*(qt[2]*qt[3] + qt[0]*qt[1]);
  matr[2][2] = qt[0]*qt[0] + qt[3]*qt[3] - qt[1]*qt[1] - qt[2]*qt[2];
  return matr;
}
/* 
generates a new permutation from the original list 
http://en.wikipedia.org/wiki/Permutation
*/
template <class List> void GeneratePermutation(List& out, int perm)  {
  const int cnt = out.Count();
  int fc = (int)Factorial(cnt-1);
  for( int i=0; i < cnt-1; i++ )  {
    int ti = (perm/fc) % (cnt - i);
    int tv = out[i+ti];
    for( int j = i+ti; j > i; j-- )
      out[j] = out[j-1];
    out[i] = tv;
    fc /= (cnt-i-1);
  }
}

static void SinCos(const double ang, double *sina, double *cosa)  {
#ifdef __WIN32__
  _asm  {
    FLD  ang
    FSINCOS
    MOV EAX, [cosa]
    FSTP  QWORD PTR [EAX]    // cosine
    MOV EAX, [sina]
    FSTP  QWORD PTR [EAX]    // sine
    FWAIT
  }
#else
  *sina = sin(ang);
  *cosa = cos(ang);
#endif  
}

EndEsdlNamespace()
#endif
