/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gxl_dbasis_H
#define __olx_gxl_dbasis_H
#include "gllabel.h"
#include "asymmunit.h"
BeginGxlNamespace()

class TDBasis: public AGlMouseHandlerImp, public TXGlLabel::ICrdTransformer  {
  TAsymmUnit* AU;
protected:
  TXGlLabel* Labels[3];
  virtual vec3d ForRaster(const TXGlLabel&) const;
  virtual vec3d ForVector(const TXGlLabel&) const;
  virtual vec3d& AdjustZ(vec3d&) const;
protected:
  vec3d _Center;
  double Zoom;
  virtual bool DoTranslate(const vec3d& t) {  _Center += t;  return true;  }
  virtual bool DoRotate(const vec3d& vec, double angle) {  return false;  }
  virtual bool DoZoom(double zoom, bool inc)  {
    if( inc ) Zoom = ValidateZoom(Zoom+zoom);
    else      Zoom = ValidateZoom(zoom);
    return true;
    return true;
  }
public:
  TDBasis(TGlRenderer& Render, const olxstr& collectionName);
  virtual ~TDBasis();
  void SetAsymmUnit(TAsymmUnit& au);
  void Create(const olxstr& cName=EmptyString());
  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d& Max, vec3d& Min)  {  return false;  }
  void ListPrimitives(TStrList& List) const;
  void UpdatePrimitives(int32_t Mask);

  void UpdateLabel();
  size_t LabelCount() const {  return 3;  }
  TXGlLabel& GetLabel(size_t i) const {  return *Labels[i];  }
  void SetVisible(bool v);
  const vec3d& GetCenter() const {  return _Center;  }
  double GetZoom() const {  return Zoom;  }

  void ToDataItem(TDataItem& di) const;
  void FromDataItem(const TDataItem& di);
};

EndGxlNamespace()
#endif
