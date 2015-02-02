/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gxl_xlattice_H
#define __olx_gxl_xlattice_H
#include "gxbase.h"
#include "glmousehandler.h"
#include "asymmunit.h"
BeginGxlNamespace()

class TXLattice: public AGlMouseHandlerImp  {
  bool Fixed;
  short Size;
  class TGlPrimitive* Lines;
  mat3d LatticeBasis;
protected:
  TEBasis Basis;
  virtual bool DoTranslate(const vec3d& t) {  Basis.Translate(t);  return true;  }
  virtual bool DoRotate(const vec3d& vec, double angle) {  Basis.Rotate(vec, angle);  return true;  }
  virtual bool DoZoom(double zoom, bool inc)  {
    if( inc )  Basis.SetZoom(ValidateZoom(Basis.GetZoom() + zoom));
    else       Basis.SetZoom(ValidateZoom(zoom));
    return true;
  }
public:
  TXLattice(TGlRenderer& Render, const olxstr& collectionName);
  virtual ~TXLattice() {}
  void Create(const olxstr& cName=EmptyString());

  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d& Max, vec3d& Min);

  DefPropC(mat3d, LatticeBasis)

  inline bool IsFixed() const {  return Fixed;  }
  void SetFixed(bool v );

  inline short GetSize() const {  return Size;  }
  void SetSize(short v);

  bool OnMouseDown(const IEObject *Sender, const TMouseData& Data);
  bool OnMouseUp(const IEObject *Sender, const TMouseData& Data);
  bool OnMouseMove(const IEObject *Sender, const TMouseData& Data);
};

EndGxlNamespace()
#endif
