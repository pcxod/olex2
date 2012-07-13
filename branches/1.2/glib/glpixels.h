/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gl_pixels_H
#define __olx_gl_pixels_H
#include "glbase.h"
#include "glmousehandler.h"
BeginGlNamespace()

class TGlPixels : public AGDrawObject  {
  int Width, Height;
  int Top, Left;
  double Z;
  unsigned char *Data;
  GLint DataFormat;
  void Init(unsigned char *data, int w, int h);
public:
  TGlPixels(TGlRenderer& Render, const olxstr& collectionName,
    int left, int top, int width, int height,
      unsigned char* data, GLenum format);
  ~TGlPixels()  {
    if( Data != NULL )
      delete [] Data;
  }
  void SetData(int width, int height, unsigned char* data, GLenum format);

  void Create(const olxstr& cName=EmptyString());

  void SetZ(double z);
  double GetZ() const {  return Z;  }
  void SetWidth(unsigned int w);
  unsigned int GetWidth() const {  return Width;  }
  void SetHeight(unsigned int w);
  unsigned int GetHeight() const {  return Height;  }
  void SetLeft(int w);
  int GetLeft() const {  return Left; }
  void SetTop(int w);
  int GetTop() const {  return Top; }

  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d& Max, vec3d& Min)  {
    return false;
  }
};

EndGlNamespace()
#endif
