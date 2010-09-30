//----------------------------------------------------------------------------//
// User object - a drawing object for unit cell
// (c) Oleg V. Dolomanov, 2004-8
//----------------------------------------------------------------------------//
#include "dusero.h"
#include "glprimitive.h"
#include "glmaterial.h"
#include "glrender.h"
#include "gpcollection.h"
#include "styles.h"

TDUserObj::TDUserObj(TGlRenderer& R, short type, const olxstr& collectionName) :
  AGlMouseHandlerImp(R, collectionName), Type(type), Vertices(NULL), Normals(NULL)
{
  SetSelectable(false);
  GlM.SetFlags(sglmAmbientF);
  GlM.AmbientF = 0;
  GlM.SetIdentityDraw(false);
  GlM.SetTransparent(false);
}
//...........................................................................
void TDUserObj::Create(const olxstr& cName, const ACreationParams* cpar)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  olxstr NewL;
  TGPCollection* GPC = Parent.FindCollectionX( GetCollectionName(), NewL);
  if( GPC == NULL )
    GPC = &Parent.NewCollection(NewL);
  GPC->AddObject(*this);
  if( GPC->PrimitiveCount() != 0 )  return;

  TGraphicsStyle& GS = GPC->GetStyle();
  TGlPrimitive& GlP = GPC->NewPrimitive("Object", Type);
  GlP.SetProperties( GS.GetMaterial("Object", GlM) );


  if( Type == sgloSphere )  {
    GlP.Params[0] = 0.2/1.5;  
    GlP.Params[1] = 6;  
    GlP.Params[2] = 6;
  }
  else  {
    if( Vertices != NULL )
      GlP.Vertices = *Vertices;
    if( Normals != NULL )
      GlP.Normals = *Normals;
  }
}
bool TDUserObj::Orient(TGlPrimitive& P)  {
  olx_gl::translate(Basis.GetCenter());
  if( Type == sgloSphere && Vertices != NULL )  {
    for( size_t i=0; i < Vertices->Count(); i++ )  {
      olx_gl::translate((*Vertices)[i]);
      P.Draw();
    }
    return true;
  }    
  return false;
}

