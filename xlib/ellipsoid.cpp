/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "ellipsoid.h"
#include "math/dmatrix.h"

TEllipsoid::TEllipsoid() : Quad(6), Esd(6) {
  Matrix.I();
  NPD = false;
}
//..............................................................................
TEllipsoid::TEllipsoid(const TEllipsoid& e)
  : NPD(e.NPD),
  Quad(e.Quad),
  Esd(e.Esd),
  Norms(e.Norms),
  Id(e.Id)
{
  if (e.IsAnharmonic()) {
    SetAnharmonicPart(new GramCharlier4(e.GetAnharmonicPart()));
  }
}
//..............................................................................
void TEllipsoid::Initialise() {
  mat3d M = ExpandQuad();
  mat3d::EigenValues(M, Matrix.I());
  if ((M[0][0] <= 0) || (M[1][1] <= 0) || (M[2][2] <= 0)) {
    NPD = true;
  }
  Norms = vec3d(sqrt(olx_abs(M[0][0])),
    sqrt(olx_abs(M[1][1])),
    sqrt(olx_abs(M[2][2])));
}
//..............................................................................
void TEllipsoid::operator = (const TEllipsoid &E) {
  Matrix = E.GetMatrix();
  Norms = E.GetNorms();
  Quad = E.Quad;
  Esd = E.Esd;
  NPD = E.NPD;
  if (E.IsAnharmonic()) {
    SetAnharmonicPart(new GramCharlier4(E.GetAnharmonicPart()));
  }
}
//..............................................................................
/* to get the quadratic for Matrix*Matr(SX^2,SY^2,SZ^2)*MatrixT */
void TEllipsoid::Mult(const mat3d &Matr) {
  if (NPD) {
    return;
  }
  mat3d N = ExpandQuad();
  // do trasformation of the eigen vectors
  N = Matr * N * mat3d::Transpose(Matr);
  // store new quadratic form
  Quad[0] = N[0][0];  Quad[1] = N[1][1];  Quad[2] = N[2][2];
  Quad[3] = N[1][2];  Quad[4] = N[0][2];  Quad[5] = N[0][1];
  // get eigen values/vectors
  mat3d::EigenValues(N, Matrix.I());
  // assign new eigen values
  Norms = vec3d(sqrt(N[0][0]), sqrt(N[1][1]), sqrt(N[2][2]));
}
//..............................................................................
void TEllipsoid::Mult(const mat3d &Matr, const ematd &J, const ematd &Jt) {
  if (NPD) {
    return;
  }
  Mult(Matr);
  if (!Esd.IsNull()) {
    ematd em(6, 6);
    for (int i = 0; i < 6; i++) {
      em[i][i] = olx_sqr(Esd[shelx_to_linear(i)]);
    }
    em = J * em * Jt;
    for (int i = 0; i < 6; i++) {
      Esd[linear_to_shelx(i)] = sqrt(em[i][i]);
    }
  }
}
//..............................................................................
void TEllipsoid::Mult(const mat3d &Matr, const ematd &VcV) {
  if (NPD)  return;
  Mult(Matr);
  for (int i = 0; i < 6; i++) {
    Esd[linear_to_shelx(i)] = sqrt(VcV[i][i]);
  }
}
//..............................................................................
void TEllipsoid::ToSpherical(double r) {
  Quad[0] = Quad[1] = Quad[2] = r;
  Quad[3] = Quad[4] = Quad[5] = 0;
  Esd.Null();
  Matrix.I();
  Norms = vec3d(r);
  NPD = r <= 0;
}
//..............................................................................
ConstMatrix<double> TEllipsoid::GetTransformationJ(const mat3d &tm) {
  ematd J(6,6);
  using namespace math;
  typedef linear_from_sym<mat3d> from_sym3d;
  typedef linear_to_sym<mat3d> to_sym3d;
  for (int i=0; i < 6; i++) {
    from_sym3d m = dmat::M_x_OneSym_x_Mt(tm,
      to_sym3d::get_i(i), to_sym3d::get_j(i));
    for (int j = 0; j < 6; j++) {
      J(j, i) = m(j); // dUdm
    }
  }
  return J;
}
//..............................................................................
double TEllipsoid::CalcScale(const vec3d &v) {
  mat3d etm = mat3d::Scale(Matrix, Norms);
  vec3d nv = (v*etm.Inverse()).Normalise()*etm;
  return sqrt(v.QLength() / nv.QLength());
}
//..............................................................................
