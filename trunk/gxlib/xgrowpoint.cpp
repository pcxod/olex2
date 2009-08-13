//----------------------------------------------------------------------------//
// namespace TXClasses: crystallographic core
// TXGrowLine
// (c) Oleg V. Dolomanov, 2006
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "xgrowpoint.h"
#include "gpcollection.h"

//----------------------------------------------------------------------------//
// TXGrowLine function bodies
//----------------------------------------------------------------------------//
TXGrowPoint::TXGrowPoint(TGlRenderer& R, const olxstr& collectionName, const vec3d& center,
  const smatd& transform) : AGDrawObject(R, collectionName)  
{
  AGDrawObject::SetGroupable(false);
  Params().Resize(1);
  Params()[0] = 1;
  Transform = transform;
  Center = center;
}
//..............................................................................
void TXGrowPoint::Create(const olxstr& cName, const ACreationParams* cpar) {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  TGPCollection& GPC = Parent.FindOrCreateCollection( GetCollectionName() );
  GPC.AddObject(*this);
  if( GPC.PrimitiveCount() != 0 )  return;

  TGraphicsStyle& GS = GPC.GetStyle();

  TGlPrimitive& GlP = GPC.NewPrimitive( GetCollectionName(), sgloTriangles );
  TGlMaterial GlM;
  GlM.SetFlags( sglmAmbientF|sglmDiffuseF);
  GlM.AmbientF = 0x000000ff;
  GlM.DiffuseF = 0xf0f0f0;
  GlM.SetMark(false);
  GlP.SetProperties( GS.GetMaterial(GetCollectionName(), GlM));

  double sz = 1;
  const double c_30 = sqrt(3.0)/2, s_30 = 0.5;
  TArrayList<vec3d> vecs;
  vecs.Add( vec3d(0, 0, sz) );
  vecs.Add( vec3d(0, sz, 0) );
  vecs.Add( vec3d(sz*c_30, -sz*s_30, 0) );
  vecs.Add( vec3d(-sz*c_30, -sz*s_30, 0) );

  GlP.Vertices.SetCount(12);  // four triangles + normals
  GlP.Normals.SetCount(4);

  GlP.Vertices[0] = vecs[0];  GlP.Vertices[1] = vecs[2];  GlP.Vertices[2] = vecs[1];
  GlP.Normals[0] = vec3d(vecs[2]-vecs[0]).XProdVec(vecs[1]-vecs[0]).Normalise();

  GlP.Vertices[3] = vecs[0];  GlP.Vertices[4] = vecs[3];  GlP.Vertices[5] = vecs[2];
  GlP.Normals[0] = vec3d(vecs[3]-vecs[0]).XProdVec(vecs[2]-vecs[0]).Normalise();

  GlP.Vertices[6] = vecs[0];  GlP.Vertices[7] = vecs[1];  GlP.Vertices[8] = vecs[3];
  GlP.Normals[0] = vec3d(vecs[1]-vecs[0]).XProdVec(vecs[3]-vecs[0]).Normalise();

  GlP.Vertices[9] = vecs[1];  GlP.Vertices[10] = vecs[2];  GlP.Vertices[11] = vecs[3];
  GlP.Normals[0] = vec3d(vecs[2]-vecs[1]).XProdVec(vecs[3]-vecs[1]).Normalise();
}
//..............................................................................
TXGrowPoint::~TXGrowPoint()  {  }
//..............................................................................
bool TXGrowPoint::Orient(TGlPrimitive& GlP)  {
  Parent.GlTranslate(Center);
  Parent.GlScale((float)Params()[0], (float)Params()[0], (float)Params()[0]);
  return false;
} 
//..............................................................................
void TXGrowPoint::SetRadius(float V)  {
  Params()[0] = V;
}
//..............................................................................
bool TXGrowPoint::GetDimensions(vec3d &Max, vec3d &Min)  {
  if( Center[0] > Max[0] )  Max[0] = Center[0];
  if( Center[1] > Max[1] )  Max[1] = Center[1];
  if( Center[2] > Max[2] )  Max[2] = Center[2];
  if( Center[0] < Min[0] )  Min[0] = Center[0];
  if( Center[1] < Min[1] )  Min[1] = Center[1];
  if( Center[2] < Min[2] )  Min[2] = Center[2];

  return true;
}
//..............................................................................

