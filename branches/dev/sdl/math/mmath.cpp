#include "mmath.h"
#include "../threex3.h"
#include "composite.h"

void math::TestQR(OlxTests& t)  {
  t.description = __OlxSrcInfo;
  const mat3d m(12, -51, 4, 6, 167, -68, -4, 24, -41);
  mat3d qr(m), r, q;
  vec3d tau;
  QR<double>::Decompose(qr, tau);
  for( int i=0; i < 3; i++ )
    for( int j=i; j < 3; j++ )
      r(i,j) = qr(i,j);
  QR<double>::Unpack(qr, tau, 3, q);
  mat3d m1 = q*r;
  //alg::print0(m1, 3, 3);
  for( int i=0; i < 3; i++ )  {
    for( int j=0; j < 3; j++ )  {
      if( olx_abs(m(i,j)-m1(i,j)) > 1e-10 )
        throw TFunctionFailedException(__OlxSourceInfo, "M!=M");
    }
  }
}

void math::TestLU(OlxTests& t)  {
  t.description = __OlxSrcInfo;
  const mat3d m(
    12, -51, 4,
    6, 167, -68,
    -42, 24, -41);
  mat3d lu(m), l, u, p;
  TVector3<size_t> pivots;
  LU<double>::Decompose(lu, pivots);
  for( int i=0; i < 3; i++ )  {
    for( int j=i; j < 3; j++ )  {
      u(i,j) = lu(i,j);
      l(j,i) = (i==j ? 1 : lu(j,i));
    }
  }
  mat3d m1 = l*u;
  for( int i=0; i < 3; i++ )
    if( pivots[i] != i )
      m1.SwapRows(i, pivots[i]);
  //alg::print0(m1, 3, 3);
  for( int i=0; i < 3; i++ )  {
    for( int j=0; j < 3; j++ )  {
      if( olx_abs(m(i,j)-m1(i,j)) > 1e-10 )
        throw TFunctionFailedException(__OlxSourceInfo, "M!=M");
    }
  }
}

void math::TestInvert(OlxTests& t)  {
  t.description = __OlxSrcInfo;
  const mat3d m(
    12, -51, 4,
    6, 167, -68,
    -42, 24, -41);
  mat3d n(m), l, u, p;
  TVector3<size_t> pivots;
  LU<double>::Invert(n);
  mat3d m1 = m*n;
  for( int i=0; i < 3; i++ )  {
    for( int j=0; j < 3; j++ )  {
      if( olx_abs(m1(i,j) - (i==j ? 1 : 0)) > 1e-10 )
        throw TFunctionFailedException(__OlxSourceInfo, "M!=I");
    }
  }
  double arr[4][4] = {{14,2,3,4}, {12,5,-6,7}, {10,8,29,10}, {0.8, 11, -12, 13}};
  ematd om(4,4);
  om.Assign(arr, 4, 4);
  ematd im(om);
  LU<double>::Invert(im);
  om *= im;
  for( int i=0; i < 4; i++ )  {
    for( int j=0; j < 4; j++ )  {
      if( olx_abs(om(i,j) - (i==j ? 1 : 0)) > 1e-10 )
        throw TFunctionFailedException(__OlxSourceInfo, "M1!=I");
    }
  }
}

void math::TestSVD(OlxTests& t)  {
  t.description = __OlxSrcInfo;
  ematd m(5,5), u, vt, w(5,5);
  evecd d;
  m[0][0] = 1000;
  m[0][4] = 2000;
  m[1][2] = 3000;
  m[4][1] = 4000;
  const ematd om(m);
  SVD::Decompose(m, 2, 2, d, u, vt);
  for( size_t i=0; i < d.Count(); i++ )
    w[i][i] = d[i];
  ematd m1 = u*w*vt;
  for( int i=0; i < 5; i++ )
    for( int j=0; j < 5; j++ )
      if( olx_abs(om(i,j)-m1(i,j)) > 1e-10 )
        throw TFunctionFailedException(__OlxSourceInfo, "M!=UWVt");

  double arr[3][5] = {
    {2.0, 2.0, 1.6, 2.0, 1.2}, {2.5, 2.5,-0.4,-0.5,-0.3}, {2.5, 2.5, 2.8, 0.5,-2.9}};
    m.Assign(arr, 3, 5);
    SVD::Decompose(m, 2, 2, d, u, vt);
  for( size_t i=0; i < m.ColCount(); i++ )
    w[i][i] = d.Count() < i ? d[i] : 0;
  u.Resize(5,5);
  m1 = u*w*vt;
  math::alg::print0(m1, 5, 5);
  math::alg::print0(u, 5, 5);
  math::alg::print0(vt, 5, 5);
  math::alg::print0(d, d.Count());
}
