//----------------------------------------------------------------------------//
// namespace TGlObj
// TDFrame - a drawing object for selection frame
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "glbackground.h"
#include "glmaterial.h"
#include "glrender.h"
#include "gpcollection.h"
#include "styles.h"

UseGlNamespace()
//..............................................................................
//..............................................................................

TGlBackground::TGlBackground(TGlRenderer& r, const olxstr& collectionName, bool ceiling):
  AGDrawObject(r, collectionName)
{
  FCeiling = ceiling;
  SetGroupable(false);
  Texture = NULL;
}
//..............................................................................
void TGlBackground::Create(const olxstr& cName, const ACreationParams* cpar) {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);
  TGPCollection& GPC = Parent.FindOrCreateCollection( GetCollectionName() );
  GPC.AddObject(*this);
  if( GPC.PrimitiveCount() != 0 )  return;

  TGlMaterial GlM;
  if( FCeiling )
    GlM.SetFlags(sglmAmbientF|sglmDiffuseF|sglmTransparent|sglmIdentityDraw);
  else
    GlM.SetFlags(sglmAmbientF|sglmIdentityDraw);
  GlM.AmbientF = 0x7f4f4f4f;
  GlM.DiffuseF = 0x7f4f4f4f;
  GlM.AmbientB = 0x7f4f4f4f;
  GlM.DiffuseB = 0x7f4f4f4f;

  TGraphicsStyle& GS = GPC.GetStyle();
  FColors[0] = GS.GetParam("A", 0xffffffff, true).ToInt();
  FColors[1] = GS.GetParam("B", 0xffffffff, true).ToInt();
  FColors[2] = GS.GetParam("C", "0", true).ToInt();
  FColors[3] = GS.GetParam("D", "0", true).ToInt();

  TGlPrimitive& GlP = GPC.NewPrimitive("Plane", sgloQuads);
  GlP.SetProperties(GlM);
  GlP.Data.Resize(6, 4);
  // texture coordinates
  GlP.Data[4][0] = 1;  GlP.Data[5][0] = 1;
  GlP.Data[4][1] = 0;  GlP.Data[5][1] = 1;
  GlP.Data[4][2] = 0;  GlP.Data[5][2] = 0;
  GlP.Data[4][3] = 1;  GlP.Data[5][3] = 0;
  if( Texture != NULL )
    GlP.SetTextureId( Texture->GetId() );
  
  Orient(GlP);
}
//..............................................................................
void TGlBackground::SetTexture(TGlTexture* tx)  {  
  Texture = tx;
}
//..............................................................................
bool TGlBackground::Orient(TGlPrimitive& P)  {
  P.GetProperties().Init();
  if( Texture != NULL )
    P.SetTextureId( Texture->GetId() );
  double Scale = Parent.GetScale();
  double HW = (Parent.GetWidth()+1)/2*Scale;
  double HH = (Parent.GetHeight()+1)/2*Scale;
  double MaxZ = Parent.GetMaxRasterZ();
  MaxZ -= 0.01;
//  if( !MaxZ )  MaxZ = -0.0001;
  if( !FCeiling )  MaxZ = -MaxZ;

  P.Data[0][0] = -HW;
  P.Data[1][0] = -HH;
  P.Data[2][0] = MaxZ;
  P.Data[3][0] = FColors[0].GetRGB();

  P.Data[0][1] = HW;
  P.Data[1][1] = -HH;
  P.Data[2][1] = MaxZ;
  P.Data[3][1] = FColors[1].GetRGB();

  P.Data[0][2] = HW;
  P.Data[1][2] = HH;
  P.Data[2][2] = MaxZ;
  P.Data[3][2] = FColors[2].GetRGB();

  P.Data[0][3] = -HW;
  P.Data[1][3] = HH;
  P.Data[2][3] = MaxZ;
  P.Data[3][3] = FColors[3].GetRGB();
  return false;
}
//..............................................................................
void TGlBackground::LT(const TGlOption& v)  {
  GetPrimitives().GetStyle().SetParam("A", v.GetRGB(), true);
  FColors[0] = v;
}
//..............................................................................
void TGlBackground::RT(const TGlOption& v)  {
  GetPrimitives().GetStyle().SetParam("B", v.GetRGB(), true);
  FColors[1] = v;
}
//..............................................................................
void TGlBackground::RB(const TGlOption& v)  {
  GetPrimitives().GetStyle().SetParam("C", v.GetRGB(), true);
  FColors[2] = v;
}
//..............................................................................
void TGlBackground::LB(const TGlOption& v)  {
  GetPrimitives().GetStyle().SetParam("D", v.GetRGB(), true);
  FColors[3] = v;
}
//..............................................................................
