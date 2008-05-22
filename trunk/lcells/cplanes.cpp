//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "cplanes.h"
#include "ematrix.h"

void _fastcall GetPlane(TVPointDList& Crds, TVectorD& par, TVPointD &X, TVPointD &Y, TVPointD &Z)  {
  AnsiString Tmp;
  TMatrixD m(3,3);
  TVPointD p, s;
  TVPointD T, P;
  double R,R1,XX;
  if( Crds.Count() > 3 )  {
    for( int i=0; i < Crds.Count(); i++ )  {
      P = Crds[i];
      m[0][0] += (P[0])*(P[0]);
      m[0][1] += (P[0])*(P[1]);
      m[0][2] += (P[0]);
      p[0] += (P[0])*(P[2]);

      m[1][1] += (P[1])*(P[1]);
      m[1][2] += (P[1]);
      p[1] += (P[1])*(P[2]);

      p[2] += (P[2]);

    } // equ: z = s[0]*x + s[1]*y + s[2]
    m[2][2] = Crds.Count();
    m[1][0] = m[0][1];

    m[2][0] = m[0][2];
    m[2][1] = m[1][2];
    TMatrixD::GauseSolve(m, p, s);
    Z[0] = s[0];
    Z[1] = s[1];
    Z[2] = -1;
    R = s[2]/Z.Length();
    Z.Normalise();
    X[0] = 100;
    X[1] = 100;
    X[2] = s[0]*X[0] + s[1]*X[1] + s[2];
    T[0] = 150;
    T[1] = -100;
    T[2] = s[0]*T[0] + s[1]*T[1] + s[2];
    X-=T;
    X.Normalise();
    Y = X.XProdVec(Z);
    Y.Normalise();
    par = s;
  }
}
//---------------------------------------------------------------------------

#pragma package(smart_init)

