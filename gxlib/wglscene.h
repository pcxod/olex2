//---------------------------------------------------------------------------
#ifndef wglsceneH
#define wglsceneH
#include "glscene.h" 
//---------------------------------------------------------------------------
// system dependent staff
//__________________________________________
#if defined(__WIN32__)
class TWGlScene:public AGlScene  {
private:
  HGLRC FGlContext;
  HDC   FWContext;
  HBITMAP FBitmap;

  void SetPixelFormatDescriptor(HDC hDc, __int8 bits);
  void SetPixelFormatDescriptorX(HDC hDc, __int8 bits);
protected:
//  GLYPHMETRICSFLOAT *FGlyphMetrics;  // represents the metrics of generateg font
//  bool CreateBmpFont(HDC Dc, HFONT Font);
//  bool CreateTtfFont(HDC Dc, HFONT Font);
//  float FFontExtrusionZ;   // specifies the width of the font in Z direction
public:
  TWGlScene();
  virtual ~TWGlScene();

  void InitialiseBMP(HBITMAP Bmp, HFONT Font);
  void InitialiseHDC(HDC Dc, HFONT Font);

  TGlFont* CreateFont(const olxstr& name, void *Data, TGlFont *Replace=NULL, bool Bmp=true, bool FixedW=true);

  void Destroy();

  void StartSelect(int x, int y, GLuint *Bf);
  int EndSelect();

  void StartDraw();
  void EndDraw();
};
#endif
//---------------------------------------------------------------------------

#endif
