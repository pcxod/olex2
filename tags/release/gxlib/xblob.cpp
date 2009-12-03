//----------------------------------------------------------------------------//
// Blob object
// (c) Oleg V. Dolomanov, 2009
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "xblob.h"

#include "glprimitive.h"
#include "glmaterial.h"
#include "glrender.h"
#include "glscene.h"
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
  TGPCollection* GPC = Parent.FindCollectionX( GetCollectionName(), NewL);
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
  Parent.GlTranslate( Basis.GetCenter() );
  glPolygonMode(GL_FRONT_AND_BACK, PolygonMode);
  glBegin(GL_TRIANGLES);
  for( size_t i=0; i < triangles.Count(); i++ )  {
    for( int j=0; j < 3; j++ )  {
      const vec3f& nr = normals[triangles[i].pointID[j]];
      glNormal3f( nr[0], nr[1], nr[2] );
      const vec3f& p = vertices[triangles[i].pointID[j]];  // cell drawing
      glVertex3f(p[0], p[1], p[2]);                  // cell drawing
    }
  }
  glEnd();
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  return true;
}

