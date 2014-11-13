/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gl_cursor_H
#define __olx_gl_cursor_H
#include "glbase.h"
#include "gdrawobject.h"
#include "actions.h"
#include "macroerror.h"
BeginGlNamespace()

class TGlCursor: public AGDrawObject, AEventsDispatcher {
  double X, Y, Z;
  bool TextStyle;
  class TGlPrimitive *Primitive;
  olxch Symbol;
  size_t FontIndex;
protected:
  bool Dispatch(int MsgId, short MsgSubId, const IEObject *Sender,
    const IEObject *Data, TActionQueue *);
public:
  TGlCursor(TGlRenderer& Render, const olxstr& collectionName,
    bool TextStyle = true);
  void Create(const olxstr& cName=EmptyString());
  virtual ~TGlCursor() {}

  void SetPosition(double x, double y, double z)  {
    X = x;
    Y = y;
    Z = z;
  }
  DefPropP(double, X)
  DefPropP(double, Y)
  DefPropP(double, Z)
  DefPropP(size_t, FontIndex)
  class TGlFont& GetFont() const;

  bool IsText() const {  return TextStyle;  }
  olxch GetSymbol() const {  return Symbol;  }
  void SetSymbol(olxch v);

  virtual bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d& Max, vec3d& Min)  {  return false;  }
  void LibSymbol(const TStrObjList& Params, TMacroData& E);
  class TLibrary* ExportLibrary(const olxstr& name="cursor");
};

EndGlNamespace()
#endif
