/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "xblob.h"
#include "glprimitive.h"
#include "glmaterial.h"
#include "glrender.h"
#include "gpcollection.h"
#include "styles.h"

struct TriangleComparator {
  mat3f orientation;
  const TTypeList<vec3f> &vertices;
  TriangleComparator(const mat3f & orientation_,
    const TTypeList<vec3f> &vertices_)
    : orientation(orientation_),
    vertices(vertices_)
  {}
  int Compare(const IsoTriangle *t1, const IsoTriangle *t2) const {
    return olx_cmp((vertices[t2->pointID[0]]*orientation)[2],
      (vertices[t1->pointID[0]] * orientation)[2]);
  }
};
//...........................................................................
TXBlob::TXBlob(TGlRenderer& R, const olxstr& collectionName) :
  AGDrawObject(R, collectionName)
{
  SetSelectable(false);
}
//...........................................................................
void TXBlob::Create(const olxstr& cName)  {
  if( !cName.IsEmpty() )
    SetCollectionName(cName);
  olxstr NewL;
  TGPCollection* GPC = Parent.FindCollectionX(GetCollectionName(), NewL);
  if (GPC == 0) {
    GPC = &Parent.NewCollection(NewL);
  }
  GPC->AddObject(*this);
  if (GPC->PrimitiveCount() != 0) {
    return;
  }

  TGraphicsStyle& GS = GPC->GetStyle();
  TGlPrimitive& GlP = GPC->NewPrimitive("Blob", sgloQuads);
  TGlMaterial GlM;
  GlM.SetFlags(sglmAmbientF);
  GlM.AmbientF = 0;
  GlM.SetIdentityDraw(false);
  GlM.SetTransparent(false);
  GlP.SetProperties( GS.GetMaterial("Blob", GlM) );
}
//...........................................................................
bool TXBlob::Orient(TGlPrimitive& P)  {
  //olx_gl::translate(Basis.GetCenter());
  olx_gl::polygonMode(GL_FRONT_AND_BACK, PolygonMode);
  bool use_color = colors.Count() == vertices.Count();
  olx_gl::FlagManager fm;
  if (use_color) {
    fm.enable(GL_COLOR_MATERIAL);
  }
  bool transparent = P.GetProperties().IsTransparent();
  float to = P.GetProperties().DiffuseF[3];
  if (transparent) {
    QuickSorter::Sort(triangles, TriangleComparator(Parent.GetBasis().GetMatrix(),
      vertices));
  }
  olx_gl::begin(GL_TRIANGLES);
  for( size_t i=0; i < triangles.Count(); i++ )  {
    if (normals.Count() == triangles.Count()) {
      olx_gl::normal(normals[i]);
    }
    for (int j = 0; j < 3; j++) {
      if (normals.Count() == vertices.Count()) {
        olx_gl::normal(normals[triangles[i][j]]);
      }
      if (use_color) {
        if (transparent) {
          olx_gl::color(colors[triangles[i][j]][0],
            colors[triangles[i][j]][1],
            colors[triangles[i][j]][2],
            to);
        }
        else {
          olx_gl::color(colors[triangles[i][j]].Data());
        }
      }
      olx_gl::vertex(vertices[triangles[i][j]]);  // cell drawing
    }
  }
  olx_gl::end();
  olx_gl::polygonMode(GL_FRONT_AND_BACK, GL_FILL);
  return true;
}
//...........................................................................
void TXBlob::UpdateNormals() {
  normals.SetCount(vertices.Count());
  for (size_t i = 0; i < normals.Count(); i++) {
    normals[i].Null();
  }
  for (size_t i = 0; i < triangles.Count(); i++) {
    vec3f vec1 = vertices[triangles[i].pointID[1]] -
      vertices[triangles[i].pointID[0]];
    vec3f vec2 = vertices[triangles[i].pointID[2]] -
      vertices[triangles[i].pointID[0]];
    vec3f normal = vec1.XProdVec(vec2);
    normals[triangles[i].pointID[0]] += normal;
    normals[triangles[i].pointID[1]] += normal;
    normals[triangles[i].pointID[2]] += normal;
  }
  for (size_t i = 0; i < normals.Count(); i++) {
    float d = normals[i].Length();
    if (d != 0) {
      normals[i] /= d;
    }
  }
}

