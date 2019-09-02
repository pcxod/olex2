/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* OlexSys Proprietrary file                                                   *
******************************************************************************/
#include "dusero.h"
#include "gxapp.h"
#include "tls.h"

namespace glx_ext {
const short
  xtls_diff_Obs_Tls = 1,
  xtls_diff_Tls_Obs = 2,
  xtls_obj_diff = 1,
  xtls_obj_rmsd = 2;

struct XTLS {
  uint32_t start_color, end_color, quality;
  bool use_gradient;
  XTLS(uint32_t start_color, uint32_t end_color, uint32_t quality=5,
    bool use_grad=true)
    : start_color(start_color),
      end_color(end_color),
      quality(quality),
      use_gradient(use_grad)
  {}

  TDUserObj *CreateTLSObject(const TXAtomPList &atoms, const TLS &tls,
    short diff_dir, float scale, short obj_type) const;

  TDUserObj *CreateUdiffObject(const vec3f_alist &crds,
    const TEllpPList &from, const TEllpPList &to,
    float scale, const olxstr &obj_name,
    short obj_type) const;

  void CreatePovRayFile(const TXAtomPList &atoms, const TLS &tls,
    short diff_dir,
    const olxstr &fn) const;
};
};

