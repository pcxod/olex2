/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_matrix_H
#define __olx_matrix_H
#include <cmath>
#include <stdio.h>
#include "emath.h"
#include "efile.h"
#include "evector.h"
#include "exception.h"
#include "estrlist.h"
#include "ebase.h"
#include "typelist.h"
#include "tptrlist.h"

#define MatrixError -2

#ifdef _MSC_VER
  #define MSVCC(a) ((double)(a))
#else
  #define MSVCC(a) (a)
#endif

BeginEsdlNamespace()

template <typename> class ConstMatrix;
template <typename> class TMatrix;
template <typename> class TVector;

template <typename heir_t, typename FT> struct MatOp {
private:
  const heir_t &Self() const { return *static_cast<const heir_t*>(this); }
  size_t ColCount_() const { return Self().ColCount(); }
  size_t RowCount_() const { return Self().RowCount(); }
  const FT& Get_(size_t i, size_t j) const { return Self()(i,j); }
public:
  /* the function multiplies a matrix by a column vector.
  */
  template <class AFT>
  ConstVector<FT> operator * (const TVector<AFT>& C) const
  {
    TVector<FT> R(RowCount_());
    return olx_vec::MulMatrixT(C, Self(), R);
  }

  template <class AType>
  ConstMatrix<FT> operator * (const TMatrix<AType>& C) const
  {
    TMatrix<FT> R(RowCount_(), C.ColCount());
    return olx_mat::MulMatrix(Self(), C, R);
  }

  ConstMatrix<FT> operator * (const ConstMatrix<FT>& C) const {
    return operator *(C.obj());
  }
  ConstMatrix<FT> operator + (FT V) const {  return TMatrix<FT>(Self()) += V;  }
  ConstMatrix<FT> operator - (FT V) const {  return TMatrix<FT>(Self()) -= V;  }
  ConstMatrix<FT> operator * (FT V) const {  return TMatrix<FT>(Self()) *= V;  }
  ConstMatrix<FT> operator / (FT V) const {  return TMatrix<FT>(Self()) /= V;  }
  template <class AFT>
  ConstMatrix<FT> operator + (const TMatrix<AFT>& c) const {
    return TMatrix<FT>(Self()) += c;
  }
  ConstMatrix<FT> operator + (const ConstMatrix<FT>& c) const {
    return TMatrix<FT>(Self()) += c.obj();
  }
  template <class AFT>
  ConstMatrix<FT> operator - (const TMatrix<AFT>& c) const {
    return TMatrix<FT>(Self()) -= c;
  }
  ConstMatrix<FT> operator - (const ConstMatrix<FT>& c) const {
    return TMatrix<FT>(Self()) -= c.obj();
  }
};

