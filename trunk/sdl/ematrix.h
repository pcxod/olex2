//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//

#ifndef tmatrixH
#define tmatrixH

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
  int Fn, Fm;
  void  Clear()  {
    if( !FData )  return;
    delete [] FData;
    FData = NULL;  }

  TVector <MatType>* FData;
public:
  TMatrix()  {
    FData = NULL;
    Fn = Fm = 0;
    SetTag(-1);
  }

  TMatrix( int VectorsNumber, int ElementsNumber)  {
    Fn = VectorsNumber;
    Fm = ElementsNumber;
    if( (!Fn) || (!Fm) )   {  FData = NULL;  return;  }
    FData = new TVector<MatType>[Fn];
    for( int i=0; i < Fn; i++ )  FData[i].Resize(Fm);
    SetTag(-1);
  }

  template <class AType> TMatrix( const TMatrix<AType> &M )  {
    FData = NULL;
    Fn = Fm = 0;
    *this = M;
  }

  TMatrix( const TMatrix &M )  {
    FData = NULL;
    Fn = Fm = 0;
    *this = M;
  }

  virtual ~TMatrix()  {  Clear();  }

  template <class MC> void Assign(const MC& M, int width, int height)  {
    Resize(width, height);
    for( int i=0; i < width; i++ )
      for(int j=0; j < height; j++ )
        FData[j][i] = M[j][i];
  }
  //  void GetData(VType *Data[]) const;
  // the function is used to obtain a copy of the matrix in an array
  // be sure that the dimensions of the array are greater or equal to
  // the dimensions of the matrix !

  inline int Vectors() const  {  return Fn;  }
  inline int Elements() const {  return Fm;  } 

#ifndef _OLX_DEBUG
  inline TVector<MatType>& operator []( int index ) const  {    return FData[index];  }
  inline TVector<MatType>& Data(int index) const  {    return FData[index];  }
#else // index checking is enabled
  TVector<MatType>& operator []( int index ) const  {
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, index, 0, Fn);
    return FData[index];
  }

  TVector<MatType>& Data(int index) const {
    TIndexOutOfRangeException::ValidateRange(__OlxSourceInfo, index, 0, Fn);
    return FData[index];
  }

