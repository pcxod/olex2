/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gl_dfame_H
#define __olx_gl_dfame_H
#include "glbase.h"
#include "gdrawobject.h"
#include "actions.h"
BeginGlNamespace()

// this class is passed a aparameter to the OSelect handlers
class TSelectionInfo: public IEObject  {
public:
  vec3d From, To;
};

class TDFrame: public AGDrawObject  {
protected:
  class TGlPrimitive* FPrimitive;
  vec3f Translation;
  TActionQList Actions;
public:
  TDFrame(TGlRenderer& Render, const olxstr& collectionName);
  virtual ~TDFrame()  {}
  void Create(const olxstr& cName=EmptyString());
  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d&, vec3d&){  return false; }

  bool OnMouseDown(const IEObject *Sender, const TMouseData& Data);
  bool OnMouseUp(const IEObject *Sender, const TMouseData& Data);
  bool OnMouseMove(const IEObject *Sender, const TMouseData& Data);

  TActionQueue& OnSelect;
};

EndGlNamespace()
#endif
