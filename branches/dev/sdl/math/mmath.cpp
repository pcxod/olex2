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
  const size_t dim_n=13, dim_m=5;
  double arr[dim_n][dim_m] = {
    {2.0, 2.0, 1.6, 2.0, 1.2},
    {2.5, 2.5,-0.4,-0.5,-0.3},
    {2.5, 2.5, 2.8, 0.5,-2.9},
    {-2.5, 2.5, 2.8, 0.5, 2.9},
    {-2.5, 2.5, 2.8, 0.5, 2.9},
    {-2.5, 2.5, 2.8, 0.5, 2.9},
    {-2.5, 2.5, 2.8, 0.5, 2.9},
    {-2.5, 2.5, 2.8, 0.5, 2.9},
    {-2.5, 0, 2.8, 0, 2.9},
    {2.5, 0, 2.8, 0, -2.9},
    {-2.5, 2.5, 2.8, 0.5, 2.9},
    {-1.5, 0.5, -2.8, 0.5, -2.9},
    {1.5, -0.5, -2.8, 10, -2.9},
  };
  
  ematd m, u, vt, w;
  for( int emem = 0; emem < 2; emem++ )  {
    for( size_t row_cnt=1; row_cnt < dim_n; row_cnt++ )  {
      m.Assign(arr, row_cnt, dim_m);
      const ematd om(m);
      evecd d;
      SVD::Decompose(m, 2, 2, d, u, vt, emem!=0);
      w.Resize(row_cnt, dim_m);
      for( size_t i=0; i < d.Count(); i++ )
        w[i][i] = d[i];
      ematd m1 = u*w*vt, up = u*ematd(u).Transpose();
      for( size_t i=0; i < m1.RowCount(); i++ )  {
        for( size_t j=0; j < m1.ColCount(); j++ )  {
          if( olx_abs(om(i,j)-m1(i,j)) > 1e-10 )
            throw TFunctionFailedException(__OlxSourceInfo, "M!=UWVt");
        }
      }
      for( size_t i=0; i < up.RowCount(); i++ )  {
        for( size_t j=0; j < up.ColCount(); j++ )  {
          if( olx_abs(up(i,j)-(i==j ? 1 : 0)) > 1e-10 )
            throw TFunctionFailedException(__OlxSourceInfo, "UxUt!=I");
        }
      }
    }
  }
}
