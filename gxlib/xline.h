/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gxl_xline_H
#define __olx_gxl_xline_H
#include "xbond.h"
#include "xatom.h"
#include "glmousehandler.h"
BeginGxlNamespace()

class TXLine : protected TXBond, AGlMouseHandler {
private:
  vec3d FBase, FEdge;
  void Init(bool update_label=true);
  virtual bool IsMaskSaveable() const { return true; }
  virtual bool IsStyleSaveable() const { return true; }
  virtual bool IsRadiusSaveable() const { return true; }
protected:
  virtual bool DoTranslate(const vec3d& t);
  virtual bool DoRotate(const vec3d& vec, double angle) {
    return false;
  }
  virtual bool DoZoom(double zoom, bool inc) {
    return false;
  }
  virtual const TGlRenderer& DoGetRenderer() const {
    return GetParent();
  }

  bool OnMouseDown(const IOlxObject *, const TMouseData& Data)  {
    return GetHandler().OnMouseDown(*this, Data);
  }
  bool OnMouseUp(const IOlxObject *, const TMouseData& Data)  {
    return GetHandler().OnMouseUp(*this, Data);
  }
  bool OnMouseMove(const IOlxObject *, const TMouseData& Data)  {
    return GetHandler().OnMouseMove(*this, Data);
  }
  bool OnDblClick(const IOlxObject *, const TMouseData& Data)  {
    return GetHandler().OnDblClick(*this, Data);
  }
public:
  TXLine(TGlRenderer& Renderer, const olxstr& collectionName,
    const vec3d& base, const vec3d& edge);
  TXLine(TGlRenderer& Renderer);
  void Create(const olxstr& cName = EmptyString());
  virtual ~TXLine() {}

  const vec3d& GetBase() const { return FBase; }
  void setBase(const vec3d &v) { FBase = v; }
  const vec3d& GetEdge() const { return FEdge;  }
  void setEdge(const vec3d &v) { FEdge = v; }
  
  virtual const vec3d& GetFromCrd() const { return FBase; }
  virtual const vec3d& GetToCrd() const { return FEdge; }

  bool IsVisible() const {  return TXBond::IsVisible();  }
  void SetVisible(bool v) {  TXBond::SetVisible(v);  }

  bool GetDimensions(vec3d& Max, vec3d& Min)  {  return false;  }
  bool Orient(TGlPrimitive& P);
  void SetRadius(double V)  {  TXBond::SetRadius(V);  }
  double GetRadius() const {  return Params()[4]; }
  void SetLength(double V);
  double GetLength() const {  return Params()[3]; }

  void ToDataItem(TDataItem &di) const;
  void FromDataItem(const TDataItem &di);
  const_strlist ToPov(olx_cdict<TGlMaterial, olxstr> &materials) const {
    return TXBond::ToPov(materials);
  }
  const_strlist ToWrl(olx_cdict<TGlMaterial, olxstr> &materials) const {
    return TXBond::ToWrl(materials);
  }
  void UpdatePrimitives(int32_t mask)  {
    TXBond::UpdatePrimitives(mask);
  }
  uint32_t GetPrimitiveMask() const { return TXBond::GetPrimitiveMask(); }
  TGPCollection& GetPrimitives() const {  return TXBond::GetPrimitives();  }
  bool IsDeleted() const { return TXBond::IsDeleted(); }
  TXGlLabel& GetGlLabel() const {  return TXBond::GetGlLabel();  }
  void UpdateLabel();
  void Update();
};

EndGxlNamespace()
#endif
