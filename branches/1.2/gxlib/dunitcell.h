/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gxl_dunitcell_H
#define __olx_gxl_dunitcell_H
#include "threex3.h"
#include "gllabel.h"
#include "glprimitive.h"
BeginGxlNamespace()

class TDUnitCell: public AGDrawObject  {
  bool Reciprocal;
  TGlPrimitive *FGlP;
  mat3d CellToCartesian, HklToCartesian;
  TXGlLabel* Labels[4];
public:
  TDUnitCell(TGlRenderer& Render, const olxstr& collectionName);
  virtual ~TDUnitCell();
  void Init(const double cell_params[6]);
  void UpdateLabel();
  size_t LabelCount() const {  return 4;  }
  TXGlLabel& GetLabel(size_t i) const {  return *Labels[i];  }
  void Create(const olxstr& cName=EmptyString());
  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d &Max, vec3d &Min);
  void ListPrimitives(TStrList &List) const;
  void UpdatePrimitives(int32_t Mask);

  size_t VertexCount() const {  return FGlP == NULL ? 0 : 8;  }
  const vec3f& GetVertex(size_t i) const {
    if( FGlP == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "uninitialised object");
    switch(i)  {
      case 0:  return FGlP->Vertices[0];  break;
      case 1:  return FGlP->Vertices[1];  break;
      case 2:  return FGlP->Vertices[3];  break;
      case 3:  return FGlP->Vertices[5];  break;
      case 4:  return FGlP->Vertices[7];  break;
      case 5:  return FGlP->Vertices[9];  break;
      case 6:  return FGlP->Vertices[13];  break;
      case 7:  return FGlP->Vertices[19];  break;
      default:
        throw TInvalidArgumentException(__OlxSourceInfo, "vertex index");
    }
  }
  size_t EdgeCount() const {  return FGlP == NULL ? 0 : 24;  }
  const vec3f& GetEdge(size_t i) const {  return FGlP->Vertices[i];  }
  inline bool IsReciprocal() const {  return Reciprocal;  }
  void SetReciprocal(bool v, double scale=1);
  virtual void SetVisible(bool v);
  inline const mat3d& GetCellToCartesian() const {  return CellToCartesian;  }
  inline const mat3d& GetHklToCartesian() const {  return HklToCartesian;  }
  void ToDataItem(TDataItem& di) const;
  void FromDataItem(const TDataItem& di);
  const_strlist ToPov(
    olxdict<TGlMaterial, olxstr, TComparableComparator> &materials) const;
};

EndGxlNamespace()
#endif
