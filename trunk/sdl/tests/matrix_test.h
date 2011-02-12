#include "math/mmath.h"
#include "threex3.h"
namespace test {

using namespace math;
void TestQR(OlxTests& t)  {
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
//...........................................................................................
void TestLU(OlxTests& t)  {
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
//...........................................................................................
void TestInvert(OlxTests& t)  {
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
//...........................................................................................
template <typename FT>
class SVDTestException : public TBasicException  {
public:
  TMatrix<FT> m, u, v;
  TVector<FT> w;
  SVDTestException(const olxstr& src, const olxstr& msg,
    const TMatrix<FT>& _m, const TMatrix<FT>& _u, const TMatrix<FT>& _v,
    const TVector<FT>& _w) :
    TBasicException(src, msg), m(_m), u(_u), v(_v), w(_w)  {}
    SVDTestException(const SVDTestException& src) : TBasicException(src),
      m(src.m), u(src.u), v(src.v), w(src.w)  {}
  virtual IEObject* Replicate() const {
    return new SVDTestException(*this);
  }
};
void TestSVD(OlxTests& t)  {
  t.description = __OlxSrcInfo;
  const size_t dim_n=13, dim_m=5;
  typedef double FT;
  FT arr[dim_n][dim_m] = {
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
  
  TMatrix<FT> m, s;
  SVD<FT> svd;
  try {
    const FT sig[2] = {1, -1};
    for( int rnd=0; rnd < 5; rnd++ )  {
      if( rnd == 1 )  {
        for( size_t i=0; i < dim_n; i++ )  {
          int val_ind = rand()%dim_m;
          for( size_t j=0; j < dim_m; j++ )  {
            arr[i][j] = (j==val_ind ? FT(rand())/(rand()+1) : FT(0));
            arr[i][j] *= sig[(rand()%2)];
          }
        }
      }
      else if( rnd == 2 )  {
        for( size_t i=0; i < dim_n; i++ )  {
          int val1_ind = rand()%dim_m;
          int val2_ind = rand()%dim_m;
          for( size_t j=0; j < dim_m; j++ )  {
            arr[i][j] = (j==val1_ind || j == val2_ind ? FT(rand())/(rand()+1) : FT(0));
            arr[i][j] *= sig[(rand()%2)];
          }
        }
      }
      else if( rnd == 3 )  {
        for( size_t i=0; i < dim_n; i++ )  {
          int val1_ind = rand()%dim_m;
          int val2_ind = rand()%dim_m;
          int val3_ind = rand()%dim_m;
          for( size_t j=0; j < dim_m; j++ )  {
            arr[i][j] = (j==val1_ind || j == val2_ind || j == val3_ind ?
              FT(rand())/(rand()+1) : FT(0));
            arr[i][j] *= sig[(rand()%2)];
          }
        }
      }
      else if( rnd == 4 )  {
        for( size_t i=0; i < dim_n; i++ )  {
          int val1_ind = rand()%dim_m;
          int val2_ind = rand()%dim_m;
          int val3_ind = rand()%dim_m;
          int val4_ind = rand()%dim_m;
          for( size_t j=0; j < dim_m; j++ )  {
            arr[i][j] = (j==val1_ind || j == val2_ind || j == val3_ind || j == val4_ind ?
              FT(rand())/(rand()+1) : FT(0));
            arr[i][j] *= sig[(rand()%2)];
          }
        }
      }
      for( int emem = 0; emem < 2; emem++ )  {
        for( size_t row_cnt=1; row_cnt < dim_n; row_cnt++ )  {
          m.Assign(arr, row_cnt, dim_m);
          const TMatrix<FT> om(m);
          svd.Decompose(m, 2, 2, emem!=0);
          s.Resize(row_cnt, dim_m).Null();
          for( size_t i=0; i < svd.w.Count(); i++ )
            s(i,i) = svd.w(i);
          TMatrix<FT> m1 = svd.u*s*svd.vt,
            up = svd.u*TMatrix<FT>(svd.u).Transpose(),
            vtp = svd.vt*TMatrix<FT>(svd.vt).Transpose();
          for( size_t i=0; i < vtp.RowCount(); i++ )  {
            for( size_t j=0; j < vtp.ColCount(); j++ )  {
              if( olx_abs(vtp(i,j)-(i==j ? 1 : 0)) > 1e-10 )
                throw SVDTestException<FT>(__OlxSourceInfo, "VtxVtt!=I", om, svd.u, svd.vt, svd.w);
            }
          }
          for( size_t i=0; i < up.RowCount(); i++ )  {
            for( size_t j=0; j < up.ColCount(); j++ )  {
              if( olx_abs(up(i,j)-(i==j ? 1 : 0)) > 1e-10 )
                throw SVDTestException<FT>(__OlxSourceInfo, "UxUt!=I", om, svd.u, svd.vt, svd.w);
            }
          }
          for( size_t i=0; i < m1.RowCount(); i++ )  {
            for( size_t j=0; j < m1.ColCount(); j++ )  {
              if( olx_abs(om(i,j)-m1(i,j)) > 1e-10 )
                throw SVDTestException<FT>(__OlxSourceInfo, "M!=UWVt", om, svd.u, svd.vt, svd.w);
            }
          }
        }
      }
    }
  }
  catch(SVDTestException<FT>& e)  {
    TMatrix<FT> s(e.m.RowCount(), e.m.ColCount());
    for( size_t i=0; i < e.w.Count(); i++ )
      s(i,i) = e.w(i);
    alg::print0_2(e.m, "Failed on:");
    alg::print0_2(e.u*s*e.v, "Res:");
    TVector<FT> q, p;
    alg::print0_2(e.u, "U:");
    alg::print0_2(e.v, "Vt:");
    alg::print0_1(e.w, "W:");
    ematd m(e.m);
    Bidiagonal<FT>::ToBidiagonal(m, q, p);
    alg::print0_2(m, "Input:");
    Bidiagonal<FT>::UnpackQ(m, q, e.u.ColCount(), e.u);
    Bidiagonal<FT>::UnpackPT(m, p, e.v.RowCount(), e.v);
    alg::print0_2(e.v, "Input Vt:");
    alg::print0_2(e.u, "Input u:");
    SVD<FT> svd;
    m = e.m;
    svd.Decompose(m);
    throw e;
  }
}
//...........................................................................................
};  //namespace test
