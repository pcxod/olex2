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
    vec3d X, Y, Z;
    int R[6];
    double Rd[6];
    double cosa = cos(angles[2]/180.0*M_PI);
    double sina = sin(angles[2]/180.0*M_PI);
    double TX = cos(angles[1]/180.0*M_PI);
    double TY = (cos(angles[0]*M_PI/180.0) - cosa*cos(angles[1]*M_PI/180.0))/sina;
    double TZ = sqrt(1.0 - TX*TX-TY*TY);

    switch(_latt)  {
    case 1:  // P
      X[0] = 1;  X[1] = 0;  X[2] = 0;
      Y[0] = 0;  Y[1] = 1;  Y[2] = 0;
      Z[0] = 0;  Z[1] = 0;  Z[2] = 1;
      break;
    case 2:  // I
      X[0] = 1;    X[1] = 0;    X[2] = 0;
      Y[0] = 0;    Y[1] = 1;    Y[2] = 0;
      Z[0] = 1./2;  Z[1] = 1./2;  Z[2] = 1./2;
      break;
    case 3:  // R
      X[0] = 2./3;  X[1] = 1./3;  X[2] = 1./3;
      Y[0] = -1./3;  Y[1] = 1./3;  Y[2] = 1./3;
      Z[0] = -1./3;  Z[1] = -2./3;  Z[2] = 1./3;
      break;
    case 4:  // F
      X[0] = 1./2;  X[1] = 0;    X[2] = 1./2;
      Y[0] = 1./2;  Y[1] = 1./2;  Y[2] = 0;
      Z[0] = 0;    Z[1] = 1./2;  Z[2] = 1./2;
      break;
    case 5:  // A
      X[0] = 1;    X[1] = 0;    X[2] = 0;
      Y[0] = 0;    Y[1] = 1;    Y[2] = 0;
      Z[0] = 0;    Z[1] = 1./2;  Z[2] = 1./2;
      break;
    case 6:  // B
      X[0] = 1./2;  X[1] = 0;    X[2] = 1./2;
      Y[0] = 0;    Y[1] = 1;    Y[2] = 0;
      Z[0] = 0;    Z[1] = 0;    Z[2] = 1;
      break;
    case 7:  // C
      X[0] = 1;    X[1] = 0;    X[2] = 0;
      Y[0] = 1./2;  Y[1] = 1./2;  Y[2] = 0;
      Z[0] = 0;    Z[1] = 0;    Z[2] = 1;
      break;
    default:
      throw TInvalidArgumentException(__OlxSourceInfo, "latt");
    }

    vec3d t = X*sides;
    X[0] = t[0] + t[1]*cosa+ t[2]*TX;
    X[1] = t[1]*sina + t[2]*TY;
    X[2] = t[2]*TZ;

    t = Y*sides;
    Y[0] = t[0] + t[1]*cosa + t[2]*TX;
    Y[1] = t[1]*sina + t[2]*TY;
    Y[2] = t[2]*TZ;

    t = Z*sides;
    Z[0] = t[0] + t[1]*cosa+ t[2]*TX;
    Z[1] = t[1]*sina + t[2]*TY;
    Z[2] = t[2]*TZ;

    sides = vec3d(X.Length(), Y.Length(), Z.Length());
    angles = vec3d(acos(Y.CAngle(Z))*180/M_PI,
      acos(X.CAngle(Z))*180/M_PI, acos(Y.CAngle(X))*180/M_PI);

    Rd[0] = olx_sqr(sides[0]);
    Rd[1] = olx_sqr(sides[1]);
    Rd[2] = olx_sqr(sides[2]);

    Rd[3] = 2*sides[1]*sides[2]*cos(angles[0]*M_PI/180);
    Rd[4] = 2*sides[0]*sides[2]*cos(angles[1]*M_PI/180);
    Rd[5] = 2*sides[0]*sides[1]*cos(angles[2]*M_PI/180);

    for( int i=0; i < 6; i++ )
      R[i] = olx_round(Rd[i]);

    bool Used = true;
    int Cycles = 0;
    while ( Used )  {
      Used = false;
      Cycles ++;
      if( Cycles > 100 )
        throw TFunctionFailedException(__OlxSourceInfo, "procedure did not converge");
      if( (R[0] > R[1]) || ((R[0] == R[1])&&(olx_abs(R[3])>olx_abs(R[4]))) )  { //1
        olx_swap(R[0], R[1]);
        olx_swap(R[3], R[4]);
        olx_swap(Rd[0], Rd[1]);
        olx_swap(Rd[3], Rd[4]);
        Used = true;
      }
      if( (R[1] > R[2]) || ((R[1] == R[2])&&(olx_abs(R[4])>olx_abs(R[5]))) )  { //2
        olx_swap(R[1], R[2]);
        olx_swap(R[4], R[5]);
        olx_swap(Rd[1], Rd[2]);
        olx_swap(Rd[4], Rd[5]);
        Used = true;
        continue;
      }
      if( R[3]*R[4]*R[5] > 0 )  { //3
        R[3] = olx_abs(R[3]);    R[4] = olx_abs(R[4]);    R[5] = olx_abs(R[5]);
        Rd[3] = olx_abs(Rd[3]);  Rd[4] = olx_abs(Rd[4]);  Rd[5] = olx_abs(Rd[5]);
      }
      else  {
        R[3] = -olx_abs(R[3]);    R[4] = -olx_abs(R[4]);    R[5] = -olx_abs(R[5]);
        Rd[3] = -olx_abs(Rd[3]);  Rd[4] = -olx_abs(Rd[4]);  Rd[5] = -olx_abs(Rd[5]);
      }
      if( (olx_abs(R[3]) > R[1]) || ((R[3]==R[1])&&(2*R[4]<R[5]) ) || //5
        ( (R[3]==-R[1])&&(R[5]<0)) )
      {
        R[2] = R[1]+R[2] - R[3]*olx_sign(R[3]);
        R[4] = R[4]-R[5]*olx_sign(R[3]);
        R[3] = R[3]-2*R[1]*olx_sign(R[3]);

        Rd[2] = Rd[1]+Rd[2] - Rd[3]*olx_sign(Rd[3]);
        Rd[4] = Rd[4]-Rd[5]*olx_sign(Rd[3]);
        Rd[3] = Rd[3]-2*Rd[1]*olx_sign(Rd[3]);
        Used = true;
        continue;
      }
      if( (olx_abs(R[4]) > R[0]) || ((R[4]==R[0])&&(2*R[3]<R[5]) ) ||  //6
        ( (R[4]==-R[0])&&(R[5]<0)) )
      {
        R[2] = R[0]+R[2] - R[4]*olx_sign(R[4]);
        R[3] = R[3]-R[5]*olx_sign(R[4]);
        R[4] = R[4]-2*R[0]*olx_sign(R[4]);

        Rd[2] = Rd[0]+Rd[2] - Rd[4]*olx_sign(Rd[4]);
        Rd[3] = Rd[3]-Rd[5]*olx_sign(Rd[4]);
        Rd[4] = Rd[4]-2*Rd[0]*olx_sign(Rd[4]);
        Used = true;
        continue;
      }
      if( (olx_abs(R[5]) > R[0]) || ((R[5] == R[0])&&(2*R[3]<R[4]) ) ||  //7
        ( (R[5]==-R[0])&&(R[4]<0)) )
      {
        R[1] = R[0]+R[1] - R[5]*olx_sign(R[5]);
        R[3] = R[3]-R[4]*olx_sign(R[5]);
        R[5] = R[5]-2*R[0]*olx_sign(R[5]);

        Rd[1] = Rd[0]+Rd[1] - Rd[5]*olx_sign(Rd[5]);
        Rd[3] = Rd[3]-Rd[4]*olx_sign(Rd[5]);
        Rd[5] = Rd[5]-2*Rd[0]*olx_sign(Rd[5]);
        Used = true;
        continue;
      }
      if( ((R[0]+R[1] + R[3]+R[4]+R[5])< 0 ) ||
          (((R[0]+R[1] + R[3]+R[4]+R[5])==0)&&(2*(R[0]+R[4])+R[5])>0) ) //8
      {
        R[2] = R[0]+R[1]+R[2]+R[3]+R[4]+R[5];
        R[3] = 2*R[1] + R[3]+R[5];
        R[4] = 2*R[0] + R[4]+R[5];

        Rd[2] = Rd[0]+Rd[1]+Rd[2]+Rd[3]+Rd[4]+Rd[5];
        Rd[3] = 2*Rd[1] + Rd[3]+Rd[5];
        Rd[4] = 2*Rd[0] + Rd[4]+Rd[5];
        Used = true;
      }
    }
    Rd[0] = sqrt(Rd[0]);
    Rd[1] = sqrt(Rd[1]);
    Rd[2] = sqrt(Rd[2]);
    sides[0] = Rd[0];
    sides[1] = Rd[1];
    sides[2] = Rd[2];
    angles[0] = acos(Rd[3]/(Rd[1]*Rd[2]*2))*180/M_PI;
    angles[1] = acos(Rd[4]/(Rd[0]*Rd[2]*2))*180/M_PI;
    angles[2] = acos(Rd[5]/(Rd[0]*Rd[1]*2))*180/M_PI;
  }
};

EndXlibNamespace()
#endif