template <class FT> class TMatrix
  : public ACollectionItem,
  public MatOp<TMatrix<FT>, FT>
{
  typedef MatOp<TMatrix<FT>, FT> OpT;
  size_t Fn, Fm;
  void  Clear() {
    if (FData == 0) {
      return;
    }
    delete[] FData;
    FData = 0;
  }
  TVector<FT>* FData;
public:
  TMatrix() {
    FData = 0;
    Fn = Fm = 0;
    SetTag(-1);
  }

  TMatrix(size_t VectorsNumber, size_t ElementsNumber) {
    Fn = VectorsNumber;
    Fm = ElementsNumber;
    if ((Fn == 0) || (Fm == 0)) {
      FData = 0;
      return;
    }
    FData = new TVector<FT>[Fn];
    for (size_t i = 0; i < Fn; i++) {
      FData[i].Resize(Fm);
    }
    SetTag(-1);
  }

  template <class AType> TMatrix(const TMatrix<AType>& M) :
    Fn(M.RowCount()), Fm(M.ColCount())
  {
    if ((Fn == 0) || (Fm == 0)) {
      FData = 0;
      return;
    }
    FData = new TVector<FT>[Fn];
    for (size_t i = 0; i < Fn; i++) {
      FData[i] = M[i];
    }
  }

  TMatrix(const TMatrix& M) : Fn(M.Fn), Fm(M.Fm) {
    if ((Fn == 0) || (Fm == 0)) {
      FData = 0;
      return;
    }
    FData = new TVector<FT>[Fn];
    for (size_t i = 0; i < Fn; i++)
      FData[i] = M[i];
  }

  TMatrix(const ConstMatrix<FT>& v) : FData(0) {
    TakeOver(v.Release(), true);
  }

  virtual ~TMatrix() { Clear(); }

  TMatrix& TakeOver(TMatrix& l, bool do_delete = false) {
    if (FData != 0) {
      delete[] FData;
    }
    Fm = l.Fm;
    Fn = l.Fn;
    FData = l.FData;
    l.Fn = 0;
    l.FData = 0;
    if (do_delete) {
      delete& l;
    }
    return *this;
  }

  size_t Vectors() const { return Fn; }
  size_t Elements() const { return Fm; }
  size_t ColCount() const { return Fm; }
  size_t RowCount() const { return Fn; }
  bool IsEmpty() const { return Fm == 0 || Fn == 0; }

  const TVector<FT>& operator [](size_t index) const { return FData[index]; }
  TVector<FT>& operator [](size_t index) { return FData[index]; }
  const TVector<FT>& Get(size_t index) const { return FData[index]; }
  TVector<FT>& Get(size_t index) { return FData[index]; }
  const FT& Get(size_t i, size_t j) const { return FData[i][j]; }
  void Set(size_t i, size_t j, const FT& v) { FData[i][j] = v; }
  const FT& operator ()(size_t i, size_t j) const { return FData[i][j]; }
  TVector<FT>& operator ()(size_t i) { return FData[i]; }
  const TVector<FT>& operator ()(size_t i) const { return FData[i]; }
  FT& operator ()(size_t i, size_t j) { return FData[i][j]; }

  template <class AType> TMatrix& operator *= (const TMatrix<AType>& C) {
    return (*this = (*this * C));
  }

  template <class AType> TMatrix& operator += (const TMatrix<AType>& c) {
    if ((Fm == c.Elements()) && (Fn == c.Vectors()))
      for (size_t i = 0; i < Fn; i++) {
        FData[i] += c[i];
      }
    else {
      throw TFunctionFailedException(__OlxSourceInfo, "incompatible matrix dimensions");
    }
    return *this;
  }

  template <class AType> TMatrix& operator -= (const TMatrix<AType>& c) {
    if ((Fm == c.Elements()) && (Fn == c.Vectors())) {
      for (size_t i = 0; i < Fn; i++) {
        FData[i] -= c[i];
      }
    }
    else {
      throw TFunctionFailedException(__OlxSourceInfo, "incompatible matrix dimensions");
    }
    return *this;
  }

  TMatrix& operator += (FT V) {
    for (size_t i = 0; i < Fn; i++) {
      FData[i] += V;
    }
    return *this;
  }

  TMatrix& operator -= (FT V) {
    for (size_t i = 0; i < Fn; i++) {
      FData[i] -= V;
    }
    return *this;
  }

  TMatrix& operator *= (FT V) {
    for (size_t i = 0; i < Fn; i++) {
      FData[i] *= V;
    }
    return *this;
  }

  TMatrix& operator /= (FT V) {
    for (size_t i = 0; i < Fn; i++) {
      FData[i] /= V;
    }
    return *this;
  }

  template <class AType> bool operator == (const TMatrix<AType>& C) const {
    if ((Fm == C.Elements()) && (Fn == C.Vectors())) {
      for (size_t i = 0; i < Fn; i++) {
        if (!(FData[i] == C[i])) {
          return false;
        }
      }
      return true;
    }
    return false;
  }

  template <class AM> TMatrix& operator = (const AM& C) {
    return  Assign(C, C.RowCount(), C.ColCount());
  }

  TMatrix& operator = (const TMatrix& C) {
    Resize(C.Vectors(), C.Elements());
    for (size_t i = 0; i < Fn; i++) {
      FData[i] = C[i];
    }
    SetTag(C.GetTag());
    return *this;
  }

  TMatrix& operator = (const ConstMatrix<FT>& v) {
    return TakeOver(v.Release(), true);
  }

  template <class MC> TMatrix& Assign(const MC& M, size_t row_cnt,
    size_t col_cnt, bool resize = true)
  {
    if (resize) {
      Resize(row_cnt, col_cnt);
    }
    else {
      if (Fn < row_cnt || Fm < col_cnt) {
        throw TFunctionFailedException(__OlxSourceInfo, "indices out of bonds");
      }
    }
    for (size_t i = 0; i < row_cnt; i++) {
      for (size_t j = 0; j < col_cnt; j++) {
        FData[i][j] = M(i, j);
      }
    }
    return *this;
  }

  TMatrix& I() {
    for (size_t i = 0; i < Fn; i++) {
      for (size_t j = 0; j < Fm; j++) {
        FData[i][j] = (i == j ? 1 : 0);
      }
    }
    return *this;
  }

  void Null() {
    for (size_t i = 0; i < Fn; i++) {
      FData[i].Null();
    }
  }

  bool IsI() const {
    for (size_t i = 0; i < Fn; i++) {
      for (size_t j = 0; j < Fm; j++) {
        if (FData[i][j] != (i == j ? 1 : 0))
          return false;
      }
    }
    return true;
  }

  TMatrix& Resize(size_t n, size_t m) {
    if (n == 0 || m == 0) {
      Clear();
    }
    else if (Fn == n) {
      if (Fm != m) {
        for (size_t i = 0; i < n; i++) {
          FData[i].Resize(m);
        }
        Fm = m;
      }
    }
    else if (n < Fn) {  // do not shrink - just change the size indicator
      if (Fm != m) {
        for (size_t i = 0; i < n; i++) {
          FData[i].Resize(m);
        }
        Fm = m;
      }
      Fn = n;
    }
    else if (n > Fn) {
      TVector<FT>* nd = new TVector<FT>[n];
      for (size_t i = 0; i < Fn; i++) {
        nd[i] = FData[i];
      }
      delete[] FData;
      FData = nd;
      Fn = n;
      Fm = m;
      for (size_t i = 0; i < Fn; i++) {
        FData[i].Resize(Fm);
      }
    }
    return *this;
  }

  void SwapRows(size_t s, size_t to) {
    for (size_t i = 0; i < Fm; i++) {
      olx_swap(FData[s][i], FData[to][i]);
    }
  }

  void SwapCols(size_t s, size_t to) {
    for (size_t i = 0; i < Fm; i++) {
      olx_swap(FData[i][s], FData[i][to]);
    }
  }

  void AddRows(size_t to, size_t which) { FData[to] += FData[which]; }

  void AddCols(size_t to, size_t which) {
    for (size_t i = 0; i < Fn; i++) {
      FData[i][to] += FData[i][which];
    }
  }

  void SubRows(size_t from, size_t which) { FData[from] -= FData[which]; }

  void SubCols(size_t from, size_t which) {
    for (size_t i = 0; i < Fn; i++) {
      FData[i][from] -= FData[i][which];
    }
  }

  void MulRow(size_t which, FT v) { FData[which] *= v; }
  void DivRow(size_t which, FT v) { FData[which] /= v; }

  void MulCol(size_t which, FT v) {
    for (size_t i = 0; i < Fn; i++) {
      FData[i][which] *= v;
    }
  }

  void DivCol(size_t which, FT v) {
    if (v == 0) {
      throw TDivException(*this, "DivCol");
    }
    for (size_t i = 0; i < Fn; i++) {
      FData[i][which] /= v;
    }
  }

  TMatrix& Transpose() {
    if (Fn == 0 || Fm == 0) {
      ;
    }
    else if (Fn != Fm) {
      TVector<FT>* data = new TVector<FT>[Fm];
      for (size_t i = 0; i < Fm; i++) {
        data[i].Resize(Fn);
        for (size_t j = 0; j < Fn; j++) {
          data[i][j] = FData[j][i];
        }
      }
      olx_swap(Fn, Fm);
      delete[] FData;
      FData = data;
    }
    else {
      for (size_t i = 0; i < Fm; i++) {
        // need to go around a half of matrix
        for (size_t j = i + 1; j < Fn; j++) {
          olx_swap(FData[j][i], FData[i][j]);
        }
      }
    }
    return *this;
  }

  static ConstMatrix<FT> Transpose(const TMatrix& matr) {
    TMatrix m(matr.ColCount(), matr.RowCount());
    for (size_t i = 0; i < matr.RowCount(); i++) {
      for (size_t j = 0; j < matr.ColCount(); j++) {
        m(j, i) = matr(i, j);
      }
    }
    return m;
  }

  FT Trace() const {
    if (Fn != Fm) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "incompatible matrix dimensions");
    }
    FT a = 0;
    for (size_t i = 0; i < Fn; i++) {
      a += FData[i][i];
    }
    return a;
  }

  TIString ToString() const {
    olxstr_buf t;
    for (size_t i = 0; i < Fn; i++) {
      t << FData[i].ToString() << "\n";
    }
    return olxstr(t);
  }

  bool LoadFromFile(const olxstr& FN) {
    TCStrList S = TEFile::ReadCLines(FN);
    if (S.IsEmpty()) {
      return false;
    }
    for (size_t i = 0; i < S.Count(); i++) {
      TCStrList Toks(S[i].Replace('\t', ' '), ' ');
      if (i == 0) {
        size_t dim = Toks.Count();
        if (dim == 0) {
          return false;
        }
        Resize(S.Count(), dim);
      }
      if (Toks.Count() != Fm) {
        throw TFunctionFailedException(__OlxSourceInfo, "file rows not even");
      }
      for (size_t j = 0; j < Fm; j++) {
        FData[i][j] = (FT)Toks[j].ToDouble();
      }
    }
    return true;
  }

  void SaveToFile(const olxstr& FN) {
    TCStrList S;
    for (size_t i = 0; i < Fn; i++) {
      olxcstr& T = S.Add();
      for (size_t j = 0; j < Fm; j++) {
        T << olxcstr::FormatFloat(5, FData[i][j], true) << '\t';
      }
    }
    TEFile::WriteLines(FN, S);
  }

  //------------------------------------------------------------------------------
    // static methods
    // searches for maximum absolute element in matrix
  static FT MatMax(const TMatrix& A, size_t& i, size_t& j) {
    FT c = olx_abs(A[0][0]);
    for (size_t a = 0; a < A.Vectors(); a++) {
      for (size_t b = 0; b < A.Elements(); b++) {
        FT d = olx_abs(A[a][b]);
        if (d > c) {
          i = a;
          j = b;
          c = d;
        }
      }
    }
    return c;
  }


  // searches for maximum abs element in top off dialgonal part of the matrix
  static FT MatMaxX(const TMatrix& A, size_t& i, size_t& j) {
    FT c = olx_abs(A[0][1]);
    i = 0;
    j = 1;
    for (size_t a = 0; a < A.Elements(); a++) {
      for (size_t b = a + 1; b < A.Elements(); b++) {
        FT d = olx_abs(A[a][b]);
        if (d > c) {
          i = a;
          j = b;
          c = d;
        }
      }
    }
    return c;
  }


  // searches for maximum abs element in a matrix row
  static FT RowMax(const TMatrix& A, size_t i, size_t& j) {
    FT c = olx_abs(A[i][0]);
    j = 0;
    for (size_t a = 1; a < A.Elements(); a++) {
      FT d = olx_abs(A[i][a]);
      if (d > c) {
        j = a;
        c = d;
      }
    }
    return c;
  }

  // searches for minimum element in a matrix row
  static FT RowMin(const TMatrix& A, size_t row, size_t& j) {
    FT a = A[row][0];
    j = 0;
    for (size_t i = 1; i < A.Elements(); i++)
      if (A[row][i] < a) {
        a = A[row][i];
        j = i;
      }
    return A[row][j];
  }


  // searches for maximum abs element in a matrix column
  static FT ColMax(const TMatrix& A, size_t i, size_t& j) {
    FT c = olx_abs(A[0][i]);
    j = 0;
    for (size_t a = 1; a < A.Vectors(); a++) {
      FT d = olx_abs(A[a][i]);
      if (d > c) {
        j = a;
        c = d;
      }
    }
    return c;
  }

  // solves a set of equations by the Gauss method {equation arr.c = b }
  static void GaussSolve(TMatrix<FT>& arr, TVector<FT>& b) {
    const size_t sz = arr.Elements();
    MatrixElementsSort(arr, b);
    for (size_t j = 1; j < sz; j++) {
      for (size_t i = j; i < sz; i++) {
        if (arr[i][j - 1] == 0) {
          continue;
        }
        const FT k = -arr[j - 1][j - 1] / arr[i][j - 1];
        arr.MulRow(i, k);
        arr.AddRows(i, j - 1);
        b[i] *= k;
        b[i] += b[j - 1];
      }
    }

    if (arr[sz - 1][sz - 1] == 0) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "dependent set of equations");
    }

    for (size_t j = sz - 1; j != InvalidIndex; j--) {
      for (size_t k1 = 1; k1 < sz - j + 1; k1++) {
        if (k1 == (sz - j)) {
          for (size_t i = sz - 1; i > sz - k1; i--) {
            b[j] -= arr[j][i] * b[i];
          }
        }
      }
      b[j] /= arr[j][j];
    }
  }

  // exteded version
  static bool GaussSolve(TMatrix<FT>& arr, TMatrix<FT>& b) {
    const size_t sz = arr.Elements();
    MatrixElementsSort(arr, b);
    for (size_t j = 1; j < sz; j++) {
      for (size_t i = j; i < sz; i++) {
        if (arr[i][j - 1] == 0) {
          continue;
        }
        const FT k = -arr[j - 1][j - 1] / arr[i][j - 1];
        arr.MulRow(i, k);
        arr.AddRows(i, j - 1);
        b.MulRow(i, k);
        b.AddRows(i, j - 1);
      }
    }
    if (arr[sz - 1][sz - 1] == 0)
      return false;
    for (size_t i = sz - 1; i != ~0; i--) {
      for (size_t j = 1; j < sz - i + 1; j++) {
        if (j == (sz - i)) {
          for (size_t k = 0; k < b.ColCount(); k++) {
            for (size_t l = sz - 1; l > sz - j; l--) {
              b[i][k] -= arr[i][l] * b[l][k];
            }
          }
        }
      }
      for (size_t j = 0; j < b.ColCount(); j++) {
        b[i][j] /= arr[i][i];
      }
    }
    return true;
  }

  // used in GaussSolve to sort the matrix
  static void MatrixElementsSort(TMatrix<FT>& arr, TVector<FT>& b) {
    const size_t dim = arr.Elements();
    for (size_t i = 0; i < dim; i++) {
      size_t max_i = 0;
      FT max_v = arr[0][i];
      for (size_t j = 1; j < dim; j++) {
        if (arr[j][i] > max_v) {
          max_i = j;
          max_v = arr[j][i];
        }
      }
      if (max_i != i) {
        arr.SwapRows(i, max_i);
        olx_swap(b[i], b[max_i]);
      }
    }
  }
  // extended version of above...
  static void MatrixElementsSort(TMatrix<FT>& arr, TMatrix<FT>& b) {
    const size_t dim = arr.Elements();
    for (size_t i = 0; i < dim; i++) {
      size_t max_i = 0;
      FT max_v = arr[0][i];
      for (size_t j = 1; j < dim; j++) {
        if (arr[j][i] > max_v) {
          max_i = j;
          max_v = arr[j][i];
        }
      }
      if (max_i != i) {
        arr.SwapRows(i, max_i);
        b.SwapRows(i, max_i);
      }
    }
  }

  /* A polynomial least square analysis of XY pairs stored in matrix[2][n]
   extent is the polynom extent (1-is for line)
   params will contain the fitting parameters
   the return value is the RMS - root mean square of the fit
  */
  static FT PLSQ(TMatrix<FT>& xy, TVector<FT>& param, size_t extent) {
    extent++;
    if (xy.Elements() < extent)
      throw TInvalidArgumentException(__OlxSourceInfo, "not enough data");
    TMatrix<FT> s(extent, extent);
    param.Resize(extent);

    for (size_t i = 0; i < extent; i++) {
      for (size_t j = i; j < extent; j++) {
        FT b = 0;
        for (size_t k = 0; k < xy.Elements(); k++) {
          if (xy[0][k] == 0) {
            continue;
          }
          FT i_a = olx_ipow(xy[0][k], i),
            j_a = (j == i ? i_a : olx_ipow(xy[0][k], j));
          b += (FT)(i_a * j_a);
        }
        s[i][j] = b;
      }
      FT a = 0;
      for (size_t j = 0; j < xy.Elements(); j++) {
        a += xy[1][j] * olx_ipow(xy[0][j], i);
      }
      param[i] = a;
    }
    // fill the bottom off diagonal part
    for (size_t i = 0; i < extent; i++) {
      for (size_t j = i + 1; j < extent; j++) {
        s[j][i] = s[i][j];
      }
    }

    GaussSolve(s, param);
    FT rms = 0;
    for (size_t i = 0; i < xy.Elements(); i++) {
      rms += olx_sqr(xy[1][i] - TVector<FT>::PolynomValue(param, xy[0][i]));
    }
    return (rms > 0) ? sqrt(rms) / xy.Elements() : 0;
  }

  // Lagrange interpolation
  static FT Lagrang(const TVector<FT>& x, const TVector<FT>& y, FT point) {
    const size_t sz = x.Count();
    FT p1 = 0;
    for (size_t i = 0; i < sz; i++) {
      FT a = 1, b = 1;
      for (size_t j = 0; j < sz; j++) {
        if (i != j) {
          a *= (point - x[j]);
          b *= (x[i] - x[j]);
        }
      }
      p1 += (a / b) * y[i];
    }
    return p1;
  }
  static FT Lagrang(const TMatrix& xy, FT point) {
    return Lagrange(xy[0], xy[1], point);
  }

  // calculates eigen values of symmetric matrix
  static void  EigenValues(TMatrix& A, TMatrix& E, FT eps = FT(1e-15)) {
    FT a = 2;
    while (olx_abs(a) > eps) {
      size_t i, j;
      MatMaxX(A, i, j);
      multMatrix(A, E, i, j);
      a = MatMaxX(A, i, j);
    }
  }

  // used in the Jacoby eigenvalues search procedure
