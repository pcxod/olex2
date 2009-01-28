#ifndef xreflectionH
#define xreflectionH
#include "gxbase.h"

#include "gdrawobject.h"

#include "hkl.h"

BeginGxlNamespace()

class TXReflection: public AGDrawObject  {
private:
  vec3d FCenter;
  vec3i hkl;
  double I;
public:
  TXReflection(const olxstr& collectionName, double minI, double maxI,
                 const TReflection& R, TAsymmUnit* au, TGlRender *Render);
  virtual ~TXReflection();
  void Create(const olxstr& cName = EmptyString, const ACreationParams* cpar = NULL);

  const vec3i& GetHKL() const {  return hkl;  }
  double GetI() const {  return I;  }
  vec3d& Center()    {  return FCenter;  }

  bool Orient(TGlPrimitive *P);
  bool GetDimensions(vec3d &Max, vec3d& Min);

  bool OnMouseDown(const IEObject *Sender, const TMouseData *Data){  return true; }
  bool OnMouseUp(const IEObject *Sender, const TMouseData *Data){  return false; }
  bool OnMouseMove(const IEObject *Sender, const TMouseData *Data){  return false; }
};

EndGxlNamespace()
#endif
 
