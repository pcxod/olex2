/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gl_light_H
#define __olx_gl_light_H
#include "glbase.h"
#include "datafile.h"
#include "gloption.h"
#include "library.h"
BeginGlNamespace()

class TGlLight : public IEObject  {
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
  TActionQList actions;
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

  TActionQueue& OnLibChange;

  void LibEnabled(const TStrObjList& Params, TMacroData& E);
  void LibSpotCutoff(const TStrObjList& Params, TMacroData& E);
  void LibSpotExponent(const TStrObjList& Params, TMacroData& E);
  void LibAmbient(const TStrObjList& Params, TMacroData& E);
  void LibDiffuse(const TStrObjList& Params, TMacroData& E);
  void LibSpecular(const TStrObjList& Params, TMacroData& E);
  void LibPosition(const TStrObjList& Params, TMacroData& E);
  void LibSpotDirection(const TStrObjList& Params, TMacroData& E);
  void LibAttenuation(const TStrObjList& Params, TMacroData& E);
  TLibrary* ExportLibrary(const olxstr& name);
};

EndGlNamespace()
#endif
