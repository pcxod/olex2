/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gl_textbox_H
#define __olx_gl_textbox_H
#include "glbase.h"
#include "estrlist.h"
#include "glmousehandler.h"
BeginGlNamespace()

class TGlTextBox: public AGlMouseHandlerImp  {
  float LineSpacing;
  uint16_t Width, Height, MaxStringLength;
  int Top, Left;
  TStringToList<olxstr, TGlMaterial*> FBuffer;  // the content
  double Z;
  uint16_t FontIndex;
  bool ScrollDirectionUp;
protected:
  vec3d Center;
  virtual bool DoTranslate(const vec3d& t) {  Center += t;  return true;  }
  virtual bool DoRotate(const vec3d&, double) {  return false;  }
  virtual bool DoZoom(double, bool)  {  return false;  }
  const vec3d& GetCenter() const {  return Center;  }
public:
  TGlTextBox(TGlRenderer& Render, const olxstr& collectionName);
  void Create(const olxstr& cName=EmptyString());
  virtual ~TGlTextBox();

  void Clear();

  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d &Max, vec3d &Min) {  return false;  }

  DefPropP(double, Z)
  DefPropP(uint16_t, Width)
  DefPropP(uint16_t, Height)
  void SetLeft(int left);
  int GetLeft() const {  return Left;  }
  void SetTop(int top);
  int GetTop() const {  return Top;  }
  void Fit();
  virtual void UpdateLabel()  {  Fit();  }

  DefPropP(uint16_t, FontIndex)
  DefPropP(uint16_t, MaxStringLength)
  DefPropP(float, LineSpacing)

  void PostText(const olxstr &S, class TGlMaterial *M=NULL);
  void PostText(const TStrList &SL, TGlMaterial *M=NULL);
  const TStringToList<olxstr, TGlMaterial*> &GetText() const {
    return FBuffer;
  }
  void NewLine()  {  FBuffer.Add();  }
  class TGlFont& GetFont() const;
  bool OnMouseUp(const IEObject *Sender, const TMouseData& Data);
};


EndGlNamespace()
#endif
