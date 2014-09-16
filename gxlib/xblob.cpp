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
  if( GPC == NULL )
    GPC = &Parent.NewCollection(NewL);
  GPC->AddObject(*this);
  if( GPC->PrimitiveCount() != 0 )  return;

  TGraphicsStyle& GS = GPC->GetStyle();
  TGlPrimitive& GlP = GPC->NewPrimitive("Blob", sgloQuads);
  TGlMaterial GlM;
  GlM.SetFlags(sglmAmbientF);
  GlM.AmbientF = 0;
  GlM.SetIdentityDraw(false);
  GlM.SetTransparent(false);
  GlP.SetProperties( GS.GetMaterial("Blob", GlM) );
}
bool TXBlob::Orient(TGlPrimitive& P)  {
  //olx_gl::translate(Basis.GetCenter());
  olx_gl::polygonMode(GL_FRONT_AND_BACK, PolygonMode);
  olx_gl::begin(GL_TRIANGLES);
  for( size_t i=0; i < triangles.Count(); i++ )  {
    for( int j=0; j < 3; j++ )  {
      olx_gl::normal(normals[triangles[i].pointID[j]]);
      olx_gl::vertex(vertices[triangles[i].pointID[j]]);  // cell drawing
    }
  }
  olx_gl::end();
  olx_gl::polygonMode(GL_FRONT_AND_BACK, GL_FILL);
  return true;
}
