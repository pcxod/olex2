/******************************************************************************
* Copyright (c) 2004-2026 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "structure_generator.h"

bool RandomStructure::special_position(const TAsymmUnit& au, const TUnitCell& uc,
  vec3d& out)
{
  for (int attempt = 0; attempt < 20; attempt++) {
    const smatd& m = uc.GetMatrix(uniform_index(uc.MatrixCount()));
    if (is_identity(m)) {
      continue;
    }
    const vec3d p(uniform(0, 1), uniform(0, 1), uniform(0, 1));
    vec3d gp = m * p;
    gp -= (gp - p).Round<int>();
    const vec3d q = (p + gp) / 2;
    if (min_qdist(au, q - m * q) < 1e-10) {
      out = q - q.Floor<int>();
      return true;
    }
  }
  return false;
}
//..............................................................................
bool RandomStructure::too_close(const TAsymmUnit& au, const TUnitCell& uc,
  const vec3d_list& crds, const vec3d& f, bool special) const
{
  const double qmin = min_distance * min_distance;
  for (size_t mi = 0; mi < uc.MatrixCount(); mi++) {
    const smatd& m = uc.GetMatrix(mi);
    const vec3d g = m * f;
    if (!is_identity(m)) {
      const double qd = min_qdist(au, f - g);
      if (qd < 1e-8 ? !special : qd < qmin) {
        return true;
      }
    }
    for (size_t i = 0; i < crds.Count(); i++) {
      if (min_qdist(au, crds[i] - g) < qmin) {
        return true;
      }
    }
  }
  return false;
}
//..............................................................................
size_t RandomStructure::generate_atoms(TLattice& latt, size_t count) {
  static const char* symbols[] = { "C", "N", "O", "Cl", "Br" };
  static const int weights[] = { 50, 15, 20, 10, 5 };
  const size_t type_count = sizeof(weights) / sizeof(weights[0]);
  const cm_Element* types[type_count];
  for (size_t i = 0; i < type_count; i++) {
    types[i] = XElementLib::FindBySymbol(symbols[i]);
    if (types[i] == 0) {
      throw TInvalidArgumentException(__OlxSourceInfo, "Element: ") << symbols[i];
    }
  }
  std::discrete_distribution<int> type_d(weights, weights + type_count);
  TAsymmUnit& au = latt.GetAsymmUnit();
  const TUnitCell& uc = latt.GetUnitCell();
  vec3d_list crds;
  size_t label_n[type_count] = { 0 };
  for (size_t attempt = 0; attempt < 200 * count && crds.Count() < count;
    attempt++)
  {
    vec3d f;
    const bool special = uniform(0, 1) < special_probability &&
      special_position(au, uc, f);
    if (!special) {
      f = vec3d(uniform(0, 1), uniform(0, 1), uniform(0, 1));
    }
    if (too_close(au, uc, crds, f, special)) {
      continue;
    }
    crds.AddCopy(f);
    // coordinates outside [0,1) describe the same structure
    if (uniform(0, 1) < outside_probability) {
      f[uniform_index(3)] += uniform(0, 1) < 0.5 ? -1 : 1;
    }
    const int t = type_d(rng);
    TCAtom& a = au.NewAtom();
    a.SetType(*types[t]);
    a.SetLabel(olxstr(symbols[t]) << ++label_n[t], false);
    a.ccrd() = f;
  }
  return crds.Count();
}
//..............................................................................
void RandomStructure::sorted_sites(const TCAtom& a, bool interactions,
  TTypeList<TCAtom::Site>& out)
{
  const size_t n = interactions ? a.AttachedSiteICount()
    : a.AttachedSiteCount();
  for (size_t i = 0; i < n; i++) {
    out.AddCopy(interactions ? a.GetAttachedSiteI(i) : a.GetAttachedSite(i));
  }
  QuickSorter::Sort(out);
}
//..............................................................................
void RandomStructure::compare_conn(const TAsymmUnit& ref, const TAsymmUnit& au,
  const olxstr& name)
{
  if (ref.AtomCount() != au.AtomCount()) {
    throw TFunctionFailedException(__OlxSourceInfo,
      olxstr(name) << ": atom count differs");
  }
  for (size_t i = 0; i < ref.AtomCount(); i++) {
    const TCAtom& a = ref.GetAtom(i), & b = au.GetAtom(i);
    if (a.IsDeleted() || b.IsDeleted()) {
      throw TFunctionFailedException(__OlxSourceInfo,
        olxstr(name) << ": unexpected deletion of " << a.GetLabel());
    }
    for (int I = 0; I < 2; I++) {
      TTypeList<TCAtom::Site> sa, sb;
      sorted_sites(a, I != 0, sa);
      sorted_sites(b, I != 0, sb);
      bool same = sa.Count() == sb.Count();
      if (same) {
        for (size_t j = 0; j < sa.Count(); j++) {
          if (!(same = (sa[j].Compare(sb[j]) == 0))) {
            vec3d ca = sa[j].matrix * sa[j].atom->ccrd();
            vec3d cb = sb[j].matrix * sb[j].atom->ccrd();
            if (ca.Equals(cb, 1e-6)) {
              same = true;
              continue;
            }
            break;
          }
        }
      }
      if (!same) {
        throw TFunctionFailedException(__OlxSourceInfo,
          olxstr(name) << ": " << (I ? "interactions" : "bonds")
          << " of " << a.GetLabel() << " differ: " << sa.Count()
          << " vs " << sb.Count());
      }
    }
  }
}
//..............................................................................
//..............................................................................
//..............................................................................
void RandomStructure::structure::reset_conn() {
  TAsymmUnit& au = latt.GetAsymmUnit();
  for (size_t i = 0; i < au.AtomCount(); i++) {
    TCAtom& a = au.GetAtom(i);
    a.ClearAttachedSites();
    a.ClearEquivs();
    a.SetDeleted(false);
  }
}
//..............................................................................
//..............................................................................
//..............................................................................
double RandomStructure::min_qdist(const TAsymmUnit& au, vec3d d) {
  d -= d.Round<int>();
  double best = 1e20;
  for (int i = -1; i <= 1; i++) {
    for (int j = -1; j <= 1; j++) {
      for (int k = -1; k <= 1; k++) {
        best = olx_min(best, au.Orthogonalise(d + vec3d(i, j, k)).QLength());
      }
    }
  }
  return best;
}
//..............................................................................
bool RandomStructure::check_metric(const mat3d& G, const smatd_list& ops) {
  const double tol = 1e-6 * (G[0][0] + G[1][1] + G[2][2]);
  for (size_t o = 0; o < ops.Count(); o++) {
    mat3d m = ops[o].r;
    mat3d Gt = m.GetT() * G * m;
    for (int i = 0; i < 3; i++) {
      for (int j = 0; j < 3; j++) {
        if (olx_abs(G[i][j] - Gt[i][j]) > tol) {
          return false;
        }
      }
    }
  }
  return true;
}
//..............................................................................
olx_object_ptr<RandomStructure::structure> RandomStructure::create_structure(
  const vec3d& axes, const vec3d& angles, const TSpaceGroup& sg)
{
  olx_object_ptr<structure> str = new structure();
  TLattice& latt = str->latt;
  TAsymmUnit& au = latt.GetAsymmUnit();
  au.GetAxes() = axes;
  au.GetAngles() = angles;
  au.InitMatrices();
  const mat3d& M = au.GetCellToCartesian();
  mat3d G = M * M.GetT();
  if (!check_metric(G, sg.GetMatrices())) {
    return 0;
  }
  au.ChangeSpaceGroup(sg);
  latt.GetUnitCell().InitMatrices();
  return str;
}
//..............................................................................
