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
void TDUserObj::Create(const olxstr& cName)  {
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
  GlP.SetProperties(GS.GetMaterial("Object", GlM));
  if( Type == sgloSphere )  {
    if( Vertices != NULL && Vertices->Count() == 3 )  {
      Params().Resize(16);
      for( int i=0; i < 3; i++ )  {
        for( int j=0; j < 3; j++ )  {
            Params()[i*4+j] = (*Vertices)[i][j];
        }
      }
      Params()[15] = 1;
    }
    GlP.Params[0] = 1;  
    GlP.Params[1] = 25;  
    GlP.Params[2] = 25;
    GlP.Compile();
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
  olx_gl::scale(Basis.GetZoom());
  if( Type == sgloSphere )  {
    if( Params().Count() == 16 )
      olx_gl::orient(Params().GetRawData());
    else if( Params().Count() == 1 )
      olx_gl::scale(Params()[0]);
  }
  else  {
    if( Type == sgloSphere && Vertices != NULL )  {
      for( size_t i=0; i < Vertices->Count(); i++ )  {
        olx_gl::translate((*Vertices)[i]);
        P.Draw();
      }
      return true;
    }    
  }
  return false;
}
