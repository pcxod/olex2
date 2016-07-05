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
#include "solid_angles.h"
BeginGxlNamespace()

class TDSphere: public AGlMouseHandlerImp  {
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
  APointAnalyser* analyser;
  size_t Generation;  //6
  bool OnDblClick(const IOlxObject *, const TMouseData& Data);
  bool OnMouseDown(const IOlxObject *, const TMouseData& Data)  {
    return AGlMouseHandlerImp::OnMouseDown(this, Data);
  }
  bool OnMouseUp(const IOlxObject *, const TMouseData& Data)  {
    return AGlMouseHandlerImp::OnMouseUp(this, Data);
  }
  TGlPrimitive &CreatePrimitive(TGPCollection &collection,
    const olxstr &name, size_t gen, bool update_vec_cnt);
  TGlPrimitive *lowq, *highq;
  size_t vec_cnt;
public:
  TDSphere(TGlRenderer& Render, const olxstr& collectionName=EmptyString());
  ~TDSphere()  {
    if (analyser != NULL) {
      delete analyser;
    }
  }
  void Create(const olxstr& cName=EmptyString());
  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d &Max, vec3d &Min) {  return false;  }
  double GetRadius() const;
  DefPropP(size_t, Generation)
  size_t GetVectorCount() const { return vec_cnt; }
  // the one must be created with new and will be managed by this object
  void SetAnalyser(APointAnalyser *a) {
    if (analyser != NULL) {
      delete analyser;
    }
    analyser = a;
  }
  void ToDataItem(TDataItem &di) const;
  void FromDataItem(const TDataItem &di);
  TEBasis Basis;
};

EndGxlNamespace()
#endif
