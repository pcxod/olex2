/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_math_spline_H
#define __olx_sdl_math_spline_H
#include "../evector.h"
#include "../emath.h"
BeginEsdlNamespace()

// inspired by ALGLIB, wikipedia and original sources
namespace math  {  namespace spline {
  const short
    boundary_type_periodic = 0,
    boundary_type_parabolic = 1,
    boundary_type_first_derivative = 2,
    boundary_type_second_derivative = 3;
  const short // values of this MAKES difference, do not change!
    boundary_condition_first_derivative = 1,
    boundary_condition_second_derivative = 2;
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
      for( size_t i=n-2; i != InvalidIndex; i-- )
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
    bool periodic, linear;
    template <typename XT, typename YT> Spline3(const XT &_x, const YT& _y) :
      x(_x), y(_y), periodic(false), linear(false)  {}
    Spline3() : periodic(false), linear(false)  {}
    double interpolate(double t) const {
      if( x.Count() < 2 )
        throw TInvalidArgumentException(__OlxSourceInfo, "spline size");
      if( periodic && ((t > x.GetLast()) || (t < x(0))) )  {  // remap
        const FT period = x.GetLast()-x(0);
        const FT k = t/period;
        t = period*(k - olx_floor(k));
      }
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
      if( linear )
        return y(start)+t*c(start);
      else  {
        const size_t i = start*3;
        return y(start)+t*(c(i)+t*(c(i+1)+t*c(i+2)));
      }
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
    static Spline3<FT>& linear(Spline3<FT>& s)  {
      s.linear = true;
      s.c.Resize(s.x.Count()-1);
      for( size_t i=0; i < s.x.Count()-1; i++ )
        s.c(i) = (s.y(i+1)-s.y(i))/(s.x(i+1)-s.x(i));
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
    /*
    lbl, rbl - one of the boundary_type,
    lb, rb - one of the boundary_condition
    */
    static Spline3<FT>& cubic(Spline3<FT>& s, short lbt, short lb, short rbt, short rb)  {
      if( (lbt == boundary_type_periodic && rbt != boundary_type_periodic) ||
          (rbt == boundary_type_periodic && lbt != boundary_type_periodic) )
      {
        throw TInvalidArgumentException(__OlxSourceInfo, "boundary type");
      }
      const size_t n = s.x.Count();
      if( n == 2 )  {
        if( lbt == boundary_type_parabolic && rbt == boundary_type_parabolic )  {
          lbt = rbt = boundary_type_second_derivative;
          lb = rb = 0;
        }
        else if( lbt == boundary_type_periodic && rbt == boundary_type_periodic )  {
          lbt = rbt = boundary_type_first_derivative;
          lb = rb = 0;
          s.y(1) = s.y(0);
        }
      }
      TVector<FT> a1(n), a2(n), a3(n), b(n), d;
      if( lbt == boundary_type_periodic && rbt == boundary_type_periodic )  {
        a1.Resize(n-1);
        a2.Resize(n-1);
        a3.Resize(n-1);
        b.Resize(n-1);
      }
      for( size_t i=1; i < n-1; i++ )  {
        a1(i) = s.x(i+1)-s.x(i);
        a2(i) = 2*(s.x(i+1)-s.x(i-1));
        a3(i) = s.x(i)-s.x(i-1);
        b(i) = 3*((s.y(i)-s.y(i-1))/a3(i)*a1(i)+ (s.y(i+1)-s.y(i))/a1(i)*a3(i));
      }
      if( lbt == boundary_type_periodic && rbt == boundary_type_periodic )  {
        a1(0) = s.x(1)-s.x(0);
        a3(0) = s.x(n-1)-s.x(n-2);
        a2(0) = 2*(a1(0)+a3(0));
        b(0) = 3*(s.y(n-1)-s.y(n-2))/a3(0)*a1(0) + (s.y(1)-s.y(0))/a1(0)*a3(0);
        util<FT>::solve_cyclic_tridiagonal(a1, a2, a3, b, d);
        d.Resize(n);
        d(n-1) = d(0);
        s.periodic = true;
        return hermite(d, s);
      }
      else  {
        //left side
        if( lbt == boundary_type_parabolic )  {
          a2(0) = a3(0) = 1;
          b(0) = 2*(s.y(1)-s.y(0))/(s.x(1)-s.x(0));
        }
        else if( lbt == boundary_type_first_derivative )  {
          a2(0) = 1;
          b(0) = lb;
        }
        else if( lbt == boundary_type_second_derivative )  {
          a2(0) = 2;
          a3(0) = 1;
          b(0) = 3*(s.y(1)-s.y(0))/(s.x(1)-s.x(0))-lb*(s.x(1)-s.x(0))/2;
        }
        // right side
        if( rbt == boundary_type_parabolic )  {
          a1(n-1) = a2(n-1) = 1;
          a3(n-1) = 0;
          b(n-1) = 2*(s.y(n-1)-s.y(n-2))/(s.x(n-1)-s.x(n-2));
        }
        else if( rbt == boundary_type_first_derivative )  {
          a1(n-1) = a3(n-1) = 0;
          a2(n-1) = 1;
          b(n-1) = rb;
        }
        else if( rbt == boundary_type_second_derivative )  {
          a1(n-1) = 1;
          a2(n-1) = 2;
          a3(n-1) = 0;
          b(n-1) = 3*(s.y(n-1)-s.y(n-2))/(s.x(n-1)-s.x(n-2)) + rb*(s.x(n-1)-s.x(n-2))/2;
        }
        util<FT>::solve_tridiagonal(a1, a2, a3, b, d);
        return hermite(d, s);
      }
    }
    // bt - boundary, parabolic or periodic
    static Spline3<FT>& catmull_rom(Spline3<FT>& s, int bt, FT tension)  {
      if( !(bt == boundary_type_periodic || bt == boundary_type_parabolic) )
        throw TInvalidArgumentException(__OlxSourceInfo, "boundary type");
      const size_t n = s.x.Count();
      if( n == 2 )  {
        if( bt == boundary_type_parabolic )
          return linear(s);
        else
          return cubic(s, boundary_type_periodic, 0, boundary_type_periodic, 0);
      }
      TVector<FT> d(n);
      for( size_t i=1; i < n-1; i++ )
        d(i) = (1-tension)*(s.y(i+1)-s.y(i-1))/(s.x(i+1)-s.x(i-1));
      if( bt == boundary_type_periodic )  {
        s.y.GetLast() = s.y(0);
        d(0) = d(n-1) = (s.y(1)-s.y(n-2))/(2*(s.x(1)-s.x(0)+s.x(n-1)-s.x(n-2)));
        s.periodic = true;
      }
      else  {
        d(0) = 2*(s.y(1)-s.y(0))/(s.x(1)-s.x(0))-d(1);
        d(n-1) = 2*(s.y(n-1)-s.y(n-2))/(s.x(n-1)-s.x(n-2)) - d(n-2);
      }
      return hermite(d, s);
    }
  };
}}; //end of the math::spline

EndEsdlNamespace()
#endif
