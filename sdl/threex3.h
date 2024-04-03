/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_3x3
#define __olx_sdl_3x3
#include "emath.h"
#include "typelist.h"
#include "talist.h"
#undef QLength

BeginEsdlNamespace()

template <typename> class TVector33;
template <typename> class TMatrix33;

template <class FT>  class TVector3 {
  FT data[3];
public:
  TVector3() { data[0] = data[1] = data[2] = 0; }
  TVector3(FT v) { data[0] = v;  data[1] = v;  data[2] = v; }
  TVector3(FT x, FT y, FT z) { data[0] = x;  data[1] = y;  data[2] = z; }

  TVector3(const TVector3<FT>& v) {
    data[0] = v[0];  data[1] = v[1];  data[2] = v[2];
  }
  template <class AT> TVector3(const TVector3<AT>& v) {
    data[0] = (FT)v[0];  data[1] = (FT)v[1];  data[2] = (FT)v[2];
  }
  // any vector
  template <class AV> static TVector3 FromAny(const AV& v) {
#ifdef _DEBUG
    if (v.Count() != 3) {
      throw TInvalidArgumentException(__OlxSourceInfo, "vector");
    }
#endif
    return TVector3(v(0), v(1), v(2));
  }

  FT& operator [] (size_t i) { return data[i]; }
  FT const& operator [] (size_t i) const { return data[i]; }
  FT& operator () (size_t i) { return data[i]; }
  FT const& operator () (size_t i) const { return data[i]; }
  const FT* GetData() const { return &data[0]; }
  FT QLength() const {
    return (data[0] * data[0] + data[1] * data[1] + data[2] * data[2]);
  }
  FT Length() const { return (FT)sqrt(QLength()); }
  static size_t Count() { return 3; }
  static bool IsEmpty() { return false; }
  static void Resize(size_t s) {
    if (s != 3) {
      throw TInvalidArgumentException(__OlxSourceInfo, "size");
    }
  }

  template <class AT> FT QDistanceTo(const TVector3<AT>& v) const {
    return static_cast<FT>(olx_sqr(data[0] - v[0]) +
      olx_sqr(data[1] - v[1]) + olx_sqr(data[2] - v[2]));
  }
  template <class AT> FT DistanceTo(const TVector3<AT>& v) const {
    return static_cast<FT>(sqrt(QDistanceTo(v)));
  }

  /* distance between this point and a line segment. Returns -1 within is true and
  the point is outside of the given segment
  http://paulbourke.net/geometry/pointlineplane/
  */
  template <class AT>
  FT DistanceTo(const TVector3<AT>& a, const TVector3<AT>& b, bool within=false) const
  {
    TVector3 v = b - a;
    FT u = (*this - a).DotProd(v)/v.QLength();
    if (within && (u < 0 || u > 1)) {
      return -1;
    }
    TVector3 intersection = a + v * u;
    return this->DistanceTo(intersection);
  }

  template <class AT> FT CAngle(const TVector3<AT>& v) const {
    FT l = QLength()*v.QLength();
    if (l == 0) {
      throw TDivException(__OlxSourceInfo);
    }
    l = DotProd(v) / sqrt(l);
    // treat possible rounding errors
    if (l > 1) {
      return 1;
    }
    if (l < -1) {
      return -1;
    }
    return l;
  }
  template <class AT> bool IsParallel(const TVector3<AT>& v,
    FT eps = FT(1e-8)) const
  {
    FT l = QLength()*v.QLength();
    if (l == 0) {
      throw TDivException(__OlxSourceInfo);
    }
    l = olx_abs(DotProd(v) / sqrt(l));
    return olx_abs(l - 1) < eps;
  }
  template <class AT> bool IsOrthogonal(const TVector3<AT>& v,
    FT eps = FT(1e-8)) const
  {
    return olx_abs(DotProd(v)) < eps;
  }
  // multiplies each element by itself
  TVector3<FT>& Qrt() {
    data[0] *= data[0];
    data[1] *= data[1];
    data[2] *= data[2];
    return *this;
  }
  // takes square root of each element (must be >= 0!)
  TVector3<FT>& Sqrt() {
    data[0] = sqrt(data[0]);
    data[1] = sqrt(data[1]);
    data[2] = sqrt(data[2]);
    return *this;
  }
  // takes square root of each element (must be >= 0!)
  static TVector3<FT> Sqrt(const TVector3<FT>& v) {
    return TVector3<FT>(v).Sqrt();
  }
  // takes absolute value of the vector elements
  TVector3<FT>& Abs() {
    data[0] = olx_abs(data[0]);
    data[1] = olx_abs(data[1]);
    data[2] = olx_abs(data[2]);
    return *this;
  }
  // returns a vector with absolute values of provided one
  static TVector3<FT> Abs(const TVector3<FT>& v) {
    return TVector3<FT>(v).Abs();
  }
  // returns vector with 1/ values
  TVector3<FT>& Inverse() {
    data[0] = FT(1) / data[0];
    data[1] = FT(1) / data[1];
    data[2] = FT(1) / data[2];
    return *this;
  }
  // returns vector with 1/ values
  static TVector3<FT> Inverse(const TVector3<FT>& v) {
    return TVector3<FT>(v).Inverse();
  }
  // returns sum of vector elements
  FT Sum() const { return data[0] + data[1] + data[2]; }
  // returns product of vector elements
  FT Prod() const { return data[0] * data[1] * data[2]; }
  // returns sum of absolute values of vector elements
  FT AbsSum() const {
    return olx_abs(data[0]) + olx_abs(data[1]) + olx_abs(data[2]);
  }
  // rounds the vector elements
  template <class AT> TVector3<AT> Round() const {
    return TVector3<AT>(
      olx_round_t<AT, FT>(data[0]),
      olx_round_t<AT, FT>(data[1]),
      olx_round_t<AT, FT>(data[2]));
  }
  // floors the vector elements
  template <class AT> TVector3<AT> Floor() const {
    return TVector3<AT>(
      olx_floor_t<AT, FT>(data[0]),
      olx_floor_t<AT, FT>(data[1]),
      olx_floor_t<AT, FT>(data[2]));
  }
  template <class AT> FT DotProd(const TVector3<AT>& v) const {
    return (FT)(data[0] * v[0] + data[1] * v[1] + data[2] * v[2]);
  }
  // ax(bxc) = b(a.c) - c(a.b)
  static TVector3<FT> TripleProd(const TVector3<FT>& a,
    const TVector3<FT>& b, const TVector3<FT>& c)
  {
    const double p1 = a.DotProd(c), p2 = a.DotProd(b);
    return TVector3<FT>(
      b[0] * p1 - c[0] * p2,
      b[1] * p1 - c[1] * p2,
      b[2] * p1 - c[2] * p2);
  }
  // a*b*sin = a*b*(1-cos^2) = a*b - DotProd^2
  template <class AT> FT XProdVal(const TVector3<AT>& v) const {
    const FT dp = DotProd(v);
    return (FT)sqrt(QLength()*v.QLength() - dp * dp);
  }
  template <class AT> TVector3<FT> XProdVec(const TVector3<AT>& v) const
  {
    return TVector3<FT>(
      data[1] * v[2] - data[2] * v[1],
      data[2] * v[0] - data[0] * v[2],
      data[0] * v[1] - data[1] * v[0]);
  }
  /* returns a normal to vector through a point (vector with same origin as
  this vector) N = this*cos(this,point)*point.length/this.length =
  this*DotProd(this, point)/this.QLength
  */
  template <class AT> TVector3<FT> NormalThrough(
    const TVector3<AT>& point) const
  {
    FT m = QLength();
    if (m == 0) {
      throw TDivException(__OlxSourceInfo);
    }
    m = DotProd(point) / m;
    return TVector3<FT>(
      point[0] - data[0] * m,
      point[1] - data[1] * m,
      point[2] - data[2] * m);
  }
  /* intesection point of a normal through this point to a vector defined
  by a and b
  */
  template <class AT>
  TVector3 NormalIntersection(const TVector3<AT>& a, const TVector3<AT>& b) const
  {
    TVector3 v = b - a;
    FT m = v.QLength();
    if (m == 0) {
      throw TDivException(__OlxSourceInfo);
    }
    FT u = (*this - a).DotProd(v) / m;
    return a + v * u;
  }

  //https://gamedev.stackexchange.com/questions/60630/how-do-i-find-the-circumcenter-of-a-triangle-in-3d
  static TVector3 Circumcenter(const TVector3& a, const TVector3& b,
    const TVector3& c)
  {
    vec3d ab = a - b, cb = c - b;
    vec3d normal = ab.XProdVec(cb);
    vec3d cc = (normal.XProdVec(ab) * cb.QLength() +
      cb.XProdVec(normal) * ab.QLength()) / (2 * normal.QLength());
    return cc + b;
  }

  // convenience method
  static TVector3 Normal(const TVector3 &a, const TVector3 &b,
    const TVector3 &c)
  {
    return (a - b).XProdVec(c - b).Normalise();
  }
  // convenience method
  template <typename arr_t>
  static TVector3 Normal(const arr_t &ar, size_t i, size_t j, size_t k) {
    return (ar[i] - ar[j]).XProdVec(ar[k] - ar[j]).Normalise();
  }
  /* projects this vector onto a plane given by the normal with center at
  the origin
  */
  TVector3 &Project(const TVector3 &normal) {
    return (this->operator -= (normal*DotProd(normal)));
  }
  /* projects this vector onto a plane given by the normal and center */
  TVector3 &Project(const TVector3 &center, const TVector3 &normal) {
    this->operator -= (center);
    return (this->operator -= (normal*DotProd(normal)));
  }
  /* returns a projection of a point to a plane given by normal with center
  at origin
  */
  TVector3 Projection(const TVector3 &normal) const {
    return TVector3(*this).Project(normal);
  }
  /* returns a projection of a point to a plane given by normal and center */
  TVector3 Projection(const TVector3 &center, const TVector3 &normal) const {
    return TVector3(*this - center).Project(normal);
  }
  TVector3<FT> operator -() const {
    return TVector3<FT>(-data[0], -data[1], -data[2]);
  }
  // returns a reflection of this vector from a plane represented by normal
  template <class AT> TVector3<FT> Reflect(
    const TVector3<AT>& normal) const
  {
    const FT m = DotProd(normal) * 2;
    return TVector3<FT>(
      data[0] - normal[0] * m,
      data[1] - normal[1] * m,
      data[2] - normal[2] * m);
  }
  TVector3<FT>& Normalise() {
    const FT l = Length();
    if (l == 0) {
      throw TDivException(__OlxSourceInfo);
    }
    return (*this /= l);
  }
  // returns previous length
  double NormaliseEx() {
    FT l = Length();
    if (l == 0) {
      throw TDivException(__OlxSourceInfo);
    }
    *this /= l;
    return l;
  }
  TVector3<FT>& NormaliseTo(FT val) {
    const FT l = Length();
    if (l == 0) {
      throw TDivException(__OlxSourceInfo);
    }
    return (*this *= (val / l));
  }
  TVector3<FT>& Null() {
    data[0] = data[1] = data[2] = 0;
    return *this;
  }
  bool IsNull() const {
    return (data[0] == 0 && data[1] == 0 && data[2] == 0);
  }
  bool IsNull(FT eps) const {
    return (olx_abs(data[0]) < eps &&
      olx_abs(data[1]) < eps &&
      olx_abs(data[2]) < eps);
  }

  bool Equals(const TVector3<FT>& v) const {
    return (data[0] == v[0] && data[1] == v[1] && data[2] == v[2]);
  }
  bool operator == (const TVector3<FT>& v) const { return Equals(v); }
  bool operator != (const TVector3<FT>& v) const { return !Equals(v); }
  // approximately equals
  bool Equals(const TVector3<FT>& v, FT eps) const {
    return (
      olx_abs(data[0] - v[0]) < eps &&
      olx_abs(data[1] - v[1]) < eps &&
      olx_abs(data[2] - v[2]) < eps);
  }
  TVector3<FT>& operator = (const TVector3<FT>& v) {
    data[0] = v[0];  data[1] = v[1];  data[2] = v[2];
    return *this;
  }
  // any vector
  template <class AT> TVector3<FT>& operator = (const AT& v) {
    data[0] = (FT)v[0];  data[1] = (FT)v[1];  data[2] = (FT)v[2];
    return *this;
  }
  template <class AT>
  TVector3<FT>& operator += (const TVector3<AT>& v) {
    data[0] += (FT)v[0];  data[1] += (FT)v[1];  data[2] += (FT)v[2];
    return *this;
  }
  template <class AT>
  TVector3<FT>& operator -= (const TVector3<AT>& v) {
    data[0] -= (FT)v[0];  data[1] -= (FT)v[1];  data[2] -= (FT)v[2];
    return *this;
  }
  template <class AT>
  TVector3<FT>& operator *= (const TVector3<AT>& v) {
    data[0] *= (FT)v[0];  data[1] *= (FT)v[1];  data[2] *= (FT)v[2];
    return *this;
  }
  template <class AT>
  TVector3<FT>& operator /= (const TVector3<AT>& v) {
    data[0] /= (FT)v[0];  data[1] /= (FT)v[1];  data[2] /= (FT)v[2];
    return *this;
  }
  template <class AT> TVector3<FT>& operator += (AT v) {
    data[0] += (FT)v;  data[1] += (FT)v;  data[2] += (FT)v;
    return *this;
  }
  template <class AT> TVector3<FT>& operator -= (AT v) {
    data[0] -= (FT)v;  data[1] -= (FT)v;  data[2] -= (FT)v;
    return *this;
  }
  template <class AT> TVector3<FT>& operator *= (AT v) {
    data[0] *= (FT)v;  data[1] *= (FT)v;  data[2] *= (FT)v;
    return *this;
  }
  template <class AT> TVector3<FT>& operator /= (AT v) {
    data[0] /= v;  data[1] /= v;  data[2] /= v;
    return *this;
  }
  template <class AT>
  TVector3<FT> operator + (const TVector3<AT>& v) const {
    return TVector3<FT>(
      static_cast<FT>(data[0] + v[0]),
      static_cast<FT>(data[1] + v[1]),
      static_cast<FT>(data[2] + v[2]));
  }
  template <class AT>
  TVector3<FT> operator - (const TVector3<AT>& v) const {
    return TVector3<FT>(
      static_cast<FT>(data[0] - v[0]),
      static_cast<FT>(data[1] - v[1]),
      static_cast<FT>(data[2] - v[2]));
  }
  template <class AT>
  TVector3<FT> operator * (const TVector3<AT>& v) const {
    return TVector3<FT>(
      static_cast<FT>(data[0] * v[0]),
      static_cast<FT>(data[1] * v[1]),
      static_cast<FT>(data[2] * v[2]));
  }
  template <class AT>
  TVector3<FT> operator / (const TVector3<AT>& v) const {
    return TVector3<FT>(
      static_cast<FT>(data[0] / v[0]),
      static_cast<FT>(data[1] / v[1]),
      static_cast<FT>(data[2] / v[2]));
  }

  template <class AT> TVector3<FT> operator + (AT v) const {
    return TVector3<FT>(
      static_cast<FT>(data[0] + v),
      static_cast<FT>(data[1] + v),
      static_cast<FT>(data[2] + v));
  }
  template <class AT> TVector3<FT> operator - (AT v) const {
    return TVector3<FT>(
      static_cast<FT>(data[0] - v),
      static_cast<FT>(data[1] - v),
      static_cast<FT>(data[2] - v));
  }
  template <class AT> TVector3<FT> operator * (AT v) const {
    return TVector3<FT>(
      static_cast<FT>(data[0] * v),
      static_cast<FT>(data[1] * v),
      static_cast<FT>(data[2] * v));
  }

  template <class AT> TVector3<FT> operator / (AT v) const {
    return TVector3<FT>(
      static_cast<FT>(data[0] / v),
      static_cast<FT>(data[1] / v),
      static_cast<FT>(data[2] / v));
  }

  template <class AT> TVector3<FT> ColMul(const TMatrix33<AT>& a) const {
    return TVector3<FT>(
      static_cast<FT>(data[0] * (a[0][0] + a[0][1] + a[0][2])),
      static_cast<FT>(data[1] * (a[1][0] + a[1][1] + a[1][2])),
      static_cast<FT>(data[2] * (a[2][0] + a[2][1] + a[2][2])));
  }

  /* beware - row vector (or Mt is used), use M.v for column vector
  multiplication
  */
  template <class AT> TVector3<FT>  operator * (const TMatrix33<AT>& a) const {
    return TVector3<FT>(
      static_cast<FT>(data[0] * a[0][0] + data[1] * a[1][0] + data[2] * a[2][0]),
      static_cast<FT>(data[0] * a[0][1] + data[1] * a[1][1] + data[2] * a[2][1]),
      static_cast<FT>(data[0] * a[0][2] + data[1] * a[1][2] + data[2] * a[2][2]));
  }
  /* beware - row vector (or Mt is used), use M.v for column vector
  multiplication
  */
  template <class AT> TVector3<FT>& operator *= (const TMatrix33<AT>& a) {
    return (*this = (*this)*a);
  }
  // updates Min/Max in a vector/array
  template <typename Avec, typename Bvec>
  static void UpdateMinMax(const TVector3<FT>& src, Avec& min, Bvec& max) {
    olx_update_min_max(src[0], min[0], max[0]);
    olx_update_min_max(src[1], min[1], max[1]);
    olx_update_min_max(src[2], min[2], max[2]);
  }
  // returns true if the value in in the range inclusive the boundaries
  template <typename Avec, typename Bvec>
  static bool IsInRangeExc(const TVector3<FT>& src, const Avec& min,
    const Bvec& max)
  {
    if (src[0] >= max[0] || src[0] <= min[0] ||
      src[1] >= max[1] || src[1] <= min[1] ||
      src[2] >= max[2] || src[2] <= min[2])
    {
      return false;
    }
    return true;
  }
  // returns true if the value in in the range exclusive the boundaries
  template <typename Avec, typename Bvec>
  static bool IsInRangeInc(const TVector3<FT>& src, const Avec& min,
    const Bvec& max)
  {
    if (src[0] > max[0] || src[0] < min[0] ||
      src[1] > max[1] || src[1] < min[1] ||
      src[2] > max[2] || src[2] < min[2])
    {
      return false;
    }
    return true;
  }
  template <class SC> SC StrRepr() const {
    SC rv(data[0], 64);
    return rv << ", " << data[1] << ", " << data[2];
  }
  TIString ToString() const { return StrRepr<olxstr>(); }
  olxcstr ToCStr() const { return StrRepr<olxcstr>(); }
  olxwstr ToWStr() const { return StrRepr<olxwstr>(); }

  int Compare(const TVector3 &v) const {
    int d = olx_cmp(data[0], v[0]);
    if (d != 0) {
      return d;
    }
    d = olx_cmp(data[1], v[1]);
    if (d != 0) {
      return d;
    }
    return olx_cmp(data[2], v[2]);
  }
public:
  typedef FT list_item_type;
  typedef FT number_type;
};

