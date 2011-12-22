/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gl_glbackground_H
#define __olx_gl_glbackground_H
#include "glbase.h"
#include "gdrawobject.h"
#include "gloption.h"
#include "glprimitive.h"
#include "gltexture.h"
BeginGlNamespace()

class TGlBackground: public AGDrawObject  {
  TGlOption  FColors[4];
  bool FCeiling;
  TGlTexture* Texture;
public:
  TGlBackground(TGlRenderer& Render, const olxstr& collectionName, bool Ceiling);
  virtual ~TGlBackground(){  return; };
  void Create(const olxstr& cName=EmptyString());
  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d &Max, vec3d &Min){  return false;};
  bool Ceiling(){  return FCeiling;  }
  void SetTexture(TGlTexture* glt);
  TGlTexture* GetTexture() const {  return Texture;  }

  void LT(const TGlOption& v);
  void RT(const TGlOption& v);
  void RB(const TGlOption& v);
  void LB(const TGlOption& v);

  const TGlOption& LT() const {  return FColors[0];  }
  const TGlOption& RT() const {  return FColors[1];  }
  const TGlOption& RB() const {  return FColors[2];  }
  const TGlOption& LB() const {  return FColors[3];  }
};


EndGlNamespace()
#endif
