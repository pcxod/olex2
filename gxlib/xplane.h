/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gxl_xplane_H
#define __olx_gxl_xplane_H
#include "gxbase.h"
#include "glrender.h"
#include "gdrawobject.h"
#include "splane.h"
BeginGxlNamespace()

class TXPlane : public TSPlane, public AGDrawObject {
private:
  mat3d RM;
  vec3d MaxV;
  double normalScale;
public:
  TXPlane(TNetwork* net, TGlRenderer& Render, const olxstr& collectionName)
    : TSPlane(net),
    AGDrawObject(Render, collectionName),
    normalScale(1.5)
  {}
  virtual ~TXPlane() {}
  void Create(const olxstr& cName = EmptyString());

  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d &, vec3d &) { return false; }
  void ListPrimitives(TStrList& List) const;
  /* Inverts the plane normal */
  void Invert();
  bool OnMouseDown(const IOlxObject *, const TMouseData &) {
    return true;
  }
  bool OnMouseUp(const IOlxObject *, const TMouseData &) {
    return false;
  }
  bool OnMouseMove(const IOlxObject *, const TMouseData &) {
    return false;
  }

  void Delete(bool v) {
    TSPlane::SetDeleted(v);
    if (v) {
      SetVisible(false);
    }
  }

  virtual vec3d CalcCenter() const { return GetCenter(); }

  const_strlist ToPov(olx_cdict<TGlMaterial, olxstr> &materials) const;
  static const_strlist PovDeclare();

  const_strlist ToWrl(olx_cdict<TGlMaterial, olxstr> &materials) const;
  static const_strlist WrlDeclare(TGlRenderer &);
  static olx_pdict<size_t, olxstr> &NamesRegistry() {
    static olx_pdict<size_t, olxstr> nr;
    return nr;
  }

  double GetNormalScale() const {
    return normalScale;
  }

  void SetNormalScale(double sn);

  void ToDataItem(TDataItem& di, const TXApp& app) const;
  void FromDataItem(const TDataItem& di, const TXApp& app);
};

typedef TTypeList<TXPlane> TXPlaneList;
typedef TPtrList<TXPlane> TXPlanePList;

EndGxlNamespace()
#endif
