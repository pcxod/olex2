/******************************************************************************
* Copyright (c) 2004-2019 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "rmsds_adp.h"
#include "gxapp.h"

TRMDSADP::TRMDSADP(TGlRenderer& r, const olxstr& collectionName,
  uint32_t quality, int type)
  : AGDrawObject(r, collectionName),
  Quality(quality),
  Type(type),
  Scale(1)
{
  SetSelectable(false);
}
//..............................................................................
void TRMDSADP::Create(const olxstr& cName) {
  if (!cName.IsEmpty()) {
    SetCollectionName(cName);
  }

  TGPCollection& GPC = Parent.FindOrCreateCollection(GetCollectionName());
  GPC.AddObject(*this);
  if (GPC.PrimitiveCount() != 0) {
    return;
  }
  TGraphicsStyle& GS = GPC.GetStyle();
  GS.SetSaveable(false);
  TGlPrimitive& GlP = GPC.NewPrimitive("MSDS", sgloCommandList);
  TGlMaterial &m = GS.GetMaterial("Object",
    TGlMaterial("85;0;4286611584;4290822336;64"));
  m.SetColorMaterial(true);
  m.SetTransparent(true);
  GlP.SetProperties(m);
  size_t a_cnt = 0, a_i = 0;
  TGXApp::AtomIterator ai = TGXApp::GetInstance().GetAtoms();
  while (ai.HasNext()) {
    const TXAtom &a = ai.Next();
    if (!a.IsVisible() || a.GetEllipsoid() == 0) {
      continue;
    }
    a_cnt++;
  }
  GlSphereEx<float, TetrahedronFP<vec3f> > sph;
  TTypeList<IndexTriangle> sph_t;
  vec3f_list sph_v;
  sph.Generate(1, olx_min(Quality, 7), sph_v, sph_t);
  vec3f_alist vecs(a_cnt * sph_t.Count());
  vec3f_alist norms(a_cnt * sph_t.Count());
  TArrayList<uint32_t> cls(a_cnt * sph_t.Count());
  ai.Reset();
  GlP.StartList();
  olx_gl::colorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
  olx_gl::begin(GL_TRIANGLES);
  while (ai.HasNext()) {
    const TXAtom &a = ai.Next();
    if (!a.IsVisible() || a.GetEllipsoid() == 0) {
      continue;
    }
    TGlMaterial *vm = a.GetPrimitives().GetStyle().FindMaterial("Sphere");
    if (vm == 0) {
      vm = &m;
    }
    if (a.GetEllipsoid()->IsNPD()) {
      vm = 0;
    }
    mat3f M = a.GetEllipsoid()->ExpandQuad();
    for (size_t i = 0; i < sph_v.Count(); i++) {
      float d = (sph_v[i] * M).DotProd(sph_v[i]);
      int sign = 1;
      if (d < 0) {
        sign = -1;
        d = -d;
      }
      if (Type == type_rmsd) {
        d = sqrt(d);
      }
      vecs[i] = a.crd() + sph_v[i] * (Scale * d);
      // NPD
      if (vm == 0) {
        if (sign == 1) {
          cls[i] = 0xFF0000;
        }
        else {
          cls[i] = 0x0000FF;
        }
      }
      else {
        cls[i] = vm->AmbientF.GetRGB();
      }
    }
    for (size_t i = 0; i < sph_t.Count(); i++) {
      vec3f vec1 = vecs[sph_t[i][1]] - vecs[sph_t[i][0]];
      vec3f vec2 = vecs[sph_t[i][2]] - vecs[sph_t[i][0]];
      vec3f normal = vec1.XProdVec(vec2);
      float S = vec1.XProdVal(vec2);
      normal /= sqrt(olx_abs(S));
      for (int j = 0; j < 3; j++) {
        norms[sph_t[i][j]] += normal;
      }
    }
    for (size_t i = 0; i < norms.Count(); i++) {
      float ql = norms[i].QLength();
      if (ql > 0) {
        norms[i] /= sqrt(ql);
      }
      else {
        norms[i][2] = 1;
      }
    }
    uint32_t last_cl = 0;
    for (size_t i = 0; i < sph_t.Count(); i++) {
      for (int j = 0; j < 3; j++) {
        olx_gl::normal(norms[sph_t[i][j]]);
        if (last_cl != cls[sph_t[i][j]]) {
          olx_gl::color(0xEF000000 | cls[sph_t[i][j]]);
          last_cl = cls[sph_t[i][j]];
        }
        olx_gl::vertex(vecs[sph_t[i][j]]);
      }
    }
    a_i++;
  }
  olx_gl::end();
  GlP.EndList();
}
//..............................................................................
bool TRMDSADP::Orient(TGlPrimitive& GlP) {
  return false;
}
//..............................................................................
bool TRMDSADP::GetDimensions(vec3d &Max, vec3d &Min) {
  return false;
};
//..............................................................................
