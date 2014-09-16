/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gxl_xreflection_H
#define __olx_gxl_xreflection_H
#include "gxbase.h"
#include "gdrawobject.h"
#include "hkl.h"
BeginGxlNamespace()

class TXReflection: public AGDrawObject  {
private:
  vec3d Center;
  vec3i hkl;
  double I;
public:
  TXReflection(TGlRenderer& Render, const olxstr& collectionName, double minI, double maxI,
                 const TReflection& R, const TAsymmUnit& au);
  virtual ~TXReflection();
  void Create(const olxstr& cName=EmptyString());

  const vec3i& GetHKL() const {  return hkl;  }
  double GetI() const {  return I;  }
  const vec3d& GetCenter()  {  return Center;  }

  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d& Max, vec3d& Min);

  bool OnMouseDown(const IEObject *Sender, const TMouseData& Data)  {  return true;  }
  bool OnMouseUp(const IEObject *Sender, const TMouseData& Data)  {  return false;  }
  bool OnMouseMove(const IEObject *Sender, const TMouseData& Data)  {  return false;  }
};

EndGxlNamespace()
#endif
