#ifndef __olx_gl_light_H
#define __olx_gl_light_H
#include "glbase.h"
#include "datafile.h"
#include "gloption.h"

BeginGlNamespace()

class TGlLight: public IEObject  {
  TGlOption Ambient;
  TGlOption Diffuse;
  TGlOption Specular;
  TGlOption Position;
  TGlOption SpotDirection;
  short SpotCutoff;
  short SpotExponent;
  TGlOption Attenuation;
  bool Enabled;
  short Index;
public:
  TGlLight();
  virtual ~TGlLight() {}

  DefPropP(short, Index)
  DefPropBIsSet(Enabled)
  DefPropP(short, SpotCutoff)
  DefPropP(short, SpotExponent)
  DefPropC(TGlOption, Ambient)
  DefPropC(TGlOption, Diffuse)
  DefPropC(TGlOption, Specular)
  DefPropC(TGlOption, Position)
  DefPropC(TGlOption, SpotDirection)
  DefPropC(TGlOption, Attenuation)

  TGlLight& operator = (const TGlLight &S );
  void ToDataItem(TDataItem& Item) const;
  bool FromDataItem(const TDataItem& Item);
};

EndGlNamespace()
#endif
