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
const uint32_t
  lmLabels   = 0x00000001,  // atom label
  lmPart     = 0x00000002,  // part
  lmAfix     = 0x00000004,  // afix
  lmOVar     = 0x00000008,  // occupancy variable
  lmOccp     = 0x00000010,  // occupancy
  lmUiso     = 0x00000020,  // Uiso
  lmUisR     = 0x00000040,  // Uiso for riding atoms (negative)
  lmAOcc     = 0x00000080,  // actuall occupancy (as read from ins )
  lmHydr     = 0x00000100,  // include hydrogens
  lmQPeak    = 0x00000200,  // include Q-peaks
  lmQPeakI   = 0x00000400,  // Q-peaks intensity
  lmFixed    = 0x00000800,  // fixed values
  lmConRes   = 0x00001000,  // restraints, constraints
  lmIdentity = 0x00002000,  // only for identity atoms
  lmCOccu    = 0x00004000,  // chemical occupancy
  lmSpec     = 0x00008000,  // enforced special position 
  lmChirality= 0x00010000,  // R/S
  lmCharge   = 0x00020000,  // Charge
  lmSame     = 0x00040000,  // same group

  lmBonds      = 0x00100000,  // exclusive for bond lengths
  lmResiName   = 0x00200000,  // residue name
  lmResiNumber = 0x00400000;  // residue number

enum LabelMaterialIndex {
  lmiMark,
  lmiDuplicateLabel,
  lmiMasked,
  lmiDefault
};

class TXGlLabels: public AGDrawObject  {
  TArrayList<uint32_t> Colors_;
  TArrayList<uint8_t> Marks;
  uint32_t Mode;
  size_t FontIndex;
  struct RenderContext {
    TGlPrimitive &primitive;
    TGlFont &font;
    bool &matInitialised;
    TGlMaterial GlM;
    int currentMaterial;
    bool optimizeATI;
    double vectorZoom;
  };
  void RenderLabel(const vec3d &crd, const olxstr &label,
    size_t index,
    RenderContext &rc) const;
public:
  TXGlLabels(TGlRenderer& Render, const olxstr& collectionName);
  void Create(const olxstr& cName=EmptyString());
  virtual ~TXGlLabels()  {}

  void Clear();
  void ClearLabelMarks(uint8_t value=(uint8_t)~0);

  DefPropP(uint32_t, Mode)
  void Selected(bool On);

  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d& Max, vec3d& Min) {  return false;  }

  void Init(bool clear=true, uint8_t value=(uint8_t)~0);
  void MarkLabel(const TXAtom& atom, bool v);
  void MarkLabel(size_t index, bool v);
  bool IsLabelMarked(const TXAtom& atom) const;
  bool IsLabelMarked(size_t index) const;
  void SetMaterialIndex(size_t idx, LabelMaterialIndex mi);
  uint8_t GetMaterialIndex(size_t i) const {
    return i < Marks.Count() ? Marks[i] : ~0;
  }
  TArrayList<uint32_t> Colors() {  return Colors_; }
  const TArrayList<uint32_t> Colors() const {  return Colors_; }
  TGlFont& GetFont() const;
};

EndGxlNamespace()
#endif
