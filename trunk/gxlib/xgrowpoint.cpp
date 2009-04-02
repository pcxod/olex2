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
TXGrowPoint::TXGrowPoint(const olxstr& collectionName, const vec3d& center,
                         const smatd& transform, TGlRenderer *Render) :
  AGDrawObject(collectionName)  {
  AGDrawObject::Groupable(false);
  FParent = Render;
  Params().Resize(1);
  Params()[0] = 1;
  Transform = transform;
  Center = center;
}
//..............................................................................
void TXGrowPoint::Create(const olxstr& cName, const ACreationParams* cpar) {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  TGPCollection* GPC = FParent->FindCollection( GetCollectionName() );
  if( GPC == NULL )
    GPC = FParent->NewCollection( GetCollectionName() );
  GPC->AddObject(this);
  if( GPC->PrimitiveCount() != 0 )  return;

  TGraphicsStyle* GS = GPC->Style();

  TGlPrimitive* GlP = GPC->NewPrimitive( GetCollectionName(), sgloTriangles );
  TGlMaterial* GlM = const_cast<TGlMaterial*>(GS->Material(GetCollectionName()));

  if( GlM->Mark() )  {
    GlM->SetFlags( sglmAmbientF|sglmDiffuseF);
    GlM->AmbientF = 0x000000ff;
    GlM->DiffuseF = 0xf0f0f0;
    GlM->Mark(false);
  }
  GlP->SetProperties(GlM);
  double sz = 1;
  GlP->Data.Resize(4, 12);  // four triangles + normals
  const double c_30 = sqrt(3.0)/2, s_30 = 0.5;
  TArrayList<vec3d> vecs;
  vecs.Add( vec3d(0, 0, sz) );
  vecs.Add( vec3d(0, sz, 0) );
  vecs.Add( vec3d(sz*c_30, -sz*s_30, 0) );
  vecs.Add( vec3d(-sz*c_30, -sz*s_30, 0) );
  GlP->Data[0][0] = vecs[0][0];  GlP->Data[0][1] = vecs[2][0];  GlP->Data[0][2] = vecs[1][0];
  GlP->Data[1][0] = vecs[0][1];  GlP->Data[1][1] = vecs[2][1];  GlP->Data[1][2] = vecs[1][1];
  GlP->Data[2][0] = vecs[0][2];  GlP->Data[2][1] = vecs[2][2];  GlP->Data[2][2] = vecs[1][2];
  vec3d n = vec3d(vecs[2]-vecs[0]).XProdVec(vecs[1]-vecs[0]).Normalise();
  GlP->Data[3][0] = n[0];  GlP->Data[3][1] = n[1];  GlP->Data[3][2] = n[2];  

  GlP->Data[0][3] = vecs[0][0];  GlP->Data[0][4] = vecs[3][0];  GlP->Data[0][5] = vecs[2][0];
  GlP->Data[1][3] = vecs[0][1];  GlP->Data[1][4] = vecs[3][1];  GlP->Data[1][5] = vecs[2][1];
  GlP->Data[2][3] = vecs[0][2];  GlP->Data[2][4] = vecs[3][2];  GlP->Data[2][5] = vecs[2][2];
  n = vec3d(vecs[3]-vecs[0]).XProdVec(vecs[2]-vecs[0]).Normalise();
  GlP->Data[3][0] = n[0];  GlP->Data[3][1] = n[1];  GlP->Data[3][2] = n[2];  

  GlP->Data[0][6] = vecs[0][0];  GlP->Data[0][7] = vecs[1][0];  GlP->Data[0][8] = vecs[3][0];
  GlP->Data[1][6] = vecs[0][1];  GlP->Data[1][7] = vecs[1][1];  GlP->Data[1][8] = vecs[3][1];
  GlP->Data[2][6] = vecs[0][2];  GlP->Data[2][7] = vecs[1][2];  GlP->Data[2][8] = vecs[3][2];
  n = vec3d(vecs[1]-vecs[0]).XProdVec(vecs[3]-vecs[0]).Normalise();
  GlP->Data[3][0] = n[0];  GlP->Data[3][1] = n[1];  GlP->Data[3][2] = n[2];  

  GlP->Data[0][9] = vecs[1][0];  GlP->Data[0][10] = vecs[2][0];  GlP->Data[0][11] = vecs[3][0];
  GlP->Data[1][9] = vecs[1][1];  GlP->Data[1][10] = vecs[2][1];  GlP->Data[1][11] = vecs[3][1];
  GlP->Data[2][9] = vecs[1][2];  GlP->Data[2][10] = vecs[2][2];  GlP->Data[2][11] = vecs[3][2];
  n = vec3d(vecs[2]-vecs[1]).XProdVec(vecs[3]-vecs[1]).Normalise();
  GlP->Data[3][0] = n[0];  GlP->Data[3][1] = n[1];  GlP->Data[3][2] = n[2];  
}
//..............................................................................
TXGrowPoint::~TXGrowPoint()  {  }
//..............................................................................
bool TXGrowPoint::Orient(TGlPrimitive *GlP)  {
  Parent()->GlTranslate(Center);
  Parent()->GlScale((float)Params()[0], (float)Params()[0], (float)Params()[0]);
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

