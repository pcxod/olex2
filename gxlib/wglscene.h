/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gxl_wglscene_H
#define __olx_gxl_wglscene_H
#include "glscene.h" 

#if defined(__WIN32__)
class TWGlScene:public AGlScene  {
  HGLRC FGlContext;
  HDC   FWContext;
  HBITMAP FBitmap;
  void SetPixelFormatDescriptor(HDC hDc, uint8_t bits);
  void SetPixelFormatDescriptorX(HDC hDc, uint8_t bits);
  virtual TGlFont& DoCreateFont(TGlFont& fnt, bool half_size) const;
public:
  TWGlScene();
  virtual ~TWGlScene()  {  Destroy();  }
  void InitialiseBMP(HBITMAP Bmp, uint8_t bpp=24);
  void InitialiseHDC(HDC Dc);
  HDC GetDC() const { return FWContext; }
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
  virtual olxstr ShowFontDialog(TGlFont* glf = NULL,
    const olxstr& fontDescription=EmptyString())
  {
    throw TNotImplementedException(__OlxSourceInfo);
  }
  virtual bool MakeCurrent();
};
#endif // __WIN32__
#endif
