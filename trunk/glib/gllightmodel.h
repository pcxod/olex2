/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gl_lightmodel_H
#define __olx_gl_lightmodel_H
#include "glbase.h"
#include "gllight.h"
#include "gloption.h"
#include "library.h"
BeginGlNamespace()

const char
  cglmLocalViewer  = 0x01,     // light model properties
  cglmSmoothShade  = 0x02,
  cglmTwoSide      = 0x04,
  cglBack          = 0x01,
  cglFront         = 0x02;

class TGlLightModel: public AActionHandler  {
private:
  char Flags;
  TGlOption ClearColor, AmbientColor;
  TGlLight Lights[8];
  virtual bool Execute(const IEObject*, const IEObject*, TActionQueue *)  {
    Init();
    return true;
  }
public:
  TGlLightModel();
  void Init();
  TGlLight& GetLight(size_t i)  {  return Lights[i]; }
  const TGlLight& GetLight(size_t i) const {  return Lights[i]; }
  DefPropC(TGlOption, ClearColor)
  DefPropC(TGlOption, AmbientColor)
  inline int GetFlags() const {  return Flags; }
  DefPropBFIsSet(LocalViewer, Flags, cglmLocalViewer)
  DefPropBFIsSet(SmoothShade, Flags, cglmSmoothShade)
  DefPropBFIsSet(TwoSides, Flags, cglmTwoSide)

  TGlLightModel& operator = (TGlLightModel &M);

  void ToDataItem(TDataItem& Item) const;
  bool FromDataItem(const TDataItem& Item);

  void LibClearColor(const TStrObjList& Params, TMacroData& E);
  void LibAmbientColor(const TStrObjList& Params, TMacroData& E);
  void LibLocalViewer(const TStrObjList& Params, TMacroData& E);
  void LibSmoothShade(const TStrObjList& Params, TMacroData& E);
  void LibTwoSides(const TStrObjList& Params, TMacroData& E);
  TLibrary* ExportLibrary(const olxstr& name=EmptyString());
};


EndGlNamespace()
#endif
