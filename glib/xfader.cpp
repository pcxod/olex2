/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "xfader.h"
#include "glmaterial.h"
#include "glrender.h"
#include "gpcollection.h"
#include "glprimitive.h"

TXFader::TXFader(TGlRenderer& R, const olxstr& collectionName):
  AGDrawObject(R, collectionName)
{
  SetSelectable(false);
  Foreground = Background = NULL;
  Step = 1./5;
  Position = 0;
  BGWidth = BGHeight = FGWidth = FGHeight = 0;
}
//..............................................................................
TXFader::~TXFader()  {
  if( Background != NULL )
    delete [] Background;
  if( Foreground != NULL )
    delete [] Foreground;
}
//..............................................................................
void TXFader::Create(const olxstr& cName)  {
  if( !cName.IsEmpty() )
    SetCollectionName(cName);

  TGPCollection& GPC = Parent.FindOrCreateCollection( GetCollectionName() );
  GPC.AddObject(*this);
  if( GPC.PrimitiveCount() != 0 )  return;

  TGlMaterial GlM;
  GlM.SetFlags(sglmAmbientF|sglmTransparent|sglmIdentityDraw);
  GlM.AmbientF = 0xffffffff;
  GlM.DiffuseF = 0xffffffff;
  //glBitmap
  TGlPrimitive& GlP = GPC.NewPrimitive("Quad", sgloQuads);
  GlP.SetProperties(GlM);
}
//..............................................................................
bool TXFader::Orient(TGlPrimitive&)  {
  if( Background == NULL && Foreground == NULL )  return true;
  double MaxZ = Parent.GetMaxRasterZ();
  //MaxZ = 0;
  double Scale = Parent.GetScale();
  olx_gl::pixelStore(GL_PACK_ALIGNMENT, 4);
  //glDrawBuffer(GL_BACK);
  if( Foreground != NULL )  {
    olx_gl::alphaFunc(GL_ALWAYS, 1);
    if( Background != NULL )  {
      olx_gl::rasterPos((-BGWidth/2)*Scale, (-BGHeight/2)*Scale, MaxZ-0.02);
      olx_gl::drawPixels(BGWidth, BGHeight, GL_RGBA, GL_UNSIGNED_BYTE, Background);
      const int sz = FGWidth*FGHeight*4;
      const char val = (char)((1-Position)*255);
      for( int i=0; i < sz; i+=4 )
        Foreground[i+3] = val;
      olx_gl::rasterPos((-FGWidth/2)*Scale, (-FGHeight/2)*Scale, MaxZ-0.01);
      olx_gl::drawPixels(FGWidth, FGHeight, GL_RGBA, GL_UNSIGNED_BYTE, Foreground);
    }
    else  {  // reverse
      if( Position == 1 )  return true;
      const int sz = FGWidth*FGHeight*4;
      const char val = (char)((1-Position)*255);
      for( int i=0; i < sz; i+=4 )
        Foreground[i+3] = val;
      olx_gl::rasterPos((0.1-(double)FGWidth/2)*Scale, (0.1-(double)FGHeight/2)*Scale, MaxZ-0.01);
      olx_gl::drawPixels(FGWidth, FGHeight, GL_RGBA, GL_UNSIGNED_BYTE, Foreground);
    }
  }
  else  {
    if( Background != NULL )  {
      olx_gl::rasterPos((-BGWidth/2)*Scale, (-BGHeight/2)*Scale, MaxZ-0.02);
      olx_gl::drawPixels(BGWidth, BGHeight, GL_RGBA, GL_UNSIGNED_BYTE, Background);
    }
  }
//  glDrawBuffer(GL_FRONT);
  return true;
}
//..............................................................................
void TXFader::BG2FG(bool zeropos)  {
  if( Background != NULL )  {
    if( Foreground != NULL && BGWidth == FGWidth && BGHeight == FGHeight )  {
      memcpy( Foreground, Background, BGWidth*BGHeight*4);
    }
    else  {
      if( Foreground != NULL )  delete [] Foreground;
      FGWidth = BGWidth;
      FGHeight = BGHeight;
      Foreground = new char [FGHeight*FGWidth*4];
      memcpy( Foreground, Background, BGWidth*BGHeight*4);
    }
    if( zeropos ) Position = 0;
  }
}
//..............................................................................
void TXFader::InitBG(bool v)  {
  if( v )  {
    if( Background != NULL )  {
      if( BGHeight != Parent.GetHeight() || BGWidth != Parent.GetWidth() )  {
        delete [] Foreground;
        Background = new char [Parent.GetHeight()*Parent.GetWidth()*4];
      }
    }
    else
      Background = new char [Parent.GetHeight()*Parent.GetWidth()*4];

    BGHeight = Parent.GetHeight();
    BGWidth = Parent.GetWidth();
    GLint oldmode;
    olx_gl::get(GL_DRAW_BUFFER, &oldmode);
//    glReadBuffer(GL_FRONT);
    olx_gl::readBuffer(GL_BACK);
    Parent.OnDraw.SetEnabled(false);
    bool vis = IsVisible();
    SetVisible(false);
    Parent.Draw();
    SetVisible(vis);
    Parent.OnDraw.SetEnabled(true);
    olx_gl::pixelStore(GL_PACK_ALIGNMENT, 4);
    olx_gl::readPixels(0, 0, BGWidth, BGHeight, GL_RGBA, GL_UNSIGNED_BYTE, Background);
    olx_gl::readBuffer(oldmode);
  }
  else  {
    if( Background != NULL )  {
      delete [] Background;
      Background = NULL;
    }
  }
}
//..............................................................................
void TXFader::InitFG(bool v)  {
  if( v )  {
    if( Foreground != NULL )  {
      if( FGHeight != Parent.GetHeight() || FGWidth != Parent.GetWidth() )  {
        delete [] Foreground;
        Foreground = new char [Parent.GetHeight()*Parent.GetWidth()*4];
      }
    }
    else
      Foreground = new char [Parent.GetHeight()*Parent.GetWidth()*4];

    FGHeight = Parent.GetHeight();
    FGWidth = Parent.GetWidth();
    GLint oldmode;
    olx_gl::get(GL_DRAW_BUFFER, &oldmode);
    olx_gl::readBuffer(GL_BACK);
    Parent.OnDraw.SetEnabled(false);
    bool vis = IsVisible();
    SetVisible(false);
    Parent.Draw();
    SetVisible(vis);
    Parent.OnDraw.SetEnabled(true);
    olx_gl::pixelStore(GL_PACK_ALIGNMENT, 4);
    olx_gl::readPixels(0, 0, FGWidth, FGHeight, GL_RGBA, GL_UNSIGNED_BYTE, Foreground);
    olx_gl::readBuffer(oldmode);
  }
  else  {
    if( Foreground != NULL )  {
      delete [] Foreground;
      Foreground = NULL;
    }
  }
}
//..............................................................................
//..............................................................................
//..............................................................................


