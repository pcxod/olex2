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
#include "asymmunit.h"
BeginGxlNamespace()

class TDUnitCell: public AGDrawObject {
  bool Reciprocal;
  mat3d CellToCartesian, HklToCartesian;
  TXGlLabel* Labels[4];
  vec3d_alist Edges;
  double Thickness;
public:
  TDUnitCell(TGlRenderer& Render, const olxstr& collectionName);
  virtual ~TDUnitCell();
  // 6 parameters are expected
  void Init(const double *cell_params);
  void Init(const TAsymmUnit &au);
  void UpdateLabel();
  size_t LabelCount() const {  return 4;  }
  TXGlLabel& GetLabel(size_t i) const {  return *Labels[i];  }
  void Create(const olxstr& cName=EmptyString());
  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d &Max, vec3d &Min);
  void ListPrimitives(TStrList &List) const;
  void UpdatePrimitives(int32_t Mask);

  size_t VertexCount() const {  return Edges.IsEmpty() ? 0 : 8;  }
  const vec3d& GetVertex(size_t i) const {
    if (Edges.IsEmpty())
      throw TFunctionFailedException(__OlxSourceInfo, "uninitialised object");
    switch(i) {
      case 0:  return Edges[0];  break;
      case 1:  return Edges[1];  break;
      case 2:  return Edges[3];  break;
      case 3:  return Edges[5];  break;
      case 4:  return Edges[7];  break;
      case 5:  return Edges[9];  break;
      case 6:  return Edges[13];  break;
      case 7:  return Edges[19];  break;
      default:
        throw TInvalidArgumentException(__OlxSourceInfo, "vertex index");
    }
  }
  size_t EdgeCount() const { return Edges.Count(); }
  vec3d& GetEdge(size_t i) { return Edges[i]; }
  const vec3d& GetEdge(size_t i) const { return Edges[i]; }
  void Translate(const vec3d &t) {
    for (size_t i = 0; i < Edges.Count(); i++) {
      Edges[i] += t;
    }
    Update();
  }
  void Update();
  bool IsReciprocal() const { return Reciprocal; }
  void SetReciprocal(bool v, double scale=1);
  virtual void SetVisible(bool v);
  const mat3d& GetCellToCartesian() const {  return CellToCartesian;  }
  const mat3d& GetHklToCartesian() const {  return HklToCartesian;  }
  DefPropP(double, Thickness)
  void ToDataItem(TDataItem& di) const;

  void FromDataItem(const TDataItem& di);
  void funThickness(const TStrObjList& Params, TMacroData& E);
  void funDrawstyle(const TStrObjList& Params, TMacroData& E);
  TLibrary *ExportLibrary(const olxstr &name = EmptyString());

  const_strlist ToPov(olx_cdict<TGlMaterial, olxstr> &materials) const;
  const_strlist ToWrl(olx_cdict<TGlMaterial, olxstr> &materials) const;
};

EndGxlNamespace()
#endif
