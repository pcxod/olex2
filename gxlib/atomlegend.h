/******************************************************************************
* Copyright (c) 2004-2016 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gxl_atomlabel_H
#define __olx_gxl_atomlabel_H

#include "gxbase.h"
#include "glmousehandler.h"
#include "gltexture.h"
#include "glrender.h"
BeginGxlNamespace()

class TAtomLegend : public AGlMouseHandlerImp {
  int Width, Height;
  int Top, Left;
  GLuint TextureId;
  double Z;
  TGlMaterial GlM;
  olx_array_ptr<uint8_t> tex_data;
protected:
  vec3d Center;
  virtual bool DoTranslate(const vec3d& t) { Center += t;  return true; }
  virtual bool DoRotate(const vec3d&, double) { return false; }
  virtual bool DoZoom(double zoom, bool inc) { return false; }
  const vec3d& GetCenter() const { return Center; }
  void Init(unsigned char* RGB, GLenum format);
public:
  TAtomLegend(TGlRenderer& Render, const olxstr& collectionName);
  void SetData(unsigned char* RGB, GLsizei width, GLsizei height,
    GLenum format);
  void Create(const olxstr& cName = EmptyString());
  void SetMaterial(const TGlMaterial &m) { GlM = m; }
  void Fit();
  virtual bool Orient(TGlPrimitive& P);
  virtual bool GetDimensions(vec3d &, vec3d &) { return false; }
  bool OnMouseUp(const IOlxObject *Sender, const TMouseData& Data);
  virtual void Update();
  virtual void UpdateLabel() { Fit(); }
  TStrList text;
};
EndGxlNamespace()

#endif
