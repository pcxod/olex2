#ifndef __olx_sdl_math_spline_H
#define __olx_sdl_math_spline_H
#include "../evector.h"
#include "../emath.h"
#include "../testsuit.h"
// inspired by ALGLIB, wikipedia and original sources
BeginEsdlNamespace()

namespace math  {  namespace spline {
  template <typename FT>
  struct util  {
    static double diff3(FT t, FT x1, FT x2, FT x3, FT y1, FT y2, FT y3)  {
      x2 -= x1;
      x3 -= x1;
      const FT a = (y3-y1-x3/x2*(y2-y1))/(x3*x3-x3*x2);
      const FT b = (y2-y1-a*x2*x2)/x2;
      return 2*a*(t-x1)+b;
    }
    template <typename AT, typename BT, typename CT, typename DT, typename XT>
    static void solve_tridiagonal(AT& a, BT& b, CT& c, DT& d, XT& x)  {
      const size_t n = a.Count();
      x.Resize(n);
      a(0) = c(c.Count()-1) = 0;
      for( size_t i=1; i < n; i++ )  {
        const FT t = a(i)/b(i-1);
        b(i) -= t*c(i-1);
        d(i) -= t*d(i-1);
      }
      x(n-1) = d(n-1)/b(n-1);
      for( size_t i=n-2; i != InvalidIndex; i++ )
        x(i) = (d(i)-c(i)*x(i+1))/b(i);
    }
    template <typename AT, typename BT, typename CT, typename DT, typename XT>
    static void solve_cyclic_tridiagonal(AT& a, BT& b, CT& c, DT& d, XT& x)  {
      const size_t n = a.Count();
      const FT alpha = c(n-1), beta = a(0), gamma = -b(0);
      b(0) *= 2;
      b(n-1) -= alpha*beta/gamma;
      TVector<FT> u(n), y, z;
      u(0) = gamma;
      u(n-1) = alpha;
      solve_tridiagonal(a, b, c, d, y);
      solve_tridiagonal(a, b, c, u, z);
      x.Resize(n);
      const FT k = (y(0)+beta*y(n-1)/gamma)/(1+z(0)+beta*z(n-1)/gamma);
      for( size_t i=0; i < n; i++ )
        x(i) = y(i)-k*z(i);
    }
  };
  template <typename FT> struct Spline3 {
    TVector<FT> x, y,  //original data
      c;  // spline coefficients
    template <typename XT, typename YT> Spline3(const XT &_x, const YT& _y) : x(_x), y(_y)  {}
    Spline3()  {}
    double interpolate(double t) const {
      if( x.IsEmpty() || t < x(0) || t > x.GetLast() )
        throw TInvalidArgumentException(__OlxSourceInfo, "spline");
      size_t start = 0, end = x.Count()-1;
      while( start != end-1 )  {
        const size_t mid = (start+end)/2;
        if( x(mid) > t )
          end = mid;
        else
          start = mid;
      }
      t -= x(start);
      const size_t i = start*3;
      return y(start)+t*(c(i)+t*(c(i+1)+t*c(i+2)));
    }
  };
  template <typename FT> struct Builder  {
    // s.x and s.y must be initialised
    template <typename VecT>
    static Spline3<FT>& hermite(const VecT& d, Spline3<FT>& s)  {
      s.c.Resize((s.x.Count()-1)*3);
      for( size_t i=0; i < s.x.Count()-1; i++ )  {
        const FT dx = s.x(i+1)-s.x(i),
          dx2 = dx*dx, dx3 = dx2*dx;
        const size_t fi = 3*i;
        s.c(fi) = d(i);
        s.c(fi+1) = (3*(s.y(i+1)-s.y(i))-(2*d(i)+d(i+1))*dx)/dx2;
        s.c(fi+2) = (2*(s.y(i)-s.y(i+1))+(d(i)+d(i+1))*dx)/dx3;
      }
      return s;
    }
    //J. of the Association for Computing Machinery, 17(4), 1970, 589-502
    static Spline3<FT>& akima(Spline3<FT>& s)  {
      TVector<FT>
        diffs(s.x.Count()-1),
        w(s.x.Count()-2),
        d(s.x.Count());
      for( size_t i=0; i < s.x.Count()-1; i++ )  {
        diffs(i) = (s.y(i+1)-s.y(i))/(s.x(i+1)-s.x(i));
        if( i > 0 )
          w(i-1) = olx_abs(diffs(i)-diffs(i-1));
      }
      for( size_t i=1; i < w.Count()-1; i++ )  {
        if( (w(i-1) + w(i+1)) != 0 )
          d(i+1) = (w(i+1)*diffs(i) + w(i-1)*diffs(i+1))/(w(i+1)+w(i-1));
        else
          d(i+1) = ((s.x(i+2)-s.x(i+1))*diffs(i)+(s.x(i+1)-s.x(i))*diffs(i+1))/(s.x(i+2)-s.x(i));
      }
      d(0) = util<FT>::diff3(s.x(0), s.x(0), s.x(1), s.x(2), s.y(0), s.y(1), s.y(2));
      d(1) = util<FT>::diff3(s.x(1), s.x(0), s.x(1), s.x(2), s.y(0), s.y(1), s.y(2));
      const size_t n = s.x.Count();
      d(n-2) = util<FT>::diff3(s.x(n-2), s.x(n-3), s.x(n-2), s.x(n-1), s.y(n-3), s.y(n-2), s.y(n-1));
      d(n-1) = util<FT>::diff3(s.x(n-1), s.x(n-3), s.x(n-2), s.x(n-1), s.y(n-3), s.y(n-2), s.y(n-1));
      return hermite(d, s);
    }
  };

  void test(OlxTests& t);
  static void AddTests(OlxTests& t)  {
    t.Add(&test);
  }
}}; //end of the math::spline
EndEsdlNamespace()
#endif
