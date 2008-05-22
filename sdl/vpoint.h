//---------------------------------------------------------------------------

#ifndef vpointH
#define vpointH
#include "evector.h"
//#include "tevpoint.h"

#undef QLength
BeginEsdlNamespace()

// forward reference
template <typename> class TEVPoint;

//------------------------------------------------------------------------------
// TVPoint is basic class for three the dimensional case
// explicit definition of the template is in "matrix.h"
template <class EType> class TVPoint: public TVector<EType>
{
private:
public:
  TVPoint(EType _x, EType _y, EType _z) : TVector<EType>(3)  {
    TVector<EType>::FData[0] = _x;
    TVector<EType>::FData[1] = _y;
    TVector<EType>::FData[2] = _z;
  }

  TVPoint() : TVector<EType>(3) {  }

  template <class T> TVPoint( const TEVPoint<T>& v ) : TVector<EType>(3)  {
    TVector<EType>::FData[0] = v[0].GetV();
    TVector<EType>::FData[1] = v[1].GetV();
    TVector<EType>::FData[2] = v[2].GetV();
  }

   /* function allows to compare slightli different values */
  template <class T> bool Eq(const TVPoint<T>& V, EType ESD)  {
    double p = DistanceTo(V);
    double L = Length()*V.Length();
    if( L != 0 )
      return ( p/L < ESD ) ?  true : false;
    else
      return ( p < ESD ) ? true : false;
   }

  inline EType Length() const  {
    return sqrt( TVector<EType>::FData[0]*TVector<EType>::FData[0] +
                 TVector<EType>::FData[1]*TVector<EType>::FData[1] +
                 TVector<EType>::FData[2]*TVector<EType>::FData[2]);
  }

  inline EType QLength() const  {
    return TVector<EType>::FData[0]*TVector<EType>::FData[0] +
           TVector<EType>::FData[1]*TVector<EType>::FData[1] +
           TVector<EType>::FData[2]*TVector<EType>::FData[2];
  }

  template <class AType> const TEVPoint<AType>& operator = (const TEVPoint<AType>& v)  {
    TVector<EType>::FData[0] = v[0].GetV();
    TVector<EType>::FData[1] = v[1].GetV();
    TVector<EType>::FData[2] = v[2].GetV();
    return v;
  }

  template <class AType> const TVector<AType>& operator = (const TVector<AType>& v)  {
    TVector<EType>::FData[0] = (EType)v[0];
    TVector<EType>::FData[1] = (EType)v[1];
    TVector<EType>::FData[2] = (EType)v[2];
    return v;
  }

  template <class AType> const TVPoint<AType>& operator = (const TVPoint<AType>& v)  {
    TVector<EType>::FData[0] = v[0];
    TVector<EType>::FData[1] = v[1];
    TVector<EType>::FData[2] = v[2];
    return v;
  }

  const TVPoint& operator = (const TVPoint& v)  {
    TVector<EType>::FData[0] = v[0];
    TVector<EType>::FData[1] = v[1];
    TVector<EType>::FData[2] = v[2];
    return v;
  }
  /*
  template <class AType> TVPoint<EType>& operator += (const TEVPoint<AType> &v)  {
    TVector<EType>::FData[0] += v[0].GetV();
    TVector<EType>::FData[1] += v[1].GetV();
    TVector<EType>::FData[2] += v[2].GetV();
    return *this;
  }

  template <class AType> TVPoint<EType>& operator -= (const TEVPoint<AType> &v)  {
    TVector<EType>::FData[0] -= v[0].GetV();
    TVector<EType>::FData[1] -= v[1].GetV();
    TVector<EType>::FData[2] -= v[2].GetV();
    return *this;
  }
  */

  template <class AType> EType DistanceTo(const TVPoint<AType> &v) const  {
    return  sqrt((v[0]-TVector<EType>::FData[0])*(v[0]-TVector<EType>::FData[0]) +
            (v[1]-TVector<EType>::FData[1])*(v[1]-TVector<EType>::FData[1]) +
            (v[2]-TVector<EType>::FData[2])*(v[2]-TVector<EType>::FData[2]) );
  }

  /* returns distance^2 - can be used in an optimisation */
  template <class AType> EType QDistanceTo(const TVPoint<AType> &v) const  {
    return  (v[0]-TVector<EType>::FData[0])*(v[0]-TVector<EType>::FData[0]) +
            (v[1]-TVector<EType>::FData[1])*(v[1]-TVector<EType>::FData[1]) +
            (v[2]-TVector<EType>::FData[2])*(v[2]-TVector<EType>::FData[2]);
  }

  template <class AType> TVPoint<EType> XProdVec (const TVPoint<AType> &v) const  {
    return TVPoint<EType>( TVector<EType>::FData[1]*v[2] - TVector<EType>::FData[2]*v[1],
                  TVector<EType>::FData[2]*v[0] - TVector<EType>::FData[0]*v[2],
                  TVector<EType>::FData[0]*v[1] - TVector<EType>::FData[1]*v[0] );
  }

  template <class AType> EType XProdVal (const TVPoint<AType> &v) const  {
    AType cang = this->CAngle(v);
    AType sang = sqrt(1-cang*cang);
    return Length()*v.Length()*sang;
  }

};

  typedef TVPoint<float>  TVPointF;
  typedef TVPoint<double>  TVPointD;

  typedef TTypeList<TVPointF>  TVPointFList;
  typedef TTypeList<TVPointD>  TVPointDList;
  typedef TPtrList<TVPointF>  TVPointFPList;
  typedef TPtrList<TVPointD>  TVPointDPList;

EndEsdlNamespace()
#endif
