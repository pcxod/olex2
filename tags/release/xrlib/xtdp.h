#ifndef __olx_tdp
#define __olx_tdp
#include "xcell.h"

BeginXlibNamespace()
struct XUani {
  double quadratic[6], esd[6];
  bool npd, initialised;
protected:
  inline void CalcEigenProperties()  {
    mat3d q(quadratic[0], quadratic[5], quadratic[4], 
            quadratic[5], quadratic[1], quadratic[3], 
            quadratic[4], quadratic[3], quadratic[2]);
    eigenVectors.I();
    mat3d::EigenValues(q, eigenVectors);
    if( q[0][0] <= 0 || q[1][1] <= 0 || q[2][2] <= 0 )
      npd = true;
    else  {  // calculate axis lengths
      eigenValues[0] = sqrt(q[0][0]);  // correspondes 50% ellipsoides
      eigenValues[0] = sqrt(q[1][1]);
      eigenValues[0] = sqrt(q[2][2]);
      npd = false;
    }
    initialised = true;
  }
public:
  vec3d eigenValues;
  mat3d eigenVectors;
  bool refinable;
  enum Uind { U11=0, U22, U33, U23, U13, U12 };
  XUani() : refinable(true), npd(false), initialised(false) {
    memset(&quadratic[0], 0, sizeof(double)*6);
    memset(&esd[0], 0, sizeof(double)*6);
  }
  template <class T> inline void SetQuadratic(const T& q)  {
    quadratic[0] = q[0];  quadratic[1] = q[1];  quadratic[2] = q[2];  
    quadratic[3] = q[3];  quadratic[4] = q[4];  quadratic[5] = q[5];  
    CalcEigenProperties();
  }
  template <class T> void SetEsd(const T& q)  {
    esd[0] = q[0];  esd[1] = q[1];  esd[2] = q[2];  
    esd[3] = q[3];  esd[4] = q[4];  esd[5] = q[5];  
    CalcEigenProperties();
  }
  inline XUani& operator *= (const mat3d& m)  {
    if( !IsValid() )  return *this;
    mat3d q(quadratic[0], quadratic[5], quadratic[4], 
            quadratic[5], quadratic[1], quadratic[3], 
            quadratic[4], quadratic[3], quadratic[2]);
    mat3d n( m*q*mat3d::Transpose(m) );  // get a new quadractic form
    // store new quadratic form
    quadratic[0] = n[0][0];  quadratic[1] = n[1][1];  quadratic[2] = n[2][2];
    quadratic[3] = n[1][2];  quadratic[4] = n[0][2];  quadratic[5] = n[0][1];
    CalcEigenProperties();
    return *this;
  }
  inline double GetUisoVal() const {  return (eigenValues[0]+eigenValues[1]+eigenValues[2])/3;  }
  inline bool IsValid() const {  return (initialised && !npd);  }
  inline XUani& UcifToUcart(const XCell& cell)  {
    cell.UcifToUcart(quadratic);
    return *this;
  }
  inline XUani& UcartToUcif(const XCell& cell)  {
    cell.UcartToUcif(quadratic);
    return *this;
  }
  template <class T> inline void UcifToUcart(const XCell& cell, T& q) const {
    q[0] = quadratic[0];  q[1] = quadratic[1];  q[2] = quadratic[2];  
    q[3] = quadratic[3];  q[4] = quadratic[4];  q[5] = quadratic[5];  
    cell.UcifToUcart(q);
    return *this;
  }
  template <class T>inline void UcartToUcif(const XCell& cell, T& q) const {
    q[0] = quadratic[0];  q[1] = quadratic[1];  q[2] = quadratic[2];  
    q[3] = quadratic[3];  q[4] = quadratic[4];  q[5] = quadratic[5];  
    cell.UcartToUcif(q);
  }
  template <class T>inline void GetQuadractic(T& q) const {
    q[0] = quadratic[0];  q[1] = quadratic[1];  q[2] = quadratic[2];  
    q[3] = quadratic[3];  q[4] = quadratic[4];  q[5] = quadratic[5];  
  }
  inline const double* GetQuadratic() const {  return quadratic;  }
};

EndXlibNamespace()

#endif
