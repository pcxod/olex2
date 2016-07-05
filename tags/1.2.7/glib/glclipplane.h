/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gl_glclipplane_H
#define __olx_gl_glclipplane_H
#include "glbase.h"
#include "evector.h"
#include "glrender.h"
BeginGlNamespace()

class TGlClipPlane  {
  evecd FEq;
  bool FEnabled;
  class TGlClipPlanes *FParent;
  int FId;  // GL_CLIPPLANE_i
public:
  TGlClipPlane( int Id, TGlClipPlanes *FParent, float A, float B, float C, float D );
  ~TGlClipPlane();
  inline bool Enabled() const     {  return FEnabled; }
  void Enabled(bool v);
  inline evecd& Equation()        {  return FEq; }
  inline TGlClipPlanes *Parent()  {  return FParent; }
  inline int Id() const           {  return FId; }
};

class TGlClipPlanes  {
  TPtrList<TGlClipPlane> FPlanes;
  class TGlRenderer *FParent;
public:
  TGlClipPlanes(TGlRenderer *R);
  ~TGlClipPlanes();
  inline TGlRenderer *Parent()  {  return FParent;  }
  inline TGlClipPlane *Plane(size_t i)  {  return FPlanes[i];  }
  inline size_t PlaneCount() const {  return FPlanes.Count();  }
  void Enable(bool v);
};


EndGlNamespace()
#endif
