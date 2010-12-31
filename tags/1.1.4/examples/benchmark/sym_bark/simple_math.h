#ifndef __TEST_SIMPLE_MATH
#define __TEST_SIMPLE_MATH

#include "fastsymm.h"

struct SimpleMath  {
  static inline void mul_i(double* v, double* m)  {
    double a1 = m[0]*v[0] + m[3]*v[1] + m[6]*v[2];
    double a2 = m[1]*v[0] + m[4]*v[1] + m[7]*v[2];
         v[2] = m[2]*v[0] + m[5]*v[1] + m[8]*v[2];
  }
  static void mul_f(double* v, double* m )  {
    double rs[3];
    rs[0] = 0;  rs[1] = 0;  rs[2] = 0;
    for( int i=0; i < 3; i++ )
      for( int j=0; j < 3; j++ )  {
        rs[i] += v[j] * m[i+j*3];
      }
    v[0] = rs[0];  v[1] = rs[1];  v[2] = rs[2];
  }
  static inline void mul_i(double* v, double m[3][3])  {
    double a1 = m[0][0]*v[0] + m[0][1]*v[1] + m[0][2]*v[2];
    double a2 = m[1][0]*v[0] + m[1][1]*v[1] + m[1][2]*v[2];
         v[2] = m[2][0]*v[0] + m[2][1]*v[1] + m[2][2]*v[2];
  }
  static void mul_f(double* v, double m[3][3] )  {
    double rs[3];
    rs[0] = 0;  rs[1] = 0;  rs[2] = 0;
    for( int i=0; i < 3; i++ )
      for( int j=0; j < 3; j++ )  {
        rs[i] += v[j] * m[i][j];
      }
    v[0] = rs[0];  v[1] = rs[1];  v[2] = rs[2];
  }
  static inline void mul_i(double* v, double** m)  {
    double a1 = m[0][0]*v[0] + m[0][1]*v[1] + m[0][2]*v[2];
    double a2 = m[1][0]*v[0] + m[1][1]*v[1] + m[1][2]*v[2];
         v[2] = m[2][0]*v[0] + m[2][1]*v[1] + m[2][2]*v[2];
  }
  static void mul_f(double* v, double** m )  {
    double rs[3];
    rs[0] = 0;  rs[1] = 0;  rs[2] = 0;
    for( int i=0; i < 3; i++ )
      for( int j=0; j < 3; j++ )  {
        rs[i] += v[j] * m[i][j];
      }
    v[0] = rs[0];  v[1] = rs[1];  v[2] = rs[2];
  }
};

inline void MultDA101201110(register double* v)  {
  register double a0 = -v[1];
  v[1] = v[0]+a0;
  v[2] = -v[2];
  v[0] = a0;
}


class TFSymmI  {
public:
  virtual void Mult(register double* v) const = 0;
};

class TFSymmImp : public TFSymmI {
public:
  inline virtual void Mult(register double* v )  const  { MultDA101201110(v); }
};

#endif