template <class FT> class TMatrix33 {
protected:
  TVector3<FT> data[3];
public:
  TMatrix33() {}
  TMatrix33(FT xx, FT xy, FT xz, FT yx, FT yy, FT yz, FT zx, FT zy, FT zz) {
    data[0][0] = xx;  data[0][1] = xy;  data[0][2] = xz;
    data[1][0] = yx;  data[1][1] = yy;  data[1][2] = yz;
    data[2][0] = zx;  data[2][1] = zy;  data[2][2] = zz;
  }
  TMatrix33(FT xx, FT xy, FT xz, FT yy, FT yz, FT zz) {
    data[0][0] = xx;  data[0][1] = xy;  data[0][2] = xz;
    data[1][0] = xy;  data[1][1] = yy;  data[1][2] = yz;
    data[2][0] = xz;  data[2][1] = yz;  data[2][2] = zz;
  }
  TMatrix33(FT xx, FT yy, FT zz) {
    data[0][0] = xx;  data[1][1] = yy;  data[2][2] = zz;
  }
  template <class vt> TMatrix33(const TVector3<vt>& x, const TVector3<vt>& y,
    const TVector3<vt>& z)
  {
    data[0] = x;  data[1] = y;  data[2] = z;
  }
  TMatrix33(const TMatrix33<FT>& v) {
    data[0][0] = v[0][0]; data[0][1] = v[0][1]; data[0][2] = v[0][2];
    data[1][0] = v[1][0]; data[1][1] = v[1][1]; data[1][2] = v[1][2];
    data[2][0] = v[2][0]; data[2][1] = v[2][1]; data[2][2] = v[2][2];
  }
  template <class AT> TMatrix33(const TMatrix33<AT>& v) {
    data[0][0] = (FT)v[0][0]; data[0][1] = (FT)v[0][1]; data[0][2] = (FT)v[0][2];
    data[1][0] = (FT)v[1][0]; data[1][1] = (FT)v[1][1]; data[1][2] = (FT)v[1][2];
    data[2][0] = (FT)v[2][0]; data[2][1] = (FT)v[2][1]; data[2][2] = (FT)v[2][2];
  }
  // any matrix
  template <class AM> static TMatrix33 FromAny(const AM& v) {
#ifdef _DEBUG
    if (v.ColCount() != 3 || v.RowCount() != 3) {
      throw TInvalidArgumentException(__OlxSourceInfo, "matrix");
    }
#endif
    TMatrix33 r;
    for (int i = 0; i < 3; i++) {
      r[i][0] = v(i, 0);
      r[i][1] = v(i, 1);
      r[i][2] = v(i, 2);
    }
    return r;
  }

  const TVector3<FT>& operator [] (size_t i) const { return data[i]; }
  TVector3<FT>& operator [] (size_t i) { return data[i]; }
  const TVector3<FT>& Get(size_t i) const { return data[i]; }
  const FT& Get(size_t i, size_t j) const { return data[i][j]; }
  FT& Set(size_t i, size_t j, const FT& v) {
    return (data[i][j] = v);
  }
  FT& operator () (size_t i, size_t j) { return data[i][j]; }
  const FT& operator () (size_t i, size_t j) const {
    return data[i][j];
  }
  static size_t ColCount() { return 3; }
  static size_t RowCount() { return 3; }
  static bool IsEmpty() { return false; }
  static void Resize(size_t w, size_t h) {
#ifdef _DEBUG
    if (w != 3 || h != 3) {
      throw TInvalidArgumentException(__OlxSourceInfo, "size");
    }
#endif
  }
  void SwapRows(size_t i, size_t j) {
    for (int _i = 0; _i < 3; _i++) {
      olx_swap(data[i][_i], data[j][_i]);
    }
  }
  void SwapCols(size_t i, size_t j) {
    for (int _i = 0; _i < 3; _i++) {
      olx_swap(data[_i][i], data[_i][j]);
    }
  }

  template <class AT> TMatrix33<FT> operator * (const TMatrix33<AT>& v) const {
    return TMatrix33<FT>(
      static_cast<FT>(data[0][0] * v[0][0] + data[0][1] * v[1][0] + data[0][2] * v[2][0]),
      static_cast<FT>(data[0][0] * v[0][1] + data[0][1] * v[1][1] + data[0][2] * v[2][1]),
      static_cast<FT>(data[0][0] * v[0][2] + data[0][1] * v[1][2] + data[0][2] * v[2][2]),
      static_cast<FT>(data[1][0] * v[0][0] + data[1][1] * v[1][0] + data[1][2] * v[2][0]),
      static_cast<FT>(data[1][0] * v[0][1] + data[1][1] * v[1][1] + data[1][2] * v[2][1]),
      static_cast<FT>(data[1][0] * v[0][2] + data[1][1] * v[1][2] + data[1][2] * v[2][2]),
      static_cast<FT>(data[2][0] * v[0][0] + data[2][1] * v[1][0] + data[2][2] * v[2][0]),
      static_cast<FT>(data[2][0] * v[0][1] + data[2][1] * v[1][1] + data[2][2] * v[2][1]),
      static_cast<FT>(data[2][0] * v[0][2] + data[2][1] * v[1][2] + data[2][2] * v[2][2]));
  }
  template <class AT> TMatrix33<FT>& operator *= (const TMatrix33<AT>& v) {
    return (*this = (*this) * v);
  }

  TMatrix33 operator -() const {
    return TMatrix33<FT>(-data[0], -data[1], -data[2]);
  }

  TMatrix33 GetTranspose() const {
    return TMatrix33<FT>(
      data[0][0], data[1][0], data[2][0],
      data[0][1], data[1][1], data[2][1],
      data[0][2], data[1][2], data[2][2]);
  }
  TMatrix33 GetT() const {
    return GetTranspose();
  }

  TMatrix33& Transpose() {
    olx_swap(data[0][1], data[1][0]);
    olx_swap(data[0][2], data[2][0]);
    olx_swap(data[1][2], data[2][1]);
    return *this;
  }

  TMatrix33& T() {
    return Transpose();
  }

  TMatrix33& I() {
    data[0][0] = 1;  data[0][1] = 0;  data[0][2] = 0;
    data[1][0] = 0;  data[1][1] = 1;  data[1][2] = 0;
    data[2][0] = 0;  data[2][1] = 0;  data[2][2] = 1;
    return *this;
  }

  static TMatrix33 Idenity() { return TMatrix33(1, 1, 1); }
  // normalises each vector
  TMatrix33<FT>& Normalise() {
    data[0].Normalise();
    data[1].Normalise();
    data[2].Normalise();
    return *this;
  }
  static TMatrix33<FT> Normalise(const TMatrix33<FT>& m) {
    return TMatrix33<FT>(m).Normalise();
  }
  // normalises each vector and returns the vector of lengths
  TVector3<FT> NormaliseEx() {
    return TVector3<FT>(data[0].NormaliseEx(),
      data[1].NormaliseEx(),
      data[2].NormaliseEx());
  }
  // normalises vectors to the given values
  TMatrix33<FT>& NormaliseTo(const TVector3<FT> &s) {
    data[0].NormaliseTo(s[0]);
    data[1].NormaliseTo(s[1]);
    data[2].NormaliseTo(s[2]);
    return *this;
  }
  // multiplies vectors by the given values
  TMatrix33<FT>& Scale(const TVector3<FT> &s) {
    data[0] *= s[0];
    data[1] *= s[1];
    data[2] *= s[2];
    return *this;
  }
  // multiplies vectors by the given values
  static TMatrix33<FT> Scale(const TMatrix33<FT>& m, const TVector3<FT> &s) {
    return TMatrix33<FT>(m).Scale(s);
  }
  bool IsI() const {
    return (data[0][0] == 1 && data[1][1] == 1 && data[2][2] == 1 &&
      data[0][1] == 0 && data[0][2] == 0 && data[1][0] == 0 &&
      data[1][2] == 0 && data[2][0] == 0 && data[2][1] == 0);
  }
  bool IsI(FT e) const {
    return (olx_abs(data[0][0] - 1) < e &&
      olx_abs(data[1][1] - 1) < e &&
      olx_abs(data[2][2] - 1) < e &&
      olx_abs(data[0][1]) < e && olx_abs(data[0][2]) < e &&
      olx_abs(data[1][0]) < e && olx_abs(data[1][2]) < e &&
      olx_abs(data[2][0]) < e && olx_abs(data[2][1]) < e);
  }
  TMatrix33<FT>& Null() {
    data[0].Null();  data[1].Null();  data[2].Null();
    return *this;
  }

  bool Equals(const TMatrix33& v) const {
    return (data[0].Equals(v[0]) && data[1].Equals(v[1]) && data[2].Equals(v[2]));
  }
  bool Equals(const TMatrix33& v, FT eps) const {
    return (data[0].Equals(v[0], eps) &&
      data[1].Equals(v[1], eps) &&
      data[2].Equals(v[2], eps));
  }
  bool operator == (const TMatrix33& v) const {
    return Equals(v);
  }
  bool operator != (const TMatrix33& v) const {
    return !(operator == (v));
  }

  TMatrix33<FT>& operator = (const TMatrix33<FT>& v) {
    data[0] = v[0];  data[1] = v[1];  data[2] = v[2];
    return *this;
  }

  template <class AT>
  TMatrix33<FT>& operator = (const TMatrix33<AT>& v) {
    data[0] = v[0];  data[1] = v[1];  data[2] = v[2];
    return *this;
  }
  template <class AM>
  TMatrix33<FT>& operator = (const AM& v) {
#ifdef _DEBUG
    if (v.ColCount() != 3 || v.RowCount() != 3) {
      throw TInvalidArgumentException(__OlxSourceInfo, "matrix");
    }
#endif
    for (int i = 0; i < 3; i++) {
      data[i][0] = v(i, 0);
      data[i][1] = v(i, 1);
      data[i][2] = v(i, 2);
    }
    return *this;
  }
  template <class AT>
  TMatrix33<FT>& operator -= (const TMatrix33<AT>& v) {
    data[0] -= v[0];
    data[1] -= v[1];
    data[2] -= v[2];
    return *this;
  }
  template <class AT>
  TMatrix33<FT> operator - (const TMatrix33<AT>& v) const {
    return TMatrix33<FT>(
      data[0][0] - v[0][0], data[0][1] - v[0][1], data[0][2] - v[0][2],
      data[1][0] - v[1][0], data[1][1] - v[1][1], data[1][2] - v[1][2],
      data[2][0] - v[2][0], data[2][1] - v[2][1], data[2][2] - v[2][2]);
  }
  template <class AT>
  TMatrix33<FT>& operator += (const TMatrix33<AT>& v) {
    data[0] += v[0];
    data[1] += v[1];
    data[2] += v[2];
    return *this;
  }
  template <class AT>
  TMatrix33<FT> operator + (const TMatrix33<AT>& v) const {
    return TMatrix33<FT>(
      data[0][0] + v[0][0], data[0][1] + v[0][1], data[0][2] + v[0][2],
      data[1][0] + v[1][0], data[1][1] + v[1][1], data[1][2] + v[1][2],
      data[2][0] + v[2][0], data[2][1] + v[2][1], data[2][2] + v[2][2]);
  }
  template <class AT> TMatrix33<FT>& operator *= (AT v) {
    data[0][0] *= v;  data[0][1] *= v;  data[0][2] *= v;
    data[1][0] *= v;  data[1][1] *= v;  data[1][2] *= v;
    data[2][0] *= v;  data[2][1] *= v;  data[2][2] *= v;
    return *this;
  }
  template <class AT> TMatrix33<FT> operator * (AT v) const {
    return TMatrix33<FT>(data[0][0] * v, data[0][1] * v, data[0][2] * v,
      data[1][0] * v, data[1][1] * v, data[1][2] * v,
      data[2][0] * v, data[2][1] * v, data[2][2] * v);
  }

  template <class AT> TMatrix33<FT>& operator /= (AT v) {
    data[0][0] /= v;  data[0][1] /= v;  data[0][2] /= v;
    data[1][0] /= v;  data[1][1] /= v;  data[1][2] /= v;
    data[2][0] /= v;  data[2][1] /= v;  data[2][2] /= v;
    return *this;
  }
  template <class AT> TMatrix33<FT> operator / (AT v) const {
    return TMatrix33<FT>(data[0][0] / v, data[0][1] / v, data[0][2] / v,
      data[1][0] / v, data[1][1] / v, data[1][2] / v,
      data[2][0] / v, data[2][1] / v, data[2][2] / v);
  }
  FT Trace() const { return data[0][0] + data[1][1] + data[2][2]; }
  FT Determinant() const {
    return data[0][0] * (data[1][1] * data[2][2] - data[1][2] * data[2][1]) -
      data[0][1] * (data[1][0] * data[2][2] - data[1][2] * data[2][0]) +
      data[0][2] * (data[1][0] * data[2][1] - data[1][1] * data[2][0]);
  }
  TMatrix33<FT> Inverse() const {
    return TMatrix33<FT>(
      data[2][2] * data[1][1] - data[2][1] * data[1][2],
      data[2][1] * data[0][2] - data[2][2] * data[0][1],
      data[1][2] * data[0][1] - data[1][1] * data[0][2],
      data[2][0] * data[1][2] - data[2][2] * data[1][0],
      data[2][2] * data[0][0] - data[2][0] * data[0][2],
      data[1][0] * data[0][2] - data[1][2] * data[0][0],
      data[2][1] * data[1][0] - data[2][0] * data[1][1],
      data[2][0] * data[0][1] - data[2][1] * data[0][0],
      data[1][1] * data[0][0] - data[1][0] * data[0][1]) /= Determinant();
  }
  /* column vector */
  template <class AT> TVector3<AT> operator * (const TVector3<AT>& a) const {
    return TVector3<AT>(
      static_cast<AT>(a[0] * data[0][0] + a[1] * data[0][1] + a[2] * data[0][2]),
      static_cast<AT>(a[0] * data[1][0] + a[1] * data[1][1] + a[2] * data[1][2]),
      static_cast<AT>(a[0] * data[2][0] + a[1] * data[2][1] + a[2] * data[2][2]));
  }
  // for real symmetric matrices only
  static void EigenValues(TMatrix33& A, TMatrix33& I) {
    size_t i, j;
    FT a = 2;
    while (olx_abs(a) > 1e-15) {
      MatMaxX(A, i, j);
      multMatrix(A, I, i, j);
      a = MatMaxX(A, i, j);
    }
  }
protected:
  /* used in the Jacoby eigenvalues search procedure. applicable to real
  symmetric matices only
  */
  static FT MatMaxX(const TMatrix33& m, size_t &i, size_t &j) {
    FT c = olx_abs(m[0][1]);
    i = 0;  j = 1;
    if (olx_abs(m[0][2]) > c) {
      j = 2;
      c = olx_abs(m[0][2]);
    }
    if (olx_abs(m[1][2]) > c) {
      i = 1;  j = 2;
      c = olx_abs(m[1][2]);
    }
    return c;
  }
  static void multMatrix(TMatrix33& D, TMatrix33& E,
    size_t i, size_t j)
  {
    FT cf, sf, cdf, sdf;
    static const FT sqr2 = (FT)sqrt(2.0) / 2;
    if (D[i][i] == D[j][j]) {
      cdf = 0;
      cf = sqr2;
      sf = olx_sign(D[i][j])*sqr2;
      sdf = olx_sign(D[i][j]);
    }
    else {
      FT tdf = 2 * D[i][j] / (D[j][j] - D[i][i]);
      FT r = tdf * tdf;
      cdf = sqrt(1.0 / (1 + r));
      cf = sqrt((1 + cdf) / 2.0);
      sdf = (sqrt(r / (1 + r)) * olx_sign(tdf));
      sf = (sqrt((1 - cdf) / 2.0)*olx_sign(tdf));
    }
    const FT ij = D[i][j], ii = D[i][i], jj = D[j][j];
    D[i][j] = D[j][i] = 0;
    D[i][i] = (ii*cf*cf + jj * sf*sf - ij * sdf);
    D[j][j] = (ii*sf*sf + jj * cf*cf + ij * sdf);

    for (size_t t = 0; t < 3; t++) {
      const FT eit = E[i][t], ejt = E[j][t];
      E[i][t] = eit * cf - ejt * sf;  //i
      E[j][t] = eit * sf + ejt * cf;  //j
      if ((t != i) && (t != j)) {
        const FT dit = D[i][t], djt = D[j][t];
        D[i][t] = D[t][i] = dit * cf - djt * sf;
        D[j][t] = D[t][j] = dit * sf + djt * cf;
      }
    }
  }
public:
  static TMatrix33 CreateRotationMatrix(const TVector3<FT>& rv, FT ca, FT sa) {
    TMatrix33 rm;
    return olx_create_rotation_matrix_(rm, rv, ca, sa);
  }
  // for cosine of the rotation angle
  static TMatrix33 CreateRotationMatrix(const TVector3<FT>& rv, FT ca) {
    TMatrix33 rm;
    return olx_create_rotation_matrix_(rm, rv, ca);
  }
  // for angle in degrees
  static TMatrix33 GetRotationMatrix(const TVector3<FT>& rv, FT angle) {
    TMatrix33 rm;
    FT ra = (angle * M_PI) / 180;
    return olx_create_rotation_matrix_(rm, rv, cos(ra), sin(ra));
  }
  /* solves a set of equations by the Cramer rule {equation arr.c = b },
  returns c
  */
  static TVector3<FT> CramerSolve(const TMatrix33<FT>& arr, const TVector3<FT>& b) {
    FT det = arr.Determinant();
    if (olx_abs(det) < 1e-15) {
      throw TDivException(__OlxSourceInfo);
    }
    // det( {b[0], a12, a13}, {b[1], a22, a23}, {b[2], a32, a33} )/det
    return TVector3<FT>(
      b[0] * (arr[1][1] * arr[2][2] - arr[1][2] * arr[2][1]) -
      arr[0][1] * (b[1] * arr[2][2] - arr[1][2] * b[2]) +
      arr[0][2] * (b[1] * arr[2][1] - arr[1][1] * b[2]),
      // det( {a11, b[0], a13}, {a21, b[1], a23}, {a31, b[2], a33} )/det
      arr[0][0] * (b[1] * arr[2][2] - arr[1][2] * b[2]) -
      b[0] * (arr[1][0] * arr[2][2] - arr[1][2] * arr[2][0]) +
      arr[0][2] * (arr[1][0] * b[2] - b[1] * arr[2][0]),
      // det( {a11, a12, b[0]}, {a21, a22, b[1]}, {a31, a32, b[2]} )/det
      arr[0][0] * (arr[1][1] * b[2] - b[1] * arr[2][1]) -
      arr[0][1] * (arr[1][0] * b[2] - b[1] * arr[2][0]) +
      b[0] * (arr[1][0] * arr[2][1] - arr[1][1] * arr[2][0])) / det;
  }
  // solves a set of equations by the Gauss method {equation arr.c = b ?c }
  static TVector3<FT> GaussSolve(TMatrix33<FT>& arr, TVector3<FT>& b) {
    MatrixElementsSort(arr, b);
    for (size_t j = 1; j < 3; j++)
      for (size_t i = j; i < 3; i++) {
        if (arr[i][j - 1] == 0) {
          continue;
        }
        b[i] *= -(arr[j - 1][j - 1] / arr[i][j - 1]);
        arr[i] *= -(arr[j - 1][j - 1] / arr[i][j - 1]);
        arr[i][0] += arr[j - 1][0];
        arr[i][1] += arr[j - 1][1];
        arr[i][2] += arr[j - 1][2];
        b[i] += b[j - 1];
      }
    if (olx_abs(arr[2][2]) < 1e-15) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "dependent set of equations");
    }
    TVector3<FT> c;
    c[2] = b[2] / arr[2][2];
    for (size_t j = 1; j != InvalidIndex; j--) {
      for (size_t k1 = 1; k1 < 4 - j; k1++) {
        if (k1 == (3 - j)) {
          for (size_t i = 2; i > 3 - k1; i--) {
            b[j] -= arr[j][i] * c[i];
          }
        }
      }
      c[j] = b[j] / arr[j][j];
    }
    return c;
  }

