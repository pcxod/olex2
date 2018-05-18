/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "vcov.h"
#include "refmodel.h"
#include "xapp.h"

//.............................................................................
VcoVMatrix::VcoVMatrix() : data(NULL), count(0) {
  U_annotations.Add("u11");
  U_annotations.Add("u22");
  U_annotations.Add("u33");
  U_annotations.Add("u23");
  U_annotations.Add("u13");
  U_annotations.Add("u12");
}
//.............................................................................
void VcoVMatrix::ReadShelxMat(const olxstr& fileName, TAsymmUnit& au) {
  Clear();
  olxstr lstFN = TEFile::ChangeFileExt(fileName, "lst");
  if (TEFile::Exists(lstFN) && TEFile::Exists(fileName)) {
    time_t lst_fa = TEFile::FileAge(lstFN);
    time_t mat_fa = TEFile::FileAge(fileName);
    if (lst_fa > mat_fa && (lst_fa - mat_fa) > 5) {
      TBasicApp::NewLogEntry(logWarning) << "The mat file is possibly out of date";
    }
  }
  TCStrList sl = TEFile::ReadCLines(fileName),
    toks;
  if (sl.Count() < 10) {
    throw TFunctionFailedException(__OlxSourceInfo, "invalid file content");
  }
  toks.Strtok(sl[3], ' ');
  const size_t param_cnt = toks[0].ToSizeT();
  TSizeList indexes;
  TDoubleList diag;
  olx_pdict<size_t, eveci> Us;
  olx_pdict<size_t, SiteSymmCon> SiteConstraints;
  if (param_cnt == 0 || sl.Count() < param_cnt + 11) {
    throw TFunctionFailedException(__OlxSourceInfo,
      "empty/invalid matrix file");
  }
  olxstr last_atom_name;
  size_t ua_index;
  TCAtom *atom = NULL;
  for (size_t i = 1; i < param_cnt; i++) { // skipp OSF
    toks.Clear();
    toks.Strtok(sl[i + 7], ' ');
    if (toks[0].ToSizeT() != i + 1 || toks.Count() != 6) {
      if (toks.Count() == 5) {
        continue;
      }
      throw TFunctionFailedException(__OlxSourceInfo, "invalid matrix file");
    }
    if (toks[4].Equals("BASF") ||
      toks[4].Equals("SWAT") ||
      toks[4].Equals("HOPE"))
    {
      continue;
    }
    if (toks[4].StartsFrom("FVAR")) {
      if (toks.Count() != 6) {
        continue;
      }
      size_t var_ind = toks[5].ToSizeT();
      if (au.GetRefMod()->Vars.VarCount() < var_ind) {
        throw TFunctionFailedException(__OlxSourceInfo,
          "model does not relate to the matrix file");
      }
      XVar& var = au.GetRefMod()->Vars.GetVar(var_ind - 1);
      const double esd = toks[2].ToDouble();
      for (size_t j = 0; j < var.RefCount(); j++) {
        XVarReference& r = var.GetRef(j);
        if (!EsdlInstanceOf(r.referencer, TCAtom)) {
          continue;
        }
        TCAtom& ca = (TCAtom&)r.referencer;
        if (r.var_index == catom_var_name_Sof) {
          ca.SetOccuEsd(esd);
        }
        else if (r.var_index == catom_var_name_Uiso) {
          ca.SetUisoEsd(esd);
        }
      }
      continue;
    }
    if (last_atom_name != toks[5]) {
      atom = au.FindCAtom(toks[5]);
      last_atom_name = toks[5];
    }
    if (atom == 0) {
      TBasicApp::NewLogEntry(logError) <<
        "Mismatching matrix file, could not locate: " << last_atom_name;
      continue;
    }
    const size_t ssci = SiteConstraints.IndexOf(atom->GetId());
    const SiteSymmCon& ssc = (ssci == InvalidIndex ?
      SiteConstraints.Add(atom->GetId(), atom->GetSiteConstraints()) :
      SiteConstraints.GetValue(ssci));
    if (toks[4].CharAt(0) == 'x') {
      diag.Add(toks[2].ToDouble());
      atom->ccrdEsd()[0] = diag.GetLast();
      short index_v = vcoviX;
      if (ssc.map[7].param == 0) {
        index_v |= vcoviY;
      }
      if (ssc.map[8].param == 0) {
        index_v |= vcoviZ;
      }
      Index.AddNew(toks[5], index_v, -1);
      indexes.Add(i);
    }
    else if (toks[4].CharAt(0) == 'y') {
      diag.Add(toks[2].ToDouble());
      atom->ccrdEsd()[1] = diag.GetLast();
      short index_v = vcoviY;
      if (ssc.map[8].param == 1) {
        index_v |= vcoviZ;
      }
      Index.AddNew(toks[5], index_v, -1);
      indexes.Add(i);
    }
    else if (toks[4].CharAt(0) == 'z') {
      diag.Add(toks[2].ToDouble());
      atom->ccrdEsd()[2] = diag.GetLast();
      Index.AddNew(toks[5], vcoviZ, -1);
      indexes.Add(i);
    }
    else if (toks[4] == "sof") {
      diag.Add(toks[2].ToDouble());
      atom->SetOccuEsd(diag.GetLast());
      Index.AddNew(toks[5], vcoviO, -1);
      indexes.Add(i);
    }
    else if ((ua_index = U_annotations.IndexOfi(toks[4])) != InvalidIndex) {
      if (atom->GetEllipsoid() != 0) {
        atom->GetEllipsoid()->SetEsd(ua_index, toks[2].ToDouble());
        eveci& v = Us.Add(atom->GetId());
        if (v.Count() == 0) {
          v.Resize(6);
          for (int vi = 0; vi < 6; vi++) {
            v[vi] = -1;
          }
        }
        v[ua_index] = (int)i;
      }
      else {
        if (ua_index != 0) {
          throw TInvalidArgumentException(__OlxSourceInfo,
            "U for isotropic atom");
        }
        atom->SetUisoEsd(toks[2].ToDouble());
      }
    }
  }
  TDoubleList all_vcov((param_cnt + 1)*param_cnt / 2);
  size_t vcov_cnt = 0;
  for (size_t i = 0; i < sl.Count(); i++) {
    const size_t ind = i + param_cnt + 10;
    if (sl[ind].Length() < 8) {
      break;
    }
    const size_t ll = sl[ind].Length();
    size_t s_ind = 0;
    while (s_ind < ll) {
      all_vcov[vcov_cnt++] = sl[ind].SubString(s_ind, 8).ToDouble();
      s_ind += 8;
    }
  }
  Allocate(diag.Count());
  for (size_t i = 0; i < indexes.Count(); i++) {
    for (size_t j = 0; j <= i; j++) {
      if (i == j) {
        data[i][j] = diag[i] * diag[i];
      }
      else {  // top diagonal to bottom diagonal
        size_t ix = indexes[j];
        size_t iy = indexes[i];
        if (ix > iy) {
          olx_swap(ix, iy);
        }
        const size_t ind = ix*(2 * param_cnt - ix - 1) / 2 + iy;
        data[i][j] = all_vcov[ind] * diag[i] * diag[j];
      }
    }
  }
  for (size_t i = 0; i < Index.Count(); i++) {
    TCAtom* ca = au.FindCAtom(Index[i].GetA());
    if (ca == 0) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "matrix is not upto date");
    }
    Index[i].c = ca->GetId();
    size_t j = i;
    while (++j < Index.Count() && Index[i].GetA().Equalsi(Index[j].GetA())) {
      Index[j].c = ca->GetId();
    }
    i = j - 1;
  }
  // expand refined parameters into crystallographic ones
  try {
    const mat3d& f2c = au.GetCellToCartesian();
    const mat3d& h2c = au.GetHklToCartesian();
    const double Ot[] = {
      h2c[0].QLength(), h2c[1].QLength(), h2c[2].QLength(),
      sqrt(Ot[1] * Ot[2]), sqrt(Ot[0] * Ot[2]), sqrt(Ot[0] * Ot[1])
    };
    evecd Ut(6);
    Ut[0] = (f2c[0][0] * f2c[0][0]);
    Ut[1] = (f2c[1][0] * f2c[1][0] + f2c[1][1] * f2c[1][1]);
    Ut[2] = (f2c[2][0] * f2c[2][0] + f2c[2][1] * f2c[2][1] + f2c[2][2] * f2c[2][2]);
    Ut[3] = 2 * (f2c[0][0] * f2c[1][0]);
    Ut[4] = 2 * (f2c[0][0] * f2c[2][0]);
    Ut[5] = 2 * (f2c[1][0] * f2c[2][0] + f2c[1][1] * f2c[2][1]);
    Ut *= 1. / 3;
    evecd Q(6), E(6);
    ematd Um(6, 6);
    for (size_t i = 0; i < au.AtomCount(); i++) {
      TCAtom& a = au.GetAtom(i);
      const size_t ssci = SiteConstraints.IndexOf(a.GetId());
      const SiteSymmCon& ssc = (ssci == InvalidIndex ?
        SiteConstraints.Add(a.GetId(), a.GetSiteConstraints()) :
        SiteConstraints.GetValue(ssci));
      const bool constrained = ssc.IsConstrained();
      if (constrained) {
        for (size_t j = 0; j < 3; j++) {
          if (ssc.map[6 + j].param >= 0) {
            a.ccrdEsd()[j] = olx_abs(a.ccrdEsd()[ssc.map[6 + j].param] *
              ssc.map[6 + j].multiplier);
          }
          else {
            a.ccrdEsd()[j] = 0;
          }
        }
      }
      TEllipsoid* e = a.GetEllipsoid();
      if (e == NULL)  continue;
      if (constrained) {
        for (size_t j = 0; j < 6; j++) {
          if (ssc.map[j].param >= 0) {
            e->SetEsd(j,
              olx_abs(e->GetEsd(ssc.map[j].param)*ssc.map[j].multiplier));
            e->SetQuad(j, e->GetQuad(ssc.map[j].param)*ssc.map[j].multiplier);
          }
          else {
            e->SetEsd(j, 0);
          }
        }
        e->GetShelxQuad(Q, E);
        e->Initialise(au.UcifToUcart(Q), E);
        a.SetUiso((Q[0] + Q[2] + Q[3]) / 3);
      }
      // get Uiso esd...
      const size_t ui = Us.IndexOf(a.GetId());
      if (ui == InvalidIndex)  continue;
      eveci& v = Us.GetValue(ui);
      bool failed = false;
      for (int vi = 0; vi < 6; vi++) {  // build U* esd's VcV
        Um[vi][vi] = olx_sqr(e->GetEsd(vi)*Ot[vi]);
        for (int vj = vi + 1; vj < 6; vj++) {
          if (ssc.map[vi].param < 0 || ssc.map[vj].param < 0) {
            Um[vi][vj] = Um[vj][vi] = 0;
            continue;
          }
          int x = v[ssc.map[vi].param];
          int y = v[ssc.map[vj].param];
          if (x == -1 || y == -1) {
            failed = true;
            TBasicApp::NewLogEntry(logError) <<
              "Failed to evaluate esd of Uiso...";
            break;
          }
          if (x > y) {
            olx_swap(x, y);
          }
          const size_t ind = x*(2 * param_cnt - x - 1) / 2 + y;
          Um[vi][vj] = Um[vj][vi] = all_vcov[ind] * e->GetEsd(vi)*e->GetEsd(vj) *
            Ot[vi] * Ot[vj];
        }
        if (failed) {
          break;
        }
      }
      if (!failed) {
        Um.SwapRows(3, 5);  //put into the right order for Ut
        Um.SwapCols(3, 5);
        const double Ueq = (Um*Ut).DotProd(Ut);
        a.SetUisoEsd(sqrt(Ueq));
      }
    }
    UpdateAtomIndex();
  }
  catch (const TExceptionBase& e) {
    TBasicApp::NewLogEntry(logError) << e.GetException()->GetFullMessage();
  }
}
//.............................................................................
void VcoVMatrix::UpdateAtomIndex() {
  AtomIdIndex.Clear();
  AtomIdIndex.SetCapacity(Index.Count());
  AtomNameIndex.Clear();
  AtomNameIndex.SetCapacity(Index.Count());
  for (size_t i = 0; i < Index.Count(); i++) {
    AtomIdIndex.Add(Index[i].GetC(), i);
    AtomNameIndex.Add(Index[i].GetA(), i);
  }
}
//.............................................................................
double VcoVMatrix::Find(const olxstr& atom, const short va,
  const short vb) const
{
  size_t ni = AtomNameIndex.IndexOf(atom);
  if (ni == InvalidIndex) {
    return 0;
  }
  size_t i1 = InvalidIndex, i2 = InvalidIndex;
  for (size_t j = ni; j < Index.Count() && Index[j].GetA() == atom; j++) {
    if (Index[j].GetB() == va)  i1 = j;
    if (Index[j].GetB() == vb)  i2 = j;
  }
  if (i1 == InvalidIndex || i2 == InvalidIndex)
    return 0;
  return (i1 <= i2) ? data[i2][i1] : data[i1][i2];
}
//.............................................................................
void VcoVMatrix::ReadSmtbxMat(const olxstr& fileName, TAsymmUnit& au) {
  TStrList in = TEFile::ReadLines(fileName);
  if (in.Count() != 3 || !in[0].Equals("VCOV")) {
    throw TInvalidArgumentException(__OlxSourceInfo, "file format");
  }
  TStrList annotations(in[1], ' '),
    values(in[2], ' ');
  if (((annotations.Count()*(annotations.Count() + 1))) / 2 != values.Count()) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      "inconsistent matrix and annotations");
  }

  olxstr last_atom_name;
  TSizeList indexes;
  TCAtom* atom = NULL;
  olx_pdict<size_t, eveci> Us;
  const mat3d& h2c = au.GetHklToCartesian();
  const double O[6] = {
    1. / h2c[0].QLength(), 1. / h2c[1].QLength(), 1. / h2c[2].QLength(),
    sqrt(O[1] * O[2]), sqrt(O[0] * O[2]), sqrt(O[0] * O[1])
  };
  size_t ua_index, d_index = 0;
  for (size_t i = 0; i < annotations.Count(); i++) {
    if (i != 0) {
      d_index += (annotations.Count() - i + 1);
    }
    const size_t di = annotations[i].IndexOf('.');
    if (di == InvalidIndex) {
      throw TInvalidArgumentException(__OlxSourceInfo, "annotation");
    }
    const olxstr atom_name = annotations[i].SubStringTo(di);
    const olxstr param_name = annotations[i].SubStringFrom(di + 1);
    if (last_atom_name != atom_name) {
      atom = au.FindCAtom(atom_name);
      last_atom_name = atom_name;
    }
    if (atom == 0) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "mismatching matrix file");
    }
    if (param_name == 'x') {
      atom->ccrdEsd()[0] = sqrt(values[d_index].ToDouble());
      Index.AddNew(atom_name, vcoviX, -1);
      indexes.Add(i);
    }
    else if (param_name == 'y') {
      atom->ccrdEsd()[1] = sqrt(values[d_index].ToDouble());
      Index.AddNew(atom_name, vcoviY, -1);
      indexes.Add(i);
    }
    else if (param_name == 'z') {
      atom->ccrdEsd()[2] = sqrt(values[d_index].ToDouble());
      Index.AddNew(atom_name, vcoviZ, -1);
      indexes.Add(i);
    }
    else if (param_name == "occu") {
      atom->SetOccuEsd(sqrt(values[d_index].ToDouble()));
      Index.AddNew(atom_name, vcoviO, -1);
      indexes.Add(i);
    }
    else if (param_name == "uiso") {
      atom->SetUisoEsd(sqrt(values[d_index].ToDouble()));
    }
    else if ((ua_index = U_annotations.IndexOf(param_name)) != InvalidIndex) {
      if (atom->GetEllipsoid() == 0) {
        throw TInvalidArgumentException(__OlxSourceInfo,
          "U for isotropic atom");
      }
      atom->GetEllipsoid()->SetEsd(ua_index, values[d_index].ToDouble());
      eveci& v = Us.Add(atom->GetId());
      if (v.Count() == 0) {
        v.Resize(6);
      }
      // put indices in the smtbx order
      if (ua_index == 3) {
        ua_index = 5;
      }
      else if (ua_index == 5) {
        ua_index = 3;
      }
      v[ua_index] = (int)i;
    }
  }
  Allocate(indexes.Count());
  for (size_t i = 0; i < indexes.Count(); i++) {
    for (size_t j = 0; j <= i; j++) {
      size_t ix = indexes[j];
      size_t iy = indexes[i];
      if (ix > iy) {
        olx_swap(ix, iy);
      }
      const size_t ind = ix*(2 * annotations.Count() - ix - 1) / 2 + iy;
      data[i][j] = values[ind].ToDouble();
    }
  }
  for (size_t i = 0; i < Index.Count(); i++) {
    TCAtom* ca = au.FindCAtom(Index[i].GetA());
    Index[i].c = ca->GetId();
    size_t j = i;
    while (++j < Index.Count() && Index[i].GetA().Equalsi(Index[j].GetA())) {
      Index[j].c = ca->GetId();
    }
    i = j - 1;
  }
  const mat3d& f2c = au.GetCellToCartesian();
  evecd Ut(6);
  Ut[0] = (f2c[0][0] * f2c[0][0]);
  Ut[1] = (f2c[1][0] * f2c[1][0] + f2c[1][1] * f2c[1][1]);
  Ut[2] = (f2c[2][0] * f2c[2][0] + f2c[2][1] * f2c[2][1] + f2c[2][2] * f2c[2][2]);
  Ut[3] = 2 * (f2c[0][0] * f2c[1][0]);
  Ut[4] = 2 * (f2c[0][0] * f2c[2][0]);
  Ut[5] = 2 * (f2c[1][0] * f2c[2][0] + f2c[1][1] * f2c[2][1]);
  Ut *= 1. / 3;
  ematd Um(6, 6);
  for (size_t i = 0; i < au.AtomCount(); i++) {
    TCAtom& a = au.GetAtom(i);
    if (a.GetEllipsoid() == 0) {
      continue;
    }
    TEllipsoid& elp = *a.GetEllipsoid();
    const size_t ui = Us.IndexOf(a.GetId());
    if (ui == InvalidIndex) {
      continue;
    }
    eveci& v = Us.GetValue(ui);
    for (int vi = 0; vi < 6; vi++) {
      int src_i = vi;
      if (src_i == 3)  src_i = 5;
      else if (src_i == 5)  src_i = 3;
      Um[vi][vi] = elp.GetEsd(src_i);
      elp.SetEsd(src_i, sqrt(elp.GetEsd(src_i))*O[src_i]);
      for (int vj = vi + 1; vj < 6; vj++) {
        int x = v[vi];
        int y = v[vj];
        if (x > y) {
          olx_swap(x, y);
        }
        const size_t ind = x*(2 * annotations.Count() - x - 1) / 2 + y;
        Um[vi][vj] = Um[vj][vi] = values[ind].ToDouble();
      }
    }
    const double Ueq = (Um*Ut).DotProd(Ut);
    a.SetUisoEsd(sqrt(Ueq));
  }
  UpdateAtomIndex();
}
//.............................................................................
void VcoVMatrix::FromCIF(TAsymmUnit& au) {
  Index.SetCapacity(au.AtomCount());
  Allocate(au.AtomCount()*3, true);
  for (size_t i=0; i < au.AtomCount(); i++) {
    TCAtom &a = au.GetAtom(i);
    Index.AddNew(a.GetLabel(), vcoviX, a.GetId());
    Index.AddNew(a.GetLabel(), vcoviY, a.GetId());
    Index.AddNew(a.GetLabel(), vcoviZ, a.GetId());
    for (int j=0; j < 3; j++) {
      data[0][i*3+j] = olx_sqr(a.ccrdEsd()[j]);
    }
  }
  UpdateAtomIndex();
}
//.............................................................................
//.............................................................................
//.............................................................................
VcoVContainer::OctahedralDistortion::OctahedralDistortion(
  const vec3d_alist& points) : points(points)
{
  size_t opp = InvalidIndex, opp1 = InvalidIndex;
  double mx = 0;
  for (size_t i = 2; i < points.Count(); i++) {
    double ang = olx_angle(points[1], points[0], points[i]);
    if (ang > mx) {
      opp = i;
      mx = ang;
    }
  }
  for (size_t i = 2; i < points.Count(); i++) {
    if (i == opp) {
      continue;
    }
    angles.AddNew(1, 0, i);
  }
  for (size_t i = 2; i < points.Count(); i++) {
    if (i == opp) continue;
    angles.AddNew(opp, 0, i);
  }
  size_t current = (opp == 2 ? 3 : 2);
  mx = 0;
  for (size_t i = 2; i < points.Count(); i++) {
    if (i == opp || i == current) continue;
    double ang = olx_angle(points[current], points[0], points[i]);
    if (ang > mx) {
      opp1 = i;
      mx = ang;
    }
  }
  for (size_t i = 2; i < points.Count(); i++) {
    if (i == opp || i == current || i == opp1) {
      continue;
    }
    angles.AddNew(current, 0, i);
  }
  for (size_t i = 2; i < points.Count(); i++) {
    if (i == opp || i == opp1 || i == current) continue;
    angles.AddNew(opp1, 0, i);
  }
}
double VcoVContainer::OctahedralDistortion::calc_angle() const {
  double sum = 0;
  for (size_t i = 0; i < angles.Count(); i++) {
    double ang = olx_angle(points[angles[i].a], points[angles[i].b],
      points[angles[i].c]);
    sum += olx_abs(ang - 90);
  }
  return sum;
}
double VcoVContainer::OctahedralDistortion::calc_d_cent() const {
  vec3d cnt = olx_mean(crd_slice(points, 1, points.Count()-1));
  return points[0].DistanceTo(cnt);
}
double VcoVContainer::OctahedralDistortion::calc_d_len() const {
  TArrayList<double> ds(points.Count(), olx_list_init::value(0.0));
  for (size_t i = 1; i < points.Count(); i++) {
    ds[i] = points[0].DistanceTo(points[i]);
    ds[0] += ds[i];
  }
  ds[0] /= (points.Count() - 1);
  double d = 0;
  for (size_t i = 1; i < points.Count(); i++) {
    d += olx_abs(ds[i] - ds[0]);
  }
  return d;
}
//.............................................................................
double VcoVContainer::TriangleTwistBP::calc() const {
  // translation for first face
  const vec3d c1 = (points[0] + points[2] + points[4]) / 3;
  // translation for second face
  const vec3d c2 = (points[1] + points[3] + points[5]) / 3;
  for (short i = 0; i < 6; i += 2) {
    pl[i] = (points[i] - c1);
    pl[i + 1] = (points[i + 1] - c2);
  }
  const PlaneInfo pi = CalcPlane(pl, weights, 0);
  for (short i = 0; i < 6; i++) {
    pl[i] = pl[i].Projection(pi.center, pi.normal);
  }
  for (short i = 0; i < 6; i += 2) {
    angles[i / 2] = acos(pl[i].CAngle(pl[i + 1]));
  }
  return (olx_sum(angles) * 180 / 3) / M_PI;
}
//.............................................................................
//.............................................................................
double VcoVContainer::OctahedralDistortionBP::calc() const {
  // translation for first face
  const vec3d c1 = (points[1] + points[3] + points[5]) / 3;
  // translation for second face
  const vec3d c2 = (points[2] + points[4] + points[6]) / 3;
  for (short i = 0; i < 6; i += 2)  {
    pl[i] = (points[i + 1] - c1);
    pl[i + 1] = (points[i + 2] - c2);
  }

  PlaneInfo pi = CalcPlane(pl, weights, 0);
  double sum = 0;
  for (int i = 0; i < 6; i++) {
    const vec3d v1 = pl[i].Projection(pi.normal);
    const vec3d v2 = pl[i == 5 ? 0 : i + 1].Projection(pi.normal);
    sum += olx_abs(M_PI / 3 - acos(v1.CAngle(v2)));
  }
  return (sum * 180 / 6) / M_PI;
}
