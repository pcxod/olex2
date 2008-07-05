#ifndef glbackgroundH
#define glbackgroundH

#include "glbase.h"
#include "gdrawobject.h"
#include "gloption.h"
#include "glprimitive.h"
#include "gltexture.h"

BeginGlNamespace()

class TGlBackground: public AGDrawObject  {
  TGlOption  FColors[4];
  bool FCeiling;
  TGlPrimitive *FPrimitive;
  TGlTexture* Texture;
public:
  TGlBackground(const olxstr& collectionName, TGlRender *Render, bool Ceiling);
  virtual ~TGlBackground(){  return; };
  void Create(const olxstr& cName = EmptyString);
  bool Orient(TGlPrimitive *P);
  bool GetDimensions(TVPointD &Max, TVPointD &Min){  return false;};
  bool Ceiling(){  return FCeiling;  }
  TGlPrimitive *Primitive(){  return FPrimitive;  }
  void SetTexture(TGlTexture* glt);
  TGlTexture* GetTexture() const {  return Texture;  }

  void LT(const TGlOption& v);
  void RT(const TGlOption& v);
  void RB(const TGlOption& v);
  void LB(const TGlOption& v);

  const TGlOption& LT() const {  return FColors[0];  }
  const TGlOption& RT() const {  return FColors[1];  }
  const TGlOption& RB() const {  return FColors[2];  }
  const TGlOption& LB() const {  return FColors[3];  }
};


EndGlNamespace()

#endif