//..............................................................................
void TXFader::LibStep(const TStrObjList& Params, TMacroError& E)  {
  if( !Params.IsEmpty() )  SetStep(Params[0].ToDouble());
  else                      E.SetRetVal(GetStep());
}
//..............................................................................
void TXFader::LibPosition(const TStrObjList& Params, TMacroError& E)  {
  if( !Params.IsEmpty() )  SetPosition(Params[0].ToDouble());
  else                      E.SetRetVal(GetPosition());
}
//..............................................................................
void TXFader::LibInitFG(const TStrObjList& Params, TMacroError& E)  {
  bool v = true;
  if( !Params.IsEmpty() )
    v = Params[0].ToBool();
  InitFG(v);
}
//..............................................................................
void TXFader::LibInitBG(const TStrObjList& Params, TMacroError& E)  {
  bool v = true;
  if( !Params.IsEmpty() )
    v = Params[0].ToBool();
  InitBG(v);
}
//..............................................................................
void TXFader::LibBG2FG(const TStrObjList& Params, TMacroError& E)  {
  bool v = true;
  if( !Params.IsEmpty() )
    v = Params[0].ToBool();
  BG2FG(v);
}
//..............................................................................


class TLibrary*  TXFader::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary(name);
  lib->Register(
    new TFunction<TXFader>(this,  &TXFader::LibStep, "Step",
      fpNone|fpOne,
    "Sets/returns xfader increment"));
  lib->Register(
    new TFunction<TXFader>(this,  &TXFader::LibPosition, "Position",
      fpNone|fpOne,
    "Sets/returns current xfader position"));
  lib->Register(
    new TFunction<TXFader>(this,  &TXFader::LibInitBG, "InitBG",
      fpNone|fpOne,
    "Initialises xfader background frame"));
  lib->Register(
    new TFunction<TXFader>(this,  &TXFader::LibInitFG, "InitFG",
      fpNone|fpOne,
    "Initialises xfader foreground frame"));
  lib->Register(
    new TFunction<TXFader>(this,  &TXFader::LibBG2FG, "BG2FG",
      fpNone|fpOne,
    "Copies current background frame to foreground frame"));
  AGDrawObject::ExportLibrary(*lib);
  return lib;
}
