/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "xgrowpoint.h"
#include "gpcollection.h"

TXGrowPoint::TXGrowPoint(TGlRenderer& R, const olxstr& collectionName, const vec3d& center,
  const smatd& transform) : AGDrawObject(R, collectionName)  
{
  AGDrawObject::SetSelectable(false);
  Params().Resize(1);
  Params()[0] = 1;
  Transform = transform;
  Center = center;
}
//..............................................................................
void TXGrowPoint::Create(const olxstr& cName) {
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
  GlP.SetProperties( GS.GetMaterial(GetCollectionName(), GlM));

  double sz = 1;
  const double c_30 = sqrt(3.0)/2, s_30 = 0.5;
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
TXGrowPoint::~TXGrowPoint()  {  }
//..............................................................................
bool TXGrowPoint::Orient(TGlPrimitive& GlP)  {
  olx_gl::translate(Center);
  olx_gl::scale(Params()[0]);
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
