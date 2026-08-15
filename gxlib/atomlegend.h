/******************************************************************************
* Copyright (c) 2004-2016 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gxl_atomlabel_H
#define __olx_gxl_atomlabel_H

#include "gxbase.h"
#include "glmousehandler.h"
#include "gltexture.h"
#include "glrender.h"
BeginGxlNamespace()

class TAtomLegend : public AGlMouseHandlerImp {
  int Width, Height;
  int Top, Left;
  // single column width - up to 3 columns with 2 char element symbol
  int ColWidth,
    // space between the legend and the text
    FirstSpacer;
  GLuint TextureId;
  double Z;
  TStrList text;
  TArrayList<TGlMaterial> materials;
  /* Rows before this one are element keys, drawn as spheres; rows from it on
  are cartoon keys, drawn as a short length of ribbon. A ball already means
  "atom type" everywhere else in this legend, so a chain or a secondary
  structure cannot borrow the same symbol without saying the wrong thing. The
  cartoon rows are always appended last, so one boundary tells them apart.
  */
  size_t first_ribbon_row;
  /* Per row, meaningful from first_ribbon_row on. The swatch takes its colour
  through glColor rather than from the material, because that is how the ribbon
  itself is coloured: the material alone leaves the ambient dark and the swatch
  comes out visibly duller than the thing it is a key to.
  */
  TArrayList<uint32_t> row_colours;
  /* Appends the key for whatever the cartoon is currently coloured by, as more
  rows of the same legend. In the same object on purpose: it then moves, saves
  its position and resets with the element key rather than needing a second
  draggable thing to keep track of.
  */
  void AddCartoonKey();
protected:
  vec3d Center;
  virtual bool DoTranslate(const vec3d& t) { Center += t;  return true; }
  virtual bool DoRotate(const vec3d&, double) { return false; }
  virtual bool DoZoom(double zoom, bool inc) { return false; }
public:
  TAtomLegend(TGlRenderer& Render, const olxstr& collectionName);
  void Create(const olxstr& cName = EmptyString());
  void Fit();
  int GetLeft() const { return Left; }
  int GetTop() const { return Top; }
  int GetWidth() const { return Width; }
  int GetHeight() const { return Height; }
  const vec3d &GetCenter() const { return Center; }

  const TStrList &GetLabels() const { return text; }
  const TArrayList<TGlMaterial> &GetMaterials() const { return materials; }
  virtual bool Orient(TGlPrimitive& P);
  virtual bool GetDimensions(vec3d &, vec3d &) { return false; }
  bool OnMouseUp(const IOlxObject *Sender, const TMouseData& Data);
  virtual void Update();
  virtual void UpdateLabel() { Fit(); }
  virtual void SetVisible(bool v);
  void SetPosition(int left, int top);
  void ResetPosition(bool right=false, bool bottom=false, int margin=0);
};
EndGxlNamespace()

#endif
