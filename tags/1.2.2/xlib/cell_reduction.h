/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xlib_cell_reduction_H
#define __olx_xlib_cell_reduction_H
#include "xbase.h"
#include "evector.h"
#include "symmlib.h"

BeginXlibNamespace()

// I. Krivy and B. Gruber. Acta Cryst. (1976) A32, 297-298
struct Niggli {
  struct EpsilonComparator  {
    double epsilon;
    EpsilonComparator(double eps=1e-3) : epsilon(eps) {}
    bool eq(double a, double b)  {
      return olx_abs(a-b) < epsilon;
    }
    bool lt(double a, double b)  {
      return a < b - epsilon;
    }
    bool gt(double a, double b)  {
      //return a > b + epsilon;
      return b < a - epsilon;
    }
  };
  template <typename arr_t>
  static evecd reduce_const(short _latt, const arr_t& arr)  {
    vec3d s(arr[0], arr[1], arr[2]), a(arr[3], arr[4], arr[5]);
    reduce(_latt, s, a);
    return evecd(s[0], s[1], s[2], a[0], a[1], a[2]);
  }
  template <typename arr_t>
  static arr_t& reduce(short _latt, arr_t& arr)  {
    vec3d s(arr[0], arr[1], arr[2]), a(arr[3], arr[4], arr[5]);
    reduce(_latt, s, a);
    arr[0] = s[0];  arr[1] = s[1];  arr[2] = s[2];
    arr[3] = a[0];  arr[4] = a[1];  arr[5] = a[2];
    return arr;
  }
  static void reduce(short _latt, vec3d& sides, vec3d& angles)  {
    double R[6];
    vec3d cs(cos(angles[0]/180.0*M_PI), cos(angles[1]/180.0*M_PI),
      cos(angles[2]/180.0*M_PI));
    double vol = sides.Prod()*sqrt(1-cs.QLength()+2*cs.Prod());
    // Acta Cryst. (2004). A60, 1–6
    EpsilonComparator cmp(1e-5*pow(vol, 1./3));

    mat3d metr_m(
      olx_sqr(sides[0]), sides[0]*sides[1]*cs[2], sides[0]*sides[2]*cs[1],
      olx_sqr(sides[1]), sides[1]*sides[2]*cs[0],
      olx_sqr(sides[2]));

    mat3d tm = TCLattice::FromPrimitive(_latt);
    tm = tm.Inverse();
    metr_m = mat3d::Transpose(tm)*metr_m*tm;

    R[0] = metr_m[0][0];
    R[1] = metr_m[1][1];
    R[2] = metr_m[2][2];

    R[3] = 2*metr_m[1][2];
    R[4] = 2*metr_m[0][2];
    R[5] = 2*metr_m[0][1];
    bool Used = true;
    int Cycles = 0;
    while ( Used )  {
      Used = false;
      Cycles ++;
      if( Cycles > 100 )
        throw TFunctionFailedException(__OlxSourceInfo, "procedure did not converge");
      if( cmp.gt(R[0],R[1]) ||
        (cmp.eq(R[0],R[1]) && cmp.gt(olx_abs(R[3]), olx_abs(R[4]))) )
      { //1
        olx_swap(R[0], R[1]);
        olx_swap(R[3], R[4]);
        Used = true;
      }
      if( cmp.gt(R[1],R[2]) ||
        (cmp.eq(R[1],R[2]) && cmp.gt(olx_abs(R[4]),olx_abs(R[5]))) )
      { //2
        olx_swap(R[1], R[2]);
        olx_swap(R[4], R[5]);
        Used = true;
        continue;
      }
      if( cmp.gt(R[3]*R[4]*R[5], 0) )  { //3
        R[3] = olx_abs(R[3]);
        R[4] = olx_abs(R[4]);
        R[5] = olx_abs(R[5]);
      }
      else  {
        R[3] = -olx_abs(R[3]);
        R[4] = -olx_abs(R[4]);
        R[5] = -olx_abs(R[5]);
      }
      if( cmp.gt(olx_abs(R[3]),R[1]) ||
        (cmp.eq(R[3],R[1]) && cmp.lt(2*R[4],R[5]) ) || //5
        (cmp.eq(R[3],-R[1])&&cmp.lt(R[5],0)) )
      {
        double sig = olx_sign(R[3]);
        R[2] += R[1] - R[3]*sig;
        R[3] -= 2*R[1]*sig;
        R[4] -= R[5]*sig;
        Used = true;
        continue;
      }
      if( cmp.gt(olx_abs(R[4]),R[0]) ||
        (cmp.eq(R[4],R[0]) && cmp.lt(2*R[3],R[5])) ||  //6
        (cmp.eq(R[4],-R[0]) && cmp.lt(R[5],0)) )
      {
        double sig = olx_sign(R[4]);
        R[2] += R[0] - R[4]*sig;
        R[3] -= R[5]*sig;
        R[4] -= 2*R[0]*sig;
        Used = true;
        continue;
      }
      if( cmp.gt(olx_abs(R[5]),R[0]) ||
        (cmp.eq(R[5],R[0]) && cmp.lt(2*R[3],R[4])) ||  //7
        (cmp.eq(R[5],-R[0]) && cmp.lt(R[4],0)) )
      {
        double sig = olx_sign(R[5]);
        R[1] += R[0] - R[5]*sig;
        R[3] -= R[4]*sig;
        R[5] -= 2*R[0]*sig;
        Used = true;
        continue;
      }
      if( cmp.lt(R[0]+R[1]+R[3]+R[4]+R[5],0) ||
          (cmp.eq(R[0]+R[1]+R[3]+R[4]+R[5],0) && cmp.gt(2*(R[0]+R[4])+R[5],0)) ) //8
      {
        R[2] += R[0]+R[1]+R[3]+R[4]+R[5];
        R[3] += 2*R[1] + R[5];
        R[4] += 2*R[0] + R[5];
        Used = true;
      }
    }
    sides[0] = sqrt(R[0]);
    sides[1] = sqrt(R[1]);
    sides[2] = sqrt(R[2]);
    angles[0] = acos(R[3]/(sides[1]*sides[2]*2))*180/M_PI;
    angles[1] = acos(R[4]/(sides[0]*sides[2]*2))*180/M_PI;
    angles[2] = acos(R[5]/(sides[0]*sides[1]*2))*180/M_PI;
  }
};

EndXlibNamespace()
#endif
