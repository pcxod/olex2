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


TDUserObj::TDUserObj(short type, TMatrixD* data, const olxstr& collectionName, TGlRender *Render) :
  TGlMouseListener(collectionName, Render), Type(type), Data(data)
{
  Groupable(false);
}
//...........................................................................
void TDUserObj::Create(const olxstr& cName)  {
  if( !cName.IsEmpty() )  SetCollectionName(cName);
  olxstr NewL;
  TGlMaterial GlM;
  const TGlMaterial *SGlM;
  GlM.SetFlags(sglmAmbientF);
  GlM.AmbientF = 0;
  TGPCollection* GPC = FParent->CollectionX( GetCollectionName(), NewL);
  if( !GPC )
    GPC = FParent->NewCollection(NewL);
  else  {
    if( GPC->PrimitiveCount() )  {
      GPC->AddObject(this);
      return;
    }
  }
  TGraphicsStyle* GS = GPC->Style();
  GPC->AddObject(this);

  TGlPrimitive* FGlP = GPC->NewPrimitive("Object");
  SGlM = GS->Material("Object");
  if( !SGlM->Mark() )  FGlP->SetProperties(SGlM);
  else  {
    GlM.SetIdentityDraw(false);
    GlM.SetTransparent(false);
    FGlP->SetProperties(&GlM);
  }
  FGlP->Type(Type);
  if( Type == sgloSphere )  {
    FGlP->Params()[0] = 0.2/1.5;  
    FGlP->Params()[1] = 6;  
    FGlP->Params()[2] = 6;
  }
  else if( Data )
    FGlP->Data() = *Data;
  else
    ; // throw an exception
}
bool TDUserObj::Orient(TGlPrimitive *P)  {
  Parent()->GlTranslate( Basis.GetCenter() );
  if( Type == sgloSphere && Data )  {
    for( int i=0; i < Data->Elements(); i++ )  {
      glTranslated( Data->Data(0)[i], Data->Data(1)[i], Data->Data(2)[i] );
      P->Draw();
    }
    return true;
  }    
  return false;
}

