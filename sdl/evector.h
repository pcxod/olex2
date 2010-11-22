#ifndef __olx_evector_H
#define __olx_evector_H
#include <math.h>
#include "ebase.h"
#include "typelist.h"
#include "tptrlist.h"
#include "emath.h"
#ifdef QLength
  #undef QLength
#endif

BeginEsdlNamespace()

// using forward reference
template <typename> class TMatrix;

template <typename VecType> class TVector {
protected:
  size_t Fn;
  VecType *FData;
public:
  TVector()  {  Fn = 0;  FData = NULL;  }

  template <typename AType> TVector(const TVector<AType>& V)  {
    Fn = V.Count();
    FData = new VecType[Fn];
    for( size_t i=0; i < Fn; i++ )
      FData[i] = V[i];
  }

  template <typename AType> TVector(size_t size, const AType& V)  {
    Fn = size;
    FData = new VecType[Fn];
    for( size_t i=0; i < Fn; i++ )
      FData[i] = V[i];
  }

  TVector(const TVector& V)  {
    Fn = V.Count();
    FData = new VecType[Fn];
    for( size_t i=0; i < Fn; i++ )
      FData[i] = V[i];
  }

  TVector(size_t ddim)  {
    Fn = ddim;
    FData = new VecType[Fn];
    Null();
  }

  virtual ~TVector()  {
    if( FData != NULL )
      delete [] FData;
  }

  inline size_t Count() const {  return Fn;  }
  inline size_t Size() const {  return Fn;  }

  TVector& Null()  {  
    for( size_t i=0; i < Fn; i++ )
      FData[i] = 0;
    return *this;
  }

  VecType Length() const {  return (VecType)sqrt((double)QLength());  }

  VecType QLength() const {
    if( Fn == 0 )
      throw TFunctionFailedException(__OlxSourceInfo, "empty vector");
    VecType l = 0;
    for( size_t i=0; i < Fn; i++ )
      l += FData[i]*FData[i];
    return l;
  }

  int Compare(const TVector& v) const {
    const size_t l = olx_min(this->Count(), v.Count() );
    for( size_t i=0; i < l; i++ )  {
      if( FData[i] < v[i] )  return -1;
      if( FData[i] > v[i] )  return 1;
    }
    return olx_cmp(Count(), v.Count());
  }

  TVector& Normalise() {
    const VecType l = Length();
    if( l == 0 )  throw TDivException(__OlxSourceInfo);
    for( size_t i=0; i < Fn; i++ )
      FData[i] /= l;
    return *this;
  }

  template <typename AT> VecType CAngle(const AT& V) const {
    if( Fn != V.Count() )  
      throw TFunctionFailedException(__OlxSourceInfo, "vectors of different size");
    double l = Length(), v=0;
    l *= V.Length();
    if( l == 0 )  throw TDivException(__OlxSourceInfo);
    for( size_t i=0; i < Fn; i++ )
      v += FData[i]*V[i];
    return (VecType)(v/l);
  }

  template <typename AT> VecType QDistanceTo(const AT& V) const {
    if( Fn != V.Count() )  
      throw TFunctionFailedException(__OlxSourceInfo, "vectors of different size");
    VecType v = 0;
    for( size_t i=0; i < Fn; i++ )
      v += olx_sqr(FData[i]-V[i]);
    return v;
  }

  template <typename AT> VecType DotProd(const AT& V) const {
    if( Fn != V.Count() )  
      throw TFunctionFailedException(__OlxSourceInfo, "vectors of different size");
    VecType v = 0;
    for( size_t i=0; i < Fn; i++ )
      v += FData[i]*V[i];
    return v;
  }

  template <typename AT> VecType DistanceTo(const AT& V) const {
    return (VecType)sqrt((double)QDistanceTo(V));  // cast needed for the case of int
  }

  template <typename AType> TVector& operator = (const AType& a)  {
    Resize(a.Count());
    for( size_t i=0; i < Fn; i++ )
      FData[i] = a[i];
    return *this;
  }

  TVector& operator = (const TVector& a)  {
    Resize( a.Count() );
    for( size_t i=0; i < Fn; i++ )
      FData[i] = a[i];
    return *this;
  }

  template <typename VC> TVector& Assign(const VC& v, size_t size)  {
    Resize(size);
    for( size_t i=0; i < size; i++ )
      FData[i] = v[i];
    return *this;
  }

  inline VecType& operator [](size_t offset) const {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, offset, 0, Fn);
#endif
    return FData[offset];
  }

  VecType& Data(size_t offset) const {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, offset, 0, Fn);
#endif
    return FData[offset];
  }

  const VecType* GetRawData() const {  return FData;  }

  VecType& GetLast() const {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, Fn-1, 0, Fn);
