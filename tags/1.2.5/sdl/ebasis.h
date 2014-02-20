/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_basis_H
#define __olx_sdl_basis_H
#include "ebase.h"
#include "threex3.h"
#include "dataitem.h"

#ifdef __GNUC__
  #undef QLength
#endif

BeginEsdlNamespace()

class TEBasis: public IEObject  {
protected:
  static float FMData[16], FMDataT[16];  // matrix to call OpenGlDirectly
  double   FRX, FRY, FRZ, // the rotaion angles
           FZoom;       // zoom
  mat3d  FMatrix;
  vec3d   FCenter;
public:
  TEBasis();
  TEBasis(const TEBasis& B);
  virtual ~TEBasis();

  const TEBasis& operator = (const TEBasis &B);
  template <class VC> void SetCenter(const VC& V)  {
    FCenter[0] = V[0];  FCenter[1] = V[1];  FCenter[2] = V[2];
  }
  const vec3d& GetCenter() const {  return FCenter; }
  void NullCenter()  {  FCenter.Null();  }

  // orientation matrix in 4x4 opengl format
  inline const float* GetMData() const {
    FMData[0] = (float)FMatrix[0][0];  FMData[1] = (float)FMatrix[0][1];  FMData[2] = (float)FMatrix[0][2];
    FMData[4] = (float)FMatrix[1][0];  FMData[5] = (float)FMatrix[1][1];  FMData[6] = (float)FMatrix[1][2];
    FMData[8] = (float)FMatrix[2][0];  FMData[9] = (float)FMatrix[2][1];  FMData[10] = (float)FMatrix[2][2];
    FMData[3] = FMData[7] = FMData[11] = 0;
    FMData[12] = (float)FCenter[0];  FMData[13] = (float)FCenter[1];  FMData[14] = (float)FCenter[2];
    FMData[15] = 1;
    return FMData;
  }
  // transposed orientation matrix in 4x4 opengl format
  inline const float* GetMDataT() const {
    FMDataT[0] = (float)FMatrix[0][0];  FMDataT[1] = (float)FMatrix[1][0];  FMDataT[2] = (float)FMatrix[2][0];
    FMDataT[4] = (float)FMatrix[0][1];  FMDataT[5] = (float)FMatrix[1][1];  FMDataT[6] = (float)FMatrix[2][1];
    FMDataT[8] = (float)FMatrix[0][2];  FMDataT[9] = (float)FMatrix[1][2];  FMDataT[10] = (float)FMatrix[2][2];
    FMDataT[3] = FMDataT[7] = FMDataT[11] = 0;
    FMDataT[12] = (float)FCenter[0];  FMDataT[13] = (float)FCenter[1];  FMDataT[14] = (float)FCenter[2];
    FMDataT[15] = 1;
    return FMDataT;
  }
  inline const mat3d& GetMatrix() const {  return FMatrix;  }
  inline double GetZoom() const {  return FZoom; }
  void SetZoom(double v);

  template <class MC> void SetMatrix(const MC& M)  {
    for( int i=0; i < 3; i++ )
      for( int j=0; j < 3; j++ )
        FMatrix[i][j] = M[i][j];
  }

//  void  Rotate( double A, double B, double C);
  void  RotateX(double A);
  inline double GetRX() const {  return FRX; }
  void  RotateY(double A);
  inline double GetRY() const {  return FRY; }
  void  RotateZ(double A);
  inline double GetRZ() const {  return FRZ; }
  // rotation using a matrix
  template <class T> void Rotate(const T& M)  {  FMatrix *= M;  }

  // rotation around an arbitrary vector, New = Current*m
  template <class VC> void Rotate(const VC& V, double angle)  {
    mat3d m;  
    olx_create_rotation_matrix(m, V, cos(angle), sin(angle) );
    FMatrix *= m;
  }

  // rotation around an arbitrary vector New = m*Current
  template <class VC> void RotateT(const VC& V, double angle)  {
    mat3d m;  
    olx_create_rotation_matrix(m, V, cos(angle), sin(angle));
    FMatrix = m * FMatrix;
  }

  template <class VC> void Translate(const VC& V)  {
    FCenter[0] += V[0];  FCenter[1] += V[1];  FCenter[2] += V[2];
  }

  void TranslateX(double x);
  void TranslateY(double y);
  void TranslateZ(double z);
  void Reset();
  void ResetAngles()  {  FRX = FRY = FRZ = 0;  }
  template <class VC> void OrientNormal(const VC& normal)  {
    SetMatrix(CalcBasis<VC, mat3d>(normal).Transpose());  
  }
  // give a normal calculates two other vectors to form orthogonal basis...
  template <typename VecType, typename MatType> static MatType CalcBasis(const VecType& normal)  {
    MatType m;
    m[2] = normal;
    m[0][0] = m[2][1];
    m[0][1] = -m[2][0];
    if( m[0].QLength() < 1e-10 )  {
      m[0][0] = 0;
      m[0][1] = m[2][2];
      m[0][2] = -m[2][1];
      if( m[0].QLength() < 1e-10 )  {
        m[0][0] = m[2][2];
        m[0][1] = 0;
        m[0][2] = -m[2][0];
        if( m[0].QLength() < 1e-10 )
          throw TFunctionFailedException(__OlxSourceInfo, "cannot evaluate basis");
      }
    }
    m[1][0] = m[2][1]*m[0][2] - m[2][2]*m[0][1];
    m[1][1] = m[2][2]*m[0][0] - m[2][0]*m[0][2];
    m[1][2] = m[2][0]*m[0][1] - m[2][1]*m[0][0];
    m[0].Normalise();
    m[1].Normalise();
    m[2].Normalise();
    return m;
  }

  /* the matrix is being transposed by default to emulate normal rotation in OpneGl
   if it is not transposed, the rotation happens around current X, Y & Z axis
   however the rotation around screen X, Y, Z is prefered
  */
  template <class VC> void Orient(const VC& X, const VC& Y, const VC& Z, bool Transpose=true)  {
    if( Transpose )  {
      FMatrix[0][0] = X[0];  FMatrix[0][1] = Y[0];  FMatrix[0][2] = Z[0];
      FMatrix[1][0] = X[1];  FMatrix[1][1] = Y[1];  FMatrix[1][2] = Z[1];
      FMatrix[2][0] = X[2];  FMatrix[2][1] = Y[2];  FMatrix[2][2] = Z[2];
    }
    else  {
      FMatrix[0][0] = X[0];  FMatrix[0][1] = X[1];  FMatrix[0][2] = X[2];
      FMatrix[1][0] = Y[0];  FMatrix[1][1] = Y[1];  FMatrix[1][2] = Y[2];
      FMatrix[2][0] = Z[0];  FMatrix[2][1] = Z[1];  FMatrix[2][2] = Z[2];
    }
  }

  template <class MC> void Orient(const MC& M, bool Transpose=true)  {
    const double zm = GetZoom();
    Reset();
    if( Transpose )  {
      for( int i=0; i < 3; i++ )
        for( int j=0; j < 3; j++ )
          FMatrix[i][j] = M[i][j];
    }
    else  {
      for( int i=0; i < 3; i++ )
        for( int j=0; j < 3; j++ )
          FMatrix[j][i] = M[i][j];
    }
    SetZoom(zm);
  }

  double DistanceTo (const TEBasis &B) const {  return FCenter.DistanceTo(B.FCenter);  }

  virtual void ToDataItem(TDataItem& Item) const;
  virtual bool FromDataItem(const TDataItem& Item);
};

EndEsdlNamespace()
#endif
