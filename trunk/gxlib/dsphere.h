/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_glx_sphere_H
#define __olx_glx_sphere_H
#include "gxbase.h"
#include "glmousehandler.h"
#include "ematrix.h"
BeginGxlNamespace()

class TDSphere: public AGlMouseHandlerImp  {
public:
  struct PointAnalyser : public IEObject {
    virtual ~PointAnalyser()  {}
    virtual uint32_t Analyse(vec3f &p) = 0;
  };
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
  PointAnalyser& analyser;
  size_t Generation;  //6
  bool OnDblClick(const IEObject *, const TMouseData& Data);
public:
  TDSphere(TGlRenderer& Render, PointAnalyser& analyser,
    const olxstr& collectionName=EmptyString());
  ~TDSphere()  {  delete &analyser;  }
  void Create(const olxstr& cName=EmptyString());
  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d &Max, vec3d &Min) {  return false;  }
  double GetRadius() const;
  DefPropP(size_t, Generation)
  TEBasis Basis;
};

EndGxlNamespace()
#endif
