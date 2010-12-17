#ifndef __olx_gxl_xplane_H
#define __olx_gxl_xplane_H
#include "gxbase.h"
#include "glrender.h"
#include "gdrawobject.h"
#include "splane.h"

BeginGxlNamespace()

class TXPlane: public TSPlane, public AGDrawObject  {
public:
  TXPlane(TNetwork* net, TGlRenderer& Render, const olxstr& collectionName) :
    TSPlane(net),
    AGDrawObject(Render, collectionName)  {}
  virtual ~TXPlane()  {}
  void Create(const olxstr& cName = EmptyString, const ACreationParams* cpar = NULL);

  // multiple inheritance...
  void SetTag(index_t v) {   TSPlane::SetTag(v);  }
  index_t GetTag()  {  return TSPlane::GetTag();  }
  index_t IncTag()  {  return TSPlane::IncTag();  }
  index_t DecTag()  {  return TSPlane::DecTag();  }

  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d &Max, vec3d &Min)  {  return false;  }
  void ListPrimitives(TStrList& List) const;

  bool OnMouseDown(const IEObject *Sender, const TMouseData& Data){  return true; }
  bool OnMouseUp(const IEObject *Sender, const TMouseData& Data)  {  return false; }
  bool OnMouseMove(const IEObject *Sender, const TMouseData& Data){  return false; }

  bool IsDeleted() const {  return TSPlane::IsDeleted();  }
  void SetDeleted(bool v)  {
    AGDrawObject::SetDeleted(v);
    TSPlane::SetDeleted(v);
  }
};

EndGxlNamespace()
#endif
 
