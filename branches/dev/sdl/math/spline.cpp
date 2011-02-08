#include "spline.h"
#include "../bapp.h"

void math::spline::test(OlxTests& t)  {
  t.description = __OlxSrcInfo;
  evecd x(11), y(11);
  for( size_t i=0; i <= 10; i++ )  {
    x(i) = M_PI*i/5;
    y(i) = sin(x(i));
  }
  typedef double FT;
  Spline3<FT> s(x, y);
  spline::Builder<FT>::akima(s);
  for( size_t i=1; i < 100; i++ )  {
    const double x = M_PI*i/50;
    TBasicApp::NewLogEntry() << x << '\t' << s.interpolate(x);
  }
}