protected:
  static void multMatrix(TMatrix<FT>& D, TMatrix<FT>& E, size_t i, size_t j)
  {
    static FT sq2o2 = FT(sqrt(2.0) / 2);
    FT cf, sf, cdf, sdf;
    if (D[i][i] == D[j][j]) {
      cdf = 0;
      cf = sq2o2;
      sf = olx_sign(D[i][j]) * cf;
      sdf = olx_sign(D[i][j]);
    }
    else {
      const FT tdf = 2 * D[i][j] / (D[j][j] - D[i][i]);
      const FT r = tdf * tdf;
      cdf = (FT)sqrt(1.0 / (1 + r));
      cf = (FT)sqrt((1 + cdf) / 2.0);
      sdf = (FT)(sqrt(r / (1 + r)) * olx_sign(tdf));
      sf = (FT)(sqrt((1 - cdf) / 2.0) * olx_sign(tdf));
    }
    const FT ij = D[i][j], ii = D[i][i], jj = D[j][j];
    D[i][j] = D[j][i] = 0;
    D[i][i] = (ii * cf * cf + jj * sf * sf - ij * sdf);
    D[j][j] = (ii * sf * sf + jj * cf * cf + ij * sdf);

    for (size_t a = 0; a < D.Vectors(); a++) {
      const FT ij = E[i][a], ji = E[j][a];
      E[i][a] = ij * cf - ji * sf;
      E[j][a] = ij * sf + ji * cf;
      if ((a != i) && (a != j)) {
        const FT ia = D[i][a], ja = D[j][a];
        D[i][a] = D[a][i] = ia * cf - ja * sf;
        D[j][a] = D[a][j] = ia * sf + ja * cf;
      }
    }
  }
public:
  typedef FT number_type;
};

  typedef TMatrix<float> ematf;
  typedef TMatrix<double> ematd;

  typedef TTypeList<ematf> ematf_list;
  typedef TTypeList<ematd> ematd_list;
  typedef TPtrList<ematf> ematf_plist;
  typedef TPtrList<ematd> ematd_plist;

template <typename FT>
class ConstMatrix
  : public const_mat<TMatrix<FT> >,
    public MatOp<ConstMatrix<FT>, FT>
{
  typedef TMatrix<FT> mat_t;
  typedef const_mat<mat_t> parent_t;
public:
  ConstMatrix(const ConstMatrix &l) : parent_t(l) {}
  ConstMatrix(mat_t *arr) : parent_t(arr) {}
  ConstMatrix(mat_t &arr) : parent_t(arr) {}
  ConstMatrix &operator = (const ConstMatrix &l) {
    parent_t::operator = (l);
    return *this;
  }
public:
  typedef FT number_type;
};
EndEsdlNamespace()
#endif
