/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gl_bitmap_H
#define __olx_gl_bitmap_H
#include "glbase.h"
#include "glmousehandler.h"
BeginGlNamespace()

class TGlBitmap : public AGlMouseHandlerImp  {
  unsigned int Width, Height;
  int Top, Left; // to clip the content
  GLuint TextureId;
  double Z;
protected:
  vec3d Center;
  double Zoom;
  virtual bool DoTranslate(const vec3d& t) {  Center += t;  return true;  }
  virtual bool DoRotate(const vec3d&, double) {  return false;  }
  virtual bool DoZoom(double zoom, bool inc)  {
    if( inc ) Zoom = ValidateZoom(Zoom+zoom);
    else      Zoom = ValidateZoom(zoom);
    return true;
  }
  const vec3d& GetCenter() const {  return Center;  }
  double GetZoom() const {  return Zoom;  }
  void Init(unsigned char* RGB, GLenum format);
public:
  TGlBitmap(TGlRenderer& Render, const olxstr& collectionName,
    int left, int top, unsigned int width, unsigned int height,
      unsigned char* RGB, GLenum format);

  void ReplaceData(int width, int height, unsigned char* RGB, GLenum format);

  void Create(const olxstr& cName=EmptyString());
  virtual ~TGlBitmap()  {}

  void SetZ(double z);
  double GetZ() const {  return Z;  }
  void SetZoom(double v)  {  DoZoom(v, false);  }
  void SetWidth(unsigned int w);
  unsigned int GetWidth() const {  return (unsigned int)(Width*GetZoom());  }
  void SetHeight(unsigned int w);
  unsigned int GetHeight() const {  return (unsigned int)(Height*GetZoom());  }
  void SetLeft(int w);
  int GetLeft() const {  return Left; }
  void SetTop(int w);
  int GetTop() const {  return Top; }

  virtual bool Orient(TGlPrimitive& P);
  virtual bool GetDimensions(vec3d& Max, vec3d& Min);
};

EndGlNamespace()
#endif
