/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "math/spline.h"

namespace test {

void spline_test(OlxTests& t)  {
  using namespace math::spline;
  t.description = __OlxSrcInfo;
  evecd x(11), y(11);
  double (*f)(double) = &sin;
  for( size_t i=0; i <= 10; i++ )  {
    x(i) = M_PI*i/5;
    y(i) = f(x(i));
  }
  typedef double FT;
  Spline3<FT> akima(x,y), cubic(x,y), catmull_rom(x,y);
  Builder<FT>::akima(akima);
  Builder<FT>::cubic(cubic,
    boundary_type_periodic, boundary_condition_second_derivative,
    boundary_type_periodic, boundary_condition_second_derivative);
  Builder<FT>::catmull_rom(catmull_rom, boundary_type_parabolic, 0.5);
  olxdict<olxstr, AnAssociation3<Spline3<FT>*,int,double>, olxstrComparator<false> > splines;
  splines.Add("Akima", Association::Create(&akima, 0, 0.0));
  splines.Add("Cubic-periodic-2", Association::Create(&cubic, 0, 0.0));
  splines.Add("Catmull-Rom, t=0.5", Association::Create(&catmull_rom, 0, 0.1));
  for( size_t si = 0; si < splines.Count(); si++ )  {
    Spline3<FT>* sp = splines.GetValue(si).a;
    for( size_t i=1; i < 200; i++ )  {
      const double x = M_PI*i/50;
      if( !sp->periodic && x >= 2*M_PI )
        continue;
      splines.GetValue(si).b++;
      splines.GetValue(si).c += olx_sqr(sp->interpolate(x)-f(x));
    }
  }
  for( size_t si = 0; si < splines.Count(); si++ )  {
    const double r = sqrt(splines.GetValue(si).GetC()/splines.GetValue(si).GetB());
    if( r > 0.07 )
      throw TFunctionFailedException(__OlxSourceInfo,
        olxstr(splines.GetKey(si)) << ", too large deviation: " << olxstr::FormatFloat(3, r, true));
  }
}
//...........................................................................................
};  //namespace test
