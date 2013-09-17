/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gxl_label_H
#define __olx_gxl_label_H
#include "gxbase.h"
#include "glmousehandler.h"
#include "glfont.h"
#include "dataitem.h"
BeginGxlNamespace()

class TXGlLabel: public AGlMouseHandlerImp  {
public:
  class ICrdTransformer  {
  public:
    virtual vec3d ForRaster(const TXGlLabel&) const = 0;
    virtual vec3d ForVector(const TXGlLabel&) const = 0;
    // returns the argument after the adjustment
    virtual vec3d& AdjustZ(vec3d& v) const = 0;
  };
private:
  olxstr FLabel;
  size_t FontIndex;
  TTextRect text_rect;
  vec3d Offset;
  ICrdTransformer* Transformer;
protected:
  vec3d _Center;
  virtual bool DoTranslate(const vec3d& t)  {  _Center += t;  return true;  }
  virtual bool DoRotate(const vec3d&, double)  {  return false;  }
  virtual bool DoZoom(double, bool)  {  return false;  }
public:
  TXGlLabel(TGlRenderer& Render, const olxstr& collectionName);
  void Create(const olxstr& cName=EmptyString());
  virtual ~TXGlLabel()  {}

  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d &, vec3d &)  {  return false;  }
  inline const olxstr& GetLabel() const {  return FLabel;  }
  void SetLabel(const olxstr& L);
  void UpdateLabel()  {  SetLabel(GetLabel());  }
  const TTextRect& GetRect() const {  return text_rect;  }
  vec3d GetRasterPosition() const;
  vec3d GetVectorPosition() const;
  // the object must be mannaged by whoever created it!
  DefPropP(ICrdTransformer*, Transformer)
  DefPropC(vec3d, Offset)
  TGlFont& GetFont() const;
  DefPropP(size_t, FontIndex)
  void TranslateBasis(const vec3d& v)  {  DoTranslate(v);  }
  const vec3d& GetCenter() const {  return _Center;  }
  void ToDataItem(TDataItem& item) const;
  void FromDataItem(const TDataItem& item);
  const_strlist ToPov(
    olxdict<TGlMaterial, olxstr, TComparableComparator> &materials) const;
  const_strlist ToWrl(
    olxdict<TGlMaterial, olxstr, TComparableComparator> &materials) const;
};

EndGxlNamespace()
#endif