#endif
  /* the function multiplies a matrix by a column vector. Only the number of vector
   elements is taken from the matrix - no error will be generated if the matrix dimensions
   (number of elements) are larger than the vector dimentions!!!
  */
  template <class AType> TVector<MatType> operator  * ( const TVector<AType>& C) const  {
    if( (Fm < C.Count()) || (Fn == 0) )
      throw TFunctionFailedException(__OlxSourceInfo, "incompatible matrix dimensions");
    TVector<MatType> R(Fn);
    for( int i = 0; i < Fn; i++ )
      for( int j =0; j < C.Count(); j++ )
        R[i] += C[j]*FData[i][j];
    return R;
  }

  template <class AType> TMatrix operator  * ( const TMatrix<AType>& C) const  {
    int X = Vectors();
    int Y = Elements();
    int Z = C.Elements();
    TMatrix<MatType> Result(X, Z);
    for( int i = 0; i < X; i ++ )
      for( int j = 0; j < Z; j ++ )
        for( int k = 0; k < Y; k ++ )
          Result[i][j] += FData[i][k]*C[k][j];
    return Result;
  }

  template <class AType> TMatrix operator  + ( const TMatrix<AType>& c) const  {
    TMatrix<MatType> Result;
    Result = *this;
    Result += c;
    return Result;
  }

  template <class AType> TMatrix operator  - ( const TMatrix<AType>& c) const {
    TMatrix<MatType> Result;
    Result = *this;
    Result -= c;
    return Result;
  }

  template <class AType> TMatrix& operator   *=( const TMatrix<AType>& C)  {
    int X = Vectors();
    int Y = Elements();
    int Z = C.Elements();
    TMatrix<MatType> Result(X, Z);
    for( int i = 0; i < X; i ++ )
      for( int j = 0; j < Z; j ++ )
        for( int k = 0; k < Y; k ++ )
          Result[i][j] += FData[i][k]*C[k][j];
    *this = Result;
    return *this;
  }

  template <class AType> TMatrix& operator   +=( const TMatrix<AType>& c)  {
    if( (Fm ==c.Elements()) && (Fn == c.Vectors()) )
      for( int i=0; i < Fn; i ++ )  FData[i] += c[i];
    else
      throw TFunctionFailedException(__OlxSourceInfo, "incompatible matrix dimensions");
    return *this;
  }

  template <class AType> TMatrix& operator   -=( const TMatrix<AType>& c)  {
    if( (Fm ==c.Elements()) && (Fn == c.Vectors()) )
      for( int i=0; i < Fn; i ++ )  FData[i] -= c[i];
    else
      throw TFunctionFailedException(__OlxSourceInfo, "incompatible matrix dimensions");
    return *this;
  }


  TMatrix operator   + ( MatType V)  {
    TMatrix<MatType> R = *this;  R += V;
    return R;
  }

  TMatrix operator   - ( MatType V)  {
    TMatrix<MatType> R = *this;  R -= V;
    return R;
    }

  TMatrix operator   * ( MatType V)  {
    TMatrix<MatType> R = *this;  R *= V;
    return R;
  }

  TMatrix operator   / ( MatType V)  {
    TMatrix<MatType> R = *this;  R /= V;
    return R;
  }

  TMatrix& operator   += ( MatType V)  {
    for( int i = 0; i < Fn; i ++ )  FData[i] += V;
    return *this;
  }

  TMatrix& operator   -= ( MatType V)  {
    for( int i = 0; i < Fn; i ++ )  FData[i] -= V;
    return *this;
  }

  TMatrix& operator   *= ( MatType V)  {
    for( int i = 0; i < Fn; i ++ )  FData[i] *= V;
    return *this;
  }
    
  TMatrix& operator   /= ( MatType V)  {
    for( int i = 0; i < Fn; i ++ )  FData[i] /= V;
    return *this;
  }

  template <class AType> bool  operator == (const TMatrix<AType>& C) const  {
    if( (Fm == C.Elements()) && (Fn == C.Vectors() ) )  {
      for( int i = 0; i < Fn; i ++ )
        if( !(FData[i] == C[i]) )  return false;
      return true;
    }
    return false;
  }

  template <class AType> const TMatrix& operator  = (const TMatrix<AType>& C)  {
    this->Resize( C.Vectors(), C.Elements() );
    for( int i = 0; i < Fn; i++)
      for( int j = 0; j < Fm; j++)
        FData[i][j] = C[i][j];
    Tag( C.Tag() );
    return C;
  }

  const TMatrix& operator  = (const TMatrix& C)  {
    this->Resize( C.Vectors(), C.Elements() );
    for( int i = 0; i < Fn; i++)
      for( int j = 0; j < Fm; j++)
        FData[i][j] = C[i][j];
    SetTag( C.GetTag() );
    return C;
  }

  void  E() {
    for(int i=0; i < Fn; i++)
      for(int j=0; j < Fm; j++)
        if( i == j )  FData[i][j]=(MatType)1.0;
        else      FData[i][j]=0;
  }

  void Null()  {
    for(int i=0; i < Fn; i++)  FData[i].Null();
  }

  bool IsE() const  {
    for(int i=0; i < Fn; i++)
      for(int j=0; j < Fm; j++)
        if( i == j ){  if( FData[i][j]!= 1.0 ) return false; }
        else        {  if( FData[i][j] != 0 )  return false; }
    return true;
  }

  //TODO: clever stuff can be done to keep original data!
  void Resize( int n, int m )  {
    if( Fn == n && Fm == m )  return;
    Clear();
    Fn = n;  Fm = m;
    FData = new TVector<MatType>[n];
    if( m )  for( int i=0; i < n; i++ )  FData[i].Resize(m);
  }

  void SwapRows(int s, int to )  {
    for( int i=0; i < Fm; i++ )  {
      MatType a = FData[s][i];
      FData[s][i] = FData[to][i];
      FData[to][i] = a;
    }
  }

  void SwapCols(int s,int to )  {
    for( int i = 0; i < Fm; i++)  {
      MatType a = FData[i][to];
      FData[i][to] = FData[i][s];
      FData[i][s] = a;
    }
  }

  inline void AddRows(int to,int which )  {  FData[to] += FData[which];  }
  void AddCols(int to, int which )    {  for( int i=0; i < Fn; i ++ )  FData[i][to] += FData[i][which];  }

  inline void SubRows(int from,int which )  {  FData[from] -= FData[which];  }
  void SubCols(int from, int which )  {  for( int i=0; i < Fn; i ++ )  FData[i][from] -= FData[i][which];  }
  inline void MulRow( int which, MatType v)  {  FData[which] *= v;  }

  void DivRow( int which, MatType v)  {  FData[which] /= v;  }

  void MulCol( int which, MatType v)  {    for( int i=0; i < Fn; i ++ )  FData[i][which] *= v;  }

  void DivCol( int which, MatType v)  {
    if( v == 0 )  throw TDivException(*this, "DivCol");
    for( int i=0; i < Fn; i ++ )    FData[i][which] /= v;
  }

  void Transpose()  {
    MatType P;
    if( Fn != Fm )
      throw TFunctionFailedException(__OlxSourceInfo, "incompatible matrix dimensions");
    for( int i=0; i < Fm; i++ )  {
      for( int j=i+1; j < Fn; j++ )  {  // need to go around a half of matrix
        if( i == j )  continue;
        P = FData[j][i];
        FData[j][i] = FData[i][j];
        FData[i][j] = P;
      }
    }
  }

  MatType Trace() const  {
    if( Fn != Fm )
      throw TFunctionFailedException(__OlxSourceInfo, "incompatible matrix dimensions");
    MatType a = 0;
    for( int i = 0; i < Fn; i++ )  a += FData[i][i];
    return a;
  }

  void Print() const  {
    printf("\n");
    for( int i = 0; i < Fn; i ++ )  {
      FData[i].Print();
      printf("\n");   }
    printf("\n");
  }


  bool LoadFromFile(const olxstr &FN)  {
    int dim;
    TStrList S, Toks;
    S.LoadFromFile( FN );
    if( S.Count() == 0 )  return false;

    for( int i=0; i < S.Count(); i++ )
      S[i].Replace('\t', ' ');
    for( int i=0; i < S.Count(); i++ )  {
      Toks.Strtok(S[i], ' ');
      if( i == 0 )  {
        dim = Toks.Count();
        if( !dim )  return false;
        this->Resize(S.Count(), dim);
      }
      if( Toks.Count() != dim )  
        throw TFunctionFailedException(__OlxSourceInfo, "file rows not even");
        
      for( int j=0; j < dim; j++ )  FData[i][j] = (MatType)Toks[j].ToDouble();
    }
    return true;
  }

  void SaveToFile(const olxstr &FN)  {
    TCStrList S;
    olxstr T(EmptyString, Fm*10);
    for( int i=0; i < Fn; i++ )  {
      T = EmptyString;
      for( int j=0; j < Fm; j++ )
        T <<olxstr::FormatFloat(5, FData[i][j], true) << '\t';
      S.Add(T);
    }
    S.SaveToFile( FN );
  }

