/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "glpixels.h"
#include "styles.h"
#include "gpcollection.h"
#include "gltexture.h"
#include "glprimitive.h"

TGlPixels::TGlPixels(TGlRenderer& Render, const olxstr& collectionName,
  int left, int top, int width, int height,
  unsigned char* data, GLenum format)
  : AGDrawObject(Render, collectionName)  
{
  Z = 0;
  Left = left;
  Top = top;
  Data = NULL;
  Width = Height = 0;
  Init(data, width, height);
  DataFormat = format;
}
//.............................................................................
void TGlPixels::Create(const olxstr& cName)  {
  if( !cName.IsEmpty() )  
    SetCollectionName(cName);

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
  GlM.SetFlags(sglmAmbientF|sglmDiffuseF|sglmIdentityDraw);
  GlM.AmbientF = 0x800f0f0f;
  GlM.DiffuseF = 0x800f0f0f;

  TGlPrimitive& GlP = GPC.NewPrimitive("Plane", sgloQuads);  //
  GlP.SetProperties(GS.GetMaterial("Plane", GlM));
  GlP.Vertices.SetCount(4);
}
//.............................................................................
void TGlPixels::Init(unsigned char *data, int w, int h)  {
  if( Data != NULL && (Width != w || Height != h) )  {
    delete [] Data;
    Data = NULL;
  }
  if( Data == NULL && data != NULL )
    Data = new unsigned char[w*h*4];
  if( data != NULL )  {
    for( int i=0; i < h; i++ )
      for( int j=0; j < w; j++ )  {
        const int src_i = ((h-i)*w+j)*4;
        const int dest_i = (i*w+j)*4;
        Data[dest_i+0] = data[src_i+2];
        Data[dest_i+1] = data[src_i+1];
        Data[dest_i+2] = data[src_i+0];
        Data[dest_i+3] = 0xff;
      }
      //memcpy(&Data[i], &data[i], w*4);
    Width = w;
    Height = h;
  }
}
//.............................................................................
void TGlPixels::SetData(int width, int height, unsigned char* data, GLenum format) {
  Init(data, width, height);
  DataFormat = format;
}
//.............................................................................
bool TGlPixels::Orient(TGlPrimitive& P)  {
  //double hw = Parent.GetWidth()/2;
  //double hh = Parent.GetHeight()/2;
  //P.Vertices[0] = vec3d((Left+Width)-hw, hh-(Top+Height), Z);
  //P.Vertices[1] = vec3d(P.Vertices[0][0], hh-Top, Z);
  //P.Vertices[2] = vec3d(Left-hw, P.Vertices[1][1], Z);
  //P.Vertices[3] = vec3d(P.Vertices[2][0], P.Vertices[0][1], Z);
  //olx_gl::scale(Parent.GetScale()*Parent.GetExtraZoom());
  //return false;
  if( Data != NULL )  {
    //double MaxZ = Parent.GetMaxRasterZ();
    double Scale = Parent.GetScale();
    olx_gl::rasterPos((-Width/2)*Scale, (-Height/2)*Scale, 0.0);
    olx_gl::pixelStore(GL_PACK_ALIGNMENT, 4);
    //olx_gl::drawBuffer(GL_BACK);
    //olx_gl::rasterPos(Left, Top, 0);
    olx_gl::drawPixels(Width, Height, DataFormat, GL_UNSIGNED_BYTE, Data); 
  }
  return true;
}
//.............................................................................
void TGlPixels::SetWidth(unsigned int w)   {
  Width = w;
//  Primitives()->Style()->ParameterValue("Width") = w;
}
//.............................................................................
void TGlPixels::SetHeight(unsigned int w)  {
  Height = w;
//  Primitives()->Style()->ParameterValue("Height") = w;
}
//.............................................................................
void TGlPixels::SetLeft(int w)    {
  Left = w;
  GetPrimitives().GetStyle().SetParam("Left", w);
}
//.............................................................................
void TGlPixels::SetTop(int w)     {
  Top = w;
  GetPrimitives().GetStyle().SetParam("Top", w);
}
//.............................................................................
