#ifndef xgrowlineH
#define xgrowlinedH

#include "xbond.h"

#include "catom.h"
#include "satom.h"

BeginGxlNamespace()

class TXGrowPoint : public AGDrawObject  {
  smatd Transform;
  vec3d Center;
public:
  TXGrowPoint(const olxstr& collectionName,
               const vec3d& center, const smatd& transform, TGlRender *Render);
  void Create(const olxstr& cName = EmptyString);
  virtual ~TXGrowPoint();

  bool GetDimensions(vec3d &Max, vec3d &Min);

  bool OnMouseDown(const IEObject *Sender, const TMouseData *Data){  return true; }
  bool OnMouseUp(const IEObject *Sender, const TMouseData *Data){  return false; }
  bool OnMouseMove(const IEObject *Sender, const TMouseData *Data){  return false; }

  bool Orient(TGlPrimitive *P);
  void SetRadius(float V);
  inline double Radius() {  return Params()[0]; }

  const vec3d& GetCenter()     const {  return Center;  }
  const smatd& GetTransform() const {  return Transform;  }
};

EndGxlNamespace()
#endif
 
