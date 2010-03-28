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
TWGlScene::~TWGlScene()
{
  Destroy();
}
//..............................................................................
void TWGlScene::SetPixelFormatDescriptor(HDC hDc, __int8 bits)
{
  int PixelFormat;
  if( !bits )
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
void TWGlScene::SetPixelFormatDescriptorX(HDC hDc, __int8 bits)
{
  int PixelFormat;
  if( !bits )
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
TGlFont* TWGlScene::CreateFont(const olxstr& name, const olxstr& fntDesc, short Flags)  {
  TGlFont *Fnt = FindFont(name);
  if( Fnt != NULL ) 
    Fnt->ClearData();
  else
    Fnt = new TGlFont(name);
  if( MetaFont::IsVectorFont(fntDesc) )  {
    olxdict<size_t, olxstr, TPrimitiveComparator> dummy;
    Fnt->CreateHershey(dummy, 120);
    if( FindFont(name) == NULL )
      Fonts.Add(Fnt);
    Fnt->SetIdString(fntDesc);
    MetaFont mf(fntDesc);
    Fnt->SetPointSize(mf.GetSize());
    return Fnt;
  }
  throw TNotImplementedException(__OlxSourceInfo);
  MetaFont meta_fnt(fntDesc);
  HFONT Font = (HFONT)NULL;
  Fnt->SetIdString("zzz");
  TPtrList<char*> Images;
  int ImageW = 36; //Font.GetPointSize()*1.5;
  RECT R = {0, 0, ImageW, ImageW};
  char *Image;
  TCHAR Char[1];
  BITMAPINFO Bmpi;

  HDC hDC = CreateCompatibleDC(NULL);
  HBITMAP Bmp = CreateCompatibleBitmap(hDC, ImageW, ImageW);
  if( !Bmp )  return NULL;
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
    Image = new char[4*(ImageW+1)*(ImageW+1)];

    GetDIBits(hDC, Bmp, 0, ImageW, NULL, &Bmpi, DIB_RGB_COLORS);
    GetDIBits(hDC, Bmp, 0, ImageW, Image, &Bmpi, DIB_RGB_COLORS);
    Fnt->CharFromRGBArray(i, (unsigned char*)Image, ImageW, ImageW);
    Images.Add(Image);
  }
  DeleteObject(Bmp);
  DeleteObject(Pen);
  DeleteObject(Brush);
  DeleteDC(hDC);
  Fnt->CreateGlyphsFromRGBArray(meta_fnt.IsFixed(), ImageW, ImageW);
  for( int i=0; i < 256; i++ )  // to find maximum height and width
    delete [] (char*)Images[i];
  return Fnt;
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
 
