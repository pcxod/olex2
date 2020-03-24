/******************************************************************************
* Copyright (c) 2004-2019 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_gxl_rmds_adp_H
#define __olx_gxl_rmds_adp_H
#include "gxbase.h"
#include "gdrawobject.h"
#include "glutil.h"
BeginGxlNamespace()

class TRMDSADP : public AGDrawObject {
public:
  enum {
    type_rmsd = 0,
    type_msd = 1
  };
  enum {
    anh_none,
    anh_anh,
    anh_all
  };
private:
  uint32_t Quality;
  int Type, AnhType;
  double Scale;
public:
  TRMDSADP(TGlRenderer& Render, const olxstr& collectionName,
    uint32_t quality=5, int type=type_rmsd, int anh_type=anh_all);
  void Create(const olxstr& cName = EmptyString());

  bool Orient(TGlPrimitive& P);
  bool GetDimensions(vec3d& Max, vec3d& Min);
  DefPropP(double, Scale);
  DefPropP(int, Type);
  DefPropP(int, AnhType);
  DefPropP(uint32_t, Quality);
};

EndGxlNamespace()
#endif