protected:  // used in GaussSolve to sort the matrix
  static void MatrixElementsSort(TMatrix33<FT>& arr, TVector3<FT>& b) {
    for (size_t i = 0; i < 3; i++) {
      FT bf[3] = { olx_abs(arr[0][i]), olx_abs(arr[1][i]), olx_abs(arr[2][i]) };
      size_t n = 0;
      if (bf[1] > bf[n]) {
        n = 1;
      }
      if (bf[2] > bf[n]) {
        n = 2;
      }
      if (n != i) {
        olx_swap(arr[i], arr[n]);
        olx_swap(b[i], b[n]);
      }
    }
  }
public:
  typedef FT number_type;
};

// multiplies each element by itself
template <typename FT>
static TVector3<FT> olx_sqr(const TVector3<FT>& v) {
  return TVector3<FT>(v[0] * v[0], v[1] * v[1], v[2] * v[2]);
}

template <typename FT>
olxstr vec2str_ext(const TVector3<FT>& n, const olxcstr &fmt) {
  return olx_print(
    olxcstr('{', fmt.Length()*3+5) << fmt << ',' << fmt << ',' << fmt << '}',
    n[0], n[1], n[2]);
}

template <typename FT>
olxcstr vec_element_fmt(const TVector3<FT>& n) {
  olxcstr f;
  if (olx_is_float(n[0])) {
    f << "%6.3" << olx_format_modifier(n[0]) << 'f';
  }
  else {
    f << "%6";
  }
  return f;
}


