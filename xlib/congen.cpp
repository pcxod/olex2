/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "congen.h"
#include "asymmunit.h"
#include "satom.h"
#include "network.h"
#include "unitcell.h"
#include "lattice.h"
#include "label_corrector.h"
#include "xapp.h"

static const double THA = acos(-1./3)*180/M_PI,
  water_angle = 104.5;
//.............................................................................
AConstraintGenerator::AConstraintGenerator(RefinementModel& rm)
  : UseRestrains(false),
  RefMod(rm)
{
  Distances(GenId(fgBH1, 3), 0.98);
  Distances(GenId(fgBH1, 5), 1.10);

  Distances(GenId(fgCH3, 1), 0.96);
  Distances(GenId(fgCH2, 2), 0.97);
  Distances(GenId(fgCH2, 1), 0.86);
  Distances(GenId(fgCH1, 3), 0.98);
  Distances(GenId(fgCH1, 2), 0.93);
  Distances(GenId(fgCH1, 1), 0.93);

  Distances(GenId(fgOH2, 0), 0.85);
  Distances(GenId(fgOH1, 0), 0.82);
  Distances(GenId(fgOH1, 1), 0.82);
  Distances(GenId(fgOH1, 3), 0.85);

  Distances(GenId(fgNH3, 0), 0.89);
  Distances(GenId(fgNH2, 0), 0.86);
  Distances(GenId(fgNH1, 0), 0.86);
  Distances(GenId(fgNH1, 3), 0.91);

  Distances(GenId(fgSiH1, 3), 1.00);
  Distances(GenId(fgSiH2, 2), 1.43);

  Distances(GenId(fgSH1, 0), 1.2);
}
//.............................................................................
void AConstraintGenerator::ApplyCorrection(double v) {
  for (size_t i = 0; i < Distances.Count(); i++) {
    Distances.GetValue(i) += v;
  }
}
//.............................................................................
void AConstraintGenerator::DoGenerateAtom(TResidue &r, TCAtomPList& created,
  TAsymmUnit& au, vec3d_list& Crds, const olxstr& StartingName)
{
  LabelCorrector lc(au, TXApp::GetMaxLabelLength(),
    TXApp::DoRenameParts());
  const bool IncLabel = (Crds.Count() != 1);
  for (size_t i = 0; i < Crds.Count(); i++) {
    TCAtom& CA = au.NewAtom(&r);
    CA.ccrd() = au.Fractionalise(Crds[i]);
    CA.SetType(XElementLib::GetByIndex(iHydrogenIndex));
    if (IncLabel) {
      olxstr lbl = (StartingName + (char)('a' + i));
      CA.SetLabel(lbl, false);
    }
    else {
      CA.SetLabel(StartingName, false);
    }
    lc.CorrectGlobal(CA);
    created.Add(CA);
  }
}
//.............................................................................
vec3d AConstraintGenerator::Generate_1(short group,
  const TAtomEnvi& envi) const
{
  double dis = Distances.Get(AConstraintGenerator::GenId(group, (uint16_t)envi.Count()));
  // proposed by Luc, see Afix 13 in shelxl
  bool AnglesEqual = (envi.Count() == 3);
  if (AnglesEqual) {
    for (size_t i = 0; i < envi.Count(); i++) {
      if (envi.GetCrd(i).DistanceTo(envi.crd()) > 1.95 &&
        envi.GetType(i) != iBromineZ)
      {
        AnglesEqual = false;
        break;
      }
    }
  }
  if (AnglesEqual) {
    vec3d Vec1 = envi.GetVec(0).Normalise();
    vec3d Vec2 = envi.GetVec(1).Normalise();
    vec3d Vec3 = envi.GetVec(2).Normalise();
    vec3d rv = (Vec1 + Vec2 + Vec3).Normalise();
    Vec1 -= Vec2;
    Vec3 -= Vec2;
    Vec3 = Vec1.XProdVec(Vec3);
    Vec3.NormaliseTo(rv.CAngle(Vec3) < 0 ? dis : -dis);
    return (Vec3 += envi.crd());
  }
  else {
    vec3d Vec1;
    for (size_t i = 0; i < envi.Count(); i++) {
      Vec1 += envi.GetVec(i).Normalise();
    }
    return Vec1.NormaliseTo(-dis) + envi.crd();
  }
}
//.............................................................................
void AConstraintGenerator::GenerateAtom(TCAtomPList& created, TAtomEnvi& envi,
  const short Group, const cm_Element& atomType, TAtomEnvi* pivoting)
{
  TAsymmUnit &au = *envi.GetBase().CAtom().GetParent();
  const TUnitCell &uc = au.GetLattice().GetUnitCell();
  mat3d M, M1;
  M.I();
  M1.I();
  // idialised triangle in XY plane
  static const vec3d Z(0, 0, 1), X(1, 0, 0);
  TSAtom* NA;
  double dis = 0;
  vec3d_list crds;
  olxstr tmp = atomType.symbol;
  tmp << envi.GetBase().GetLabel().SubStringFrom(
    envi.GetBase().GetType().symbol.Length());
  if (tmp.Length() > 3) {
    tmp.SetLength(3);
  }

  switch (Group) {
  case fgCH3:
  case fgCH3x2:
    dis = Distances.Get(GenId(fgCH3, 1));
    if (envi.Count() == 1) {
      NA = envi.GetBase().GetNetwork().GetLattice().FindSAtom(
        envi.GetCAtom(0));
      TAtomEnvi NEnvi = envi.GetUC().GetAtomEnviList(*NA);
      NEnvi.Exclude(envi.GetBase().CAtom());
      /* best approximation, though not really required (one atom in plane of
      the subs and two - out)...
      */
      if (NEnvi.Count() >= 2) {
        vec3d RotVec = (NEnvi.crd() - envi.crd()).Normalise();
        olx_create_rotation_matrix(M, RotVec, -0.5);

        vec3d Vec1 = NEnvi.GetVec(0);
        if (olx_abs(olx_abs(Vec1.CAngle(RotVec)) - 1) < 1e-6)
          Vec1 = NEnvi.GetCrd(1) - NEnvi.crd();
        Vec1 = M * Vec1;

        vec3d Vec2 = RotVec.XProdVec(Vec1).Normalise();
        olx_create_rotation_matrix(M1, Vec2, cos(M_PI*THA / 180));

        RotVec = M1 * RotVec;
        RotVec *= dis;
        crds.AddNew(RotVec);
        RotVec = M * RotVec;  // 120 degree
        crds.AddNew(RotVec);
        RotVec = M * RotVec;  // 240 degree
        crds.AddNew(RotVec);
        for (int i = 0; i < 3; i++) {
          crds[i] += envi.crd();
        }
      }
    }
    if (crds.IsEmpty()) {
      vec3d PlaneN = envi.GetVec(0).Normalise();
      vec3d RotVec = PlaneN.XProdVec(PlaneN.IsParallel(Z) ? X : Z).Normalise();
      olx_create_rotation_matrix(M, RotVec, cos(M_PI*THA / 180));
      crds.AddNew(M*PlaneN);
      olx_create_rotation_matrix(M, PlaneN, cos(M_PI*120. / 180));
      crds.AddNew(M*crds[0]);
      crds.AddNew(M*crds[1]);

      for (size_t i = 0; i < crds.Count(); i++) {
        crds[i] *= dis;
        crds[i] += envi.crd();
      }
    }
    if (Group == fgCH3x2) {
      vec3d RotVec = (olx_mean(crds) - envi.crd()).Normalise();
      olx_create_rotation_matrix(M, RotVec, cos(M_PI*60. / 180));
      size_t cnt = crds.Count();
      for (size_t i = 0; i < cnt; i++) {
        crds.AddCopy(M*(crds[i] - envi.crd())
          + envi.crd());
      }
    }
    break;
  case fgCH2:
    if (envi.Count() == 2) {
      dis = -Distances.Get(GenId(fgCH2, 2));
      // summ vector
      vec3d Vec1 = (envi.GetVec(0).Normalise() +
        envi.GetVec(1).Normalise()).Normalise();
      vec3d RotVec = envi.GetVec(0).XProdVec(envi.GetVec(1)).Normalise();
      olx_create_rotation_matrix(M, RotVec,
        cos(M_PI*THA / 360), sin(M_PI*THA / 360));
      crds.AddNew(M*Vec1);
      olx_create_rotation_matrix(M, RotVec,
        cos(-M_PI*THA / 360), sin(-M_PI*THA / 360));
      crds.AddNew(M*Vec1);
      // final 90 degree rotation
      olx_create_rotation_matrix(M, Vec1, 0, 1);
      crds[0] = (M*crds[0])*dis + envi.crd();
      crds[1] = (M*crds[1])*dis + envi.crd();
    }
    else if (envi.Count() == 1) {
      dis = Distances.Get(GenId(fgCH2, 1));
      NA = envi.GetBase().GetNetwork().GetLattice().FindSAtom(
        envi.GetCAtom(0));
      TAtomEnvi NEnvi = envi.GetUC().GetAtomEnviList(*NA);
      if (NEnvi.Count() >= 2) {  // have to create in plane
        NEnvi.Exclude(envi.GetBase().CAtom());
        if (NEnvi.Count() < 1) {
          throw TFunctionFailedException(__OlxSourceInfo, "assert");
        }
        vec3d Vec1 = NEnvi.GetVec(0);
        vec3d Vec2 = envi.crd() - NEnvi.crd();
        vec3d PlaneN = Vec1.XProdVec(Vec2).Normalise();
        olx_create_rotation_matrix(M, PlaneN, -0.5);
        Vec1 = (NEnvi.crd() - envi.crd()).Normalise();

        Vec1 = M * Vec1;
        crds.AddNew(Vec1*dis + envi.crd());

        Vec1 = M * Vec1;
        crds.AddNew(Vec1*dis + envi.crd());
      }
    }
    break;
  case fgCH1:
  {
    crds.AddNew(Generate_1(fgCH1, envi));
  }
  break;
  case fgOH3:
    break;
  case fgOH2:
    dis = Distances.Get(GenId(fgOH2, 0));
    if (envi.IsEmpty()) {
      vec3d_list h_crds;
      TCAtom &a = envi.GetBase().CAtom();
      for (size_t si = 0; si < a.AttachedSiteICount(); si++) {
        TCAtom::Site &s = a.GetAttachedSiteI(si);
        if (s.atom->GetType() == iHydrogenZ) {
          smatd m = uc.MulMatrix(s.matrix, envi.GetBase().GetMatrix());
          h_crds.AddCopy(au.Orthogonalise(m*s.atom->ccrd()));
        }
      }
      if (h_crds.Count() == 2) {  // tetrahedral, check first
        vec3d Vec1 = -((h_crds[0] - envi.crd()).Normalise() +
          (h_crds[1] - envi.crd()).Normalise()).Normalise();
        vec3d RotVec = (h_crds[0] - envi.crd()).XProdVec(
          h_crds[1] - envi.crd()).Normalise();
        double ang = M_PI*THA / 360;
        olx_create_rotation_matrix(M, RotVec, cos(ang), sin(ang));
        crds.AddNew(M*Vec1);
        olx_create_rotation_matrix(M, RotVec, cos(-ang), sin(-ang));
        crds.AddNew(M*Vec1);
        // final 180 degree rotation
        olx_create_rotation_matrix(M, Vec1, 0, 1);
        crds[0] = (M*crds[0])*dis + envi.crd();
        crds[1] = (M*crds[1])*dis + envi.crd();
      }
      else if (pivoting != 0 && !pivoting->IsEmpty()) {
        if (pivoting->Count() >= 2) {
          vec3d Vec1 = pivoting->GetVec(0).NormaliseTo(dis);
          crds.AddNew(pivoting->crd() + Vec1);
          vec3d Vec2 = pivoting->GetVec(1);
          vec3d  RotVec = Vec1.XProdVec(Vec2).Normalise();
          double ang = water_angle * M_PI / 180;
          olx_create_rotation_matrix(M, RotVec, cos(-ang), sin(-ang));
          crds.AddNew(pivoting->crd() + M*Vec1);
        }
        else if (pivoting->Count() == 1) {
          vec3d RotVec;
          vec3d PlaneN = (envi.crd() - pivoting->GetCrd(0)).Normalise();
          if (PlaneN.IsParallel(Z, 1e-3)) {
            RotVec = X;
          }
          else {
            RotVec = Z;
          }
          RotVec = RotVec.XProdVec(PlaneN).Normalise();
          PlaneN.NormaliseTo(dis);
          double ang = M_PI*THA / 360;
          olx_create_rotation_matrix(M, RotVec, cos(ang), sin(ang));
          crds.AddCopy(envi.crd() + M*PlaneN);
          olx_create_rotation_matrix(M, RotVec, cos(-ang), sin(-ang));
          crds.AddCopy(envi.crd() + M*PlaneN);
        }
      }
    }
    else if (envi.Count() == 1) {
      if (pivoting == 0 || pivoting->IsEmpty()) {
        vec3d RotVec;
        vec3d PlaneN = (envi.crd() - envi.GetCrd(0)).Normalise();
        if (PlaneN.IsParallel(Z, 1e-3)) {
          RotVec = X;
        }
        else {
          RotVec = Z;
        }
        RotVec = RotVec.XProdVec(PlaneN).Normalise();
        PlaneN.NormaliseTo(dis);
        double ang = M_PI*water_angle / 360;
        olx_create_rotation_matrix(M, RotVec, cos(ang), sin(ang));
        crds.AddCopy(envi.crd() + M*PlaneN);
        olx_create_rotation_matrix(M, RotVec, cos(-ang), sin(-ang));
        crds.AddCopy(envi.crd() + M*PlaneN);
      }
      else if (pivoting->Count() == 2) {
        double tv = olx_tetrahedron_volume(envi.crd(),
          envi.GetCrd(0), pivoting->GetCrd(0), pivoting->GetCrd(1));
        if (tv < 1) {  //flat-ish? ~ 2A^3 for 'true' tetrahedral
          vec3d v1 = envi.GetVec(0);
          vec3d v2 = (pivoting->GetCrd(0) - envi.crd());
          olx_create_rotation_matrix(M, v1.XProdVec(v2).Normalise(),
            cos(water_angle *M_PI/180));
          crds.AddCopy(v2.NormaliseTo(dis));
          crds.AddCopy(v2 * M);
        }
        else {
          vec3d PlaneN = -envi.GetVec(0).Normalise();
          vec3d pv1 = pivoting->GetCrd(0) - envi.crd();
          vec3d pv2 = pivoting->GetCrd(1) - envi.crd();
          vec3d RotVec = PlaneN.XProdVec(pv1).Normalise();
          double ang = M_PI * (180 - THA) / 180;
          olx_create_rotation_matrix(M, RotVec, cos(ang));
          crds.AddCopy((PlaneN*M).NormaliseTo(dis));
          ang = (cos(water_angle*M_PI / 180) - olx_sqr(cos((180 - THA)*M_PI / 180)))
            / olx_sqr(cos((THA - 90)*M_PI / 180));
          olx_create_rotation_matrix(M, PlaneN, ang);
          vec3d t_v1 = crds[0] * M;
          vec3d t_v2 = M * crds[0];
          if (pv2.CAngle(t_v1) > pv2.CAngle(t_v2)) {
            crds.AddCopy(t_v1.NormaliseTo(dis));
          }
          else {
            crds.AddCopy(t_v2.NormaliseTo(dis));
          }
        }
        crds[0] += envi.crd();
        crds[1] += envi.crd();
      }
      else {
        vec3d PlaneN = -envi.GetVec(0).Normalise();
        vec3d RotVec = PlaneN.XProdVec(pivoting->GetCrd(0) - envi.crd())
          .Normalise();
        double ang = M_PI*(180 - THA) / 180;
        olx_create_rotation_matrix(M, RotVec, cos(ang));
        crds.AddCopy((PlaneN*M).NormaliseTo(dis));
        ang = (cos(water_angle*M_PI / 180) - olx_sqr(cos((180 - THA)*M_PI / 180)))
          / olx_sqr(cos((THA-90)*M_PI/180));
        olx_create_rotation_matrix(M, PlaneN, ang);
        crds.AddCopy(envi.crd() + M*crds[0]);
        crds[0] += envi.crd();
      }
    }
    else if (envi.Count() >= 2) { // TH
      vec3d Vec1 = -(envi.GetVec(0).Normalise() +
        envi.GetVec(1).Normalise()).Normalise();
      vec3d RotVec = envi.GetVec(0).XProdVec(envi.GetVec(1)).Normalise();
      double ang = water_angle*M_PI / 360;
      olx_create_rotation_matrix(M, RotVec, cos(ang));
      crds.AddNew(M*Vec1);
      olx_create_rotation_matrix(M, RotVec, cos(-ang), sin(-ang));
      crds.AddNew(M*Vec1);
      // final 180 degree rotation
      olx_create_rotation_matrix(M, Vec1, 0, 1);
      crds[0] = (M*crds[0])*dis + envi.crd();
      crds[1] = (M*crds[1])*dis + envi.crd();
    }
    if (crds.IsEmpty()) { // "random" in Z-X placement
      vec3d Vec1(0, 0, 1);
      vec3d RotVec(0, 1, 0);
      double ang = water_angle * M_PI / 360;
      olx_create_rotation_matrix(M, RotVec, cos(ang));
      crds.AddNew(M* Vec1);
      olx_create_rotation_matrix(M, RotVec, cos(-ang), sin(-ang));
      crds.AddNew(M* Vec1);
      // final 180 degree rotation
      olx_create_rotation_matrix(M, Vec1, 0, 1);
      crds[0] = (M * crds[0]) * dis + envi.crd();
      crds[1] = (M * crds[1]) * dis + envi.crd();
    }
    break;
  case fgOH1:
    dis = Distances.Get(GenId(fgOH1, envi.Count() == 3 ? 3 : (envi.Count() == 1 ? 1 :0)));
    // special case AFIX 13
    if (envi.Count() == 3) {
      crds.AddNew(Generate_1(fgOH1, envi));
    }
    // any possible H-bonds?
    else if (envi.Count() > 0 && pivoting != 0 && pivoting->Count() >= 1) {
      vec3d Vec1 = pivoting->GetCrd(0) - envi.crd();
      vec3d Vec2 = envi.GetVec(0);
      vec3d RotVec = Vec1.XProdVec(Vec2).Normalise();
      olx_create_rotation_matrix(M, RotVec, cos(M_PI*THA / 180));
      crds.AddNew(Vec2);
      crds[0] = M * crds[0];
      crds[0].NormaliseTo(dis);
      crds[0] += envi.crd();
    }
    else {
      if (envi.Count() == 1) {
        if (pivoting != 0 && pivoting->Count() >= 1) {  // any possible H-bonds?
          vec3d Vec1 = pivoting->GetCrd(0) - envi.crd();
          vec3d Vec2 = envi.GetVec(0);
          vec3d RotVec;
          if (!pivoting->GetCAtom(0).IsHAttached()) {
            RotVec = Vec1.XProdVec(Vec2).Normalise();
          }
          else {
            RotVec = Vec2.XProdVec(Vec1).Normalise();
          }
          olx_create_rotation_matrix(M, RotVec, cos(M_PI*THA / 180));

          crds.AddNew(Vec2);
          crds[0] = M * crds[0];
          crds[0].NormaliseTo(dis);
          crds[0] += envi.crd();
        }
        else {
          // Ar-B(OH)2 ?
          if (envi.GetType(0) == iBoronZ) {
            NA = envi.GetLatt().FindSAtom(envi.GetCAtom(0));
            TAtomEnvi NEnvi = envi.GetUC().GetAtomEnviList(*NA);
            NEnvi.Exclude(envi.GetBase().CAtom());
            if (NEnvi.Count() == 2) { // ArC-BO
              /* in this case the could be cis or trans, put them cis as in Ar-B-O-H ...*/
              if (NEnvi.GetType(0) == iOxygenZ ||  // make sure deal with the right one
                NEnvi.GetType(1) == iOxygenZ) {
                vec3d Vec1;
                if (NEnvi.GetType(0) == iOxygenZ) {
                  Vec1 = NEnvi.GetCrd(1);
                }
                else {
                  Vec1 = NEnvi.GetCrd(0);
                }
                Vec1 -= NEnvi.crd();
                vec3d Vec2 = envi.crd() - NEnvi.crd();
                vec3d RotVec = Vec1.XProdVec(Vec2).Normalise();
                olx_create_rotation_matrix(M, RotVec, -cos(M_PI*THA / 180));
                Vec2.Normalise();
                Vec2 = M * Vec2;
                Vec2 *= dis;
                crds.AddNew(Vec2 + envi.crd());
              }
            }
          }
        }
      }
      else if (envi.Count() == 2) {
        const double d1 = envi.Distance(0);
        const double d2 = envi.Distance(1);
        if ((d1 > 1.8 && d2 < 1.8) || (d2 > 1.8 && d1 < 1.8)) {
          vec3d Vec1 = envi.GetVec(0);
          vec3d Vec2 = envi.GetVec(1);
          vec3d RotVec;
          if (d1 < 1.8) {
            RotVec = Vec1.XProdVec(Vec2).Normalise();
            crds.AddNew(Vec1);
          }
          else {
            RotVec = Vec2.XProdVec(Vec1).Normalise();
            crds.AddNew(Vec2);
          }
          olx_create_rotation_matrix(M, RotVec, cos(M_PI*THA / 180));
          crds[0] = M * crds[0];
          crds[0].NormaliseTo(dis);
          crds[0] += envi.crd();
        }
      }
      if (crds.IsEmpty()) {  // generic case, random placement...
        vec3d PlaneN = envi.GetVec(0);
        vec3d ov = PlaneN.IsParallel(Z) ? X : Z;
        double ca = ov.CAngle(PlaneN);
        vec3d RotVec = PlaneN.XProdVec(ov).Normalise();
        olx_create_rotation_matrix(M, RotVec, ca);
        crds.AddNew(0, -sin(M_PI*THA / 180), cos(M_PI*THA / 180));
        crds[0] = M * crds[0];
        crds[0] *= dis;
        crds[0] += envi.crd();
      }
    }
    break;
  case fgNH4:
    break;
  case fgNH3:
    if (envi.Count() == 1) {
      dis = Distances.Get(GenId(fgNH3, 0));
      vec3d PlaneN = envi.GetVec(0).Normalise();
      vec3d RotVec = PlaneN.XProdVec(PlaneN.IsParallel(Z) ? X : Z).Normalise();
      olx_create_rotation_matrix(M, RotVec, cos(M_PI*THA / 180));
      crds.AddNew(M*PlaneN);
      olx_create_rotation_matrix(M, PlaneN, cos(M_PI*120. / 180));
      crds.AddNew(M*crds[0]);
      crds.AddNew(M*crds[1]);

      for (size_t i = 0; i < crds.Count(); i++) {
        crds[i] *= dis;
        crds[i] += envi.crd();
      }
    }
    break;
  case fgNH2:
    dis = Distances.Get(GenId(fgNH2, 0));
    if (envi.Count() == 1) {
      if (pivoting == 0) {
        vec3d PlaneN = envi.GetVec(0).Normalise();
        vec3d ov = PlaneN.IsParallel(Z) ? X : Z;
        double ca = ov.CAngle(PlaneN);
        vec3d RotVec = PlaneN.XProdVec(ov);
        RotVec.Normalise();
        olx_create_rotation_matrix(M, RotVec, ca);

        crds.AddNew(0, -sin(M_PI*THA / 180), cos(M_PI*THA / 180));
        crds[0] = M * crds[0];
        crds[0] *= dis;
        crds.AddNew(crds[0]);
        crds[0] += envi.crd();

        olx_create_rotation_matrix(M, PlaneN, -0.5);
        crds[1] = M * crds[1];
        crds[1] += envi.crd();
      }
      else {  // C=NH2
        pivoting->Exclude(envi.GetBase().CAtom());
        if (pivoting->Count() < 1) {
          throw TFunctionFailedException(__OlxSourceInfo, "assert");
        }
        vec3d Vec1 = pivoting->GetVec(0);
        vec3d Vec2 = envi.crd() - pivoting->crd();
        vec3d PlaneN = Vec1.XProdVec(Vec2).Normalise();
        olx_create_rotation_matrix(M, PlaneN, -0.5);
        Vec1 = (pivoting->crd() - envi.crd()).NormaliseTo(dis);
        Vec1 = M * Vec1;

        crds.AddNew(Vec1);
        crds[0] += envi.crd();

        Vec1 = M * Vec1;
        crds.AddNew(Vec1);
        crds[1] += envi.crd();
      }
    }
    else if (envi.Count() == 2) {
      dis = -dis;
      vec3d Vec1 = (envi.GetVec(0).Normalise() +
        envi.GetVec(1).Normalise()).Normalise();
      vec3d RotVec = envi.GetVec(0).XProdVec(envi.GetVec(1)).Normalise();
      olx_create_rotation_matrix(M, RotVec, cos(M_PI / 3), sin(M_PI / 3));
      crds.AddNew(M*Vec1);
      olx_create_rotation_matrix(M, RotVec, cos(-M_PI / 3), sin(-M_PI / 3));
      crds.AddNew(M*Vec1);
      // final 90 degree rotation
      olx_create_rotation_matrix(M, Vec1, 0, 1);
      crds[0] = (M*crds[0])*dis + envi.crd();
      crds[1] = (M*crds[1])*dis + envi.crd();
    }
    break;
  case fgNH1:
    if (envi.Count() >= 2) {
      if (envi.Count() == 3) {
        dis = Distances.Get(GenId(fgNH1, 3));
      }
      else {
        dis = Distances.Get(GenId(fgNH1, 0));  // generic...
      }
      vec3d Vec1;
      for (size_t i = 0; i < envi.Count(); i++) {
        Vec1 += (envi.GetCrd(i) - envi.crd()).Normalise();
      }
      crds.AddNew(Vec1.NormaliseTo(-dis) + envi.crd());
    }
    break;
  case fgNH1t:
    if (envi.Count() == 2) {
      dis = Distances.Get(GenId(fgNH1, 0));
      vec3d Vec1 = envi.GetVec(0);
      vec3d Vec2 = envi.GetVec(1);
      vec3d Vec3 = Vec1.XProdVec(Vec2);
      vec3d RotVec = Vec3.XProdVec(
        vec3d(Vec1).Normalise() + vec3d(Vec2).Normalise()).Normalise();
      olx_create_rotation_matrix(M, RotVec, cos(M_PI / 4), sin(M_PI / 4));
      // try to find conjugation with a ring
      NA = envi.GetLatt().FindSAtom(
        envi.GetCAtom(Vec1.QLength() < Vec2.QLength() ? 1 : 0));
      TAtomEnvi NEnvi = envi.GetUC().GetAtomEnviList(*NA);
      NEnvi.Exclude(envi.GetBase().CAtom());
      vec3d v,
        p1 = M*Vec3,
        p2 = (-Vec3)*M;
      if (NEnvi.Count() == 3) { // check sterics
        vec3d closest;
        double sd = 1000;
        for (int i = 0; i < 3; i++) {
          double d = (envi.crd() - NEnvi.GetCrd(i)).QLength();
          if (d < sd) {
            sd = d;
            closest = NEnvi.GetCrd(i);
          }
        }
        closest -= NEnvi.crd();
        if (closest.DotProd(p1) < closest.DotProd(p2)) { // should look opposite
          v = p1;
        }
        else {
          v = p2;
        }
      }
      else if (NEnvi.Count() == 2) {
        vec3d rn = NEnvi.GetVec(0).XProdVec(NEnvi.GetVec(1)).Normalise();
        if (olx_abs(p1.DotProd(rn)) > olx_abs(p2.DotProd(rn))) {
          v = p2;
        }
        else {
          v = p1;
        }
      }
      else if (NEnvi.Count() == 1) {
        vec3d rn = NEnvi.GetVec(0).XProdVec(
          envi.crd() + NEnvi.GetCrd(0)).Normalise();
        if (olx_abs(p1.DotProd(rn)) > olx_abs(p2.DotProd(rn))) {
          v = p2;
        }
        else {
          v = p1;
        }
      }
      else { // hah?
        v = p1;
      }
      crds.AddNew(v.NormaliseTo(dis) + envi.crd());
    }
    break;
  case fgBH1:
    if (envi.Count() == 3) {
      dis = Distances.Get(GenId(fgBH1, 3));
      vec3d Vec1 = envi.GetVec(0).Normalise();
      vec3d Vec2 = envi.GetVec(1).Normalise();
      vec3d Vec3 = envi.GetVec(2).Normalise();
      crds.AddNew((Vec1 + Vec2 + Vec3).Normalise());
      Vec1 -= Vec2;
      Vec3 -= Vec2;
      Vec3 = Vec1.XProdVec(Vec3);
      Vec3.NormaliseTo(crds[0].CAngle(Vec3) < 0 ? dis : -dis);
      crds[0] = (Vec3 += envi.crd());
    }
    else if (envi.Count() == 4 || envi.Count() == 5) {
      dis = Distances.Get(GenId(fgBH1, 5));
      bool create = true;
      vec3d Vec1;
      for (size_t i = 0; i < envi.Count(); i++) {
        Vec1 += (envi.GetCrd(i) - envi.crd()).Normalise();
        //          if( !(envi.GetBAI(i).GetIndex() == iCarbonIndex ||
        //                envi.GetBAI(i).GetIndex() == iBoronIndex) )  {
        //            create = false;
        //            break;
        //          }
      }
      if (create) {
        crds.AddNew(Vec1.NormaliseTo(-dis) + envi.crd());
      }
    }
    break;
  case fgSiH1:
    if (envi.Count() == 3) {
      dis = Distances.Get(GenId(fgSiH1, 3));
      vec3d Vec1 = envi.GetVec(0).Normalise();
      vec3d Vec2 = envi.GetVec(1).Normalise();
      vec3d Vec3 = envi.GetVec(2).Normalise();
      crds.AddNew((Vec1 + Vec2 + Vec3).Normalise());
      Vec1 -= Vec2;
      Vec3 -= Vec2;
      Vec3 = Vec1.XProdVec(Vec3);
      Vec3.NormaliseTo(crds[0].CAngle(Vec3) < 0 ? dis : -dis);
      crds[0] = (Vec3 += envi.crd());
    }
    break;
  case fgSiH2:
    if (envi.Count() == 2) {
      dis = -Distances.Get(GenId(fgSiH2, 2));
      // summ vector
      vec3d Vec1 = (envi.GetVec(0).Normalise() +
        envi.GetVec(1).Normalise()).Normalise();
      vec3d RotVec = envi.GetVec(0).XProdVec(
        envi.GetCrd(1) - envi.crd()).Normalise();
      olx_create_rotation_matrix(M, RotVec, cos(M_PI / 3), sin(M_PI / 3));
      crds.AddNew(M*Vec1);
      olx_create_rotation_matrix(M, RotVec, cos(-M_PI / 3), sin(-M_PI / 3));
      crds.AddNew(M*Vec1);
      // final 90 degree rotation
      olx_create_rotation_matrix(M, Vec1, 0, 1);
      crds[0] = (M*crds[0])*dis + envi.crd();
      crds[1] = (M*crds[1])*dis + envi.crd();
    }
    break;
  case fgSH1:
    dis = Distances.Get(GenId(fgSH1, 0));
    if (envi.Count() == 1) {
      if (pivoting != 0 && pivoting->Count() >= 1) {  // any possible H-bonds?
        vec3d Vec1 = pivoting->GetCrd(0) - envi.crd();
        vec3d Vec2 = envi.GetCrd(0) - envi.crd();
        vec3d RotVec;
        if (!pivoting->GetCAtom(0).IsHAttached()) {
          RotVec = Vec1.XProdVec(Vec2).Normalise();
        }
        else {
          RotVec = Vec2.XProdVec(Vec1).Normalise();
        }
        olx_create_rotation_matrix(M, RotVec, cos(M_PI*THA / 180));

        crds.AddNew(Vec2);
        crds[0] = M * crds[0];
        crds[0].NormaliseTo(dis);
        crds[0] += envi.crd();
      }
    }
    if (crds.IsEmpty()) {  // generic case - random placement ...
      vec3d PlaneN = envi.GetCrd(0) - envi.crd();
      vec3d ov = PlaneN.IsParallel(Z) ? X : Z;
      double ca = ov.CAngle(PlaneN);
      vec3d RotVec = PlaneN.XProdVec(ov).Normalise();
      olx_create_rotation_matrix(M, RotVec, ca);

      crds.AddNew(0, -sin(M_PI*THA / 180), cos(M_PI*THA / 180));
      crds[0] = M * crds[0];
      crds[0] *= dis;
      crds[0] += envi.crd();
    }
    break;
  }
  envi.GetBase().CAtom().SetHAttached(!crds.IsEmpty());
  DoGenerateAtom(au.GetResidue(envi.GetBase().CAtom().GetResiId()),
    created, au, crds, tmp);
}
//.............................................................................
uint32_t AConstraintGenerator::AfixM2GroupId(int M, int Z) {
  switch (M) {
  case 1:
    if (Z == iBoronZ) {
      return GenId(fgBH1, 3);
    }
    else if (Z == iCarbonZ) {
      return GenId(fgCH1, 3);
    }
    else if (Z == iNitrogenZ) {
      return GenId(fgNH1, 3);
    }
    else if (Z == iOxygenZ) {
      return GenId(fgOH1, 3);
    }
    else if (Z == iSiliconZ) {
      return GenId(fgSiH1, 3);
    }
    break;
  case 2:
    if (Z == iCarbonZ) {
      return GenId(fgCH2, 2);
    }
    else if (Z == iNitrogenZ) {
      return GenId(fgNH2, 2);
    }
    break;
  case 3:
  case 12:
  case 13:
    if (Z == iCarbonZ) {
      return GenId(fgCH3, 1);
    }
    else if (Z == iNitrogenZ) {
      return GenId(fgNH3, 1);
    }
    break;
  case 4:
    if (Z == iCarbonZ) {
      return GenId(fgCH1, 2);
    }
    else if (Z == iNitrogenZ) {
      return GenId(fgNH1, 2);
    }
    break;
  case 8:
  case 14:
    if (Z == iOxygenZ) {
      return GenId(fgOH1, 1);
    }
    break;
  case 9:
    if (Z == iCarbonZ) {
      return GenId(fgCH2, 2);
    }
    else if (Z == iNitrogenZ) {
      return GenId(fgNH2, 2);
    }
    break;
  case 15:
    if (Z == iBoronZ) {
      return GenId(fgBH1, 5);
    }
  }
  return 0;
}
//.............................................................................
olx_pair_t<uint16_t, uint16_t> AConstraintGenerator::GetZGroupRange(unsigned Z) {
  switch (Z) {
  case iBoronZ:
    return olx_pair::make(fgB_start,fgB_end);
  case iCarbonZ:
    return olx_pair::make(fgC_start, fgC_end);
  case iNitrogenZ:
    return olx_pair::make(fgN_start, fgN_end);
  case iOxygenZ:
    return olx_pair::make(fgO_start, fgO_end);
  case iSiliconZ:
    return olx_pair::make(fgSi_start, fgSi_end);
  case iSulphurZ:
    return olx_pair::make(fgS_start, fgS_end);
  }
  return olx_pair_t<uint16_t, uint16_t>(0, 0);
}
//.............................................................................
void AConstraintGenerator::UpdateFromFile(const olxstr& fn,
  bool apply_temp_correction, double def)
{
  double extra = apply_temp_correction ? GetTempCorrection() : 0;
  if (def >= 0) {
    def += extra;
    for (size_t i = 0; i < Distances.Count(); i++) {
      Distances.GetValue(i) = def;
    }
  }
  TStrList lines = TEFile::ReadLines(fn);
  if (!lines.IsEmpty() && lines[0].StartsFrom('#')) {
    TBasicApp::NewLogEntry() << lines[0].SubStringFrom(1);
  }
  olx_pset<uint32_t> defaults;
  for (size_t i = 1; i < lines.Count(); i++) {
    olxstr l = lines[i].TrimWhiteChars();
    if (l.IsEmpty() || l.StartsFrom('#')) {
      continue;
    }
    TStrList toks(l, ' ');
    if (toks.Count() < 2) {
      continue;
    }
    double d = toks.GetLastString().ToDouble() + extra;
    if (toks.Count() == 2) { // no AFIX_m
      olx_pair_t<uint16_t, uint16_t> range = GetZGroupRange(toks[0].ToUInt());
      if (range.a != 0) {
        size_t cnt = 0, tcnt = (range.b - range.a + 1);
        for (size_t di = 0; di < Distances.Count(); di++) {
          uint32_t gi = GroupFromId(Distances.GetKey(di));
          if (gi >= range.a && gi <= range.b) {
            Distances.GetValue(di) = d;
            if (++cnt > tcnt) {
              break;
            }
          }
        }
      }
    }
    else {
      unsigned Z = toks[0].ToUInt();
      for (size_t mc = 0; mc < toks.Count() - 2; mc++) {
        unsigned m = toks[mc + 1].ToUInt();
        uint32_t gi = AfixM2GroupId(m, Z);
        if (gi == 0) {
          continue;
        }
        uint32_t g = GroupFromId(gi);
        if (!defaults.Contains(g)) {
          defaults.Add(g);
          Distances.Add(GenId(g, 0), d, true);
        }
        Distances.Add(gi, d, true);
      }
    }
  }
}