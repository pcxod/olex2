#ifdef __BORLANDC__
  #pragma hdrstop
#endif

#include "xfader.h"
#include "glmaterial.h"
#include "glrender.h"
#include "gpcollection.h"


UseGlNamespace()
//..............................................................................
//..............................................................................

TXFader::TXFader(const olxstr& collectionName, TGlRender *Render):
    AGDrawObject(collectionName)  {
  AGDrawObject::Parent(Render);
  Groupable(false);
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
  if( cName.Length() )  SetCollectionName(cName);
  TGlPrimitive *GlP;
  TGPCollection *GPC;
  GlM.SetFlags(sglmAmbientF|sglmTransparent|sglmIdentityDraw);
  GlM.AmbientF = 0xffffffff;
  GlM.DiffuseF = 0xffffffff;

  GPC = FParent->FindCollection( GetCollectionName() );
  if( !GPC )    GPC = FParent->NewCollection( GetCollectionName() );
  GPC->AddObject(this);

  //glBitmap
  GlP = GPC->NewPrimitive("Quad");
  GlP->SetProperties( &GlM );
  GlP->Type(sgloQuads);
  GlP->Data().Resize(4, 4);
}
//..............................................................................
bool TXFader::Orient(TGlPrimitive *P)  {
  if( Background == NULL && Foreground == NULL )  return true;
  double MaxZ = FParent->GetMaxRasterZ();
//  MaxZ -= 0.015;
  double Scale = FParent->GetScale();
  glPixelStorei(GL_PACK_ALIGNMENT, 4);
  //glDrawBuffer(GL_BACK);
  if( Foreground != NULL )  {
    glAlphaFunc(GL_ALWAYS, 1);
    if( Background != NULL )  {
      glRasterPos3d((-BGWidth/2)*Scale, (-BGHeight/2)*Scale, MaxZ-0.02);
      glDrawPixels(BGWidth, BGHeight, GL_RGBA, GL_UNSIGNED_BYTE, Background);
      const int sz = FGWidth*FGHeight*4;
      const char val = (char)((1-Position)*255);
      for( int i=0; i < sz; i+=4 )
        Foreground[i+3] = val;
      glRasterPos3d((-FGWidth/2)*Scale, (-FGHeight/2)*Scale, MaxZ-0.01);
      glDrawPixels(FGWidth, FGHeight, GL_RGBA, GL_UNSIGNED_BYTE, Foreground);
    }
    else  {  // reverse
      if( Position == 1 )  return true;
      const int sz = FGWidth*FGHeight*4;
      const char val = (char)((1-Position)*255);
      for( int i=0; i < sz; i+=4 )
        Foreground[i+3] = val;
      glRasterPos3d((0.1-(double)FGWidth/2)*Scale, (0.1-(double)FGHeight/2)*Scale, MaxZ-0.01);
      glDrawPixels(FGWidth, FGHeight, GL_RGBA, GL_UNSIGNED_BYTE, Foreground);
    }
  }
  else  {
    if( Background != NULL )  {
      glRasterPos3d((-BGWidth/2)*Scale, (-BGHeight/2)*Scale, MaxZ-0.02);
      glDrawPixels(BGWidth, BGHeight, GL_RGBA, GL_UNSIGNED_BYTE, Background);
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
      if( BGHeight != FParent->GetHeight() || BGWidth != FParent->GetWidth() )  {
        delete [] Foreground;
        Background = new char [FParent->GetHeight()*FParent->GetWidth()*4];
      }
    }
    else
      Background = new char [FParent->GetHeight()*FParent->GetWidth()*4];

    BGHeight = FParent->GetHeight();
    BGWidth = FParent->GetWidth();
    GLint oldmode;
    glGetIntegerv(GL_DRAW_BUFFER, &oldmode);
//    glReadBuffer(GL_FRONT);
    glReadBuffer(GL_BACK);
    FParent->OnDraw->SetEnabled(false);
    bool vis = Visible();
    Visible(false);
    FParent->Draw();
    Visible(vis);
    FParent->OnDraw->SetEnabled(true);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glReadPixels(0, 0, BGWidth, BGHeight, GL_RGBA, GL_UNSIGNED_BYTE, Background);
    glReadBuffer(oldmode);
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
      if( FGHeight != FParent->GetHeight() || FGWidth != FParent->GetWidth() )  {
        delete [] Foreground;
        Foreground = new char [FParent->GetHeight()*FParent->GetWidth()*4];
      }
    }
    else
      Foreground = new char [FParent->GetHeight()*FParent->GetWidth()*4];

    FGHeight = FParent->GetHeight();
    FGWidth = FParent->GetWidth();
    GLint oldmode;
    glGetIntegerv(GL_DRAW_BUFFER, &oldmode);
    glReadBuffer(GL_BACK);
    FParent->OnDraw->SetEnabled(false);
    bool vis = Visible();
    Visible(false);
    FParent->Draw();
    Visible(vis);
    FParent->OnDraw->SetEnabled(true);
    glPixelStorei(GL_PACK_ALIGNMENT, 4);
    glReadPixels(0, 0, FGWidth, FGHeight, GL_RGBA, GL_UNSIGNED_BYTE, Foreground);
    glReadBuffer(oldmode);
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
  if( Params.Count() !=0 )  SetStep( Params[0].ToDouble() );
  else                      E.SetRetVal( GetStep() );
}
//..............................................................................
void TXFader::LibPosition(const TStrObjList& Params, TMacroError& E)  {
  if( Params.Count() !=0 )  SetPosition( Params[0].ToDouble() );
  else                      E.SetRetVal( GetPosition() );
}
//..............................................................................
void TXFader::LibInitFG(const TStrObjList& Params, TMacroError& E)  {
  bool v = true;
  if( Params.Count() != 0 )
    v = Params[0].ToBool();
  InitFG(v);
}
//..............................................................................
void TXFader::LibInitBG(const TStrObjList& Params, TMacroError& E)  {
  bool v = true;
  if( Params.Count() != 0 )
    v = Params[0].ToBool();
  InitBG(v);
}
//..............................................................................
void TXFader::LibBG2FG(const TStrObjList& Params, TMacroError& E)  {
  bool v = true;
  if( Params.Count() != 0 )
    v = Params[0].ToBool();
  BG2FG(v);
}
//..............................................................................


class TLibrary*  TXFader::ExportLibrary(const olxstr& name)  {
  TLibrary* lib = new TLibrary(name);
  lib->RegisterFunction<TXFader>( new TFunction<TXFader>(this,  &TXFader::LibStep, "Step", fpNone|fpOne,
"Sets/returns xfader increment") );
  lib->RegisterFunction<TXFader>( new TFunction<TXFader>(this,  &TXFader::LibPosition, "Position", fpNone|fpOne,
"Sets/returns current xfader position") );
  lib->RegisterFunction<TXFader>( new TFunction<TXFader>(this,  &TXFader::LibInitBG, "InitBG", fpNone|fpOne,
"Initialises xfader background frame") );
  lib->RegisterFunction<TXFader>( new TFunction<TXFader>(this,  &TXFader::LibInitFG, "InitFG", fpNone|fpOne,
"Initialises xfader foreground frame") );
  lib->RegisterFunction<TXFader>( new TFunction<TXFader>(this,  &TXFader::LibBG2FG, "BG2FG", fpNone|fpOne,
"Copies current background frame to foreground frame") );
  AGDrawObject::ExportLibrary(*lib);
  return lib;
}

