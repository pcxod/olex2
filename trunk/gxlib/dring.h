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
  
  static int Quality(TGlRenderer &r, int v);

  class Settings : public AGOSettings {
    mutable double tube_r;
    mutable int mask, tube_s, torus_s;
    Settings(TGlRenderer &r)
      : AGOSettings(r, "RingParams")
    {
      set_defaults();
    }
    void set_defaults() {
      tube_r = -1;
      mask = tube_s = torus_s = QualityValue = -1;
    }
    void OnStyleChange() {
      set_defaults();
      ClearPrimitives();
    }
    void OnRendererClear() {
      ClearPrimitives();
    }
    void CreatePrimitives();
    TStringToList<olxstr, TGlPrimitive*> primitives;
  public:
    double GetTubeR() const {
      return GetParam("TubeRadius", tube_r, double(0.075));
    }
    void SetTubeR(double v) const {
      return style->SetParam("TubeRadius", (tube_r = v), true);
    }

    int GetMask() const { return GetParam("DefM", mask, int(7)); }
    int GetTubeS() const { return GetParam("TubeSections", tube_s, int(8)); }
    void SetTubeS(int v) const {
      return style->SetParam("TubeSections", (tube_s = v), true);
    }
    int GetTorusS() const { return GetParam("TorusSections", torus_s, int(8)); }
    void SetTorusS(int v) const {
      return style->SetParam("TorusSections", (torus_s = v), true);
    }
    int QualityValue;
    const TStringToList<olxstr, TGlPrimitive*> GetPrimitives() const {
      return primitives;
    }
    const TStringToList<olxstr, TGlPrimitive*> GetPrimitives(bool check) {
      if (check && primitives.IsEmpty()) {
        CreatePrimitives();
      }
      return primitives;
    }

    void ClearPrimitives() {
      primitives.Clear();
    }
    static Settings& GetInstance(TGlRenderer &r) {
      AGOSettings *s = r.FindSettings("ring");
      if (s == NULL) {
        return (Settings &)r.RegisterSettings(*(new Settings(r)), "ring");
      }
      else {
        return dynamic_cast<Settings &>(*s);
      }
    }
  };
protected:
  mutable Settings *settings;
public:
  Settings &GetSettings() const {
    return *(settings == 0 ? (settings = &Settings::GetInstance(Parent))
      : settings);
  }
  static Settings &GetSettings(TGlRenderer &r) {
    return Settings::GetInstance(r);
  }
  virtual void OnStyleChange() {
    settings = &Settings::GetInstance(Parent);
  }
  virtual void OnPrimitivesCleared() {
    GetSettings().ClearPrimitives();
  }

};

EndGxlNamespace()
#endif
