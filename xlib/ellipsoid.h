/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_ellipsoid_H
#define __olx_xl_ellipsoid_H
#include "xbase.h"
#include "ematrix.h"
#include "threex3.h"
#include "tptrlist.h"
#include "math/tensor.h"
#include "ecomplex.h"

BeginXlibNamespace()

/* Ellipsoid always must be in the cartesian frame */
struct GramCharlier4;
class TEllipsoid : public ACollectionItem {
  bool NPD;  // not positive defined
  evecd Quad, Esd;  // quadratic form of the elipsoid and esd
  mat3d Matrix;  // normalised eigen vectors
  vec3d Norms;  // lengths of the vectors
  /* do not change this value ! it is equal to the position in the AsymmUnit
  list
  */
  size_t Id;
  olx_object_ptr<GramCharlier4> anharmonic;
public:
  TEllipsoid();
  TEllipsoid(const TEllipsoid& e);
  template <class T> TEllipsoid(const T& Q) { Initialise(Q); }
  template <class T> TEllipsoid(const T& Q, const T& E) { Initialise(Q, E); }
  virtual ~TEllipsoid() {}
  void operator = (const TEllipsoid& E);
  /* multiplies the tensor by a matrix */
  void Mult(const mat3d& M);
  /* multiplies the tensor by a matrix and updates the Esds */
  void Mult(const mat3d& M, const ematd& J, const ematd& Jt);
  /* multiplies the tensor by a matrix and updates the Esds. The VcV matrix
  is expected to be as a linear map (11, 12, 13, 22, 23, 33)
  */
  void Mult(const mat3d& M, const ematd& VcV);
  // return true if the ellipsoid is not positively defined
  bool IsNPD() const { return NPD; }

  bool IsAnharmonic() const { return anharmonic.ok(); }

  const olx_object_ptr<GramCharlier4>& GetAnharmonicPart() const {
    return anharmonic;
  }

  olx_object_ptr<GramCharlier4>& GetAnharmonicPart() {
    return anharmonic;
  }

  void SetAnharmonicPart(GramCharlier4* anh) {
    anharmonic = anh;
  }

  template <class T> TEllipsoid& Initialise(const T& ShelxQ, const T& ShelxE) {
    Quad.Resize(6);
    Esd.Resize(6);
    for (size_t i = 0; i < 6; i++) {
      Quad[i] = ShelxQ[i];
      Esd[i] = ShelxE[i];
    }
    Initialise();
    return *this;
  }
  template <class T> TEllipsoid& Initialise(const T& ShelxQ) {
    Quad.Resize(6);
    Esd.Resize(6);
    for (size_t i = 0; i < 6; i++) {
      Quad[i] = ShelxQ[i];
      Esd[i] = 0;
    }
    Initialise();
    return *this;
  }
  // calculates eigen values and vectors; FQuad must be initialised
  void Initialise();

  template <class T> void GetShelxQuad(T& Q) const {
    for (size_t i = 0; i < 6; i++)
      Q[i] = Quad[i];
  }
  template <class T> void GetShelxQuad(T& Q, T& E) const {
    for (size_t i = 0; i < 6; i++) {
      Q[i] = Quad[i];
      E[i] = Esd[i];
    }
  }
  void SetEsd(size_t i, double v) { Esd[i] = v; }
  const double& GetEsd(size_t i) const { return Esd[i]; }
  void SetQuad(size_t i, double v) { Quad[i] = v; }
  const double& GetQuad(size_t i) const { return Quad[i]; }
  const vec3d& GetNorms() const { return Norms; }
  double CalcVolume() const {
    return 4 * Norms.Prod() * M_PI / 3;
  }
  double GetUeq() const { return (Quad[0] + Quad[1] + Quad[2]) / 3; }
  const evecd& GetQuad() const { return Quad; }
  const evecd& GetEsd() const { return Esd; }
  /* the matrix is the transposed eigenvalues of the quadratic form
  */
  const mat3d& GetMatrix() const { return Matrix; }
  mat3d ExpandQuad() const { return ExpandShelxQuad(Quad); }
  template <typename QT>
  static mat3d ExpandShelxQuad(const QT& Q) {
    return mat3d(Q[0], Q[5], Q[4], Q[1], Q[3], Q[2]);
  }
  /* returns 6x6 matrix suitable for the esd VcV matrix transformation like:
    new_VcV = J*olx_VcV*Jt assuming the VcV is constructed like:
    U11 U12 U13 U22 U23 U33
  */
  static ConstMatrix<double> GetTransformationJ(const mat3d& m);
  /* transforms a shelx Quad index (11, 22, 33, 23, 13, 12) to linear map:
  (11, 12, 13, 22, 23, 33)
  */
  static int shelx_to_linear(size_t i) {
    static int a[6] = { 0, 5, 4, 1, 3, 2 };
    return a[i];
  }
  /* transforms a linear map Quad index (11, 12, 13, 22, 23, 33) to shelx:
  (11, 22, 33, 23, 13, 12)
  */
  static int linear_to_shelx(size_t i) {
    static int a[6] = { 0, 3, 5, 4, 2, 1 };
    return a[i];
  }
  // resets the ellipsoid into a sphere of given radius
  void ToSpherical(double r);
  /* Calculates the scale to be applied to the ellipsoid in order v to lie on
  its surface
  */
  double CalcScale(const vec3d& v);
  DefPropP(size_t, Id);
};

struct GramCharlier4 {
  tensor::tensor_rank_3 C;
  tensor::tensor_rank_4 D;

  GramCharlier4() {}

  GramCharlier4(const GramCharlier4 &o)
    : C(o.C), D(o.D)
  {}

  compd calculate(const vec3i &h) const {
    const double pi_sq = M_PI * M_PI;
    double c = C.sum_up(h), d = D.sum_up(h);
    return compd(
      1 + d * pi_sq*pi_sq * 2 / 3,
      -c * pi_sq*M_PI * 4 / 3);
  }

};

  typedef TTypeList<TEllipsoid>  TEllpList;
  typedef TPtrList<TEllipsoid>  TEllpPList;
EndXlibNamespace()
#endif
