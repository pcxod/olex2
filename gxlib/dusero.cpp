//----------------------------------------------------------------------------//
// namespace TEXLib
// User object - a drawing object for unit cell
// (c) Oleg V. Dolomanov, 2004-8
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "dusero.h"

#include "glprimitive.h"
#include "glmaterial.h"
#include "glrender.h"
#include "glscene.h"
#include "gpcollection.h"

#include "styles.h"


TDUserObj::TDUserObj(TGlRenderer& R, short type, ematd* data, const olxstr& collectionName) :
  TGlMouseListener(R, collectionName), Type(type), Data(data)
{
  SetGroupable(false);
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
  TGlMaterial GlM;
  GlM.SetFlags(sglmAmbientF);
  GlM.AmbientF = 0;
  GlM.SetIdentityDraw(false);
  GlM.SetTransparent(false);
  GlP.SetProperties( GS.GetMaterial("Object", GlM) );


  if( Type == sgloSphere )  {
    GlP.Params[0] = 0.2/1.5;  
    GlP.Params[1] = 6;  
    GlP.Params[2] = 6;
  }
  else if( Data )
    GlP.Data = *Data;
  else
    ; // throw an exception
}
bool TDUserObj::Orient(TGlPrimitive& P)  {
  Parent.GlTranslate( Basis.GetCenter() );
  if( Type == sgloSphere && Data )  {
    for( int i=0; i < Data->Elements(); i++ )  {
      glTranslated( Data->Data(0)[i], Data->Data(1)[i], Data->Data(2)[i] );
      P.Draw();
    }
    return true;
  }    
  return false;
}

