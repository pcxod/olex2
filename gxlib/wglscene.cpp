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
void TWGlScene::SetPixelFormatDescriptor(HDC hDc, __int8 bits)  {
  int PixelFormat;
  if( bits == 0 )
    bits = 24;  // by default
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
  PixelFormat = ChoosePixelFormat(hDc, &pfd);
  SetPixelFormat(hDc, PixelFormat, &pfd);
}
//..............................................................................
void TWGlScene::SetPixelFormatDescriptorX(HDC hDc, __int8 bits) {
  int PixelFormat;
  if( bits == 0 )
    bits = 24;  // by default
  PIXELFORMATDESCRIPTOR pfd = {
    sizeof(PIXELFORMATDESCRIPTOR),
    1,
    PFD_DRAW_TO_WINDOW |	// support window
	  PFD_SUPPORT_OPENGL |	// support OpenGL
	  PFD_DOUBLEBUFFER,	// double buffered
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
TGlFont& TWGlScene::DoCreateFont(TGlFont& glf) const {
  glf.ClearData();
  if( MetaFont::IsVectorFont(glf.GetIdString()) )  {
    olxdict<size_t, olxstr, TPrimitiveComparator> dummy;
    glf.CreateHershey(dummy, 120);
    MetaFont mf;
    mf.SetIdString(glf.GetIdString());
    glf.SetPointSize(mf.GetSize());
    return glf;
  }
  throw TNotImplementedException(__OlxSourceInfo);
  MetaFont meta_fnt;
  meta_fnt.SetIdString(glf.GetIdString());
  HFONT Font = (HFONT)NULL;
  TPtrList<char> Images(256);
  int ImageW = 36; //Font.GetPointSize()*1.5;
  RECT R = {0, 0, ImageW, ImageW};
  TCHAR Char[1];
  BITMAPINFO Bmpi;
  HDC hDC = CreateCompatibleDC(NULL);
  HBITMAP Bmp = CreateCompatibleBitmap(hDC, ImageW, ImageW);
  if( Bmp == NULL ) 
    throw TFunctionFailedException(__OlxSourceInfo, "NULL handle");
  HBRUSH Brush = CreateSolidBrush(0);
  HPEN Pen = CreatePen(PS_SOLID, 1, 0);
  SelectObject(hDC, Font);
  SelectObject(hDC, Brush);
  SelectObject(hDC, Pen);
  for( int i=0; i < 256; i++ )  {
    SelectObject(hDC, Bmp);
    FillRect(hDC, &R, Brush);
    Char[0] = i;
    TextOut(hDC, 0, 0, Char, 1);
    SelectObject(hDC, NULL);
    char* Image = new char[4*(ImageW+1)*(ImageW+1)];
    GetDIBits(hDC, Bmp, 0, ImageW, NULL, &Bmpi, DIB_RGB_COLORS);
    GetDIBits(hDC, Bmp, 0, ImageW, Image, &Bmpi, DIB_RGB_COLORS);
    glf.CharFromRGBArray(i, (unsigned char*)Image, ImageW, ImageW);
    Images[i] = Image;
  }
  DeleteObject(Bmp);
  DeleteObject(Pen);
  DeleteObject(Brush);
  DeleteDC(hDC);
  glf.CreateGlyphsFromRGBArray(meta_fnt.IsFixed(), ImageW, ImageW);
  Images.Delete();
  return glf;
}
//..............................................................................
void TWGlScene::InitialiseBMP(HBITMAP Bmp)  {
  Destroy();
  if( FBitmap == NULL )
    throw TInvalidArgumentException(__OlxSourceInfo, "bitmap=NULL");
  FWContext = CreateCompatibleDC(NULL);
  FBitmap = Bmp;
  SelectObject(FWContext, FBitmap);
  SetPixelFormatDescriptor(FWContext, 24);
  FGlContext = wglCreateContext(FWContext);
  if( FGlContext == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "could not create gl context");
  if( wglMakeCurrent(FWContext, FGlContext) == FALSE )
    throw TFunctionFailedException(__OlxSourceInfo, "could not make current context");

  if( FWContext != NULL )  {
    DeleteDC(FWContext);
    FWContext = NULL;
  }
}
//..............................................................................
void TWGlScene::InitialiseHDC(HDC Dc)  {
  FBitmap = NULL;
  Destroy();
  FWContext = Dc;
  SetPixelFormatDescriptorX(FWContext, 24);
  FGlContext = wglCreateContext(FWContext);
  if( !FGlContext )
    throw TFunctionFailedException(__OlxSourceInfo, "could not create gl context");
  if( wglMakeCurrent(FWContext, FGlContext) == FALSE )
    throw TFunctionFailedException(__OlxSourceInfo, "could not make curernt context");
}
//..............................................................................
void TWGlScene::StartDraw()  {
  if( FBitmap != NULL )  {
    FWContext = CreateCompatibleDC(NULL);
    SelectObject(FWContext, FBitmap);
    SetPixelFormatDescriptor(FWContext, 24);
    if( wglMakeCurrent(FWContext, FGlContext) == FALSE )
      throw TFunctionFailedException(__OlxSourceInfo, "could not mak current context");
  }
  AGlScene::StartDraw();
}
//..............................................................................
void TWGlScene::EndDraw()  {
  AGlScene::EndDraw();
  if( FBitmap == NULL )  {
    //if( FWContext != NULL )
    //  SwapBuffers(FWContext);
  }
  else if( FWContext != NULL )  {
    SelectObject(FWContext, NULL);
    DeleteDC(FWContext);
    FWContext = NULL;
  }
}
//..............................................................................
void TWGlScene::StartSelect(int x, int y, GLuint *Bf)  {
  if( FBitmap != NULL )  {
    FWContext = CreateCompatibleDC(NULL);
    SelectObject(FWContext, FBitmap);
    SetPixelFormatDescriptor(FWContext, 24);
    if( wglMakeCurrent(FWContext, FGlContext) == FALSE )
      throw TFunctionFailedException(__OlxSourceInfo, "could not make current context");
  }
  AGlScene::StartSelect(x, y, Bf);
}
//..............................................................................
int TWGlScene::EndSelect()  {
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
void TWGlScene::Destroy()  {
  if( FGlContext != NULL )  {
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(FGlContext);
    FGlContext = NULL;
  }
  if( FWContext != NULL && FBitmap != NULL )  {
    DeleteDC(FWContext);
    FWContext = NULL;
  }
  AGlScene::Destroy();
}
//..............................................................................
#endif // end Win32 section
 
