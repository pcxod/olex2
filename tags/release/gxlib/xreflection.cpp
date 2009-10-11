//----------------------------------------------------------------------------//
// namespace TXClasses: crystallographic core
// TXReflection
// (c) Oleg V. Dolomanov, 2006
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "xreflection.h"
#include "gpcollection.h"

#include "glrender.h"

#include "styles.h"

#include "glmaterial.h"
#include "glprimitive.h"

//----------------------------------------------------------------------------//
// TXReflection function bodies
//----------------------------------------------------------------------------//
TXReflection::TXReflection(TGlRenderer& r, const olxstr& collectionName, double minI, double maxI,
                            const TReflection& R, TAsymmUnit* au) :
  AGDrawObject(r, collectionName)
{
  hkl = vec3i( R.GetH(), R.GetK(), R.GetL() );
  I = R.GetI();
  // scaling has to be optimised, but as it is for now
  vec3d v(hkl), scaleV;
  scaleV[0] = scaleV[1] = scaleV[2] = 0.3;
  v = au->GetHklToCartesian()*v;
  scaleV = (au->GetHklToCartesian())*scaleV;
  double scale = olx_max(scaleV[2],  olx_max(scaleV[0], scaleV[1]) );
  v *= (2.0/scale);                    // extra scaling
  FCenter = v;
  Params().Resize(1);
  Params()[0] = (I - minI) / (maxI + olx_abs(minI));
  if( Params()[0] > 0 )  
    Params()[0] = sqrt( Params()[0] ); 
}
//..............................................................................
void TXReflection::Create(const olxstr& cName, const ACreationParams* cpar) {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);

  TGPCollection& GPC = Parent.FindOrCreateCollection( GetCollectionName() );
  GPC.AddObject(*this);
  if( GPC.PrimitiveCount() != 0 )  return;

  TGraphicsStyle& GS = GPC.GetStyle();
  TGlPrimitive& GlP = GPC.NewPrimitive("Reflection", sgloTriangles);

  TGlMaterial GlM;
  GlM.FromString("85;1.000,0.000,0.059,0.000;2138535799;1.000,1.000,1.000,0.500;36");
  GlP.SetProperties( GS.GetMaterial("Reflection", GlM) );
  double sz = 0.5;
  static const double c_30 = sqrt(3.0)/2, s_30 = 0.5;
  vec3d edges [] = {
    vec3d(0, 0, sz),
    vec3d(0, sz, 0),
    vec3d(sz*c_30, -sz*s_30, 0),
    vec3d(-sz*c_30, -sz*s_30, 0)
  };
  const vec3d cnt = (edges[0] + edges[1] + edges[2] + edges[3])/4;
  vec3d faces[] = {
    edges[0], edges[2], edges[1],
    edges[0], edges[3], edges[2],
    edges[0], edges[1], edges[3],
    edges[1], edges[2], edges[3]
  };
  GlP.Vertices.SetCount(12);  // four triangles + normals
  GlP.Normals.SetCount(4);
  for( int i=0; i < 12; i++ )  {
    if( (i%3) == 0 )  {
      const int ind = i/3;
      GlP.Normals[ind] = ((faces[ind]+faces[ind+1]+faces[ind+2])/3-cnt).Normalise();
    }
    GlP.Vertices[i] = faces[i];
  }
}
//..............................................................................
TXReflection::~TXReflection()  {  }
//..............................................................................
bool TXReflection::Orient(TGlPrimitive& GlP)  {
  Parent.GlTranslate(FCenter);
  // scale the larger reflections up
  double scale1 = sqrt(atan(FParams[0])*2/M_PI);
  Parent.GlScale( (float)(1+scale1*6) );

  if( IsSelected() || !GlP.GetProperties().IsTransparent() )  
    return false;

  TGlMaterial GlM = GlP.GetProperties();

  double scale = (1.0-FParams[0]);
  GlM.AmbientF[3] = (float)scale;
  GlM.Init();
  return false;
}
//..............................................................................
bool TXReflection::GetDimensions(vec3d &Max, vec3d &Min)
{
  Min = FCenter;
  Max = FCenter;
  return true;
};
//..............................................................................
