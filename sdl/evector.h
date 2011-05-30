#ifndef __olx_evector_H
#define __olx_evector_H
#include <math.h>
#include "ebase.h"
#include "typelist.h"
#include "tptrlist.h"
#include "emath.h"
#include "istream.h"
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

  TVector(const VecType& a, const VecType& b, const VecType& c)  {
    Fn = 3;  FData = new VecType[Fn];
    FData[0] = a;  FData[1] = b;  FData[2] = c;
  }
  TVector(const VecType& a, const VecType& b, const VecType& c, const VecType& d,
          const VecType& e, const VecType& f)
  {
    Fn = 6;  FData = new VecType[Fn];
    FData[0] = a;  FData[1] = b;  FData[2] = c;
    FData[3] = d;  FData[4] = e;  FData[5] = f;
  }

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
    if( Fn == 0 )
      FData = NULL;
    else
      FData = new VecType[Fn];
    Null();
  }

  virtual ~TVector()  {
    if( FData != NULL )
      delete [] FData;
  }

  inline size_t Count() const {  return Fn;  }
  inline size_t Size() const {  return Fn;  }
  inline bool IsEmpty() const {  return Count() == 0;  }
  inline const VecType& operator [](size_t offset) const {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, offset, 0, Fn);
#endif
    return FData[offset];
  }
  inline VecType& operator [](size_t offset)  {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, offset, 0, Fn);
#endif
    return FData[offset];
  }

  inline const VecType& operator ()(size_t offset) const {  return operator [] (offset);  }
  inline VecType& operator ()(size_t offset)  {  return operator [] (offset);  }
  
  const VecType* GetRawData() const {  return FData;  }

  VecType& GetLast() const {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, Fn-1, 0, Fn);
