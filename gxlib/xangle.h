/******************************************************************************
* Copyright (c) 2004-2024 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/
#pragma once
#include "gxbase.h"
#include "glrender.h"
#include "gdrawobject.h"
#include "ematrix.h"
#include "edict.h"
#include "gllabel.h"
BeginGxlNamespace()

class TXAngle : public AGDrawObject {
  TXGlLabel* Label;
  TGlMaterial material;
  vec3d center, from, to;
  virtual bool IsMaskSaveable() const { return true; }
  virtual bool IsStyleSaveable() const { return true; }
  virtual bool IsRadiusSaveable() const { return true; }
protected:
  public:
  TXAngle(TGlRenderer& Render, const olxstr& collectionName,
    const vec3d& cnt, const vec3d &from, const vec3d& to);
  TXAngle(TGlRenderer& Render, const TDataItem &di);
  virtual ~TXAngle();
  void SetMaterial(const olxstr& mat) { material.FromString(mat); }
  void SetMaterial(const TGlMaterial& glm) { material = glm; }
  void Create(const olxstr& cName = EmptyString());
  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d& Max, vec3d& Min) { return false; }

  void Update();
  void ListParams(TStrList& List, TGlPrimitive* Primitive);
  // for internal object parameters
  void ListParams(TStrList& List);
  // fills the list with proposal primitives to construct object
  void ListPrimitives(TStrList& List) const;
  void ListDrawingStyles(TStrList& List);
  uint32_t GetPrimitiveMask() const;

  void UpdatePrimitiveParams(TGlPrimitive* Primitive);
  void SetVisible(bool v);
  void ToDataItem(TDataItem& di) const;
  void FromDataItem(const TDataItem& di);
  TXGlLabel& GetGlLabel() const { return *Label; }
  void UpdateLabel() { GetGlLabel().UpdateLabel(); }
  //const_strlist ToPov(olx_cdict<TGlMaterial, olxstr>& materials) const;
  const vec3d &GetCenter() const {
    return center;
  }
  const vec3d& GetFrom() const {
    return from;
  }
  const vec3d& GetTo() const {
    return to;
  }
  void SetCenter(const vec3d& v) {
    center = v;
  }
  void SetFrom(const vec3d& v) {
    from = v;
  }
  void SetTo(const vec3d& v) {
    to = v;
  }

  class Settings : public AGOSettings {
    mutable int mask, sections;
    Settings(TGlRenderer& r)
      : AGOSettings(r, "AngleParams")
    {
      set_defaults();
    }
    void set_defaults() {
      mask = sections = -1;
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
    int GetMask() const { return GetParam("DefM", mask, int(1)); }
    void SetMask(int v) { style->SetParam("DefM", (mask = v), true); }
    int GetSections() const { return GetParam("Sections", sections, int(10)); }
    void SetSections(int v) {
      style->SetParam("Sections", (sections = v), true);
    }
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

    static Settings& GetInstance(TGlRenderer& r) {
      AGOSettings* s = r.FindSettings("angle");
      if (s == 0) {
        return (Settings&)r.RegisterSettings(*(new Settings(r)), "angle");
      }
      else {
        return dynamic_cast<Settings&>(*s);
      }
    }
  };
protected:
  mutable Settings* settings;
public:
  Settings& GetSettings() const {
    return *(settings == 0 ? (settings = &Settings::GetInstance(Parent))
      : settings);
  }
  static Settings& GetSettings(TGlRenderer& r) {
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

