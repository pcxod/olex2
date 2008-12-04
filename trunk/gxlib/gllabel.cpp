//----------------------------------------------------------------------------//
// namespace TEXLib
// TGlXLabel - a drawing object for atom label
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "gllabel.h"
#include "gpcollection.h"
#include "styles.h"

#include "glrender.h"
#include "glscene.h"

TXGlLabel::TXGlLabel(const olxstr& collectionName, TGlRender *Render) :
    TGlMouseListener(collectionName, Render)  {
  Move2D(false);
  Moveable(true);
  Zoomable(false);
  Groupable(false);
  OffsetX = OffsetY = 0;
  FFontIndex = -1;
};
TXGlLabel::~TXGlLabel(){}

void TXGlLabel::Create(const olxstr& cName)  {
  if( cName.Length() != 0 )  SetCollectionName(cName);
  TGlPrimitive *GlP;
  TGPCollection *GPC;
  TGraphicsStyle *GS;

  TGlMaterial *GlM;

  GPC = FParent->FindCollection( GetCollectionName() );
  if( !GPC )   GPC = FParent->NewCollection( GetCollectionName() );
  else  {
    GPC->AddObject(this);
    return;
  }
  GS = GPC->Style();
  GS->SetPersistent(true);
  GPC->AddObject(this);

  GlM = const_cast<TGlMaterial*>(GS->Material("Plane"));
  if( GlM->Mark() )  {
    GlM->SetFlags( sglmAmbientF|sglmIdentityDraw|sglmTransparent  );
    GlM->AmbientF = 0x800f0f0f;
    GlM->DiffuseF = 0x800f0f0f;
    GlM->ShininessF = 128;
    GlM->Mark(false);
  }
  GlP = GPC->NewPrimitive("Plane");  // a sphere at the basis of the object {0,0,0}
  GlP->SetProperties(GlM);
  GlP->Type(sgloQuads);
  GlP->Data().Resize(3, 4);

  GlM = const_cast<TGlMaterial*>(GS->Material("Text"));
  if( GlM->Mark() )
    *GlM = Font()->GetMaterial();

  GlP = GPC->NewPrimitive("Text");
  GlP->SetProperties(GlM);
  GlP->Type(sgloText);
  GlP->Params()[0] = -1;  //bitmap; TTF by default
  GlP->Font(Font());
}
//..............................................................................
void TXGlLabel::SetLabel(const olxstr& L)   {
  FLabel = L;  
  if( Font() != NULL )  {
    OffsetX = Font()->TextWidth( FLabel )/2;
    OffsetY = Font()->TextHeight()/2;
  }
}
//..............................................................................
bool TXGlLabel::Orient(TGlPrimitive *P)  {
  vec3d T( Basis.GetCenter() );
  const double Scale = FParent->GetScale();
  if( P->Type() == sgloText )  {
    T += FParent->GetBasis().GetCenter();
    T *= FParent->GetBasis().GetMatrix();
    T[2] += 5;
    T /= Scale;
    T[0] -= OffsetX;
    T[1] -= OffsetY;
    FParent->DrawTextSafe(T, FLabel, *Font());
    return true;
  }
  else  {
    double xinc = OffsetX*Scale;
    double yinc = OffsetY*Scale;
    T += FParent->GetBasis().GetCenter();
    T *= FParent->GetBasis().GetMatrix();
    //T[0] += 0.15;  T[1] += 0.15;
    T[2] += 4.8;
    Parent()->GlTranslate(T);
    P->Data()[0][0] = -xinc;  P->Data()[1][0] = yinc;
    P->Data()[0][1] = xinc;   P->Data()[1][1] = yinc;
    P->Data()[0][2] = xinc;   P->Data()[1][2] = -yinc;
    P->Data()[0][3] = -xinc;  P->Data()[1][3] = -yinc;
    P->Data()[2][0] = -0.1;
    P->Data()[2][1] = -0.1;
    P->Data()[2][2] = -0.1;
    P->Data()[2][3] = -0.1;
  }
  return false;
}
//..............................................................................
TGlFont* TXGlLabel::Font() const {  return FParent->Scene()->Font(FFontIndex); }
//..............................................................................
