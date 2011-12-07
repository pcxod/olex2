/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gl_xfader_H
#define __olx_gl_xfader_H
#include "glbase.h"
#include "gdrawobject.h"
#include "library.h"
BeginGlNamespace()

class TXFader: public AGDrawObject  {
  char *Background, *Foreground;
  int BGWidth, BGHeight;
  int FGWidth, FGHeight;
  double Step, Position;
public:
  TXFader(TGlRenderer& Render, const olxstr& collectionName);
  virtual ~TXFader();
  void Create(const olxstr& cName=EmptyString());
  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d&, vec3d&)  {  return false;  }

  void InitBG(bool init=true);
  void InitFG(bool init=true);
  void BG2FG(bool zeropos=true);
  DefPropP(double, Step)
  DefPropP(double, Position)
  bool Increment()  {
    Position += Step;
    if( Position > 1 )  {
      Position = 1;
      return false;
    }
    return true;
  }

  void LibStep(const TStrObjList& Params, TMacroError& E);
  void LibPosition(const TStrObjList& Params, TMacroError& E);
  void LibInitFG(const TStrObjList& Params, TMacroError& E);
  void LibInitBG(const TStrObjList& Params, TMacroError& E);
  void LibBG2FG(const TStrObjList& Params, TMacroError& E);
  class TLibrary*  ExportLibrary(const olxstr& name="fader");
};

EndGlNamespace()
#endif
