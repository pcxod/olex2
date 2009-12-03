//----------------------------------------------------------------------------//
// namespace TXClasses: crystallographic core
// TEBond
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
//---------------------------------------------------------------------------
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "ellipsoid.h"

//----------------------------------------------------------------------------//
// TEllipsoid function bodies
//----------------------------------------------------------------------------//
TEllipsoid::TEllipsoid()  {
  Matrix.I();
  SX = SY = SZ = 0;
  for( int i=0; i < 6; i++ )  {    
    FEsd[i] = FQuad[i] = 0;
  }
  FNPD = false;
}
//..............................................................................
void TEllipsoid::Initialise()  {
  mat3d M(FQuad[0], FQuad[5], FQuad[4], FQuad[1], FQuad[3], FQuad[2]), 
        I;
  // calc eigen values and vectors
  mat3d::EigenValues(M, I.I());
  if( (M[0][0] <= 0) || (M[1][1] <= 0) || (M[2][2] <= 0) )  {
    SX = SY = SZ = 0;
    FNPD = true;
  }
  else  {
    // calculate axis lengths
    SX = sqrt(M[0][0]);  // correspondes 50% ellipsoides
    SY = sqrt(M[1][1]);
    SZ = sqrt(M[2][2]);
    FNPD = false;
  }
  Matrix = I;
  Matrix[0].Normalise();
  Matrix[1].Normalise();
  Matrix[2].Normalise();
}
//..............................................................................
void TEllipsoid::operator = (const TEllipsoid&E)  {
  Matrix = E.GetMatrix();
  SX = E.GetSX();
  SY = E.GetSY();
  SZ = E.GetSZ();
  for( int i=0; i < 6; i++ )  {
    FQuad[i] = E.FQuad[i];
    FEsd[i] = E.FEsd[i];
  }
  FNPD = E.FNPD;
}
//..............................................................................
/* to get the quadratic for MatrixT*Matr(SX^2,SY^2,SZ^2)*Matrix*/
void TEllipsoid::MultMatrix(const mat3d& Matr)  {
  if( FNPD )  return;
  mat3d N(FQuad[0], FQuad[5], FQuad[4], FQuad[1], FQuad[3], FQuad[2]), 
        M(mat3d::Transpose(Matr)), 
        E(FEsd[0], FEsd[5], FEsd[4], FEsd[1], FEsd[3], FEsd[2]);
  // do trasformation of the eigen vectors
  N = Matr*N*M;
  E = Matr*E*M;
  // store new quadratic form
  FQuad[0] = N[0][0];  FQuad[1] = N[1][1];  FQuad[2] = N[2][2];
  FQuad[3] = N[1][2];  FQuad[4] = N[0][2];  FQuad[5] = N[0][1];
  // strore new Esd's
  FEsd[0]  = E[0][0];  FEsd[1]  = E[1][1];  FEsd[2]  = E[2][2];
  FEsd[3]  = E[1][2];  FEsd[4]  = E[0][2];  FEsd[5]  = E[0][1];
  // get eigen values/vectors
  mat3d::EigenValues(N, M.I());
  // assign new eigen values

  if( (N[0][0] <= 0) || (N[1][1] <= 0) || (N[2][2] <= 0) )  {
    SX = SY = SZ = 0;
    FNPD = true;
  }
  else  {
    FNPD = false;
    SX = sqrt(N[0][0]);  SY = sqrt(N[1][1]);  SZ = sqrt(N[2][2]);
  }
  // assign new eigen vectors
  Matrix = M;
  //Matrix.Transpose(); // see initialise for details
  Matrix[0].Normalise();
  Matrix[1].Normalise();
  Matrix[2].Normalise();
}
//..............................................................................

