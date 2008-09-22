//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "cplanes.h"
#include "splane.h"

void _fastcall GetPlane(vec3d_list& Crds, vec3d& Z)  {
  if( Crds.Count() < 3 )  return;
  TTypeList< AnAssociation2<vec3d, double> > points;
  points.SetCapacity(Crds.Count());
  for( int i=0; i < Crds.Count(); i++ )
    points.AddNew( Crds[i], 1.0 );
  vec3d center;
  TSPlane::CalcPlane(points, Z, center);
  Z.Normalise();
}
//---------------------------------------------------------------------------

#pragma package(smart_init)

