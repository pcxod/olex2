//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
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
#include "datafile.h"
#include "typelist.h"
#include "tptrlist.h"

#define MatrixError -2

#ifdef _MSC_VER
  #define MSVCC(a) ((double)(a))
#else
  #define MSVCC(a) (a)
#endif

BeginEsdlNamespace()

template <typename> class TMatrix;
template <typename> class TVector;

//------------------------------------------------------------------------------
template <class MatType> class TMatrix: public ACollectionItem  {
  size_t Fn, Fm;
  void  Clear()  {
    if( FData == NULL )  return;
    delete [] FData;
    FData = NULL;  
  }
  TVector <MatType>* FData;
public:
  TMatrix()  {
    FData = NULL;
    Fn = Fm = 0;
    SetTag(-1);
  }

  TMatrix(size_t VectorsNumber, size_t ElementsNumber)  {
    Fn = VectorsNumber;
    Fm = ElementsNumber;
    if( (Fn == 0) || (Fm == 0) )   {  FData = NULL;  return;  }
    FData = new TVector<MatType>[Fn];
    for( size_t i=0; i < Fn; i++ )  
      FData[i].Resize(Fm);
    SetTag(-1);
  }

  template <class AType> TMatrix(const TMatrix<AType>& M) : Fn(M.Fn), Fm(M.Fm)  {
    if( (Fn == 0) || (Fm == 0) )   {  FData = NULL;  return;  }
    FData = new TVector<MatType>[Fn];
    for( size_t i=0; i < Fn; i++ )  
      FData[i] = M[i];
  }

  TMatrix(const TMatrix& M) : Fn(M.Fn), Fm(M.Fm)  {
    if( (Fn == 0) || (Fm == 0) )   {  FData = NULL;  return;  }
    FData = new TVector<MatType>[Fn];
    for( size_t i=0; i < Fn; i++ )  
      FData[i] = M[i];
  }

  virtual ~TMatrix()  {  Clear();  }

  size_t Vectors() const {  return Fn;  }
  size_t Elements() const {  return Fm;  } 
  size_t ColCount() const {  return Fm;  }
  size_t RowCount() const {  return Fn;  }

  const TVector<MatType>& operator [](size_t index) const {  return FData[index];  }
  TVector<MatType>& operator [](size_t index) {  return FData[index];  }
  const TVector<MatType>& Get(size_t index) const {  return FData[index];  }
  TVector<MatType>& Get(size_t index) {  return FData[index];  }
  const MatType& Get(size_t i, size_t j) const {  return FData[i][j];  }
  void Set(size_t i, size_t j, const MatType& v) {  FData[i][j] = v;  }
  /* the function multiplies a matrix by a column vector. Only the number of vector
   elements is taken from the matrix - no error will be generated if the matrix dimensions
   (number of elements) are larger than the vector dimentions!!!
  */
  template <class AType> TVector<MatType> operator * (const TVector<AType>& C) const  {
    if( (Fm < C.Count()) || (Fn == 0) )
      throw TFunctionFailedException(__OlxSourceInfo, "incompatible matrix dimensions");
    TVector<MatType> R(Fn);
    for( size_t i = 0; i < Fn; i++ )
      for( size_t j =0; j < C.Count(); j++ )
        R[i] += C[j]*FData[i][j];
    return R;
  }

  template <class AType> TMatrix operator * (const TMatrix<AType>& C) const {
    const size_t X = Vectors();
    const size_t Y = Elements();
    const size_t Z = C.Elements();
    TMatrix<MatType> Result(X, Z);
    for( size_t i = 0; i < X; i++ )
      for( size_t j = 0; j < Z; j++ )
        for( size_t k = 0; k < Y; k++ )
          Result[i][j] += FData[i][k]*C[k][j];
    return Result;
  }

  template <class AType> TMatrix operator  + (const TMatrix<AType>& c) const {
    return TMatrix<MatType>(*this) += c;
  }

  template <class AType> TMatrix operator  - (const TMatrix<AType>& c) const {
    return TMatrix<MatType>(*this) -= c;
  }

  template <class AType> TMatrix& operator *= (const TMatrix<AType>& C)  {
    return (*this = (*this * C));
  }

  template <class AType> TMatrix& operator += (const TMatrix<AType>& c)  {
    if( (Fm ==c.Elements()) && (Fn == c.Vectors()) )
      for( size_t i=0; i < Fn; i++ )  
        FData[i] += c[i];
    else
      throw TFunctionFailedException(__OlxSourceInfo, "incompatible matrix dimensions");
    return *this;
  }

  template <class AType> TMatrix& operator -=(const TMatrix<AType>& c)  {
    if( (Fm ==c.Elements()) && (Fn == c.Vectors()) )
      for( size_t i=0; i < Fn; i++ )  
        FData[i] -= c[i];
    else
      throw TFunctionFailedException(__OlxSourceInfo, "incompatible matrix dimensions");
    return *this;
  }

  TMatrix operator + (MatType V)  {  return TMatrix<MatType>(*this) += V;  }
  TMatrix operator - (MatType V)  {  return TMatrix<MatType>(*this) -= V;  }
  TMatrix operator * (MatType V)  {  return TMatrix<MatType>(*this) *= V;  }
  TMatrix operator / (MatType V)  {  return TMatrix<MatType>(*this) /= V;  }

  TMatrix& operator += (MatType V)  {
    for( size_t i = 0; i < Fn; i++ )  
      FData[i] += V;
    return *this;
  }

  TMatrix& operator -= (MatType V)  {
    for( size_t i = 0; i < Fn; i++ )  
      FData[i] -= V;
    return *this;
  }

  TMatrix& operator *= (MatType V)  {
    for( size_t i = 0; i < Fn; i++ )  
      FData[i] *= V;
    return *this;
  }
    
  TMatrix& operator /= (MatType V)  {
    for( size_t i = 0; i < Fn; i++ )  
      FData[i] /= V;
    return *this;
  }

  template <class AType> bool  operator == (const TMatrix<AType>& C) const {
    if( (Fm == C.Elements()) && (Fn == C.Vectors() ) )  {
      for( size_t i = 0; i < Fn; i++ )
        if( !(FData[i] == C[i]) )
          return false;
      return true;
    }
    return false;
  }

  template <class AType> TMatrix& operator = (const TMatrix<AType>& C)  {
    this->Resize(C.Vectors(), C.Elements());
    for( size_t i = 0; i < Fn; i++)
      FData[i] = C[i];
    SetTag( C.GetTag() );
    return *this;
  }

  TMatrix& operator = (const TMatrix& C)  {
    Resize(C.Vectors(), C.Elements());
    for( size_t i = 0; i < Fn; i++ )
      FData[i] = C[i];
    SetTag( C.GetTag() );
    return *this;
  }

  template <class MC> TMatrix& Assign(const MC& M, size_t width, size_t height)  {
    Resize(width, height);
    for( size_t i=0; i < width; i++ )
      for(size_t j=0; j < height; j++ )
        FData[j][i] = M[j][i];
    return *this;
  }

  TMatrix& I() {
    for( size_t i=0; i < Fn; i++ )  {
      for( size_t j=0; j < Fm; j++ )  {
        if( i == j )  
          FData[i][j]=(MatType)1.0;
        else      
          FData[i][j]=0;
      }
    }
    return *this;
  }

  void Null()  {
    for( size_t i=0; i < Fn; i++ )  
      FData[i].Null();
  }

  bool IsI() const  {
    for( size_t i=0; i < Fn; i++ )  {
      for( size_t j=0; j < Fm; j++ )  {
        if( i == j )  {  
          if( FData[i][j] != 1.0 ) 
            return false; 
        }
        else  {  
          if( FData[i][j] != 0.0 )  
            return false; 
        }
      }
    }
    return true;
  }

  TMatrix& Resize(size_t n, size_t m)  {
    if( n == 0 || m == 0 )  {
      Clear();
    }
    else if( Fn == n )  {
      if( Fm != m )  {
        for( size_t i=0; i < n; i++ )  
          FData[i].Resize(m);
        Fm = m;
      }
    }
    else if( n < Fn )  {  // do not shrink - just change the size indicator
      if( Fm != m )  {
        for( size_t i=0; i < n; i++ )  
          FData[i].Resize(m);
        Fm = m;
      }
      Fn = n;
    }
    else if( n > Fn )  {
      TVector<MatType>* nd = new TVector<MatType>[n];
      for( size_t i=0; i < Fn; i++ )
        nd[i] = FData[i];
      delete [] FData;
      FData = nd;
      Fn = n;  
      Fm = m;
      for( size_t i=0; i < Fn; i++ )
        FData[i].Resize(Fm);
    }
    return *this;
  }

  void SwapRows(size_t s, size_t to)  {
    for( size_t i=0; i < Fm; i++ )
      olx_swap(FData[s][i], FData[to][i]);
  }

  void SwapCols(size_t s,size_t to)  {
    for( size_t i = 0; i < Fm; i++ )
      olx_swap(FData[i][s], FData[i][to]);
  }

  void AddRows(size_t to, size_t which )  {  FData[to] += FData[which];  }
  
  void AddCols(size_t to, size_t which )    {  
    for( size_t i=0; i < Fn; i++ )  
      FData[i][to] += FData[i][which];  
  }

  void SubRows(size_t from, size_t which )  {  FData[from] -= FData[which];  }
  
  void SubCols(size_t from, size_t which)  {  
    for( size_t i=0; i < Fn; i++ )  
      FData[i][from] -= FData[i][which];  
  }
  
  void MulRow(size_t which, MatType v)  {  FData[which] *= v;  }
  void DivRow(size_t which, MatType v)  {  FData[which] /= v;  }

  void MulCol(size_t which, MatType v)  {    
    for( size_t i=0; i < Fn; i++ )  
      FData[i][which] *= v;  
  }

  void DivCol(size_t which, MatType v)  {
    if( v == 0 )
      throw TDivException(*this, "DivCol");
    for( size_t i=0; i < Fn; i++ )
      FData[i][which] /= v;
  }

  TMatrix& Transpose()  {
    if( Fn != Fm )
      throw TFunctionFailedException(__OlxSourceInfo, "incompatible matrix dimensions");
    for( size_t i=0; i < Fm; i++ )  {
      for( size_t j=i+1; j < Fn; j++ )  {  // need to go around a half of matrix
        if( i == j )  continue;
        olx_swap(FData[j][i], FData[i][j]);
      }
    }
    return *this;
  }
  static TMatrix Transpose(const TMatrix& matr)  {
    TMatrix rv(matr.Elements(), matr.Vectors());
    for( size_t i = 0; i < matr.Vectors(); i++ )  {
      for( size_t j = 0; j < matr.Elements(); j++ )
        rv[j][i] = matr[i][j];
    }
    return rv;
  }

  MatType Trace() const  {
    if( Fn != Fm )
      throw TFunctionFailedException(__OlxSourceInfo, "incompatible matrix dimensions");
    MatType a = 0;
    for( size_t i = 0; i < Fn; i++ )  
      a += FData[i][i];
    return a;
  }

  void Print() const  {
    printf("\n");
    for( size_t i = 0; i < Fn; i++ )  {
      FData[i].Print();
      printf("\n");   }
    printf("\n");
  }


  bool LoadFromFile(const olxstr& FN)  {
    TStrList S;
    S.LoadFromFile( FN );
    if( S.IsEmpty() )  return false;
    for( size_t i=0; i < S.Count(); i++ )  {
      TStrList Toks(S[i].Replace('\t', ' '), ' ');
      if( i == 0 )  {
        size_t dim = Toks.Count();
        if( dim == 0 )  return false;
        Resize(S.Count(), dim);
      }
      if( Toks.Count() != Fm )  
        throw TFunctionFailedException(__OlxSourceInfo, "file rows not even");
      for( size_t j=0; j < Fm; j++ )
        FData[i][j] = (MatType)Toks[j].ToDouble();
    }
    return true;
  }

  void SaveToFile(const olxstr& FN)  {
    TCStrList S;
    for( size_t i=0; i < Fn; i++ )  {
      olxstr& T = S.Add(EmptyString());
      for( size_t j=0; j < Fm; j++ )
        T << olxstr::FormatFloat(5, FData[i][j], true) << '\t';
    }
    S.SaveToFile(FN);
  }

//------------------------------------------------------------------------------
  // static methods
  // searches for maximum element in matrix
  static MatType MatMax(const TMatrix& A, size_t& i, size_t& j)  {
    MatType c = olx_abs(A[0][0]);
    for( size_t a = 0; a < A.Vectors(); a++ )  {
      for( size_t b = 0; b < A.Elements(); b++ )  {
        if( a == 0 && b == 0 )  continue;
        if( olx_abs(A[a][b]) > c )  {
          i = a;
          j = b;
          c = olx_abs(A[a][b]);
        }
      }
    }
    return c;
  }


  // searches for maximum element in matrix dialgonal matrix
  // without consideration of the main diagonal
  static MatType MatMaxX(const TMatrix& A, size_t& i, size_t& j)  {
    MatType c = olx_abs(A[0][1]);
    i = 0;
    j = 1;
    if( A.Elements() == 2 )  {
      i = 0;
      j = 1;
      return c;
    }
    for( size_t a = 0; a < A.Elements(); a++ )  {
      for( size_t b = a+1; b < A.Elements(); b++ )  {
        if( olx_abs(A[a][b]) > c ) {
          i = a;
          j = b;
          c = olx_abs(A[a][b]);
        }
      }
    }
    return c;
  }


  // searches for maximum element in a matrix row
  static MatType RowMax(const TMatrix& A, size_t i, size_t& j)  {
    MatType c = olx_abs(A[i][0]);
    j = 0;
    for( size_t a = 1; a < A.Elements(); a++ )  {
      if( olx_abs(A[i][a]) > c )  {
        j = a;
        c = olx_abs(A[i][a]);
      }
    }
    return c;
  }

  // searches for minimum element in a matrix row
  static MatType RowMin(const TMatrix& A, size_t row, size_t& j)  {
    MatType a = A[row][0];
    j = 0;
    for( size_t i=1; i < A.Elements(); i++ )
      if( A[row][i] < a )  {
        a = A[row][i];
        j = i;
      }
    return A[row][j];
  }


  // searches for maximum element in a matrix column
  static MatType ColMax(const TMatrix&  A, size_t i, size_t& j )  {
    MatType c = olx_abs(A[0][i]);
    j = 0;
    for( size_t a = 1; a < A.Vectors(); a++ )  {
      if( olx_abs(A[a][i]) > c )  {
        j = a;
        c = olx_abs(A[a][i]);
      }
    }
   return c;
  }

  // solves a set of equations by the Gauss method {equation arr.b = c }
  static void GaussSolve(TMatrix<MatType>& arr, TVector<MatType>& b, TVector<MatType>& c) {
    const size_t sz = arr.Elements();
    MatrixElementsSort(arr, b );
    for ( size_t j = 1; j < sz; j++ )
      for( size_t i = j; i < sz; i++ )  {
        if( arr[i][j-1] ==0 )  continue;
        b[i]  *= -(arr[j-1][j-1]/arr[i][j-1]);
        arr.MulRow(i, -(arr[j-1][j-1]/arr[i][j-1]));
        arr.AddRows( i, j-1);
        b[i]+=b[j-1];
      }
    if( arr[sz-1][sz-1]==0)
      throw TFunctionFailedException(__OlxSourceInfo, "dependent set of equations");

    c[sz-1] = b[sz-1]/arr[sz-1][sz-1];
    for( size_t j = sz-2; j != ~0; j-- )  {
      for( size_t k1=1; k1 < sz-j+1; k1++ )  {
        if( k1 == (sz-j) )
          for( size_t i=sz-1; i > sz-k1; i-- )
            b[j]-=arr[j][i]*c[i];
      }
      c[j]= b[j]/arr[j][j];
     }
   }

      // used in GaussSolve to sort the matrix
  static void MatrixElementsSort(TMatrix<MatType>& arr, TVector<MatType>& b)  {
    const size_t dim = arr.Elements();
    MatType *bf = new MatType[dim];
    for( size_t i = 0; i < dim; i++ )  {
      for( size_t j = 0; j < dim; j++ )
        bf[j] = olx_abs(arr[j][i]);
      size_t n;
      TVector<MatType>::ArrayMax(bf, n, dim);
      if( n != i )  {
        arr.SwapRows(i,n);
        olx_swap(b[i], b[n]);
      }
    }
    delete [] bf;
  }

  /* A polynomial least square analysis of XY pairs strored in matrix[2][n]
   extent is the polynom extent (1-is for line)
   params will contain the fitting parameters
   the return value is the RMS - root mean square of the fit
  */
  static MatType PLSQ(TMatrix<MatType>& xy, TVector<MatType>& param, size_t extent)  {
    extent ++;
    if( xy.Elements() < extent )
      throw TInvalidArgumentException(__OlxSourceInfo, "not enough data");
    TMatrix<MatType> s(extent, extent);
    TVector<MatType> r(extent);
    param.Resize(extent);

    for( size_t i = 0; i < extent; i++ )  {
      for( size_t j = i; j < extent; j++ )  {
        MatType b = 0;
        for( size_t k = 0; k < xy.Elements(); k++ )  {
          if( xy[0][k] == 0 )    continue;
          MatType i_a, j_a;
          if( i == 0 )       i_a = 1;
          else if( i == 1 )  i_a = xy[0][k];
          else               i_a = pow(MSVCC(xy[0][k]), MSVCC(i));

          if( j == i )       j_a = i_a;
          else if( j == 0 )  j_a = 1;
          else if( j == 1 )  j_a = xy[0][k];
          else               j_a = pow(MSVCC(xy[0][k]), MSVCC(j));
          b += (MatType)(i_a*j_a);
        }
        s[i][j] = b;
      }
      MatType a = 0;
      for( size_t j=0; j < xy.Elements(); j++ )  {
        if( xy[0][j] == 0 )  continue;
        a += (MatType)(xy[1][j]*pow( MSVCC(xy[0][j]), MSVCC(i) ) );
      }
      r[i] = a;
    }
    // fill the bottom off diagonal part
    for( size_t i = 0; i < extent; i++ )
      for( size_t j = i+1; j < extent; j++ )
        s[j][i] = s[i][j];

    GaussSolve(s, r, param);
    double rms = 0;
    for( size_t i=0; i < xy.Elements(); i++ )
      rms += olx_sqr( xy[1][i]-TVector<MatType>::PolynomValue(param, xy[0][i]) );
    return (rms > 0) ? sqrt(rms)/xy.Elements() : 0;
  }

  // Lagrange interpolation
  static double  Lagrang(TVector<MatType>& x, const TVector<MatType>& y, double point )  {
    int sz = x.Count();
    double a, b, p1 = 0;
    for(int i = 0; i < sz; i ++ )  {
      a = b = 1;
      for(int j = 0; j < sz; j ++ )  {
       if( i != j )  {
         a *= (point - x[j]);
         b *= (x[i] - x[j]);
        }
     }
     p1 += (a/b)*y[i];
    }
    return p1;
  }
  static double Lagrang(TMatrix& xy, double point)  {
    return Lagrange(xy[0], xy[1]);
  }

    // calculates eigen values of symmetric matrix
  static void  EigenValues(TMatrix& A, TMatrix& E)  {
    size_t i, j;
    MatType a = 2;
    while( olx_abs(a) > 1e-15 )  {
      MatMaxX(A, i, j);
      multMatrix(A, E, i, j);
      a = MatMaxX(A, i, j);
    }
  }

      // used in the Jacoby eigenvalues search procedure
  protected: static void multMatrix(TMatrix<MatType>& D, TMatrix<MatType>& E, size_t i, size_t j)  {
    MatType cf, sf, cdf, sdf;
    if( D[i][i] == D[j][j] )  {
      cdf = 0;
      cf  = (MatType)sqrt( MSVCC(2.0) )/2;
      sf  = (MatType)olx_sign(D[i][j])*cf;
      sdf = (MatType)olx_sign(D[i][j]);
    }
    else  {
      const MatType tdf = 2*D[i][j]/(D[j][j] - D[i][i]);
      const MatType r = (MatType)(tdf*tdf);
      cdf = (MatType)sqrt( MSVCC(1.0/(1+r)) );
      cf  = (MatType)sqrt( (1+cdf)/2.0);
      sdf = (MatType)(sqrt( MSVCC(r/(1+r)))*olx_sign(tdf));
      sf  = (MatType)(sqrt((1-cdf)/2.0)*olx_sign(tdf));
    }
    const MatType ij = D[i][j], ii = D[i][i], jj = D[j][j];
    D[i][j] = D[j][i] = 0;
    D[i][i] = (MatType)(ii*cf*cf + jj*sf*sf - ij*sdf);
    D[j][j] = (MatType)(ii*sf*sf + jj*cf*cf + ij*sdf);

    for( size_t a = 0; a < D.Vectors(); a++ )  {
      const MatType ij = E[i][a], ji = E[j][a];
      E[i][a] = ij*cf - ji*sf;
      E[j][a] = ij*sf + ji*cf;
      if((a != i) && (a != j ) )  {
        const double ia = D[i][a], ja = D[j][a];
        D[i][a] = D[a][i] = ia*cf - ja*sf;
        D[j][a] = D[a][j] = ia*sf + ja*cf;
      }
    }
  }
};
//------------------------------------------------------------------------------

  typedef TMatrix<float> ematf;
  typedef TMatrix<double> ematd;

  typedef TTypeList<ematf> ematf_list;
  typedef TTypeList<ematd> ematd_list;
  typedef TPtrList<ematf> ematf_plist;
  typedef TPtrList<ematd> ematd_plist;

EndEsdlNamespace()
#endif

