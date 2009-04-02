//----------------------------------------------------------------------------//
// TGlText - a drawing object for text
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "gltext.h"
#include "glmaterial.h"
#include "gpcollection.h"
#include "glrender.h"
#include "glscene.h"

UseGlNamespace()
//..............................................................................
//..............................................................................

TGlText::TGlText(const olxstr& collectionName, TGlRenderer *Render, const olxstr& Text):
    TGlMouseListener(collectionName, Render)  {
  this->Text = Text;
  Move2D(false);
  Moveable(true);
  Roteable(true);
  Zoomable(false);
  TextFlags = 0;
  SetStaticPos(false);
  SetStaticWidth(false);
  CalcWidth = 0;
  Width = 100;
  FontIndex = -1;
}
//..............................................................................
void TGlText::SetStaticPos(bool On)  {
  SetBit(On, TextFlags, gltStaticPos);
  Move2D(On);  
  Roteable(On); 
}
//..............................................................................
void TGlText::SetStaticWidth(bool On)  {
  SetBit(On, TextFlags, gltStaticWidth);
}
//..............................................................................
void TGlText::SetStaticFace(bool On)  {
  SetBit(On, TextFlags, gltStaticFace);
}
//..............................................................................
void TGlText::CalcWH()  {
  CalcWidth = 0;
  CalcHeight = 0;
/*  int i, tl = FText.Length();
  if( !FParent->Scene.GlyphMetrics )  return;
  GLYPHMETRICSFLOAT *GM;
  for( i=0; i < tl; i++ )
  {
    GM = &FParent->Scene.GlyphMetrics[FText[i]];
    if( i < (tl - 1) )
    {   FCalcWidth += GM->gmfCellIncX;  FCalcHeight += GM->gmfCellIncY; }
    else
    {   FCalcWidth += GM->gmfBlackBoxX; FCalcHeight += GM->gmfBlackBoxY; }
  }*/
}
//..............................................................................
void TGlText::SetWidth(float W)  {
  Width = W;
}
//..............................................................................
void TGlText::SetText(const olxstr &T)  {
  Text = T;
  CalcWH();
}
//..............................................................................
void TGlText::Create(const olxstr& cName, const ACreationParams* cpar)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  TGPCollection* GPC = FParent->FindCollection( GetCollectionName() );
  if( GPC == NULL )    
    GPC = FParent->NewCollection( GetCollectionName() );
  GPC->AddObject(this);
  if( GPC->PrimitiveCount() != 0 )  return;

  TGlPrimitive* GlP = GPC->NewPrimitive("Text", sgloText);
  TGlMaterial GlM;
  GlM.SetFlags(sglmAmbientF|sglmIdentityDraw);

  GlM.AmbientF = 0x7fff7f;
  GlP->SetProperties(&GlM);
  GlP->Params[0] = -1;  //bitmap; TTF by default
  CalcWH();
}
//..............................................................................
bool TGlText::Orient(TGlPrimitive *P)  {
  TGlFont *Fnt = Font();
  if( Fnt == NULL )  return false;
  P->SetFont(Fnt);
  vec3d T( Basis.GetCenter() );
//  if( StaticPos() )
//  {
//    T *= Scale;
//    T *= FParent->Basis().Matrix();
//  }
  if( P->Params[0] < 0 )  {  // bitmap
    FParent->DrawTextSafe(T, Text, *Fnt ); 
    return true;
  }
  else  // ttf
    Parent()->GlTranslate(T);

/*  if( StaticWidth() )
  {
    if( FWidth && FCalcWidth )
      Parent()->GlScale(Basis.Zoom()*Scale*FWidth/FCalcWidth);
    else
      Parent()->GlScale(Basis.Zoom()*Scale);
  }
  else
  {
    Parent()->GlScale(Basis.Zoom());
  }
//  if( StaticFace() )  GlOrient(M);
  if( StaticFace() )
  {
    TMatrixD M(4,4);
    M = FParent->Basis().Matrix();
    M.Transpose();
    M = Basis.Matrix()*M;
    Parent()->GlOrient(M);
  }            */
  P->SetString(&Text);
  return false;
}
//..............................................................................
TGlFont *TGlText::Font()  const   {  return FParent->Scene()->Font(FontIndex); }
//..............................................................................
