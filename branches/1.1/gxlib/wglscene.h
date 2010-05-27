#ifndef __olx_gxl_wglscene_H
#define __olx_gxl_wglscene_H
#include "glscene.h" 
#if defined(__WIN32__)
class TWGlScene:public AGlScene  {
  HGLRC FGlContext;
  HDC   FWContext;
  HBITMAP FBitmap;
  void SetPixelFormatDescriptor(HDC hDc, __int8 bits);
  void SetPixelFormatDescriptorX(HDC hDc, __int8 bits);
public:
  TWGlScene();
  virtual ~TWGlScene();

  void InitialiseBMP(HBITMAP Bmp);
  void InitialiseHDC(HDC Dc);

  virtual TGlFont* CreateFont(const olxstr& name, const olxstr& fntDescription, short Flags=TGlFont::fntBmp);

  void Destroy();

  void StartSelect(int x, int y, GLuint *Bf);
  int EndSelect();

  void StartDraw();
  void EndDraw();
  virtual void ScaleFonts(double scale)  {
    throw TNotImplementedException(__OlxSourceInfo);
  }
  // restores the font sizes after a call to the ScaleFonts
  virtual void RestoreFontScale()  {
    throw TNotImplementedException(__OlxSourceInfo);
  }
  virtual olxstr ShowFontDialog(TGlFont* glf = NULL, const olxstr& fontDescription = EmptyString)  {
    throw TNotImplementedException(__OlxSourceInfo);
  }
};
#endif // __WIN32__
#endif
