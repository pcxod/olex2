/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gl_material_H
#define __olx_gl_material_H
#include "glbase.h"
#include "groupobj.h"
#include "gloption.h"
#include "datafile.h"
#include "emath.h"
BeginGlNamespace()

extern const TGlOption BlackColor;

const uint16_t
  sglmAmbientF   = 0x0001,    // material properties
  sglmAmbientB   = 0x0002,
  sglmAmbientFB  = 0x0003,

  sglmDiffuseF   = 0x0004,
  sglmDiffuseB   = 0x0008,
  sglmDiffuseFB  = 0x000C,

  sglmSpecularF  = 0x0010,
  sglmSpecularB  = 0x0020,
  sglmSpecularFB = 0x0030,

  sglmShininessF = 0x0040,
  sglmShininessB = 0x0080,
  sglmShininessFB= 0x00C0,

  sglmEmissionF =  0x0100,
  sglmEmissionB =  0x0200,
  sglmEmissionFB = 0x0300,

  sglmTransparent =0x0400,
  sglmIdentityDraw=0x0800,
//  sglmStaticDraw  =0x1000,

  sglmLighting   =0x2000,
  sglmColorMat   =0x4000;

class TGlMaterial: public AGOProperties, public IEObject  {
  uint16_t Flags;
public:
  TGlOption EmissionF;
  TGlOption SpecularF;
  short ShininessF;

  TGlOption EmissionB;
  TGlOption SpecularB;
  short ShininessB;

  TGlOption AmbientF;
  TGlOption DiffuseF;

  TGlOption AmbientB;
  TGlOption DiffuseB;

  TGlMaterial() : Flags(0), ShininessF(0), ShininessB(0) {}
  TGlMaterial(const olxstr& str)  {  FromString(str);  }

  void Init(bool skip) const;
  const TGlMaterial& Intensity(TGlOption& ClearColor, double intensity) const;

  DefPropP(uint16_t, Flags)

  // Has/Set
  DefPropBFHasSet(AmbientF,   Flags, sglmAmbientF)
  DefPropBFHasSet(DiffuseF,   Flags, sglmDiffuseF)
  DefPropBFHasSet(EmissionF,  Flags, sglmEmissionF)
  DefPropBFHasSet(SpecularF,  Flags, sglmSpecularF)
  DefPropBFHasSet(ShininessF, Flags, sglmShininessF)

  DefPropBFHasSet(AmbientB,   Flags, sglmAmbientB)
  DefPropBFHasSet(DiffuseB,   Flags, sglmDiffuseB)
  DefPropBFHasSet(EmissionB,  Flags, sglmEmissionB)
  DefPropBFHasSet(SpecularB,  Flags, sglmSpecularB)
  DefPropBFHasSet(ShininessB, Flags, sglmShininessB)

  DefPropBFHasSet(Lighting,  Flags, sglmLighting)
  DefPropBFHasSet(ColorMaterial,  Flags, sglmColorMat)
  DefPropBFIsSet(Transparent,  Flags, sglmTransparent)
  DefPropBFIsSet(IdentityDraw,  Flags, sglmIdentityDraw)

  TGlMaterial& operator = (const TGlMaterial &G);
  AGOProperties& operator = (const AGOProperties &G);
  bool operator == (const TGlMaterial &G) const;
  bool operator == (const AGOProperties &G) const; // !!!
  int Compare(const TGlMaterial &m) const;

  void ToDataItem(TDataItem& Item) const;
  bool FromDataItem(const TDataItem& Item);

  TIString ToString() const;
  void FromString(const olxstr& str);
  olxstr ToPOV() const;
  olxstr ToWRL() const;
};

EndGlNamespace()
#endif
