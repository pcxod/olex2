//---------------------------------------------------------------------------//
// namespace TXClasses: crystallographic core
// TSPlane implementation
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "splane.h"
#include "ematrix.h"
#include "satom.h"
#include "sbond.h"

//..............................................................................
TSPlane::TSPlane(TNetwork* Parent):TSObject(Parent)  {
  FDistance = 0;
  Deleted = false;
}
//..............................................................................
TSPlane::~TSPlane()  {  }
//..............................................................................
void TSPlane::AddCrd(const TVPointD& C)  {
  FCenter.Null();
  Crds.AddCCopy(C);
  for( int i=0; i < Crds.Count(); i++ )
    FCenter += Crds[i];
  FCenter /= Crds.Count();
}
//..............................................................................
double TSPlane::CalcRMS(const TSAtomPList& atoms)  {
  if( atoms.Count() < 3 )  return 0;
  TMatrixD m(3,3);
  TVectorD p(3), s(3), Z(3);

  for( int i=0; i < atoms.Count(); i++ )  {
    const TVPointD& P = atoms[i]->GetCenter();
    m[0][0] += (P[0]*P[0]);
    m[0][1] += (P[0]*P[1]);
    m[0][2] += (P[0]);
    p[0] += (P[0]*P[2]);

    m[1][1] += (P[1]*P[1]);
    m[1][2] += (P[1]);
    p[1] += (P[1]*P[2]);

    m[2][2] += 1;
    p[2] += (P[2]);
  } // equ: z = s[0]*x + s[1]*y + s[2]
  m[1][0] = m[0][1];

  m[2][0] = m[0][2];
  m[2][1] = m[1][2];
  TMatrixD::GauseSolve(m, p, s);

  Z[0] = s[0];
  Z[1] = s[1];
  Z[2] = -1;
  double d = s[2]/Z.Length(), rms=0;
  Z.Normalise();
  for( int i=0; i < atoms.Count(); i++ )  {
    const TVPointD& P = atoms[i]->GetCenter();
    rms += QRT(P[2]*Z[2] + (P[0]*Z[0] + P[1]*Z[1] + d));
  }
  if( rms <= 0 )  return 0;
  return sqrt(rms/atoms.Count());
}
//..............................................................................
void TSPlane::CalcPlane(const TTypeList< AnAssociation2<TVPointD, double> >& Points, TVectorD &Params)  {
  if( Points.Count() < 3 )  return;
  TMatrixD m(3,3);
  TVectorD p(3), s(3), P(3);

  for( int i=0; i < Points.Count(); i++ )  {
    P = Points[i].GetA();
    m[0][0] += (P[0]*P[0]*Points[i].GetB());
    m[0][1] += (P[0]*P[1]*Points[i].GetB());
    m[0][2] += (P[0]*Points[i].GetB());
    p[0] += (P[0]*P[2]*Points[i].GetB());

    m[1][1] += (P[1]*P[1]*Points[i].GetB());
    m[1][2] += (P[1]*Points[i].GetB());
    p[1] += (P[1]*P[2]*Points[i].GetB());

    m[2][2] += Points[i].GetB();
    p[2] += (P[2]*Points[i].GetB());
  } // equ: z = s[0]*x + s[1]*y + s[2]
  m[1][0] = m[0][1];

  m[2][0] = m[0][2];
  m[2][1] = m[1][2];
  TMatrixD::GauseSolve(m, p, s);
  Params = s;
}
//..............................................................................
double TSPlane::DistanceTo(const TVPointD& Crd) const {
  return Crd[2]*FNormal[2] + (Crd[0]*FNormal[0] + Crd[1]*FNormal[1] + FDistance);
}
//..............................................................................
double TSPlane::DistanceTo(const TSAtom& A) const  {
  return DistanceTo(A.GetCenter());
}
//..............................................................................
double TSPlane::Angle( const TVPointD &A,  const TVPointD &B) const  {
  TVPointD V;
  V = B-A;
  double ca = FNormal.CAngle(V), angle;
  angle = acos(ca)*180/M_PI;
  return angle;
}
//..............................................................................
double TSPlane::Angle(const TSBond& Bd) const  {
  return Angle(Bd.GetA().GetCenter(), Bd.GetB().GetCenter());
}
//..............................................................................
double TSPlane::Angle(const TSPlane& P) const  {
  TVPointD A;
  return Angle(A, P.GetNormal());
}
//..............................................................................
double TSPlane::Z(double X, double Y) const  {
  if( !FNormal[2] )  return 0;
  return (FNormal[0]*X + FNormal[1]*Y + FDistance)/FNormal[2];
}
//..............................................................................
 