template <typename FT>
olxstr mat2str_ext(const TMatrix33<FT>& n, const olxcstr &fmt) {
  olxcstr f = olxcstr('{', fmt.Length() * 3 + 5) << fmt << ',' << fmt << ',' << fmt << '}';
  return olxstr_buf(olx_print(f, n[0][0], n[0][1], n[0][2]))
    << '\n' << olx_print(f, n[1][0], n[1][1], n[1][2])
    << '\n' << olx_print(f, n[2][0], n[2][1], n[2][2]);
}

template <typename FT>
olxstr vec2str(const TVector3<FT>& n) {
  return vec2str_ext(n, vec_element_fmt(n));
}

template <typename FT>
olxstr mat2str(const TMatrix33<FT>& n) {
  return mat2str_ext(n, vec_element_fmt(n[0]));
}

template <typename FT>
olxstr strof(const TVector3<FT>& n) { return vec2str(n); }

template <typename FT>
olxstr strof(const TMatrix33<FT>& n) { return mat2str(n); }


typedef TVector3<float>  vec3f;
typedef TVector3<double> vec3d;
typedef TVector3<int>    vec3i;
typedef TVector3<size_t> vec3s;

typedef TTypeList<vec3i> vec3i_list;
typedef TTypeList<vec3f> vec3f_list;
typedef TTypeList<vec3d> vec3d_list;

