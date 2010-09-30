//---------------------------------------------------------------------------//
// Implementstion of Vector, Matrix, TVPoint (3D vector),
// TEValue (value with an error), TEVPoint (3D Vector with errors)
// TEBasis - A matrix assosiated with Rotate functions
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//

#ifndef evpointH
#define evpointH
#include <math.h>
#include "ebase.h"
#include "datafile.h"
#include "evalue.h"
#include "ematrix.h"

BeginEsdlNamespace()

template <typename> class TEVPoint;
//---------------------------------------------------------------------------
template <class EType> class TEVPoint: public IEObject  {
private:
  TEValue<EType> FValues[3];
public:
  TEVPoint(EType _x, EType _y, EType _z, EType ex, EType ey, EType ez)  {
    FValues[0].V() = _x;  FValues[1].V() = _y;  FValues[2].V() = _z;
    FValues[0].E() = ex;  FValues[1].E() = ey;  FValues[2].E() = ez;
  }

  TEVPoint()  {  }

  TEValue<EType> DistanceTo(const TEVPoint &p)  const {
    TEValue<EType> V;
    V = (FValues[0]-p.FValues[0])*(FValues[0]-p.FValues[0]) +
        (FValues[1]-p.FValues[1])*(FValues[1]-p.FValues[1]) +
        (FValues[2]-p.FValues[2])*(FValues[2]-p.FValues[2]);
    return V.SelfSqrt();
  }

  TEValue<EType> CAngle(const TEVPoint &p)  const {
    TEValue<EType> V, L = Length(), L1 = p.Length();
    L *= L1;
    if( L.V == 0 || L.E == 0 )
      throw TDivException(__OlxSourceInfo);
    V  = FValues[0]*p.FValues[0] + FValues[1]*p.FValues[1] + FValues[2]*p.FValues[2];
    return (V /= L);
  }

  TEVPoint&  Null()  {
    FValues[0].V() = 0;  FValues[1].V() = 0;  FValues[2].V() = 0;
    FValues[0].E() = 0;  FValues[1].E() = 0;  FValues[2].E() = 0;
    return *this;
  }

  TEValue<EType> Length() const  {
    TEValue<EType> V;
    V = FValues[0]*FValues[0] + FValues[1]*FValues[1] + FValues[1]*FValues[1];
    return V.SelfSqrt();
  }

  TEVPoint&  Normalise()  {
    double L = sqrt( olx_sqr(FValues[0].V()) + olx_sqr(FValues[1].V()) + olx_sqr(FValues[2].V()));
    if( L == 0 )  throw TDivException(__OlxSourceInfo);
    *this /= (EType)L;
    return *this;
  }

  inline TEValue<EType> const& operator [](int offset) const {
    return FValues[offset];
  }
  inline TEValue<EType>& operator [](int offset) {
    return FValues[offset];
  }
  // convinience function for any vector, array types
  template <class VC> TEVPoint& Assign(const VC& vec)  {
    FValues[0].V() = vec[0];  FValues[1].V() = vec[1];  FValues[2].V() = vec[2];
    return *this;
  }

  template <class AType>
    const TEVPoint<AType>& operator =(const TEVPoint<AType> &S)  {
      FValues[0] = S[0];
      FValues[1] = S[1];
      FValues[2] = S[2];
      return S;
    }

  const TEVPoint& operator = (const TEVPoint& S)  {
    FValues[0] = S[0];  FValues[1] = S[1];  FValues[2] = S[2];
    return S;
  }

  TEVPoint operator  + (EType a ) const  {
    TEVPoint<EType> V;
    V = *this;
    return V += a;
  }

  TEVPoint operator  - (EType a ) const  {
    TEVPoint<EType> V;
    V = *this;
    return V -= a;
  }

  TEVPoint operator  * (EType a ) const  {
    TEVPoint<EType> V;
    V = *this;
    return V *= a;
  }

  TEVPoint operator  / (EType a ) const  {
    TEVPoint<EType> V;
    V = *this;
    return V /= a;
  }

  TEVPoint& operator  += (EType a )  {
    FValues[0] += a;
    FValues[1] += a;
    FValues[2] += a;
    return *this;
  }

  TEVPoint& operator  -= (EType a )  {
    FValues[0] -= a;
    FValues[1] -= a;
    FValues[2] -= a;
    return *this;
  }

  TEVPoint& operator  *= (EType a )  {
    FValues[0] *= a;
    FValues[1] *= a;
    FValues[2] *= a;
    return *this;
  }

  TEVPoint& operator  /= (EType a )  {
    if( a == 0 )
      throw TDivException(__OlxSourceInfo);
    FValues[0] /= a;
    FValues[1] /= a;
    FValues[2] /= a;
    return *this;
  }



  template <class AType>
    TEVPoint  operator  + (const TEVPoint<AType>& a) const  {
      TEVPoint<EType> V;
      V = *this;
      return (V += a);
    }

  template <class AType>
    TEVPoint  operator  - (const TEVPoint<AType>& a) const  {
      TEVPoint<EType> V;
      V = *this;
      return (V -= a);
    }

  template <class AType>
    TEVPoint  operator  * (const TEVPoint<AType>& a) const  {
      TEVPoint<EType> V;
      V = *this;
      return (V *= a);
    }

  template <class AType>
    TEVPoint  operator  / (const TEVPoint<AType>& a) const  {
      TEVPoint<EType> V;
      V = *this;
      return (V /= a);
    }


  template <class AType>
    TEVPoint&  operator  += (const TEVPoint<AType>& a)  {
      FValues[0].V() += a[0].GetV();  FValues[1].V() += a[1].GetV();  FValues[2].V() += a[2].GetV();
      FValues[0].E() += a[0].GetE();  FValues[1].E() += a[1].GetE();  FValues[2].E() += a[2].GetE();
      return *this;
    }

  template <class AType>
    TEVPoint& operator  -= (const TEVPoint<AType>& a)  {
      FValues[0].V() -= a[0].GetV();  FValues[1].V() -= a[1].GetV();  FValues[2].V() -= a[2].GetV();
      FValues[0].E() -= a[0].GetE();  FValues[1].E() -= a[1].GetE();  FValues[2].E() -= a[2].GetE();
      return *this;
    }

  template <class AType>
    TEVPoint& operator  *= (const TEVPoint<AType>& a)  {
      FValues[0].V() *= a[0].GetV();  FValues[1].V() *= a[1].GetV();  FValues[2].V() *= a[2].GetV();
      FValues[0].E() *= a[0].GetE();  FValues[1].E() *= a[1].GetE();  FValues[2].E() *= a[2].GetE();
      return *this;
    }

  template <class AType>
    TEVPoint& operator  /= (const TEVPoint<AType>& a)  {
      FValues[0].V() /= a[0].GetV();  FValues[1].V() /= a[1].GetV();  FValues[2].V() /= a[2].GetV();
      FValues[0].E() /= a[0].GetE();  FValues[1].E() /= a[1].GetE();  FValues[2].E() /= a[2].GetE();
      return *this;
    }

  template <class AType>
    TEVPoint& operator *= ( const TMatrix<AType>& m )  {
      TEValue<EType> X0 = FValues[0], Y0 = FValues[1], Z0 = FValues[2];
      FValues[0] = X0*m[0][0] + Y0*m[0][1] + Z0*m[0][2];
      FValues[1] = X0*m[1][0] + Y0*m[1][1] + Z0*m[1][2];
      FValues[2] = X0*m[2][0] + Y0*m[2][1] + Z0*m[2][2];
      return *this;
    }
};

  typedef TEVPoint<float>  TEVPointF;
  typedef TEVPoint<double>  TEVPointD;

EndEsdlNamespace()
#endif

