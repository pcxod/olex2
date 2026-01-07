/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* OlexSys Proprietrary file                                                   *
******************************************************************************/
#if !defined(_CONSOLE)
#include "xtls.h"
#include "glutil.h"

using namespace glx_ext;

TDUserObj *XTLS::CreateTLSObject(const TXAtomPList &atoms, const TLS &tls,
  short diff_dir, float scale, short obj_type) const
{
  vec3f_alist crds(atoms.Count());
  TEllpPList utls(atoms.Count()), uobs(atoms.Count());
  for (size_t i=0; i < atoms.Count(); i++) {
    crds[i] = atoms[i]->crd();
    utls[i] = new TEllipsoid(tls.GetElpList()[i]);
    uobs[i] = atoms[i]->GetEllipsoid();
  }
  TDUserObj *rv = CreateUdiffObject(crds,
    diff_dir == xtls_diff_Obs_Tls ? uobs : utls,
    diff_dir == xtls_diff_Obs_Tls ? utls : uobs,
    scale,
    "tlso", obj_type);
  utls.DeleteItems(false);
  return rv;
}
//.............................................................................
TDUserObj *XTLS::CreateUdiffObject(const vec3f_alist &crds,
  const TEllpPList &from, const TEllpPList &to,
  float scale, const olxstr &obj_name, short obj_type) const
{
  if (from.Count() != to.Count() || to.Count() != crds.Count()) {
    throw TInvalidArgumentException(__OlxSourceInfo, "array's size");
  }
  TGXApp &app = TGXApp::GetInstance();
  TDUserObj *obj = app.FindUserObject(obj_name);
  olx_object_ptr<TDUserObj> obj_ptr;
  if (obj == 0) {
    TGlMaterial m("16469;4278190080;4286611584;4290822336;64");
    m.SetColorMaterial(true);
    obj_ptr = obj = new TDUserObj(app.GetRenderer(), sgloTriangles, obj_name);
    obj->SetMaterial(m);
  }
  else {
    app.GetRenderer().RemoveCollection(obj->GetPrimitives());
    // need to delete the collection
  }
  GlSphereEx<float, TetrahedronFP<vec3f> > sph;
  //GlSphereEx<float, OctahedronFP<vec3f> > sph;
  TTypeList<IndexTriangle> sph_t;
  vec3f_list sph_v;
  sph.Generate(1, quality, sph_v, sph_t);
  vec3f_alist &allv = *(new vec3f_alist(sph_t.Count()*from.Count()*3));
  vec3f_alist &alln = *(new vec3f_alist(sph_t.Count()*from.Count()*3));
  TArrayList<uint32_t> &colors =
    *(new TArrayList<uint32_t>(sph_t.Count()*from.Count()*3));
  obj->SetVertices(&allv);
  obj->SetNormals(&alln);
  obj->SetColors(&colors);
  TArrayList<float> pd(sph_v.Count()*from.Count());
  float max_d = -1000, min_d = 1000;
  mat3d m1, m2, etm, Mdiff;
  for (size_t i=0; i < from.Count(); i++) {
    vec3f_alist lc(sph_v.Count());
    if (obj_type == xtls_obj_diff) {
      m1 = from[i]->GetMatrix();
      m1.Scale(from[i]->GetNorms());
      m2 = to[i]->GetMatrix();
      m2.Scale(to[i]->GetNorms());
      etm = m2.GetInverse();
    }
    else if (obj_type == xtls_obj_rmsd) {
      mat3f qm1 = from[i]->ExpandQuad(),
        qm2 = to[i]->ExpandQuad();
      Mdiff = qm2 - qm1;
    }
    for (size_t j=0; j < sph_v.Count(); j++) {
      float d;
      if (obj_type == xtls_obj_diff) {
        vec3f n = lc[j] = sph_v[j] * m1;
        float d1 = n.Length();
        n /= d1;
        vec3f p1 = (n*etm).Normalise()*m2;
        d = p1.Length() - d1;
      }
      else if (obj_type == xtls_obj_rmsd) {
        float d_sq = (sph_v[j] * Mdiff).DotProd(sph_v[j]);
        d = olx_sign(d_sq)*sqrt(olx_abs(d_sq));
        lc[j] = sph_v[j];
      }
      pd[sph_v.Count()*i + j] = d;
      olx_update_min_max(d, min_d, max_d);
      lc[j] = lc[j].NormaliseTo(olx_abs(d)*scale);
    }
    const size_t off = sph_t.Count()*i*3;
    for (size_t j=0; j < sph_t.Count(); j++) {
      size_t idx = off + j*3;
      vec3f vec1 = lc[sph_t[j][1]] - lc[sph_t[j][0]];
      vec3f vec2 = lc[sph_t[j][2]] - lc[sph_t[j][0]];
      float S = vec1.XProdVal(vec2);
      vec3f normal = vec1.XProdVec(vec2);
      if (S > 0) {  // weight up normals of little triangles (hight curvature)
        normal = normal.Normalise() / sqrt(S);
      }
      for (int k=0; k < 3; k++) {
        allv[idx+k] = lc[sph_t[j][k]] +crds[i];
        alln[idx+k] += normal;
      }
    }
  }
  short m_c[3] = {
    (short)(OLX_GetRValue(start_color)+OLX_GetRValue(end_color)),
    (short)(OLX_GetGValue(start_color)+OLX_GetGValue(end_color)),
    (short)(OLX_GetBValue(start_color)+OLX_GetBValue(end_color)),
  };
  short s_c[3] = {
    (short)(OLX_GetRValue(end_color)-OLX_GetRValue(start_color)),
    (short)(OLX_GetGValue(end_color)-OLX_GetGValue(start_color)),
    (short)(OLX_GetBValue(end_color)-OLX_GetBValue(start_color)),
  };
  for (int i=0; i < 3; i++) {
    m_c[i] /= 2;
    s_c[i] /= 2;
  }
  for (size_t i=0; i < from.Count(); i++) {
    const size_t off = sph_t.Count()*i*3;
    const size_t voff = sph_v.Count()*i;
    for (size_t j=0; j < sph_t.Count(); j++) {
      size_t idx = off + j*3;
      for (int k=0; k < 3; k++) {
        float val = pd[voff+sph_t[j][k]];
        if (use_gradient) {
          float s = val < 0 ? -val/min_d : val/max_d;
          colors[idx+k] = OLX_RGB(
            (short)(m_c[0] + s*s_c[0]),
            (short)(m_c[1] + s*s_c[1]),
            (short)(m_c[2] + s*s_c[2]));
        }
        else {
          colors[idx+k] = val < 0 ? start_color : end_color;
        }
      }
    }
  }
  // Normalise normals.
  for (size_t i = 0; i < alln.Count(); i++) {
    float ql = alln[i].QLength();
    if (ql > 1e-6) {
      alln[i] /= sqrt(ql);
    }
    else {
      alln[i][2] = 1;
    }
  }
  obj->SetVisible(true);
  obj->Create();
  if (obj_ptr.ok()) {
    app.AddObjectToCreate(obj_ptr.release());
  }
  return obj;
}
//.............................................................................
void XTLS::CreatePovRayFile(const TXAtomPList &atoms, const TLS &tls,
  short diff_dir,
  const olxstr &fn) const
{
  TGXApp &app = TGXApp::GetInstance();
  olx_cdict<TGlMaterial, olxstr> materials;
  TStrList out = app.GetRenderer().GetScene().ToPov();
  out << TXAtom::PovDeclare(app.GetRenderer());
  out << TXBond::PovDeclare(app.GetRenderer());
  out << TXPlane::PovDeclare();
  out.Add("union {");
  TGXApp::AtomIterator ai = app.GetAtoms();
  while (ai.HasNext())  ai.Next().SetTag(0);
  atoms.ForEach(ACollectionItem::TagSetter(1));
  ai.Reset();
  while (ai.HasNext()) {
    TXAtom &a = ai.Next();
    if (a.IsVisible() && a.GetTag() == 0) {
      out << a.ToPov(materials);
    }
  }
  TGXApp::BondIterator bi = app.GetBonds();
  while (bi.HasNext()) {
    TXBond &b = bi.Next();
    if (b.IsVisible()) {
      out << b.ToPov(materials);
    }
  }
  TGXApp::PlaneIterator pi = app.GetPlanes();
  while (pi.HasNext()) {
    TXPlane &p = pi.Next();
    if (p.IsVisible()) {
      out << p.ToPov(materials);
    }
  }
  evecd Q(6);
  for (size_t i = 0; i < atoms.Count(); i++) {
    TStrList o;
    o << "object { difference {";
    if (diff_dir == xtls_diff_Obs_Tls) {
      o << atoms[i]->ToPov(materials);
      atoms[i]->GetEllipsoid()->GetShelxQuad(Q);
      atoms[i]->GetEllipsoid()->Initialise(tls.GetElpList()[i]);
    }
    else {
      atoms[i]->GetEllipsoid()->GetShelxQuad(Q);
      atoms[i]->GetEllipsoid()->Initialise(tls.GetElpList()[i]);
      o << atoms[i]->ToPov(materials);
      atoms[i]->GetEllipsoid()->Initialise(Q);
    }
    o << atoms[i]->ToPov(materials);
    o << "}}";
    out << o;
    atoms[i]->GetEllipsoid()->Initialise(Q);
  }
  out.Add("}");
  TStrList mat_out;
  for (size_t i = 0; i < materials.Count(); i++) {
    mat_out.Add("#declare ") << materials.GetValue(i) << '=';
    mat_out.Add(materials.GetKey(i).ToPOV());
  }
  TEFile::WriteLines(fn, TCStrList(mat_out << out));
}
//.............................................................................
#endif // _CONSOLE
