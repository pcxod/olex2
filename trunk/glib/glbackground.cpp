/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "glbackground.h"
#include "glmaterial.h"
#include "gpcollection.h"
#include "styles.h"
UseGlNamespace()

TGlBackground::TGlBackground(TGlRenderer& r, const olxstr& collectionName, bool ceiling):
  AGDrawObject(r, collectionName)
{
  FCeiling = ceiling;
  SetSelectable(false);
  Texture = NULL;
}
//..............................................................................
void TGlBackground::Create(const olxstr& cName) {
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
  GlP.Vertices.SetCount(4);
  GlP.Colors.SetCount(4);
  GlP.TextureCrds.SetCount(4);
  GlP.TextureCrds[0].s = 1;  GlP.TextureCrds[0].t = 1;
  GlP.TextureCrds[1].s = 0;  GlP.TextureCrds[1].t = 1;
  GlP.TextureCrds[2].s = 0;  GlP.TextureCrds[2].t = 0;
  GlP.TextureCrds[3].s = 1;  GlP.TextureCrds[3].t = 0;
}
//..............................................................................
void TGlBackground::SetTexture(TGlTexture* tx)  {
  Texture = tx;
}
//..............................................................................
bool TGlBackground::Orient(TGlPrimitive& P)  {
  if( Parent.IsColorStereo() )  return true;
  if( Texture != NULL )
    P.SetTextureId(Texture->GetId());
  double Scale = Parent.GetScale();
  double HW = (Parent.GetWidth()+1)/2*Scale;
  double HH = (Parent.GetHeight()+1)/2*Scale;
  double MaxZ = Parent.CalcRasterZ(0.001);
  if( !FCeiling )  MaxZ = -MaxZ;

  P.Vertices[0] = vec3d(-HW, -HH, MaxZ);
  P.Colors[0] = FColors[0].GetRGB();
  P.Vertices[1] = vec3d(HW, -HH, MaxZ);
  P.Colors[1] = FColors[1].GetRGB();
  P.Vertices[2] = vec3d(HW, HH, MaxZ);
  P.Colors[2] = FColors[2].GetRGB();
  P.Vertices[3] = vec3d(-HW, HH, MaxZ);
  P.Colors[3] = FColors[3].GetRGB();
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
