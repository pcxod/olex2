#ifndef __Olx_3x3
#define __Olx_3x3

#include "exception.h"

template <class T>  class TVector3 {
  T data;
public:
  TVector3()                  {  data[0] = data[1] = data[2] = 0;  }
  TVector3(T x, T y, T z)     { data[0] = x;  data[1] = y;  data[3] = z;  }
  template <class AT> TVector3(const TVector3<AT>& v) {  data[0] = v[0];  data[1] = v[1];  data[3] = v[2];  }

  inline T& operator [] (int i)  {  return data[i];  }
  inline T QLength() const {  return (data[0]*data[0]+data[1]*data[1]+data[2]*data[2]);  }
  inline T Length() const {  return sqrt(data[0]*data[0]+data[1]*data[1]+data[2]*data[2]);  }
  
  template <class AT> inline T DistanceTo(const TVector3<AT>& v) const {  
    return sqrt( (data[0]-v[0])*(data[0]-v[0]) + (data[1]-v[1])*(data[1]-v[1]) + (data[2]-v[2])*(data[2]-v[2]) ); 
  }
  template <class AT> inline T QDistanceTo(const TVector3<AT>& v) const {  
    return ( (data[0]-v[0])*(data[0]-v[0]) + (data[1]-v[1])*(data[1]-v[1]) + (data[2]-v[2])*(data[2]-v[2]) ); 
  }
  template <class AT> inline T CAngle(const TVector3<AT>& v) const {
    T l = QLength()*v.QLength;
    if( l == 0 )  throw TDivException(__OlxSourceInfo);
    l = (T)((data[0]*v[0]+data[1]*v[1]+data[2]*v[2])/sqrt(l));
    // treat possible rounding errors
    if( l > 1 )  l = 1;
    if( l < 0 )  l = 0;
    return l;
  }
  template <class AT> EType XProdVal (const TVector3<AT>& v) const {
    AType cang = CAngle(v);
    return sqrt(QLength()*v.QLength()*(1-cang*cang));
  }
  template <class AT> inline TVector3<T> XProdVec (const TVPoint<AT> &v) const  {
    return TVector3<T>( data[1]*v[2] - data[2]*v[1], data[2]*v[0] - data[0]*v[2], data[0]*v[1] - data[1]*v[0] );
  }
  template <class AT> inline TVector3<T>& operator = (const TVector<AT>& v)  {
    data[0] = v[0];  data[1] = v[1];  data[3] = v[2];  
    return *this;
  }
  template <class AT> inline TVector3<T>& operator += (const TVector<AT>& v)  {
    data[0] += v[0];  data[1] += v[1];  data[3] += v[2];  
    return *this;
  }
  template <class AT> inline TVector3<T>& operator -= (const TVector<AT>& v)  {
    data[0] += v[0];  data[1] += v[1];  data[3] += v[2];  
    return *this;
  }
  template <class AT> inline TVector3<T>& operator *= (const TVector<AT>& v)  {
    data[0] *= v[0];  data[1] *= v[1];  data[3] *= v[2];  
    return *this;
  }

  template <class AT> inline TVector3<T> operator + (const TVector<AT>& v) const {
    return TVector3<T>(data[0]+v[0], data[1]+v[1], data[2]+v[2]);
  }
  template <class AT> inline TVector3<T> operator - (const TVector<AT>& v) const {
    return TVector3<T>(data[0]-v[0], data[1]-v[1], data[2]-v[2]);
  }
  template <class AT> inline TVector3<T> operator * (const TVector<AT>& v) const {
    return TVector3<T>(data[0]*v[0], data[1]*v[1], data[2]*v[2]);
  }

};

