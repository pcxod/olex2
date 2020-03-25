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
  uint32_t quality, int type, int anh_type)
  : AGDrawObject(r, collectionName),
  Quality(quality),
  Type(type),
  AnhType(anh_type),
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
  if (!GPC.GetObjects().Contains(this)) {
    GPC.AddObject(*this);
  }
  if (GPC.PrimitiveCount() != 0) {
    return;
  }
  TGraphicsStyle& GS = GPC.GetStyle();
  GS.SetSaveable(false);
  TGlPrimitive& GlP = GPC.NewPrimitive("MSDS", sgloCommandList);
  TGlMaterial &m = GS.GetMaterial("MSDS",
    TGlMaterial("85;0;4286611584;4290822336;64"));
  m.SetColorMaterial(true);
  GlP.SetProperties(m);
  size_t a_cnt = 0, a_i = 0;
  const TAsymmUnit &au = TGXApp::GetInstance().XFile().GetAsymmUnit();
  mat3d cell2cart_c = au.GetCellToCartesian()*au.GetCellToCartesian();
  mat3d cell2cart_d = cell2cart_c *au.GetCellToCartesian();
  mat3d cell2cart_ct = mat3d::Transpose(cell2cart_c);
  mat3d cell2cart_dt = mat3d::Transpose(cell2cart_d);

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
  tensor::tensor_rank_3 tmp_t3;
  tensor::tensor_rank_4 tmp_t4;
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
    olx_object_ptr<GramCharlier4> t = a.GetEllipsoid()->GetAnharmonicPart();
    
    if (t.is_valid() && AnhType != anh_none) {
      GramCharlier4 &gc = t();
      mat3d m3, m4;
      for (size_t ti = 0; ti < 3; ti++) {
        for (size_t tj = 0; tj < 3; tj++) {
          for (size_t tk = 0; tk < 3; tk++) {
            m3[tj][tk] = gc.C(ti, tj, tk);
            for (size_t tl = 0; tl < 3; tl++) {
              m4[tk][tl] = gc.D(ti, tj, tk, tl);
            }
          }
          m4 = cell2cart_d * m4 * cell2cart_dt;
          for (size_t tk = 0; tk < 3; tk++) {
            for (size_t tl = 0; tl < 3; tl++) {
              tmp_t4(ti, tj, tk, tl) = m4[tk][tl];
            }
          }
        }
        m3 = cell2cart_c * m3 * cell2cart_ct;
        for (size_t tj = 0; tj < 3; tj++) {
          for (size_t tk = 0; tk < 3; tk++) {
            tmp_t3(ti, tj, tk) = m3[tj][tk];
          }
        }
      }
    }
    for (size_t i = 0; i < sph_v.Count(); i++) {
      float d = 0;
      if (t.is_valid() && AnhType != anh_none) {
        float c_ = tmp_t3.sum_up(sph_v[i]);
        float d_ = tmp_t4.sum_up(sph_v[i]);
        if (AnhType == anh_all) {
          d = (compf(1 + c_, d_) * (sph_v[i] * M).DotProd(sph_v[i])).mod();
        }
        else {
          d = compf(c_, d_).mod();
        }
        if (d == 0) {
          continue;
        }
      }
      else {
        d = (sph_v[i] * M).DotProd(sph_v[i]);
      }
      
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
      if (normal.IsNull()) {
        continue;
      }
      float S = vec1.XProdVal(vec2);
      normal = normal.Normalise()/sqrt(olx_abs(S));
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
