/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xblob_H
#define __olx_xblob_H
#include "gxbase.h"
#include "gdrawobject.h"
#include "IsoSurface.h"
BeginGxlNamespace()

class TXBlob: public AGDrawObject  {
  uint32_t PolygonMode;
public:
  TXBlob(TGlRenderer& Render, const olxstr& collectionName);
  virtual ~TXBlob()  {}
  void Create(const olxstr& cName=EmptyString());
  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d &Max, vec3d &Min)  {  return false;  }
  void UpdateNormals();

  DefPropP(uint32_t, PolygonMode)

  TTypeList<vec3f> vertices;
  TTypeList<vec3f> normals;
  TTypeList<TGlOption> colors;
  TTypeList<IsoTriangle> triangles;
};

EndGxlNamespace()
#endif
