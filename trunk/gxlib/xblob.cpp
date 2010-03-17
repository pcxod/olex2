//----------------------------------------------------------------------------//
// Blob object
// (c) Oleg V. Dolomanov, 2009
//----------------------------------------------------------------------------//
#include "xblob.h"
#include "glprimitive.h"
#include "glmaterial.h"
#include "glrender.h"
#include "gpcollection.h"
#include "styles.h"


TDBlob::TDBlob(TGlRenderer& R, const olxstr& collectionName) :
  TGlMouseListener(R, collectionName)
{
  SetGroupable(false);
}
//...........................................................................
void TDBlob::Create(const olxstr& cName, const ACreationParams* cpar)  {
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
bool TDBlob::Orient(TGlPrimitive& P)  {
  olx_gl::translate(Basis.GetCenter());
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