#endif
    return FData[Fn-1];
  }

  template <typename vec_t>
  static vec_t& Null(vec_t& v, size_t sz)  {  
    for( size_t i=0; i < sz; i++ )  v[i] = 0;
    return v;
  }
  template <typename vec_t> static vec_t& Null(vec_t& v)  {
    return Null(v, v.Count());
  }
  TVector& Null()  {  
    Null(FData, Fn);
    return *this;
  }

  int Compare(const TVector& v) const {
    const size_t l = olx_min(this->Count(), v.Count());
    for( size_t i=0; i < l; i++ )  {
      if( FData[i] < v[i] )  return -1;
      if( FData[i] > v[i] )  return 1;
    }
    return olx_cmp(Count(), v.Count());
  }


  template <typename vec_t>
  static VecType Length(const vec_t& v, size_t cnt)  {  return QLength(v, cnt);  }
  template <typename vec_t>
  static VecType Length(const vec_t& v)  {  return QLength(v);  }
  VecType Length() const {  return (VecType)sqrt((double)QLength());  }

  template <typename vec_t> VecType QLength(const vec_t& v, size_t cnt)  {
    if( cnt == 0 )  throw TInvalidArgumentException(__OlxSourceInfo, "size");
    VecType l = 0;
    for( size_t i=0; i < cnt; i++ )  l += olx_sqr(v[i]);
    return l;
  }
  template <typename vec_t> VecType QLength(const vec_t& v)  {  return QLength(v, v.Count());  }
  VecType QLength() const {  return QLength(*this);  }

  template <typename vec_t>
  static vec_t& Normalise(vec_t& v, size_t cnt)  {
    const VecType l = Length(v, cnt);
    if( l == 0 )  throw TDivException(__OlxSourceInfo);
    for( size_t i=0; i < cnt; i++ )  v[i] /= l;
    return v;
  }
  template <typename vec_t>
  static vec_t& Normalise(vec_t& v)  {  return Normalise(v, v.Count());  }
  TVector& Normalise()  {  return Normalise(*this);  }

  template <typename vec1_t, typename vec2_t>
  static VecType CAngle(const vec1_t& a, const vec2_t& b, size_t cnt)  {
    if( cnt == 0 )  throw TInvalidArgumentException(__OlxSourceInfo, "size");
    const VecType l = sqrt(QLength(a, cnt)*QLength(b, cnt));
    if( l == 0 )  throw TDivException(__OlxSourceInfo);
    VecType v = 0;
    for( size_t i=0; i < cnt; i++ )  v += a[i]*b[i];
    return v/l;
  }
  template <typename vec1_t, typename vec2_t>
  static VecType CAngle(const vec1_t& a, const vec2_t& b)  {
    const size_t  cnt = a.Count();
    if( cnt != b.Count() )  
      throw TFunctionFailedException(__OlxSourceInfo, "vector size differs");
    return CAngle(a, b, cnt);
  }
  template <typename AT> VecType CAngle(const AT& V) const {  return CAngle(*this, V);  }
  
  template <typename vec1_t, typename vec2_t>
  static VecType QDistance(const vec1_t& a, const vec2_t& b, size_t cnt)  {
    if( cnt == 0 )  throw TInvalidArgumentException(__OlxSourceInfo, "size");
    VecType v = 0;
    for( size_t i=0; i < cnt; i++ )  v += olx_sqr(a[i]-b[i]);
    return v;
  }
  template <typename vec1_t, typename vec2_t>
  static VecType QDistance(const vec1_t& a, const vec2_t& b)  {
    const size_t  cnt = a.Count();
    if( cnt != b.Count() )  
      throw TFunctionFailedException(__OlxSourceInfo, "vector size differs");
    return QDistance(a, b, cnt);
  }
  template <typename AT> VecType QDistanceTo(const AT& v) const {  return QDistance(*this, v);  }

  template <typename vec1_t, typename vec2_t>
  static VecType DotProd(const vec1_t& a, const vec2_t& b, size_t cnt)  {
    if( cnt == 0 )  throw TInvalidArgumentException(__OlxSourceInfo, "size");
    VecType v = 0;
    for( size_t i=0; i < cnt; i++ )  v += a[i]*b[i];
    return v;
  }
  template <typename vec1_t, typename vec2_t>
  static VecType DotProd(const vec1_t& a, const vec2_t& b)  {
    const size_t  cnt = a.Count();
    if( cnt != b.Count() )  
      throw TFunctionFailedException(__OlxSourceInfo, "vector size differs");
    return DotProd(a, b, cnt);
  }
  template <typename AT> VecType DotProd(const AT& v) const {  return DotProd(*this, v);  }

  template <typename vec1_t, typename vec2_t>
  VecType Distance(const vec1_t& a, const vec2_t& b, size_t cnt) const {
    return sqrt(QDistance(a, b, cnt));
  }
  template <typename vec1_t, typename vec2_t>
  static VecType Distance(const vec1_t& a, const vec2_t& b)  {
    const size_t  cnt = a.Count();
    if( cnt != b.Count() )  
      throw TFunctionFailedException(__OlxSourceInfo, "vector size differs");
    return sqrt(QDistanceTo(a, b, cnt));
  }
  template <typename AT> VecType DistanceTo(const AT& v) const {
    return sqrt(QDistance(*this, v));
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
      printf("%05.4e\t", FData[i]);
  }

  template <typename SC> SC StrRepr() const {
    if( Fn == 0 )  return SC();
    SC rv;
    rv << FData[0];
    for( size_t i=1; i < Fn; i++ )
      rv << ", " << FData[i];
    return rv;
  }
  inline TIString ToString() const {  return StrRepr<olxstr>();  }
  inline olxcstr  ToCStr() const {  return StrRepr<olxcstr>();  }
  inline olxwstr  ToWStr() const {  return StrRepr<olxwstr>();  }
  void ToStream(IOutputStream& out) const {
    uint32_t sz = (uint32_t)Fn; //TODO: check overflow
    out.Write(&sz, sizeof(sz));
    out.Write(FData, sizeof(VecType)*Fn);
  }
  void FromStream(IInputStream& in)  {
    uint32_t sz;
    in >> sz;
    Resize(sz);
    in.Read(FData, sizeof(VecType)*sz);
  }
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
  static VecType ArrayMin(const VecType* a, size_t& n, size_t sz)  {
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
