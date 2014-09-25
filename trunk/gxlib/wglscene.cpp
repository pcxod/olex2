/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "wglscene.h"
#include "glfont.h"
#include "exception.h"
#include "glrender.h"
#include "glscene.h"

#if defined(__WIN32__)

TWGlScene::TWGlScene() {
  FBitmap = NULL;
  FWContext = NULL;
  FGlContext = NULL;
//  FGlyphMetrics = NULL;

//  FFontExtrusionZ = 0.1f;
}
//..............................................................................
void TWGlScene::SetPixelFormatDescriptor(HDC hDc, uint8_t bits) {
  if (bits == 0) bits = 24;
  PIXELFORMATDESCRIPTOR pfd = {
    sizeof(PIXELFORMATDESCRIPTOR),
    1,
    PFD_SUPPORT_OPENGL | PFD_SUPPORT_GDI | PFD_DRAW_TO_BITMAP,
    PFD_TYPE_RGBA,
    bits,
    0,0,0,0,0,0,
    0,0,
    0,0,0,0,0,
    32,
    0,
    0,
    PFD_MAIN_PLANE,
    0,
    0,0,
  };
  int PixelFormat = ChoosePixelFormat(hDc, &pfd);
  SetPixelFormat(hDc, PixelFormat, &pfd);
}
//..............................................................................
void TWGlScene::SetPixelFormatDescriptorX(HDC hDc, uint8_t bits) {
  int PixelFormat;
  if( bits == 0 )
    bits = 24;  // by default
  PIXELFORMATDESCRIPTOR pfd = {
    sizeof(PIXELFORMATDESCRIPTOR),
    1,
    PFD_DRAW_TO_WINDOW | // support window
    PFD_SUPPORT_OPENGL |  // support OpenGL
    PFD_DOUBLEBUFFER,    // double buffered
    PFD_TYPE_RGBA,
    bits,
    0,0,0,0,0,0,
    0,0,
    0,0,0,0,0,
    32,
    0,
    0,
    PFD_MAIN_PLANE,
    0,
    0,0,
  };
  PixelFormat = ChoosePixelFormat(hDc, &pfd);
  SetPixelFormat(hDc, PixelFormat, &pfd);
//  const char * (*GetExtensions)(HDC) = (WINAPI)wglGetProcAddress("wglGetEntensionsStringARB");
}
//..............................................................................
TGlFont& TWGlScene::DoCreateFont(TGlFont& glf, bool half_size) const {
  glf.ClearData();
  if( MetaFont::IsVectorFont(glf.GetIdString()) )  {
    olx_pdict<size_t, olxstr> dummy;
    glf.CreateHershey(dummy, 120);
    MetaFont mf;
    mf.SetIdString(glf.GetIdString());
    glf.SetPointSize(mf.GetSize());
    return glf;
  }
  //throw TNotImplementedException(__OlxSourceInfo);
  MetaFont meta_fnt;
  meta_fnt.SetIdString(glf.GetIdString());
  int fs = 24;
  HFONT Font = ::CreateFontW(half_size ? fs/2 : fs, 0, 0, 0, FW_NORMAL,
    FALSE, FALSE, FALSE,
    DEFAULT_CHARSET,
    OUT_RASTER_PRECIS,
    CLIP_CHARACTER_PRECIS,
    NONANTIALIASED_QUALITY,
    FIXED_PITCH,
    L"Times New Roman");
  TPtrList<unsigned char> Images(256);
  int ImageW = 32; //Font.GetPointSize()*1.5;
  RECT R = {0, 0, ImageW, ImageW};
  TCHAR Char[1];
  BITMAPINFO Bmpi;
  memset(&Bmpi, 0, sizeof(Bmpi));
  Bmpi.bmiHeader.biSize = sizeof(Bmpi);
  HDC hDC = CreateCompatibleDC(NULL);
  HBITMAP Bmp = CreateCompatibleBitmap(hDC, ImageW, ImageW);
  if( Bmp == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "NULL handle");
  HBRUSH Brush = CreateSolidBrush(0xffffff);
  SetTextColor(hDC, 0);
  SelectObject(hDC, Font);
  SelectObject(hDC, Bmp);
  GetDIBits(hDC, Bmp, 0, ImageW, NULL, &Bmpi, DIB_RGB_COLORS);
  Bmpi.bmiHeader.biBitCount = 24;
  Bmpi.bmiHeader.biCompression = BI_RGB;
  for( int i=0; i < 256; i++ )  {
    FillRect(hDC, &R, Brush);
    Char[0] = i;
    TextOut(hDC, 0, 0, Char, 1);
    unsigned char* Image = new unsigned char[3*ImageW*ImageW];
    GetDIBits(hDC, Bmp, 0, ImageW, Image, &Bmpi, DIB_RGB_COLORS);
    for (int ix=0; ix < ImageW/2; ix++) {
      size_t offt = ix*ImageW;
      size_t offb = (ImageW-ix-1)*ImageW;
      for (int jx=0; jx < ImageW; jx++) {
        int off1 = (offt+jx)*3;
        int off2 = (offb+jx)*3;
        olx_swap(Image[off1+0], Image[off2+0]);
        olx_swap(Image[off1+1], Image[off2+1]);
        olx_swap(Image[off1+2], Image[off2+2]);
      }
    }
    glf.CharFromRGBArray(i, Image, ImageW, ImageW);
    Images[i] = Image;
  }
  ::SelectObject(hDC, NULL);
  ::DeleteObject(Bmp);
  ::DeleteObject(Brush);
  ::DeleteObject(Font);
  DeleteDC(hDC);
  glf.CreateGlyphsFromRGBArray(meta_fnt.IsFixed(), ImageW, ImageW);
  Images.DeleteItems();
  return glf;
}
//..............................................................................
void TWGlScene::InitialiseBMP(HBITMAP Bmp, uint8_t bpp) {
  if (Bmp == NULL)
    throw TInvalidArgumentException(__OlxSourceInfo, "bitmap=NULL");
  Destroy();
  FBitmap = Bmp;
  FWContext = CreateCompatibleDC(NULL);
  SelectObject(FWContext, FBitmap);
  SetPixelFormatDescriptor(FWContext, bpp);
  FGlContext = wglCreateContext(FWContext);
  if (FGlContext == NULL) {
    throw TFunctionFailedException(__OlxSourceInfo,
     "could not create gl context");
  }
  if (!MakeCurrent()) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "could not make current context");
  }
}
//..............................................................................
void TWGlScene::InitialiseHDC(HDC Dc) {
  FBitmap = NULL;
  Destroy();
  FWContext = Dc;
  SetPixelFormatDescriptorX(FWContext, 24);
  FGlContext = wglCreateContext(FWContext);
  if (FGlContext ==NULL) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "could not create gl context");
  }
  MakeCurrent();
}
//..............................................................................
bool TWGlScene::MakeCurrent() {
  if (FWContext != NULL && FGlContext != NULL)
    return wglMakeCurrent(FWContext, FGlContext) != FALSE;
  return false;
}
//..............................................................................
void TWGlScene::StartDraw() {
  AGlScene::StartDraw();
}
//..............................................................................
void TWGlScene::EndDraw() {
  AGlScene::EndDraw();
}
//..............................................................................
void TWGlScene::StartSelect(int x, int y, GLuint *Bf) {
  AGlScene::StartSelect(x, y, Bf);
}
//..............................................................................
int TWGlScene::EndSelect() {
  const int rv = AGlScene::EndSelect();
  return rv;
  //if( FBitmap == NULL )  {
  //  if( FWContext != NULL )
  //    SwapBuffers(FWContext);
  //  FParent->SetView();
  //}
  //else if( FWContext != NULL )  {
  //  SelectObject(FWContext, NULL);
  //  DeleteDC(FWContext);
  //  FWContext = NULL;
  //}
}
//..............................................................................
void TWGlScene::Destroy() {
  if (FGlContext != NULL) {
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(FGlContext);
    FGlContext = NULL;
  }
  if (FWContext != NULL && FBitmap != NULL) {
    SelectObject(FWContext, NULL);
    DeleteDC(FWContext);
    FWContext = NULL;
  }
  AGlScene::Destroy();
}
//..............................................................................
#endif // end Win32 section
