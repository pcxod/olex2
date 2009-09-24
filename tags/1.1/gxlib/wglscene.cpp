#ifdef __BORLANDC__
#pragma hdrstop
#endif

#define WGL_WGLEXT_PROTOTYPES

#include "wglscene.h"
#include "glfont.h"
#include "exception.h"
#include "glrender.h"
#include "glscene.h"

//---------------------------------------------------------------------------
// TGlScene
//---------------------------------------------------------------------------
#if defined(__WIN32__)
TWGlScene::TWGlScene()
{
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
TGlFont* TWGlScene::CreateFont(const olxstr& name, void *Data, TGlFont *ReplaceFnt,
         bool BmpF, bool FixedW)  {
  TGlFont *Fnt;
  HFONT Font = (HFONT)Data;
  if( ReplaceFnt != NULL )  {
    Fnt = ReplaceFnt;
    Fnt->ClearData();
  }
  else
    Fnt = new TGlFont(name);

  Fnt->IdString("zzz");
  TEList Images;
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
  Fnt->CreateGlyphs(FixedW, ImageW, ImageW);
  for( int i=0; i < 256; i++ )  // to find maximum height and width
    delete [] (char*)Images[i];

  if( ReplaceFnt == NULL )
    Fonts.Add(Fnt);

  return Fnt;
}
//..............................................................................
/*
int TWGlScene::TtfTextWidth(const olxstr &Text)
{
  if( !FGlyphMetrics )  return -1;
  int FCalcWidth = 0;
  int i, tl = Text.Length();
  GLYPHMETRICSFLOAT *GM;
  for( i=0; i < tl; i++ )
  {
    GM = &FGlyphMetrics[Text[i]];
    if( i < (tl - 1) )  { FCalcWidth += GM->gmfCellIncX;  }
    else                { FCalcWidth += GM->gmfBlackBoxX; }
  }
  return FCalcWidth;
}
//..............................................................................
int TWGlScene::TtfTextHeight(const olxstr &Text)
{
  if( !FGlyphMetrics )  return -1;
  int FCalcHeight = 0;
  int i, tl = Text.Length();
  GLYPHMETRICSFLOAT *GM;
  for( i=0; i < tl; i++ )
  {
    GM = &FGlyphMetrics[Text[i]];
    if( i < (tl - 1) )  { FCalcHeight += GM->gmfCellIncY; }
    else                { FCalcHeight += GM->gmfBlackBoxY; }
  }
  return FCalcHeight;
}
//..............................................................................
bool TWGlScene::CreateBmpFont(void *Data)
{
  if( FWContext )  return CreateBmpFont(FWContext, (HFONT)Data);
  else
  {
    HDC dc = CreateCompatibleDC(NULL);
    return CreateBmpFont(dc, (HFONT)Data);
  }
}
//..............................................................................
bool TWGlScene::CreateTtfFont(void *Data)
{
  if( FWContext )  return CreateTtfFont(FWContext, (HFONT)Data);
  else
  {
    HDC dc = CreateCompatibleDC(NULL);
    return CreateTtfFont(dc, (HFONT)Data);
  }
}
//..............................................................................
bool TWGlScene::CreateBmpFont(HDC Dc, HFONT Font)
{
  bool res = true;
  if( FBmpFontBase == -1 )
  {
    SelectObject(Dc, Font);
    if( wglUseFontBitmaps(Dc, 0, 256, 5000) == TRUE )
    {    FBmpFontBase = 5000;  }
    else
    {    FBmpFontBase = -1;  res = false; }
  }
  return res;
}
//..............................................................................
bool TWGlScene::CreateTtfFont(HDC Dc, HFONT Font)
{
  bool res = true;
  if( FTtfFontBase == -1 )
  {
    SelectObject(Dc, Font);
    if( !FGlyphMetrics )    FGlyphMetrics = new GLYPHMETRICSFLOAT[256];
    if( wglUseFontOutlines(Dc, 0, 255, 6000, 5.0f, FFontExtrusionZ, WGL_FONT_POLYGONS, FGlyphMetrics) == TRUE )
    {    FTtfFontBase = 6000;  }
    else
    {    FTtfFontBase = -1;  delete [] FGlyphMetrics;  FGlyphMetrics = NULL; res = false; }
  }
  return res;
} */
//..............................................................................
void TWGlScene::InitialiseBMP(HBITMAP Bmp, HFONT Font)
{
  Destroy();
  FWContext = CreateCompatibleDC(NULL);
  FBitmap = Bmp;
  if( !FBitmap )
    throw TInvalidArgumentException(__OlxSourceInfo, "bitmap=NULL");
  SelectObject(FWContext, FBitmap);
  SetPixelFormatDescriptor(FWContext, 24);
  FGlContext = wglCreateContext(FWContext);
  if( !FGlContext )
    throw TFunctionFailedException(__OlxSourceInfo, "could not create gl context");
  if( wglMakeCurrent(FWContext, FGlContext) == FALSE )
    throw TFunctionFailedException(__OlxSourceInfo, "could not make current context");

  // create fonts
//  CreateBmpFont(FWContext, Font);
//  CreateTtfFont(FWContext, Font);

  if( FWContext )
  {
    DeleteDC(FWContext);
    FWContext = NULL;
  }
}
//..............................................................................
void TWGlScene::InitialiseHDC(HDC Dc, HFONT Font)
{
  FBitmap = NULL;
  Destroy();
  FWContext = Dc;
  SetPixelFormatDescriptorX(FWContext, 24);
  FGlContext = wglCreateContext(FWContext);
  if( !FGlContext )
    throw TFunctionFailedException(__OlxSourceInfo, "could not create gl context");
  if( wglMakeCurrent(FWContext, FGlContext) == FALSE )
    throw TFunctionFailedException(__OlxSourceInfo, "could not make curernt context");

  // create fonts
//  CreateBmpFont(Dc, Font);
//  CreateTtfFont(Dc, Font);

  FParent->SetView();
}
//..............................................................................
void TWGlScene::StartDraw()
{
  if( FBitmap )
  {
    FWContext = CreateCompatibleDC(NULL);
    SelectObject(FWContext, FBitmap);
    SetPixelFormatDescriptor(FWContext, 24);
    if( wglMakeCurrent(FWContext, FGlContext) == FALSE )
      throw TFunctionFailedException(__OlxSourceInfo, "could not mak current context");
    FParent->SetView();
  }
  AGlScene::StartDraw();
}
//..............................................................................
void TWGlScene::EndDraw()
{
  AGlScene::EndDraw();
  if( !FBitmap )
  {
    if( FWContext )  SwapBuffers(FWContext);
    return;
  }
  if( FWContext )
  {
    SelectObject(FWContext, NULL);
    DeleteDC(FWContext);
    FWContext = NULL;
  }
}
//..............................................................................
void TWGlScene::StartSelect(int x, int y, GLuint *Bf)
{
  if( FBitmap )
  {
    FWContext = CreateCompatibleDC(NULL);
    SelectObject(FWContext, FBitmap);
    SetPixelFormatDescriptor(FWContext, 24);
    if( wglMakeCurrent(FWContext, FGlContext) == FALSE )
      throw TFunctionFailedException(__OlxSourceInfo, "could not make current context");
  }
  AGlScene::StartSelect(x, y, Bf);
}
//..............................................................................
void TWGlScene::EndSelect()
{
  AGlScene::EndSelect();
  if( !FBitmap  )
  {

    if( FWContext )  SwapBuffers(FWContext);
    FParent->SetView();
    return;
  }
  if( FWContext )
  {
    SelectObject(FWContext, NULL);
    DeleteDC(FWContext);
    FWContext = NULL;
  }
}
//..............................................................................
void TWGlScene::Destroy()
{
//  if( FBmpFontBase > 0 )
//  { glDeleteLists(FBmpFontBase, 255);  FBmpFontBase = -1; }
//  if( FTtfFontBase > 0 )
//  { glDeleteLists(FTtfFontBase, 255);  FTtfFontBase = -1; }
//  if( FGlyphMetrics )
//  {    delete [] FGlyphMetrics;  FGlyphMetrics = NULL; }
  if( FGlContext)
  {
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(FGlContext);
    FGlContext = NULL;
  }
  if( FWContext && FBitmap )
  {
    DeleteDC(FWContext);
    FWContext = NULL;
  }
  AGlScene::Destroy();
}
//..............................................................................
/*bool TWGlScene::CreateBmpFont(HFONT Font)
{
  HDC hDc = CreateCompatibleDC(NULL);
  SelectObject(hDc, Font);
  if( wglUseFontBitmaps(hDc, 0, 256, 5000) == TRUE )
  {    FBmpFontBase = 5000;  }
  else
  {    FBmpFontBase = -1;  }
  DeleteDC(hDc);
  if( FBmpFontBase < 0 )  return false;
  return true;
}
//..............................................................................
bool TWGlScene::CreateTtfFont(HFONT Font)
{
  HDC hDc = CreateCompatibleDC(NULL);
  SelectObject(hDc, Font);

  if( wglUseFontOutlines(hDc, 0, 255, 6000, 0.0f, 0.1f, WGL_FONT_POLYGONS, NULL) == TRUE )
  {    FTtfFontBase = 6000;  }
  else
  {    FTtfFontBase = -1;  }
  DeleteDC(hDc);
  if( FTtfFontBase < 0 )  return false;
  return true;
} */
//..............................................................................
#endif // end Win32 section
 
