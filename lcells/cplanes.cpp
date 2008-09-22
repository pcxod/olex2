//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "cplanes.h"
#include "splane.h"

void _fastcall GetPlane(vec3d_list& Crds, vec3d& X, vec3d &Y, vec3d& Z)  {
  if( Crds.Count() < 3 )  return;
  TTypeList< AnAssociation2<vec3d, double> > points;
  points.SetCapacity(Crds.Count());
  for( int i=0; i < Crds.Count(); i++ )
    points.AddNew( Crds[i], 1.0 );
  vec3d par, center, T;
  TSPlane::CalcPlane(points, par, center);
  double d = center.DotProd(par)/par.Length();
  par.Normalise();
  if( par[2] == 0 )  return;
  X[0] = 100;
  X[1] = 100;
  X[2] = (par[0]*X[0] + par[1]*X[1] + d)/par[2];
  T[0] = 150;
  T[1] = -100;
  T[2] = (par[0]*T[0] + par[1]*T[1] + d)/par[2];
  X -= T;
  X.Normalise();
  Y = X.XProdVec(Z);
  Y.Normalise();
}
//---------------------------------------------------------------------------

#pragma package(smart_init)

