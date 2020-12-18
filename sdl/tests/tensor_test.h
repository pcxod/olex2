/******************************************************************************
* Copyright (c) 2004-2020 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "threex3.h"
#include "math/tensor.h"
namespace test {
  using namespace tensor;

  vec3d mul(tensor::tensor_rank_3& t, const vec3d& v) {
    vec3d  r;
    for (size_t i = 0; i < 3; i++) {
      for (size_t j = 0; j < 3; j++) {
        for (size_t k = 0; k < 3; k++) {
          r[i] += v[j] * v[k] * t(i, j, k);
        }
      }
    }
    return r;
  }

  vec3d mul(tensor::tensor_rank_4& t, const vec3d& v) {
    vec3d  r;
    for (size_t i = 0; i < 3; i++) {
      for (size_t j = 0; j < 3; j++) {
        for (size_t k = 0; k < 3; k++) {
          for (size_t l = 0; l < 3; l++) {
            r[i] += v[j] * v[k] * v[l] * t(i, j, k, l);
          }
        }
      }
    }
    return r;
  }

  tensor_rank_3 mul(tensor::tensor_rank_3& t, const mat3d& x) {
    tensor::tensor_rank_3 r;
    double tmp[3][3][3];
    memset(&tmp[0][0][0], 0, sizeof(tmp));
    for (size_t i = 0; i < 3; i++) {
      for (size_t j = 0; j < 3; j++) {
        for (size_t k = 0; k < 3; k++) {
          for (size_t l = 0; l < 3; l++) {
            for (size_t m = 0; m < 3; m++) {
              for (size_t n = 0; n < 3; n++) {
                tmp[i][j][k] += t(l, m, n) * x[i][l] * x[j][m] * x[k][n];
              }
            }
          }
        }
      }
    }
    for (size_t i = 0; i < 3; i++) {
      for (size_t j = 0; j < 3; j++) {
        for (size_t k = 0; k < 3; k++) {
          r(i, j, k) = tmp[i][j][k];
        }
      }
    }
    return r;
  }

  tensor_rank_4 mul(tensor::tensor_rank_4& t, const mat3d& x) {
    tensor::tensor_rank_4 r;
    double tmp[3][3][3][3];
    memset(&tmp[0][0][0][0], 0, sizeof(tmp));
    for (size_t i = 0; i < 3; i++) {
      for (size_t j = 0; j < 3; j++) {
        for (size_t k = 0; k < 3; k++) {
          for (size_t l = 0; l < 3; l++) {
            for (size_t m = 0; m < 3; m++) {
              for (size_t n = 0; n < 3; n++) {
                for (size_t o = 0; o < 3; o++) {
                  for (size_t p = 0; p < 3; p++) {
                    tmp[i][j][k][l] += t(m, n, o, p) * x[i][m] * x[j][n] * x[k][o] * x[l][p];
                  }
                }
              }
            }
          }
        }
      }
    }
    for (size_t i = 0; i < 3; i++) {
      for (size_t j = 0; j < 3; j++) {
        for (size_t k = 0; k < 3; k++) {
          for (size_t l = 0; l < 3; l++) {
            r(i, j, k, l) = tmp[i][j][k][l];
          }
        }
      }
    }
    return r;
  }

  void TensorTransform(OlxTests& t_) {
    t_.description = __OlxSrcInfo;
    tensor::tensor_rank_3 t3;
    tensor::tensor_rank_4 t4;
    for (size_t i = 0; i < t4.data().Count(); i++) {
      double v = (double)rand() / (rand() + 1) * (rand() > 25 ? -1 : 1);
      if ( i < t3.data().Count()) {
        t3.data()[i] = v;
      }
      t4.data()[i] = v;
    }
  
    mat3d m;
    double angle = 31;
    olx_create_rotation_matrix(m, vec3d(1,2,3), cos(angle), sin(angle));
    tensor::tensor_rank_3 r3 = t3.transform(m);

    tensor::tensor_rank_3 tmp3 = mul(t3, m);
    for (size_t ii = 0; ii < tmp3.data().Count(); ii++) {
      double p = r3.data()[ii] / tmp3.data()[ii];
      if (olx_abs(p - 1) > 1e-8) {
        throw TFunctionFailedException(__OlxSourceInfo, "a!=a");
      }
    }

    tensor::tensor_rank_4 r4 = t4.transform(m);
    tensor::tensor_rank_4 tmp4 = mul(t4, m);
    for (size_t ii = 0; ii < tmp4.data().Count(); ii++) {
      double p = r4.data()[ii] / tmp4.data()[ii];
      if (olx_abs(p - 1) > 1e-8) {
        throw TFunctionFailedException(__OlxSourceInfo, "a!=a");
      }
    }

    vec3d testv(2, 29, 43);
    double d = t3.sum_up(testv) - testv.DotProd(mul(t3, testv));
    if (olx_abs(d) > 1e-8) {
      throw TFunctionFailedException(__OlxSourceInfo, "a!=a");
    }
    d = t4.sum_up(testv) - testv.DotProd(mul(t4, testv));
    if (olx_abs(d) > 1e-8) {
      throw TFunctionFailedException(__OlxSourceInfo, "a!=a");
    }

  }
};