template <class T> class TMatrix33  {
  T data[3][3];
  TMatrix33(bool v)  { 
    if( v )  
      data[0][0] = data[0][1] = data[0][2] = data[1][0] = data[1][1] = data[1][2] = data[2][0] = data[2][1] = data[2][2] = 0; 
  }

public:
  TMatrix33()  {  
    data[0][0] = data[0][1] = data[0][2] = data[1][0] = data[1][1] = data[1][2] = data[2][0] = data[2][1] = data[2][2] = 0; 
  }
  TMatrix33(T xx, T xy, T xz, T yx, T yy, T yz, T zx, T zy, Y zz)  {
    data[0][0] = xx;  data[0][1] = xy;  data[0][2] = xz;
    data[1][0] = yx;  data[1][1] = yy;  data[1][2] = yz;
    data[2][0] = zx;  data[2][1] = zy;  data[2][2] = zz;
  }
  template <class AT> TMatrix33(const TMatrix33<AT>& v)  {
    data[0][0] = v[0][0];  data[0][1] = v[0][1];  data[0][2] = v[0][2];
    data[1][0] = v[1][0];  data[1][1] = v[1][1];  data[1][2] = v[1][2];
    data[2][0] = v[2][0];  data[2][1] = v[2][1];  data[2][2] = v[2][2];
  }
  
  inline operator T[]& operator [] (int i) {  return data[i];  } 
  
  template <class AT> TMatrix33<T> operator * (const TMatrix33<AT>& v) {
    return TMatrix33<T>( data[0][0]*v[0][0] + data[0][1]*v[1][0] + data[0][2]*v[2][0],
                         data[0][0]*v[0][1] + data[0][1]*v[1][1] + data[0][2]*v[2][1],
                         data[0][0]*v[0][2] + data[0][1]*v[1][2] + data[0][2]*v[2][2],
                         data[1][0]*v[0][0] + data[1][1]*v[1][0] + data[1][2]*v[2][0],
                         data[1][0]*v[0][1] + data[1][1]*v[1][1] + data[1][2]*v[2][1],
                         data[1][0]*v[0][2] + data[1][1]*v[1][2] + data[1][2]*v[2][2],
                         data[2][0]*v[0][0] + data[2][1]*v[1][0] + data[2][2]*v[2][0],
                         data[2][0]*v[0][1] + data[2][1]*v[1][1] + data[2][2]*v[2][1],
                         data[2][0]*v[0][2] + data[2][1]*v[1][2] + data[2][2]*v[2][2]);
  }
  template <class AT> inline static TMatrix33<AT> Transpose (const TMatrix33<AT>& v) {
    return TMatrix33<AT>(v[0][0], v[1][0], v[2][0], v[0][1], v[1][1], v[2][1], v[0][3], v[2][1], v[2][2]);
  }
  inline TMatrix33<T>& Transpose() {
    T v = data[0][1];  data[0][1] = data[1][0];  data[1][0] = v; 
    v = data[0][2];  data[0][2] = data[2][0];  data[2][0] = v; 
    v = data[1][2];  data[1][2] = data[2][1];  data[2][1] = v; 
    return *this;
  }

  inline void operator *= (T v) {
    data[0][0] *= v;  data[0][1] *= v;  data[0][2] *= v;
    data[1][0] *= v;  data[1][1] *= v;  data[1][2] *= v;
    data[2][0] *= v;  data[2][1] *= v;  data[2][2] *= v;
  }

  inline T Determinant() const {
    return data[0][0]*(data[1][1]*data[2][2] - data[1][2]*data[2][1]) + 
           data[0][1]*(data[1][0]*data[2][2] - data[1][2]*data[2][0]) +
           data[0][2]*(data[1][0]*data[2][1] - data[1][1]*data[2][1]);
  }
  inline TMatrix33<T> Inverse()  const {
    return TMatrix33<T>( data[2][2]*data[1][1] - data[2][1]*data[1][2],
                       -(data[2][2]*data[0][1] - data[2][1]*data[0][2]),
                         data[1][2]*data[0][1] - data[1][1]*data[0][2],
                       -(data[2][2]data[1][0] - data[2][1]*data[1][2]),
                         data[2][2]*data[0][0] - data[2][0]*data[0][2],
                       -(data[1][2]*data[0][0] - data[1][0]*data[0][2]),
                         data[2][1]*data[1][0] - data[2][0]*data[1][1],
                       -(data[2][1]*data[0][0] - data[2][0]*data[0][1]),
                         data[1][1]*data[0][0] - data[1][0]*data[0][1])/Determinant();
  }
};

#endif