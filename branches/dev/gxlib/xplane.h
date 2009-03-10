#ifndef xplaneH
#define xplaneH
#include "gxbase.h"

#include "glrender.h"
#include "gdrawobject.h"

#include "splane.h"

BeginGxlNamespace()

class TXPlane: public AGDrawObject  {
private:
  TSPlane *FPlane;
public:
  TXPlane(const olxstr& collectionName, TSPlane *Plane, TGlRenderer *Render);
  void Create(const olxstr& cName = EmptyString, const ACreationParams* cpar = NULL);
  virtual ~TXPlane();

  inline operator TSPlane* () const {  return FPlane;  }
  inline TSPlane& Plane()     const {  return *FPlane; }

  bool Orient(TGlPrimitive *P);
  bool GetDimensions(vec3d &Max, vec3d &Min){  return false; };

  bool OnMouseDown(const IEObject *Sender, const TMouseData *Data){  return true; };
  bool OnMouseUp(const IEObject *Sender, const TMouseData *Data){  return false; };
  bool OnMouseMove(const IEObject *Sender, const TMouseData *Data){  return false; };

  inline bool Deleted()  const {  return AGDrawObject::Deleted(); }
  void Deleted(bool v){  AGDrawObject::Deleted(v);  FPlane->SetDeleted(v); }
};

EndGxlNamespace()
#endif
 
