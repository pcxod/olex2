/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gxl_labels_H
#define __olx_gxl_labels_H
#include "gxbase.h"
#include "gdrawobject.h"
#include "bitarray.h"
#include "glfont.h"
#include "glmaterial.h"
BeginGxlNamespace()

class TXAtom;
// label modes
const short
  lmLabels   = 0x0001,  // atom label
  lmPart     = 0x0002,  // part
  lmAfix     = 0x0004,  // afix
  lmOVar     = 0x0008,  // occupancy variable
  lmOccp     = 0x0010,  // occupancy
  lmUiso     = 0x0020,  // Uiso
  lmUisR     = 0x0040,  // Uiso for riding atoms (negative)
  lmAOcc     = 0x0080,  // actuall occupancy (as read from ins )
  lmHydr     = 0x0100,  // include hydrogens
  lmQPeak    = 0x0200,  // include Q-peaks
  lmQPeakI   = 0x0400,  // Q-peaks intensity
  lmFixed    = 0x0800,  // fixed values
  lmConRes   = 0x1000,  // restraints, constraints
  lmIdentity = 0x2000,  // only for identity atoms
  lmCOccu    = 0x4000;  // chemical occupancy

class TXGlLabels: public AGDrawObject  {
  TGlMaterial FMarkMaterial;
  TEBitArray Marks;
  short Mode;
  size_t FontIndex;
public:
  TXGlLabels(TGlRenderer& Render, const olxstr& collectionName);
  void Create(const olxstr& cName=EmptyString());
  virtual ~TXGlLabels()  {}

  void Clear();
  void ClearLabelMarks();

  DefPropP(short, Mode)
  void Selected(bool On);

  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d& Max, vec3d& Min) {  return false;  }
 
  void Init();
  void MarkLabel(const TXAtom& atom, bool v);
  void MarkLabel(size_t index, bool v);
  bool IsLabelMarked(const TXAtom& atom) const;
  bool IsLabelMarked(size_t index) const;
  TGlMaterial& MarkMaterial()  {  return FMarkMaterial;  }
  TGlFont& GetFont() const;
};

EndGxlNamespace()
#endif
