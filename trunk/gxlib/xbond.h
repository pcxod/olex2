/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gxl_xbond_H
#define __olx_gxl_xbond_H
#include "glrender.h"
#include "gdrawobject.h"
#include "glprimitive.h"
#include "gllabel.h"
#include "styles.h"
#include "sbond.h"
#include "talist.h"
BeginGxlNamespace()

class TXAtom;

class TXBond: public TSBond, public AGDrawObject  {
private:
  TXGlLabel* Label;
  bool label_forced;
  int FDrawStyle;
protected:
  void GetDefSphereMaterial(TGlMaterial &M);
  void GetDefRimMaterial(TGlMaterial &M);
  virtual bool IsMaskSaveable() const {  return false;  }
  virtual bool IsStyleSaveable() const {  return false; }
  virtual bool IsRadiusSaveable() const {  return false; }
public:
  TXBond(TNetwork* net, TGlRenderer& Render, const olxstr& collectionName);
  void Create(const olxstr& cName=EmptyString());
  virtual ~TXBond();

  // multiple inheritance...
  void SetTag(index_t v) {  TSBond::SetTag(v);  }
  index_t GetTag() const {  return TSBond::GetTag();  }
  index_t IncTag()  {  return TSBond::IncTag();  }
  index_t DecTag()  {  return TSBond::DecTag();  }

  TXAtom& A() const {  return (TXAtom&)TSBond::A();  }
  TXAtom& B() const {  return (TXAtom&)TSBond::B();  }
  TXAtom& Another(const TSAtom& a) const {
    return (TXAtom&)TSBond::Another(a);
  }

  TXGlLabel& GetGlLabel() const {  return *Label;  }
  void UpdateLabel()  {  GetGlLabel().UpdateLabel();  }
  // creates legend up three levels (0 to 2)
  static olxstr GetLegend(const TSBond& B, size_t level);

  void SetRadius(double V);
  double GetRadius() const {  return FParams[4]; }

  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d &, vec3d &)  {  return false; }

  // for parameters of a specific primitive
  void ListParams(TStrList &List, TGlPrimitive *Primitive);
  // for internal object parameters
  void ListParams(TStrList &List);
  // fills the list with proposal primitives to construct object
  void ListPrimitives(TStrList &List) const;
  // updates primitive properties from atoms
  void UpdateStyle();

  bool OnMouseDown(const IEObject *Sender, const TMouseData& Data);
  bool OnMouseUp(const IEObject *Sender, const TMouseData& Data);
  bool OnMouseMove(const IEObject *Sender, const TMouseData& Data);

  void SetVisible(bool v) {
    AGDrawObject::SetVisible(v);
    if (!v) {
      if (Label->IsVisible()) {
        Label->SetVisible(false);
        label_forced = true;
      }
    }
    else if (label_forced) {
      Label->SetVisible(true);
      label_forced = false;
    }
  }
  void Delete() {
    TSBond::SetDeleted(true);
    SetVisible(false);
  }
  void ListDrawingStyles(TStrList &List);
  uint32_t GetPrimitiveMask() const;
  short DrawStyle() const {  return FDrawStyle; }

  void UpdatePrimitiveParams(TGlPrimitive *Primitive);
  static int Quality(TGlRenderer &r, int Val);
  // should be called when atom coordinates have changed
  virtual void Update();

  const_strlist ToPov(olx_cdict<TGlMaterial, olxstr> &materials) const;
  static const_strlist PovDeclare(TGlRenderer &p);

  const_strlist ToWrl(olx_cdict<TGlMaterial, olxstr> &materials) const;
  static const_strlist WrlDeclare(TGlRenderer &p);

  virtual const vec3d &GetBaseCrd() const;

  static olxstr_dict<olxstr> &NamesRegistry() {
    static olxstr_dict<olxstr> nr;
    return nr;
  }

  class Settings : public AGOSettings {
    mutable double radius;
    mutable int mask, cone_q, cone_stipples, quality;
    Settings(TGlRenderer &r)
      : AGOSettings(r, "BondParams")
    {
      set_defaults();
    }
    void set_defaults() {
      radius = -1;
      mask = cone_q = cone_stipples = -1;
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
    double GetRadius() const { return GetParam("DefR", radius, double(1)); }
    void SetRadius(double v) const {
      return style->SetParam("DefR", (radius = v), true);
    }
    int GetConeQ() const { return GetParam("ConeQ", cone_q, int(15)); }
    void SetConeQ(int v) const {
      return style->SetParam("ConeQ", (cone_q = v), true);
    }
    int GetConeStipples() const {
      return GetParam("ConeStipples", cone_stipples, int(6));
    }
    int GetMask() const { return GetParam("DefM", mask, int(7)); }
    void SetMask(int v) { style->SetParam("DefM", (mask = v), true); }
    const TStringToList<olxstr, TGlPrimitive*> &GetPrimitives() const {
      return primitives;
    }
    const TStringToList<olxstr, TGlPrimitive*> &GetPrimitives(bool check) {
      if (check && primitives.IsEmpty()) {
        CreatePrimitives();
      }
      return primitives;
    }
    void ClearPrimitives() {
      primitives.Clear();
    }
    static Settings& GetInstance(TGlRenderer &r) {
      AGOSettings *s = r.FindSettings("bond");
      if (s == NULL) {
        return (Settings &)r.RegisterSettings(*(new Settings(r)), "bond");
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
    settings = &GetSettings(Parent);
  }
  virtual void OnPrimitivesCleared() {
    GetSettings().ClearPrimitives();
  }
};

EndGxlNamespace()
#endif
