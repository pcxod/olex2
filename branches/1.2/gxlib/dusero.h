/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_glx_duserobj_H
#define __olx_glx_duserobj_H
#include "gxbase.h"
#include "glmousehandler.h"
#include "ematrix.h"
#include "edict.h"
BeginGxlNamespace()

class TDUserObj: public AGlMouseHandlerImp  {
  short Type;
  TArrayList<vec3f>* Vertices, *Normals;
  TArrayList<uint32_t> *Colors;
  TGlMaterial GlM;
protected:
  virtual bool DoTranslate(const vec3d& t) {
    Basis.Translate(t);  return true;
  }
  virtual bool DoRotate(const vec3d& vec, double angle) {
    Basis.Rotate(vec, angle);  return true;
  }
  virtual bool DoZoom(double zoom, bool inc)  {
    if (inc) Basis.SetZoom(ValidateZoom(Basis.GetZoom() + zoom));
    else Basis.SetZoom(ValidateZoom(zoom));
    return true;
  }
public:
  TDUserObj(TGlRenderer& Render, short type, const olxstr& collectionName);
  TDUserObj(TGlRenderer& Render, const TDataItem &di);
  virtual ~TDUserObj()  {
    if (Vertices != NULL) delete Vertices;
    if (Normals != NULL) delete Normals;
    if (Colors != NULL) delete Colors;
  }
  void SetVertices(TArrayList<vec3f>* vertices)  {
    if (Vertices != NULL) delete Vertices;
    Vertices = vertices;
  }
  void SetNormals(TArrayList<vec3f>* normals)  {
    if (Normals != NULL) delete Normals;
    Normals = normals;
  }
  void SetColors(TArrayList<uint32_t>* colors)  {
    if (Colors != NULL) delete Colors;
    Colors = colors;
  }
  void SetMaterial(const olxstr& mat)  {  GlM.FromString(mat);  }
  void SetMaterial(const TGlMaterial& glm)  {  GlM = glm;  }
  void Create(const olxstr& cName=EmptyString());
  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d &Max, vec3d &Min){  return false;  }
  void ToDataItem(TDataItem &di) const;
  void FromDataItem(const TDataItem &di);
  const_strlist ToPov(
    olxdict<TGlMaterial, olxstr, TComparableComparator> &materials) const;
  short GetType() const { return Type; }
  TEBasis Basis;
};

EndGxlNamespace()
#endif
