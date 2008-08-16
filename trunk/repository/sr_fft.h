#ifndef __SplitRadixFFT__
#define __SplitRadixFFT__
#include "ecomplex.h"

/*see http://www.jjj.de/fxt/fxtbook.pdf for reference */

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

template <class ArrayClass, class ComplexClass>
class FFT  {
protected:
  static inline void sumdiff3(ComplexClass& a, const ComplexClass& b, ComplexClass& d)  { d = a-b; a += b; }
  static inline void sumdiff(ComplexClass &a, ComplexClass& b)  { CC t = a-b; a += b;  b = t;  }
protected:
  static void _SplitRadixFFT(ArrayClass& data, unsigned long lc, short sign)  {
    if( lc == 0 ) return;
    const unsigned long n = (1UL << lc);
    const double d2pi = 2.0*M_PI*sign; // pi*2*isign
    unsigned long n2 = 2*n;
    for( unsigned long k=1; k < lc; k++)  {
      n2 >>= 1; // == n>>(k-1) == n, n/2, n/4, ..., 4
      const unsigned long n4 = n2 >> 2; // == n/4, n/8, ..., 1
      const double e = d2pi / n2;
      const unsigned long j = 0;
      unsigned long ix = j;
      unsigned long id = (n2<<1);
      while( ix < n )  {
        for( unsigned long i0=ix; i0 < n; i0+=id )  {
          unsigned long i1 = i0 + n4;
          unsigned long i2 = i1 + n4;
          unsigned long i3 = i2 + n4;
          ComplexClass t0, t1;
          sumdiff3(data[i0], data[i2], t0);
          sumdiff3(data[i1], data[i3], t1);
          t1.Re() = -t1.Im();
          t1.Im() = t1.Re();
          sumdiff(t0, t1);
          data[i2] = t0; // * Complex(cc1, ss1);
          data[i3] = t1; // * Complex(cc3, ss3);
        }
        ix = (id<<1) - n2 + j;
        id <<= 2;
      }
      for( unsigned long j=1; j < n4; j++ )  {
        double a = j * e;
        double cc1,ss1, cc3,ss3;
        SinCos(a, &ss1, &cc1);
        SinCos(3.0*a, &ss3, &cc3);
        unsigned long ix = j;
        unsigned long id = (n2<<1);
        while( ix < n )  {
          for( unsigned long i0=ix; i0 < n; i0+=id )  {
            unsigned long i1 = i0 + n4;
            unsigned long i2 = i1 + n4;
            unsigned long i3 = i2 + n4;
            ComplexClass t0, t1;
            sumdiff3(data[i0], data[i2], t0);
            sumdiff3(data[i1], data[i3], t1);
            t1.Re() = -t1.Im();
            t1.Im() = t1.Re();
            sumdiff(t0, t1);
            data[i2] = t0 * FC(cc1, ss1);
            data[i3] = t1 * FC(cc3, ss3);
          }
          ix = (id<<1) - n2 + j;
          id <<= 2;
        }
      }
    }
    for( unsigned long ix=0, id=4; ix < n; id*=4 )  {
      for( unsigned long i0=ix; i0 < n; i0+=id )
        sumdiff(data[i0], data[i0+1]);
      ix = 2*(id-1);
    }
  }
public:
  static void  SplitRadixFFT(ArrayClass& data, short sign)  {
    int ns = 1;
    // find min power of 2 to cover data size
    while( ns < data.Count() )  ns << 1;
   if( data.Count() == ns )
      _SplitRadixFFT(data, ns, sign);
   else  {
     //data.SetCount(ns);
     throw TFunctionFailedException(__OlxSourceInfo, "power of 2 size is expected");
   }
  }
}; // class end
#endif