//------------------------------------------------------------------------------
  // static methods
  // searches for maximum element in matrix
  static MatType  MatMax(TMatrix& A,   int &i, int &j )  {
    MatType c = (MatType)fabs(  MSVCC(A[0][0]) );
    for( int a = 0; a < A.Vectors(); a ++ )  {
      for( int b = 0; b < A.Elements(); b ++ )  {
        if( !a && !b )  continue;
        if( fabs( MSVCC(A[a][b]) ) > c )  {
          i = a;
          j = b;
          c = (MatType)fabs( MSVCC(A[a][b]) );
        }
      }
    }
    return c;
  }


  // searches for maximum element in matrix dialgonal matrix
  // without consideration of the main diagonal
  static MatType  MatMaxX(TMatrix& A,   int &i, int &j )  {
    MatType c = (MatType)fabs( MSVCC(A[0][1]) );
    i = 0;
    j = 1;
    if( A.Elements() == 2 )  {
      i = 0;
      j = 1;
      return c;
    }
    for( int a = 0; a < A.Elements(); a ++ )  {
      for( int b = a+1; b < A.Elements(); b ++ )  {
        if( fabs( MSVCC(A[a][b]) ) > c ) {
          i = a;
          j = b;
          c = (MatType)fabs( MSVCC(A[a][b]) );
        }
      }
    }
    return c;
  }


  // searches for maximum element in a matrix row
  static MatType  RowMax(TMatrix& A,   int i, int &j )  {
    MatType c = (MatType)fabs( MSVCC(A[i][0]) );
    j=0;
    for( int a = 1; a < A.Elements(); a ++ )  {
      if( fabs( MSVCC(A[i][a]) ) > c )  {
        j = a;
        c = (MatType)fabs( MSVCC(A[i][a]) );
      }
    }
    return c;
  }

  // searches for minimum element in a matrix row
  static MatType  RowMin(TMatrix& A,   int row, int &j )  {
    MatType a = A[row][0];
    j = 0;
    for(int i=1; i < A.Elements(); i++ )
      if( A[row][i] < a )  {
        a = A[row][i];
        j = i;
      }
    return A[row][j];
  }


  // searches for maximum element in a matrix column
  static MatType  ColMax(TMatrix & A, int i, int &j )  {
    MatType c = (MatType)fabs( MSVCC(A[0][i]) );
    j=0;
    for( int a = 1; a < A.Vectors(); a ++ )  {
      if( fabs( MSVCC(A[a][i]) ) > c )  {
        j = a;
        c = (MatType)fabs( MSVCC(A[a][i]) );
      }
    }
   return c;
  }

  // solves a set of equations by the Gauss method {equation arr.b = c }
  static void GauseSolve(TMatrix<MatType>& arr, TVector<MatType>& b, TVector<MatType>& c) {
    int sz = arr.Elements();
    MatrixElementsSort(arr, b );
    for ( int j = 1; j < sz; j++ )
      for( int i = j; i < sz; i++ )  {
        if( arr[i][j-1] ==0 )  continue;
        b[i]  *= -(arr[j-1][j-1]/arr[i][j-1]);
        arr.MulRow(i, -(arr[j-1][j-1]/arr[i][j-1]));
        arr.AddRows( i, j-1);
        b[i]+=b[j-1];
      }
    if( arr[sz-1][sz-1]==0)
      throw TFunctionFailedException(__OlxSourceInfo, "dependent set of equations");

    c[sz-1] = b[sz-1]/arr[sz-1][sz-1];
    for(int j = sz-2; j >=0; j--)  {
      for(int k1=1; k1 < sz-j+1; k1++)  {
        if( k1 == (sz-j) )
          for( int i=sz-1; i > sz-k1; i-- )  b[j]-=arr[j][i]*c[i];
      }
      c[j]= b[j]/arr[j][j];
     }
   }

      // used in GauseSolve to sort the matrix
  static void MatrixElementsSort(TMatrix<MatType> &arr, TVector<MatType> &b)  {
    MatType *bf, c;
    int n, dim = arr.Elements();
    bf = new MatType[dim];
    for( int i = 0; i < dim; i++ )  {
      for(int j = 0; j < dim; j++ )  {
        bf[j] = (MatType)fabs( MSVCC(arr[j][i]) );
      }
      TVector<MatType>::ArrayMax( bf, n, dim );
      if( n != i )  {
        arr.SwapRows(i,n);
        // changing b[i] and b[n]
        c = b[i];     b[i] = b[n];     b[n] = c;
      }
    }
    delete [] bf;
  }

  /* A polynomial least square analysis of XY pairs strired in matrix[2][n]
   extent is the polynom extent (1-is for line)
   params will contain the fitting parameters
   the return value is the RMS - root mean square of the fit
  */
  static MatType PLSQ(TMatrix<MatType> &xy, TVector<MatType> &param, int extent)  {
    extent ++;
    if( xy.Elements() < extent )
      throw TInvalidArgumentException(__OlxSourceInfo, "not enough data");
    TMatrix<MatType> s(extent, extent);
    TVector<MatType> r(extent);
    param.Resize(extent);

    for(int i = 0; i < extent; i++)  {
      for(int j = i; j < extent; j++)  {
        MatType b = 0;
        for(int k = 0; k < xy.Elements(); k++)  {
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
      for(int j=0; j < xy.Elements(); j++)  {
        if( xy[0][j] == 0 )  continue;
        a += (MatType)(xy[1][j]*pow( MSVCC(xy[0][j]), MSVCC(i) ) );
      }
      r[i] = a;
    }
    // fill the bottom off diagonal part
    for(int i = 0; i < extent; i++)
      for(int j = i+1; j < extent; j++)
        s[j][i] = s[i][j];

    GauseSolve(s,r,param);
    double rms = 0;
    for(int i=0; i < xy.Elements(); i++ )
      rms += QRT( xy[1][i]-TVector<MatType>::PolynomValue(param, xy[0][i]) );
    return (rms > 0) ? sqrt(rms)/xy.Elements() : 0;
  }

  // Lagrange interpolation
  static double  Lagrang(TVector<MatType> &x, const TVector<MatType> &y, double point )  {
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
  static inline double Lagrang(TMatrix &xy, double point )  {
    return Lagrange(xy[0], xy[1]);
  }

    // calculates eigen values of symmetric matrix
  static void  EigenValues(TMatrix& A, TMatrix &E)  {
    int i, j;
    MatType a = 2;
    while( fabs( MSVCC(a) ) > 1e-15 )  {
      MatMaxX( A, i, j );
      multMatrix( A, E, i, j );
      a = MatMaxX(A, i, j );
    }
  }

      // used in the Jacoby eigenvalues serahc procedure
  protected: static void multMatrix( TMatrix<MatType> & D, TMatrix<MatType> & E, int i, int j )  {
    MatType cf, sf, cdf, sdf;
    if( D[i][i] == D[j][j] )  {
      cdf = 0;
      cf  = (MatType)sqrt( MSVCC(2.0) )/2;
      sf  = (MatType)Sign(D[i][j])*cf;
      sdf = (MatType)Sign(D[i][j]);
    }
    else  {
      MatType tdf = 2*D[i][j]/(D[j][j] - D[i][i]);
      MatType r = (MatType)QRT(tdf);

      cdf = (MatType)sqrt( MSVCC(1.0/(1+r)) );
      cf  = (MatType)sqrt( (1+cdf)/2.0);
      sdf = (MatType)(sqrt( MSVCC(r/(1+r)) ) * Sign(tdf));
      sf  = (MatType)(sqrt((1-cdf)/2.0)*Sign(tdf));
    }
    MatType ji, jj,ij,ii,ja,ia;
    ij = D[i][j];
    ii = D[i][i];
    jj = D[j][j];
    D[i][j] = D[j][i] = 0;
    D[i][i] = (MatType)(ii*QRT(cf) + jj*QRT(sf) - ij*sdf);
    D[j][j] = (MatType)(ii*QRT(sf) + jj*QRT(cf) + ij*sdf);

    for( int a = 0; a < D.Vectors(); a++ )  {
      ij = E[i][a];
      ji = E[j][a];

      E[i][a] = ij*cf - ji*sf; //i
      E[j][a] = ij*sf + ji*cf;   //j
    }
    for( int a = 0; a < D.Vectors(); a++ )  {
      if((a != i) && (a != j ) )  {
        ia = D[i][a];
        ja = D[j][a];
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

