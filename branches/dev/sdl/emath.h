#ifndef __olx_emath_H
#define __olx_emath_H
#include <math.h>
#include "exception.h"
// Linux stuff....
#undef QLength

BeginEsdlNamespace()
//---------------------------------------------------------------------------
// returns corresponding character for sign
template <typename T> inline olxch olx_sign_char(const T& p)  {  return (p<0) ? olxT('-') : olxT('+');  }
// determines the sign of a number
template <typename T> inline T olx_sign(const T& a)  {  return (T)((a < 0 ) ? -1 : 1);  }
// comparison function (useful for the size_t on Win64, where size_t=uint64_t and int is int32_t)
template <typename T1, typename T2>
  inline int olx_cmp(T1 a, T2 b)  {  return a < b ? -1 : (a > b ? 1 : 0);  }
/* solves an equation by the Newton method
 f - function, df - first derivative, point - starting point */
extern double olx_newton_solve( double (*f)(double), double (*df)(double), double point);
// calculates factorial of a number
template <typename arg_t> double olx_factorial(const arg_t& a)  {
  double b=1;
  for( arg_t i=2; i <= a; i++ )  b *= i;
  return b;
}
template <typename ret_t, typename arg_t> ret_t olx_factorial_t(const arg_t& a)  {
  arg_t b=1;
  for( arg_t i=2; i <= a; i++ )  b *= i;
  return static_cast<ret_t>(b);
}
// rounds a floating point number
template <typename float_t> inline long olx_round(const float_t a)  {
  long b = (long)a;  // |b| is always smaller than a
  return ((a < 0) ? (((b-a) >= .5) ? --b : b) : (((a-b) >= .5) ? ++b : b));
}
template <typename int_t, typename float_t> inline int_t olx_round_t(const float_t a)  {
  int_t b = (int_t)a;  // |b| is always smaller than a
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
template <typename num> inline num olx_sqr(num n) {  return n*n;  } 

template <typename A, typename B>
  inline void olx_set_bit(const bool Set, A &V, const B Bit)  {
    if( Set )  V |= Bit;
    else       V &= ~Bit;
  }
template <typename A, typename B>
  inline void olx_set_true(A &V, const B Bit)  {
    V |= Bit;
  }
template <typename A, typename B>
  inline void olx_set_false(A &V, const B Bit)  {
    V &= ~Bit;
  }

// throws an exception
template <class VC>
  double olx_tetrahedron_volume(const VC& A, const VC& B, const VC& C,const VC& D)  {
    const VC a(A-B), b(C-B);
    if( a.QLength()*b.QLength() < 1e-15 )
      throw TDivException(__OlxSourceInfo);
    double caS = a.CAngle(b);
    if( olx_abs(caS) > (1.0-1e-15) )  
      return 0;
    const double sa = sqrt(1 - caS*caS);
    caS = a.Length() * b.Length() * sa / 2;
    VC n = a.XProdVec(b);
    double d = n[2]*A[2] + n[1]*A[1] + n[0]*A[0];
    d = n[2]*D[2] + n[1]*D[1] + n[0]*D[0] - d;
    d /= n.Length();
    return olx_abs(caS*d/3);
  }
  // dihedral angle in degrees [0..180], throws an exception
template <class VC>
  double olx_dihedral_angle(const VC& v1, const VC& v2, const VC& v3, const VC& v4)  {
    const VC a( (v1-v2).XProdVec(v3-v2) ), 
             b( (v2-v3).XProdVec(v4-v3) ); 
    if( a.QLength()*b.QLength() < 1e-15 )
      throw TDivException(__OlxSourceInfo);
    return acos(a.CAngle(b))*180/M_PI;
  }
  /* dihedral angle in degrees (-180..180] 
  http://en.wikipedia.org/wiki/Dihedral_angle
  */
template <class VC>
  double olx_dihedral_angle_signed(const VC& v1, const VC& v2, const VC& v3, const VC& v4)  {
    const VC b1(v2-v1), b2(v3-v2), b3(v4-v3),
             b2xb3(b2.XProdVec(b3));
    return atan2(b2.Length()*b1.DotProd(b2xb3), b1.XProdVec(b2).DotProd(b2xb3))*180/M_PI;
  }
  //angle in degrees for three coordinates A-B B-C angle
template <class VC>
  double olx_angle(const VC& v1, const VC& v2, const VC& v3)  {
    const VC a(v1-v2),
             b(v3-v2);
    if( a.QLength()*b.QLength() < 1e-15 )
      throw TDivException(__OlxSourceInfo);
    return acos(a.CAngle(b))*180.0/M_PI;
  }
  //angle in degrees for four coordinates A-B D-C angle
template <class VC>
  double olx_angle(const VC& v1, const VC& v2, const VC& v3, const VC& v4)  {
    const VC a(v1-v2),
             b(v4-v3);
    if( a.QLength()*b.QLength() < 1e-15 )
      throw TDivException(__OlxSourceInfo);
    return acos( a.CAngle(b) )*180.0/M_PI;
  }

// greatest common denominator
extern unsigned int olx_gcd(unsigned int u, unsigned int v);

//extern void olx_set_bit( const bool Set, short &V, const short Bit );
//returns volume of a sphere of radius r
inline double olx_sphere_volume(double r)  {  return 4.*M_PI/3.0*r*r*r;  }
//returns radius of a sphere of volume v
inline double olx_sphere_radius(double v)  {  return pow(v*3.0/(4.0*M_PI), 1./3.);  }
// creates a 3D rotation matrix aroung rv vector, provided cosine of the rotation angle

template <typename FloatType, typename MC, typename VC>
MC& olx_create_rotation_matrix_(MC& rm, const VC& rv, FloatType ca)  {
  const FloatType sa = (olx_abs(ca) > 1e-15) ? sqrt(1-ca*ca) : 1;
  const FloatType t = 1-ca;
  rm[0][0] = t*rv[0]*rv[0] + ca;
  rm[0][1] = t*rv[0]*rv[1] + sa*rv[2];
  rm[0][2] = t*rv[0]*rv[2] - sa*rv[1];

  rm[1][0] = t*rv[0]*rv[1] - sa*rv[2];
  rm[1][1] = t*rv[1]*rv[1] + ca;
  rm[1][2] = t*rv[1]*rv[2] + sa*rv[0];

  rm[2][0] = t*rv[0]*rv[2] + sa*rv[1];
  rm[2][1] = t*rv[1]*rv[2] - sa*rv[0];
  rm[2][2] = t*rv[2]*rv[2] + ca;
  return rm;
}
template <typename FloatType, typename MC, typename VC>
MC& olx_create_rotation_matrix_(MC& rm, const VC& rv, FloatType ca, FloatType sa)  {
  const FloatType t = 1-ca;
  rm[0][0] = t*rv[0]*rv[0] + ca;
  rm[0][1] = t*rv[0]*rv[1] + sa*rv[2];
  rm[0][2] = t*rv[0]*rv[2] - sa*rv[1];

  rm[1][0] = t*rv[0]*rv[1] - sa*rv[2];
  rm[1][1] = t*rv[1]*rv[1] + ca;
  rm[1][2] = t*rv[1]*rv[2] + sa*rv[0];

  rm[2][0] = t*rv[0]*rv[2] + sa*rv[1];
  rm[2][1] = t*rv[1]*rv[2] - sa*rv[0];
  rm[2][2] = t*rv[2]*rv[2] + ca;
  return rm;
}

template <typename MC, typename VC>
MC& olx_create_rotation_matrix(MC& rm, const VC& rv, double ca)  {
  return olx_create_rotation_matrix_<double, MC, VC>(rm, rv, ca);
}
template <typename MC, typename VC>
MC& olx_create_rotation_matrix(MC& rm, const VC& rv, double ca, double sa)  {
  return olx_create_rotation_matrix_<double, MC, VC>(rm, rv, ca, sa);
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
template <class List> void GeneratePermutation(List& out, size_t perm)  {
  const size_t cnt = out.Count();
  size_t fc = olx_factorial_t<size_t, size_t>(cnt-1);
  for( size_t i=0; i < cnt-1; i++ )  {
    size_t ti = (perm/fc) % (cnt - i);
    size_t tv = out[i+ti];
    for( size_t j = i+ti; j > i; j-- )
      out[j] = out[j-1];
    out[i] = tv;
    fc /= (cnt-i-1);
  }
}

template <typename float_type>
  static void olx_sincos(const float_type ang, float_type *sina, float_type *cosa)  {
#if defined(__WIN32__) && !defined(_WIN64)
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