typedef TArrayList<vec3i> vec3i_alist;
typedef TArrayList<vec3f> vec3f_alist;
typedef TArrayList<vec3d> vec3d_alist;

typedef TPtrList<vec3i> vec3i_plist;
typedef TPtrList<vec3f> vec3f_plist;
typedef TPtrList<vec3d> vec3d_plist;

typedef TMatrix33<int>    mat3i;
typedef TMatrix33<float>  mat3f;
typedef TMatrix33<double> mat3d;

typedef TTypeList<mat3i> mat3i_list;
typedef TTypeList<mat3f> mat3f_list;
typedef TTypeList<mat3d> mat3d_list;

typedef TArrayList<mat3i> mat3i_alist;
typedef TArrayList<mat3f> mat3f_alist;
typedef TArrayList<mat3d> mat3d_alist;

typedef TPtrList<mat3i> mat3i_plist;
typedef TPtrList<mat3f> mat3f_plist;
typedef TPtrList<mat3d> mat3d_plist;

template <class FT>
olxstr strof(const TTypeList<TVector3<FT> > &v) {
  return olxstr(",").Join(v, FunctionAccessor::MakeStatic(&vec2str<FT>));
}

template <class FT>
olxstr strof(const TArrayList<TVector3<FT> >& v) {
  return olxstr(",").Join(v, FunctionAccessor::MakeStatic(&vec2str<FT>));
}


EndEsdlNamespace()
#endif
