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
#include "glrender.h"
#include "glmousehandler.h"
#include "ematrix.h"
#include "dataitem.h"
BeginGxlNamespace()

class TDRing: public AGlMouseHandlerImp  {
protected:
  virtual bool DoTranslate(const vec3d& t) {
    Basis.Translate(t);
    return true;
  }
  virtual bool DoRotate(const vec3d& vec, double angle) {
    Basis.Rotate(vec, angle);
    return true;
  }
  virtual bool DoZoom(double zoom, bool inc)  {
    if (inc)
      Basis.SetZoom(ValidateZoom(Basis.GetZoom() + zoom));
    else
      Basis.SetZoom(ValidateZoom(zoom));
    return true;
  }
  class TContextClear: public AActionHandler {
  public:
    TContextClear(TGlRenderer& Render);
    bool Enter(const IEObject *Sender, const IEObject *Data, TActionQueue *);
  };
  static bool & Initialised() {
    static bool i = false;
    return i;
  }
  class TStylesClear : public AActionHandler  {
  public:
    TStylesClear(TGlRenderer& Render)  { Render.OnStylesClear.Add(this); }
    bool Enter(const IEObject *Sender, const IEObject *Data, TActionQueue *);
    bool Exit(const IEObject *Sender, const IEObject *Data, TActionQueue *);
  };
  static void ValidateGlobalStyle();
  static TGraphicsStyle *&GlobalStyle() {
    static TGraphicsStyle *p = NULL;
    return p;
  }
  static double &DefTubeRadius() {
    static double r = -1;
    return r;
  }
public:
  TDRing(TGlRenderer& Render, const olxstr& collectionName);
  void Create(const olxstr& cName=EmptyString());
  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d &Max, vec3d &Min) { return false; }
  void Update();
  double GetRadius() const;
  void ToDataItem(TDataItem &i) const;
  void FromDataItem(const TDataItem &i);
  TEBasis Basis;
  TGlMaterial material;

  static TStringToList<olxstr, TGlPrimitive*> &GetStaticPrimitives() {
    static TStringToList<olxstr, TGlPrimitive*> sp;
    return sp;
  }
  static void CreateStaticObjects(TGlRenderer &r);
  static void ClearStaticObjects() {
    GetStaticPrimitives().Clear();
  }

  static double GetDefTubeRadius();
  static void SetDefTubeRadius(double v);
  static int16_t Quality(int16_t v);
};

EndGxlNamespace()
#endif
