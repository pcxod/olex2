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
  Matrix(e.Matrix),
  Norms(e.Norms),
  Id(e.Id)
{
  if (e.IsAnharmonic()) {
    SetAnharmonicPart(new GramCharlier(e.GetAnharmonicPart()));
  }
}
//..............................................................................
void TEllipsoid::Initialise() {
  mat3d M = ExpandQuad();
  mat3d::EigenValues(M, Matrix.I());
  NPD = ((M[0][0] <= 0) || (M[1][1] <= 0) || (M[2][2] <= 0));
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
    SetAnharmonicPart(new GramCharlier(E.GetAnharmonicPart()));
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
  N = Matr * N * Matr.GetT();
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
  if (NPD) {
    return;
  }
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
  for (size_t i=0; i < 6; i++) {
    from_sym3d m = dmat::M_x_OneSym_x_Mt(tm,
      to_sym3d::get_i(i), to_sym3d::get_j(i));
    for (size_t j = 0; j < 6; j++) {
      J(j, i) = m(j); // dUdm
    }
  }
  return J;
}
//..............................................................................
double TEllipsoid::CalcScale(const vec3d &v) {
  if (NPD) {
    return 1;
  }
  mat3d etm = mat3d::Scale(Matrix, Norms);
  vec3d nv = (v*etm.Inverse()).Normalise()*etm;
  return sqrt(v.QLength() / nv.QLength());
}
//..............................................................................
//..............................................................................
//..............................................................................
GramCharlier::GramCharlier(int order)
  : order(order)
{
  if (order < 3 || order > 4) {
    throw TInvalidArgumentException(__OlxSrcInfo, "order");
  }
}
//..............................................................................
compd GramCharlier::Calculate(const vec3i& h) const {
  const double pi_sq = M_PI * M_PI;
  double c = C.sum_up(h), d = D.sum_up(h);
  if (order == 3) {
    return compd(
      1,
      -c * pi_sq * M_PI * 4 / 3);
  }
  else if (order == 4) {
    return compd(
      1 + d * pi_sq * pi_sq * 2 / 3,
      -c * pi_sq * M_PI * 4 / 3);
  }
  return compd(1, 0);
}
//..............................................................................
void GramCharlier::FromStrings(const IStrList& str) {
  if (str.Count() >= 10) {
    order = 3;
    for (size_t i = 0; i < 10; i++) {
      C[i] = str[i].ToDouble();
    }
    if (str.Count() >= 25) {
      order = 4;
      for (size_t i = 0; i < 15; i++) {
        D[i] = str[i + 10].ToDouble();
      }
    }
  }
  else {
    throw TInvalidArgumentException(__OlxSrcInfo, "argument number");
  }
}
//..............................................................................
TDataItem& GramCharlier::ToDataItem(TDataItem& ai) const {
  olxstr_buf tmp;
  olxstr sep = ' ';
  if (order >= 3) {
    for (size_t ci = 0; ci < C.size(); ci++) {
      tmp << sep << olx_print("%.4le", C[ci]);
    }
    ai.AddField("Cijk", olxstr(tmp).SubStringFrom(1));
  }
  tmp.Clear();
  if (order >= 4) {
    for (size_t ci = 0; ci < D.size(); ci++) {
      tmp << sep << olx_print("%.4le", D[ci]);
    }
    ai.AddField("Dijkl", olxstr(tmp).SubStringFrom(1));
  }
  return ai;
}
//..............................................................................
olx_object_ptr<GramCharlier> GramCharlier::FromDataItem(const TDataItem& ai) {
  olx_object_ptr<GramCharlier> rv = new GramCharlier();
  GramCharlier& p = rv;
  TStrList c_toks(ai.FindField("Cijk"), ' ');
  TStrList d_toks(ai.FindField("Dijkl"), ' ');
  if (c_toks.Count() == 10) {
    for (size_t ci = 0; ci < 10; ci++) {
      p.C[ci] = c_toks[ci].ToDouble();
    }
    p.order = 3;
    if (d_toks.Count() == 15) {
      p.order = 4;
      for (size_t ci = 0; ci < 15; ci++) {
        p.D[ci] = d_toks[ci].ToDouble();
      }
    }
  }
  else {
    rv.reset();
  }
  return rv;
}
//..............................................................................
