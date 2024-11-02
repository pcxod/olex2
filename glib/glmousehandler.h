/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gl_mousehandlerimp_H
#define __olx_gl_mousehandlerimp_H
#include "glbase.h"
#include "gdrawobject.h"
#include "ebasis.h"
BeginGlNamespace()

// TGlMouseListner flags
const uint16_t
  glmlMove2d    = 0x0001,
  glmlMoveable  = 0x0002,
  glmlRoteable  = 0x0004,
  glmlZoomable  = 0x0008,
  glmlMove2dz   = 0x0010;  // the translation is divided by current zoom

class AGlMouseHandler  {
  // default handler implementation
  struct EventHandler {
    int SX, SY;
    bool MouseDown, MouseHandled;
    EventHandler() : SX(-1), SY(-1), MouseDown(false), MouseHandled(false) {}
    bool OnMouseDown(AGlMouseHandler& Sender, const TMouseData& Data);
    bool OnMouseUp(AGlMouseHandler& Sender, const TMouseData& Data);
    bool OnMouseMove(AGlMouseHandler& Sender, const TMouseData& Data);
    bool OnDblClick(AGlMouseHandler&, const TMouseData&)  {  return false;  }
  } handler;
protected:
  uint16_t glml_Flags;
  virtual bool DoTranslate(const vec3d& t) = 0;
  virtual bool DoRotate(const vec3d& vec, double angle) = 0;
  virtual bool DoZoom(double zoom, bool inc) = 0;
  virtual const TGlRenderer& DoGetRenderer() const = 0;
  EventHandler& GetHandler() {  return handler;  }
public:
  AGlMouseHandler() : glml_Flags(0) {}
  virtual ~AGlMouseHandler() {}
  DefPropBFIsSet(Move2DZ, glml_Flags, glmlMove2dz)
  DefPropBFIsSet(Move2D, glml_Flags, glmlMove2d)
  DefPropBFIsSet(Moveable, glml_Flags, glmlMoveable)
  DefPropBFIsSet(Roteable, glml_Flags, glmlRoteable)
  DefPropBFIsSet(Zoomable, glml_Flags, glmlZoomable)

  static double ValidateZoom(double v) {  return v < 0.01 ? 0.01 : v;  }
};

class AGlMouseHandlerImp: public AGDrawObject, public AGlMouseHandler  {
  virtual const TGlRenderer& DoGetRenderer() const {  return GetParent();  }
public:
  AGlMouseHandlerImp(TGlRenderer& R, const olxstr& collectionName) :
    AGDrawObject(R, collectionName)  {}

  ~AGlMouseHandlerImp()  {}

  bool OnMouseDown(const IOlxObject *, const TMouseData& Data)  {
    return GetHandler().OnMouseDown(*this, Data);
  }
  bool OnMouseUp(const IOlxObject *, const TMouseData& Data)  {
    return GetHandler().OnMouseUp(*this, Data);
  }
  bool OnMouseMove(const IOlxObject *, const TMouseData& Data)  {
    return GetHandler().OnMouseMove(*this, Data);
  }
  bool OnDblClick(const IOlxObject *, const TMouseData& Data)  {
    return GetHandler().OnDblClick(*this, Data);
  }
};

EndGlNamespace()
#endif
