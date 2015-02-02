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

class TXPlane: public TSPlane, public AGDrawObject  {
  mat3d RM;
  vec3d MaxV;
public:
  TXPlane(TNetwork* net, TGlRenderer& Render, const olxstr& collectionName)
    : TSPlane(net),
      AGDrawObject(Render, collectionName)
  {}
  virtual ~TXPlane()  {}
  void Create(const olxstr& cName=EmptyString());

  // multiple inheritance...
  void SetTag(index_t v) {   TSPlane::SetTag(v);  }
  index_t GetTag() const {  return TSPlane::GetTag();  }
  index_t IncTag()  {  return TSPlane::IncTag();  }
  index_t DecTag()  {  return TSPlane::DecTag();  }

  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d &, vec3d &)  {  return false;  }
  void ListPrimitives(TStrList& List) const;

  bool OnMouseDown(const IEObject *, const TMouseData &)  {
    return true;
  }
  bool OnMouseUp(const IEObject *, const TMouseData &)  {
    return false;
  }
  bool OnMouseMove(const IEObject *, const TMouseData &)  {
    return false;
  }

  void Delete(bool v) {
    TSPlane::SetDeleted(v);
    if (v)
      SetVisible(false);
  }

  const_strlist ToPov(olx_cdict<TGlMaterial, olxstr> &materials) const;
  static const_strlist PovDeclare();

  static olx_pdict<size_t, olxstr> &NamesRegistry() {
    static olx_pdict<size_t, olxstr> nr;
    return nr;
  }
};

EndGxlNamespace()
#endif
