/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_ellipsoid_H
#define __olx_xl_ellipsoid_H
#include "xbase.h"
#include "evector.h"
#include "threex3.h"
#include "tptrlist.h"

BeginXlibNamespace()

/* Ellipsoid always must be in the cartesian frame */

class TEllipsoid: public ACollectionItem  {
  bool FNPD;  // not positive defined
  double FQuad[6], FEsd[6];  // quadratic form of the elipsoid and esd
  mat3d Matrix;  // normalised eigen vectors
  double SX, SY, SZ;    // lengths of the vectores
  /* do not change this value ! it is equal to the position in the AsymmUnit
  list
  */
  size_t Id;
public:
  TEllipsoid();
  TEllipsoid( const TEllipsoid& e)  {  this->operator = (e);  }
  template <class T> TEllipsoid(const T& Q)  {  Initialise(Q);  }
  template <class T> TEllipsoid(const T& Q, const T& E)  { Initialise(Q, E);  }
  virtual ~TEllipsoid()  {}
  void operator = (const TEllipsoid &E);
  // processes a symmetry matrix and updates object's data
  void MultMatrix(const mat3d& M);
  // return true if the ellipsoid is not positively defined
  bool IsNPD() const {  return FNPD;  }

  template <class T> TEllipsoid& Initialise(const T& Q, const T& E)  {
    for( size_t i=0; i < 6; i++ )  {
      FQuad[i] = Q[i];
      FEsd[i]  = E[i];
    }
    Initialise();
    return *this;
  }
  template <class T> TEllipsoid& Initialise(const T& Q)  {
    for( size_t i=0; i < 6; i++ )  {
      FQuad[i] = Q[i];
      FEsd[i]  = 0;
    }
    Initialise();
    return *this;
  }
  // calculates eigen values and vectors; FQuad must be initialised
  void Initialise();  // 
  
  template <class T> void GetQuad(T& Q) const {
    for( size_t i=0; i < 6; i++ )
      Q[i] = FQuad[i];
  }
  template <class T> void GetQuad(T& Q, T& E) const {
    for( size_t i=0; i < 6; i++ )  {
      Q[i] = FQuad[i];
      E[i] = FEsd[i];
    }
  }
  void SetEsd(size_t i, double v)  {  FEsd[i] = v;  } 
  const double& GetEsd(size_t i) const {  return FEsd[i];  } 
  void SetValue(size_t i, double v)  {  FQuad[i] = v;  } 
  const double& GetValue(size_t i) const {  return FQuad[i];  } 
  void ToShelx()  {
    olx_swap(FEsd[3], FEsd[5]);
    olx_swap(FQuad[3], FQuad[5]);
  }
  double GetSX() const {  return SX;  }
  double GetSY() const {  return SY;  }
  double GetSZ() const {  return SZ;  }
  double GetUeq() const {  return (FQuad[0]+FQuad[1]+FQuad[2])/3;  }
  const mat3d& GetMatrix() const {  return Matrix;  }
  // resets the ellipsoid into a sphere of given radius
  void ToSpherical(double r);
  DefPropP(size_t, Id)
};
  typedef TTypeList<TEllipsoid>  TEllpList;
  typedef TPtrList<TEllipsoid>  TEllpPList;
EndXlibNamespace()
#endif

