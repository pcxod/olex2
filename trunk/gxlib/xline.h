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
BeginGxlNamespace()

class TXLine: protected TXBond  {
  vec3d FBase, FEdge;
  void Init(bool update_label=true);
public:
  TXLine(TGlRenderer& Renderer, const olxstr& collectionName,
    const vec3d& base, const vec3d& edge);
  TXLine(TGlRenderer& Renderer) : TXBond(NULL, Renderer, EmptyString()) {}
  void Create(const olxstr& cName=EmptyString()) {
    TXBond::Create(cName);
  }
  virtual ~TXLine() {}

  vec3d& Base()  {  return FBase;  }
  vec3d& Edge()  {  return FEdge;  }

  bool IsVisible() const {  return TXBond::IsVisible();  }
  void SetVisible(bool v) {  TXBond::SetVisible(v);  }

  double Length() const {  return FBase.DistanceTo(FEdge);  }

  bool GetDimensions(vec3d& Max, vec3d& Min)  {  return false;  }
  bool Orient(TGlPrimitive& P);
  void SetRadius(double V)  {  TXBond::SetRadius(V);  }
  double GetRadius() const {  return Params()[4]; }
  void SetLength(double V)  {  Params()[3] = V;  }
  double GetLength() const {  return Params()[3]; }

  const vec3d &GetBaseCrd() const {  return FBase;  }
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
};

EndGxlNamespace()
#endif
