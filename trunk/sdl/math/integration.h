/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_math_int_H
#define __olx_math_int_H
#include "../emath.h"

BeginEsdlNamespace()

/* simple rectangular integration
*/
template <typename e_t>
double integrate_r(e_t f, double s, double e, double i = 1e-4) {
  double hs = i / 2, v = s + hs, r = 0;
  while (v < e) {
    r += f(v)*i;
    v += i;
  }
  return r;
}

/* simple trapezoid integration
*/
template <typename e_t>
double integrate_t(e_t f, double s, double e, double i = 1e-3) {
  double v = s, r = 0,
    f1 = f(s), hs = i / 2;
  while (v < e) {
    double f2 = f((v += i));
    r += (f1 + f2)*hs;
    f1 = f2;
  }
  return r;
}

/* trapezoid integration with cheking the final trapezoid lies in the given
interval [s,e]
*/
template <typename e_t>
double integrate_tx(e_t f, double s, double e, double i = 1e-3) {
  double v = s, r = 0,
    f1 = f(s), hs = i / 2;
  while (v < e) {
    if ((v += i) > e) {
      hs = (i - (e - v)) / 2;
      v = e;
    }
    double f2 = f(v);
    r += (f1 + f2)*hs;
    f1 = f2;
  }
  return r;
}

EndEsdlNamespace()
#endif
