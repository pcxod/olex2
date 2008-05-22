#ifndef xreflectionH
#define xreflectionH
#include "gxbase.h"

#include "gdrawobject.h"

#include "hkl.h"

BeginGxlNamespace()

class TXReflection: public AGDrawObject  {
private:
  TReflection *FReflection;
  TVPointD FCenter;
public:
  TXReflection(const olxstr& collectionName, THklFile& HklFile,
                 TReflection& R, TAsymmUnit* au, TGlRender *Render);
  virtual ~TXReflection();
  void Create(const olxstr& cName = EmptyString);

  TReflection *Reflection()   {  return FReflection; }
  TVPointD& Center()    {  return FCenter;  }

  bool Orient(TGlPrimitive *P);
  bool GetDimensions(TVPointD &Max, TVPointD &Min);

  bool OnMouseDown(const IEObject *Sender, const TMouseData *Data){  return true; }
  bool OnMouseUp(const IEObject *Sender, const TMouseData *Data){  return false; }
  bool OnMouseMove(const IEObject *Sender, const TMouseData *Data){  return false; }
};

EndGxlNamespace()
#endif
 
