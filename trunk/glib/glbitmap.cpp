/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "glbitmap.h"
#include "styles.h"
#include "gpcollection.h"
#include "gltexture.h"
#include "glprimitive.h"

TGlBitmap::TGlBitmap(TGlRenderer& Render, const olxstr& collectionName,
  int left, int top, unsigned int width, unsigned int height,
  unsigned char* RGB, GLenum format) :
AGlMouseHandlerImp(Render, collectionName)
{
  Z = 0.0;
  Zoom = 1;
  Left = left;
  Top = top;
  Width = width;
  Height = height;

  SetMove2D(true);
  SetMoveable(true);
  SetSelectable(false);
  SetRoteable(false);
  SetZoomable(false);
  TextureId = ~0;
  if( RGB != NULL )
    Init(RGB, format);
}
//.............................................................................
void TGlBitmap::Init(unsigned char* RGB, GLenum format)  {
  if( olx_is_valid_index(TextureId) )  return;
  TextureId = Parent.GetTextureManager().Add2DTexture(
    GetCollectionName(), 0, Width, Height, 0,
      format, RGB);

  TGlTexture* tex = Parent.GetTextureManager().FindTexture(TextureId);
  tex->SetEnvMode(tpeDecal);
  tex->SetSCrdWrapping(tpCrdClamp);
  tex->SetTCrdWrapping(tpCrdClamp);

  tex->SetMagFilter(tpFilterNearest);
  tex->SetMinFilter(tpFilterLinear);
  tex->SetEnabled(true);
}
//.............................................................................
void TGlBitmap::Create(const olxstr& cName)  {
  if( !cName.IsEmpty() )
    SetCollectionName(cName);

  //TGlTexture* tex = Parent.GetTextureManager().FindTexture(TextureId);
  //olxstr Name = EsdlObjectName(*this) + tex->GetName();
  TGPCollection& GPC = Parent.FindOrCreateCollection(GetCollectionName());
  GPC.AddObject(*this);
  if( GPC.PrimitiveCount() != 0 )  return;

  TGraphicsStyle& GS = GPC.GetStyle();
  Left = GS.GetParam("Left", Left).ToInt();
  Top = GS.GetParam("Top", Top).ToInt();
  Z = GS.GetParam("Z", Z).ToDouble();

  TGlMaterial GlM;

  GlM.SetFlags(0);
  GlM.ShininessF = 128;
  GlM.SetFlags(sglmAmbientF|sglmDiffuseF|sglmIdentityDraw|sglmTransparent);
  GlM.AmbientF = 0x800f0f0f;
  GlM.DiffuseF = 0x800f0f0f;

  TGlPrimitive& GlP = GPC.NewPrimitive("Plane", sgloQuads);  //
  GlP.SetTextureId( TextureId );
  GlP.SetProperties( GS.GetMaterial("Plane", GlM) );
  // texture coordinates
  GlP.TextureCrds.SetCount(4);
  GlP.Vertices.SetCount(4);
  GlP.TextureCrds[0].s = 0;  GlP.TextureCrds[0].t = 1;
  GlP.TextureCrds[1].s = 0;  GlP.TextureCrds[1].t = 0;
  GlP.TextureCrds[2].s = 1;  GlP.TextureCrds[2].t = 0;
  GlP.TextureCrds[3].s = 1;  GlP.TextureCrds[3].t = 1;
}
//.............................................................................
void TGlBitmap::ReplaceData(int width, int height, unsigned char* RGB,
  GLenum format)
{
  Width = width;
  Height = height;
  if( olx_is_valid_index(TextureId) )  {
    TGlTexture* tex = Parent.GetTextureManager().FindTexture(TextureId);
    Parent.GetTextureManager().Replace2DTexture(
      *tex, 0, width, height, 0, format, RGB);
  }
  else
    Init(RGB, format);
}
//.............................................................................
bool TGlBitmap::Orient(TGlPrimitive& P)  {
  P.SetTextureId(TextureId);
  double Scale = Parent.GetScale(),
    es = Parent.GetExtraZoom()*Parent.GetViewZoom();
  Scale *= es;
  const double hw = Parent.GetWidth()/(2*es), w = Width;
  const double hh = Parent.GetHeight()/(2*es), h = Height;
  double xx = GetCenter()[0], xy = -GetCenter()[1];
  P.Vertices[0] = vec3d((Left+w+xx-hw)*Scale, -(Top+h+xy-hh)*Scale, Z);
  P.Vertices[1] = vec3d(P.Vertices[0][0], -(Top+xy-hh)*Scale, Z);
  P.Vertices[2] = vec3d((Left+xx-hw)*Scale, -(Top+xy-hh)*Scale, Z);
  P.Vertices[3] = vec3d(P.Vertices[2][0], -(Top+h+xy-hh)*Scale, Z);
  return false;
}
//.............................................................................
bool TGlBitmap::GetDimensions(vec3d &Max, vec3d &Min)  {
  return false;
}
//.............................................................................
void TGlBitmap::SetWidth(unsigned int w)   {
  Width = w;
//  Primitives()->Style()->ParameterValue("Width") = w;
}
//.............................................................................
void TGlBitmap::SetHeight(unsigned int w)  {
  Height = w;
//  Primitives()->Style()->ParameterValue("Height") = w;
}
//.............................................................................
void TGlBitmap::SetLeft(int w)    {
  Left = w;
  GetPrimitives().GetStyle().SetParam("Left", w);
}
//.............................................................................
void TGlBitmap::SetTop(int w)     {
  Top = w;
  GetPrimitives().GetStyle().SetParam("Top", w);
}
//.............................................................................
void TGlBitmap::SetZ(double z)  {
  Z = z;
  GetPrimitives().GetStyle().SetParam("Z", z);
}
