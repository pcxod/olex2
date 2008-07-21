#ifndef xreflectionH
#define xreflectionH
#include "gxbase.h"

#include "gdrawobject.h"

#include "hkl.h"

BeginGxlNamespace()

class TXReflection: public AGDrawObject  {
private:
  TReflection *FReflection;
  vec3d FCenter;
public:
  TXReflection(const olxstr& collectionName, THklFile& HklFile,
                 TReflection& R, TAsymmUnit* au, TGlRender *Render);
  virtual ~TXReflection();
  void Create(const olxstr& cName = EmptyString);

  TReflection *Reflection()   {  return FReflection; }
  vec3d& Center()    {  return FCenter;  }

  bool Orient(TGlPrimitive *P);
  bool GetDimensions(vec3d &Max, vec3d &Min);

  bool OnMouseDown(const IEObject *Sender, const TMouseData *Data){  return true; }
  bool OnMouseUp(const IEObject *Sender, const TMouseData *Data){  return false; }
  bool OnMouseMove(const IEObject *Sender, const TMouseData *Data){  return false; }
};

EndGxlNamespace()
#endif
 