#endif
    return FData[Fn-1];
  }

  TVector operator + (VecType a) const {
    return TVector(*this) += a;
  }

  TVector operator - (VecType a) const {
    return TVector(*this) -= a;
  }

  TVector operator * (VecType a) const {
    return TVector(*this) *= a;
  }

  TVector operator / (VecType a) const {
    return TVector<VecType>(*this) /= a;
  }

  TVector& operator += (VecType v)  {
    for( size_t i=0; i < Fn; i++ )
      FData[i] += v;
    return *this;
  }

  TVector& operator -= (VecType v)  {
    for( size_t i=0; i < Fn; i++ )
      FData[i] -= v;
    return *this;
  }

  TVector& operator *= (VecType v)  {
    for( size_t i=0; i < Fn; i++ )
      FData[i] *= v;
    return *this;
  }

  TVector& operator /= (VecType v)  {
    if( v == 0 )  throw TDivException(__OlxSourceInfo);
    for( size_t i=0; i < Fn; i++ )    
      FData[i] /= v;
    return *this;
  }

  template <typename AType>
    TVector operator + (const TVector<AType>& a) const {  return TVector<VecType>(*this) += a;  }
  template <typename AType>
    TVector operator - (const TVector<AType>& a) const {  return TVector<VecType>(*this) -= a;  }
  template <typename AType>
    TVector operator * (const TVector<AType>& a) const {  return TVector<VecType>(*this) *= a;  }
  template <typename AType>
    TVector operator / (const TVector<AType>& a) const {  return TVector<VecType>(*this) /= a;  }

  template <typename AType> TVector& operator += (const TVector<AType>& a)  {
    for( size_t i=0; i < Fn; i++ )
      FData[i] += a[i];
    return *this;
  }

  template <typename AType> TVector& operator -= (const TVector<AType>& a)  {
    for( size_t i=0; i < Fn; i++ )
      FData[i] -= a[i];
    return *this;
  }

  template <typename AType> TVector& operator *= (const TVector<AType>& a)  {
    for( size_t i=0; i < Fn; i++ )
      FData[i] *=a[i];
    return *this;
  }

  template <typename AType> TVector& operator /= (const TVector<AType>& a)  {
    for( size_t i=0; i < Fn; i++ )
      FData[i] /= a[i];
    return *this;
  }

  /* beware - transposed form (col*matrix), use M.v for row*matrix multiplication
    if matrix has more elements (in vectors) than given vector - only
    number of vector elements is used
  */
  template <typename AType> TVector operator * (const TMatrix<AType>& a) const {
    if( a.Elements() < Fn || a.Vectors() == 0 )
      throw TInvalidArgumentException(__OlxSourceInfo, "dimension");
    TVector V(a.Vectors());
    for( size_t i=0; i < a.Vectors(); i++ )
      for( size_t j=0; j < Fn; j++ )
        V[i] += (VecType)(FData[j]*a[j][i]);
    return V;
  }

  /* beware - transposed (col*matrix) form, use M.v for row*matrix multiplication
    if matrix has more elements (in vectors) than given vector - only
    number of vector elements is used
  */
  template <typename AType> TVector& operator *= (const TMatrix<AType>& a)  {
    return (*this = (*this*a));
  }

  template <typename AType> bool operator == (const TVector<AType>& a) const {
    if( Fn != a.Count() )  return false;
    for( size_t i=0; i < Fn; i++ )
      if( FData[i] != a[i] )  
        return false;
    return true;
  }

  void Print() const {
    for( size_t i = 0; i < Fn; i ++ )
      printf("%05.4e\t", FData[i] );
  }

  template <typename SC> SC StrRepr() const {
    SC rv;
    for( size_t i=0; i < Fn; i++ )  {
      rv << FData[i];
      if( (i+1) < Fn )  rv << ',' << ' ';
    }
    return rv;
  }
  inline TIString ToString() const {  return StrRepr<olxstr>();  }
  inline olxcstr  ToCStr() const {  return StrRepr<olxcstr>();  }
  inline olxwstr  ToWStr() const {  return StrRepr<olxwstr>();  }

  TVector& Resize(size_t newsize)  {
    if( newsize <= Fn )
      Fn = newsize;
    else if( newsize == 0 )
      Fn = 0;  
    else  {
      if( FData != NULL )  {
        VecType* ND = new VecType[newsize];
        for( size_t i=0; i < Fn; i++ )
          ND[i] = FData[i];
        for( size_t i=Fn; i < newsize; i++ )
          ND[i] = 0;
        Fn = newsize;
        delete [] FData;
        FData = ND;
      }
      else  {
        Fn = newsize;
        FData = new VecType[Fn];
        Null();
      }
    }
    return *this;
  }

//------------------------------------------------------------------------------
  // searches maximum of an array
  static VecType ArrayMax(const VecType* a, size_t& n, size_t sz)  {
    VecType b;
    b = a[0];
    n = 0;
    for( size_t i = 1; i < sz; i++)
      if( a[i] > b )  {
        b = a[i];  n = i;  }
    return b;
  }
  // searches minimum of an array
  static VecType  ArrayMin(const VecType* a, size_t& n, size_t sz)  {
    VecType b;
    b = a[0];
    n = 0;
    for( size_t i = 1; i < sz; i++)
      if( a[i] < b )  {
        b = a[i];  n = i;
      }
    return b;
  }
  // can be used to evaluate polynom value
  static double PolynomValue(const TVector& coeff, double arg)  {
    if( coeff.Count() == 2 )
      return coeff[0] + coeff[1]*arg;
    double c = coeff[0];
    for( size_t i = 1; i < coeff.Count(); i++)
      c += coeff[i]*pow(arg,(double)i);
    return c;
  }
};
//------------------------------------------------------------------------------


  typedef TVector<float> evecf;
  typedef TVector<double> evecd;
  typedef TVector<int> eveci;
  typedef TVector<size_t> evecsz;

  typedef TTypeList<eveci> eveci_list;
  typedef TTypeList<evecf> evecf_list;
  typedef TTypeList<evecd> evecd_list;

  typedef TPtrList<eveci> eveci_plist;
  typedef TPtrList<evecf> evecf_plist;
  typedef TPtrList<evecd> evecd_plist;

EndEsdlNamespace()
#endif
