/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_glx_ring_H
#define __olx_glx_ring_H
#include "gxbase.h"
#include "glmousehandler.h"
#include "ematrix.h"
BeginGxlNamespace()

class TDRing: public AGlMouseHandlerImp  {
protected:
  virtual bool DoTranslate(const vec3d& t) {
    Basis.Translate(t);  return true;
  }
  virtual bool DoRotate(const vec3d& vec, double angle) {
    Basis.Rotate(vec, angle);
    return true;
  }
  virtual bool DoZoom(double zoom, bool inc)  {
    if( inc )  Basis.SetZoom(ValidateZoom(Basis.GetZoom() + zoom));
    else       Basis.SetZoom(ValidateZoom(zoom));
    return true;
  }
  class TContextClear: public AActionHandler  {
  public:
    TContextClear(TGlRenderer& Render);
    bool Enter(const IEObject *Sender, const IEObject *Data, TActionQueue *);
  };
  static TGlPrimitive* torus;
  static bool initialised;
public:
  TDRing(TGlRenderer& Render, const olxstr& collectionName);
  void Create(const olxstr& cName=EmptyString());
  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d &Max, vec3d &Min) {  return false;  }
  double GetRadius() const;
  TEBasis Basis;
  TGlMaterial material;
};

EndGxlNamespace()
#endif