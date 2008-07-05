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
#include "asymmunit.h"

//----------------------------------------------------------------------------//
// TEllipsoid function bodies
//----------------------------------------------------------------------------//
TEllipsoid::TEllipsoid():Matrix(3,3)  {
  Matrix.E();
  SX = SY = SZ = 0;
  for( int i=0; i < 6; i++ )    FQuad[i] = 0;
  FNPD = false;
}
//..............................................................................
TEllipsoid::TEllipsoid(const TVectorD &Q):Matrix(3,3)  {
  Matrix.E();
  Initialise(Q);
}
//..............................................................................
TEllipsoid::~TEllipsoid(){  return;  }
//..............................................................................
void TEllipsoid::GetQuad(TVectorD &V) const  {
  V.Resize(6);
  for( int i=0; i < 6; i++ )  V[i] = FQuad[i];
}
//..............................................................................
void TEllipsoid::Initialise(const TVectorD &Q)  {
  for( int i=0; i < 6; i++ )
    FQuad[i] = Q[i];
  Initialise();
}
//..............................................................................
void TEllipsoid::Initialise()  {
  TMatrixD M(3,3), E(3,3);
  // expand quadratic form
  M[0][0] = FQuad[0];  M[1][1] = FQuad[1];  M[2][2] = FQuad[2];
  M[1][2] = M[2][1] = FQuad[3];
  M[0][2] = M[2][0] = FQuad[4];
  M[0][1] = M[1][0] = FQuad[5];
  E.E();
  // cala eigen values and vectors
  TMatrixD::EigenValues(M, E);
  if( (M[0][0] <= 0) || (M[1][1] <= 0) || (M[2][2] <= 0) )
    FNPD = true;
  else  {
    // calculate axis lengths
    SX = (float)sqrt(M[0][0]);  // correspondes 50% ellipsoides
    SY = (float)sqrt(M[1][1]);
    SZ = (float)sqrt(M[2][2]);
    FNPD = false;
  }
  Matrix = E;
  Matrix[0].Normalise();
  Matrix[1].Normalise();
  Matrix[2].Normalise();
}
//..............................................................................
void TEllipsoid::operator = (const TEllipsoid&E )  {
  Matrix = E.GetMatrix();
  SX = E.GetSX();
  SY = E.GetSY();
  SZ = E.GetSZ();
  //SetId( E.GetId() );
  for( int i=0; i < 6; i++ )  FQuad[i] = E.FQuad[i];
  FNPD = E.FNPD;
}
//..............................................................................
/* to get the quadratic for MatrixT*Matr(SX^2,SY^2,SZ^2)*Matrix*/
void TEllipsoid::MultMatrix(const TMatrixD &Matr)  {
  if( FNPD )  return;
  TMatrixD N(3,3), M(Matr);
  // do trasformation of the eigen vecytors
  // get a new quadractic form
  N[0][0] = FQuad[0];  N[1][1] = FQuad[1];  N[2][2] = FQuad[2];
  N[1][2] = N[2][1] = FQuad[3];
  N[0][2] = N[2][0] = FQuad[4];
  N[0][1] = N[1][0] = FQuad[5];
  M.Transpose();
  N = Matr*N*M;
  // store new quadratic form
  FQuad[0] = N[0][0];  FQuad[1] = N[1][1];  FQuad[2] = N[2][2];
  FQuad[3] = N[1][2];  FQuad[4] = N[0][2];  FQuad[5] = N[0][1];
  // get eigne values/vectors
  M.E();  // identity matrix
  TMatrixD::EigenValues(N, M);
  // assign new eigen values

  if( (N[0][0] <= 0) || (N[1][1] <= 0) || (N[2][2] <= 0) )
    FNPD = true;
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
olxstr TEllipsoid::UcifToUcart( TAsymmUnit& au )  {
  TMatrixD N(3,3), M(3,3), A(3,3), At(3,3);

  A = au.GetCellToCartesian();
  At = A;
  At.Transpose();

  N[0][0] = 1./au.Axes()[0].GetV();
  N[1][1] = 1./au.Axes()[1].GetV();
  N[2][2] = 1./au.Axes()[2].GetV();

  M[0][0] = FQuad[0];  M[1][1] = FQuad[1];  M[2][2] = FQuad[2];
  M[1][2] = M[2][1] = FQuad[3];
  M[0][2] = M[2][0] = FQuad[4];
  M[0][1] = M[1][0] = FQuad[5];

  M = N*M*N;
  M = A*M*At;

  olxstr t(M[0].ToString());
  t << '\n' << M[1].ToString() << '\n' << M[2].ToString();
  return t;
}
//..............................................................................
olxstr TEllipsoid::UcartToUcif( TAsymmUnit& au )  {
  TMatrixD N(3,3), M(3,3), A(3,3), At(3,3);

  A = au.GetCartesianToCell();
  At = A;
  At.Transpose();

  N[0][0] = au.Axes()[0].GetV();
  N[1][1] = au.Axes()[1].GetV();
  N[2][2] = au.Axes()[2].GetV();

  M[0][0] = FQuad[0];  M[1][1] = FQuad[1];  M[2][2] = FQuad[2];
  M[1][2] = M[2][1] = FQuad[3];
  M[0][2] = M[2][0] = FQuad[4];
  M[0][1] = M[1][0] = FQuad[5];

  M = A*M*At;
  M = N*M*N;
  olxstr t(M[0].ToString());
  t << '\n' << M[1].ToString() << '\n' << M[2].ToString();
  return t;
}
//..............................................................................

