#ifndef __olx_xblob_H
#define __olx_xblob_H
#include "gxbase.h"
#include "glmouselistener.h"
#include "IsoSurface.h"

BeginGxlNamespace()

class TDBlob: public TGlMouseListener  {
  TTypeList<vec3f> vertices;
  TTypeList<vec3f> normals;
  TTypeList<IsoTriangle> triangles;
  uint32_t PolygonMode;
public:
  TDBlob(TGlRenderer& Render, const olxstr& collectionName);
  virtual ~TDBlob()  {}
  void Create(const olxstr& cName = EmptyString, const ACreationParams* cpar = NULL);
  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d &Max, vec3d &Min){  return false;  }

  DefPropP(uint32_t, PolygonMode)
};


EndGxlNamespace()
#endif
