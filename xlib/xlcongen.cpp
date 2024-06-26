/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "xlcongen.h"
#include "unitcell.h"

bool TXlConGen::FixParam(const short paramMask, TStrList& res,
  const TCAtomPList& atoms, const TFixedValueList& values)
{
  throw TNotImplementedException( __OlxSourceInfo );
}
//..............................................................................
void TXlConGen::AnalyseMultipart(const TAtomEnvi& envi,
  const TTypeList<TCAtomPList>& parts)
{
  size_t cnt = 0;
  for (size_t i = 0; i < parts.Count(); i++) {
    if (!parts[i].IsEmpty() && parts[i][0]->GetParentAfixGroup() != NULL &&
      parts[i][0]->GetParentAfixGroup()->IsRefinable())
    {
      cnt++;
    }
  }
  if (cnt > 1) {  // have to change the refineable groups to riding group...
    for (size_t i = 0; i < parts.Count(); i++) {
      if (parts[i].IsEmpty()) {
        continue;
      }
      if (parts[i][0]->GetParentAfixGroup() != 0 &&
        parts[i][0]->GetParentAfixGroup()->IsRefinable())
      {
        const int m = parts[i][0]->GetParentAfixGroup()->GetM();
        if (m == 13) {
          // new Shelxl seems to deal with it properly
          //parts[i][0]->GetParentAfixGroup()->SetAfix(33);
        }
        else if (m == 14) {
          parts[i][0]->GetParentAfixGroup()->SetAfix(83);
        }
        else {
          parts[i][0]->GetParentAfixGroup()->SetAfix(m * 10 + 3);
        }
      }
    }
  }
  //else if( envi.GetBase().GetType() == iOxygenZ && envi.Count()
}
//..............................................................................
bool TXlConGen::FixAtom(TAtomEnvi& envi, const short Group,
  const cm_Element& atomType, TAtomEnvi* pivoting, TCAtomPList* generated)
{
  try {
    TSimpleRestraint* sr;
    TCAtomPList CreatedAtoms;
    GenerateAtom(CreatedAtoms, envi, Group, atomType, pivoting);
    if (CreatedAtoms.IsEmpty()) { // nothing inserted
      return false;
    }
    bool negative_part = false;
    double occu_mult = 1.0, dis;
    short afix = -1;
    const size_t bondex_cnt = envi.CountCovalent();
    switch (Group) {
    case fgNH3:
    case fgCH3:
      if (envi.Count() == 1) {
        afix = 137;
        negative_part = (envi.GetBase().CAtom().GetDegeneracy() != 1) &&
          envi.GetBase().CAtom().GetPart() == 0;
      }
      break;
    case fgCH2:
      if (envi.Count() == 2) {
        afix = 23;
      }
      else if (envi.Count() == 1) {
        afix = 93;
      }
      break;
    case fgCH1:
      if (envi.Count() == 3) {
        afix = 13;
      }
      else if (envi.Count() == 2) {
        afix = 43;
      }
      else if (envi.Count() == 1) {
        afix = 163;
      }
      break;
    case fgSiH1:
      if (envi.Count() == 3) {
        afix = 13;
      }
      break;
    case fgOH3:
      break;
    case fgOH2:
      dis = Distances.Get(GenId(fgOH2, 0));
      if (CreatedAtoms.Count() == 2) {
        bool force_restraints = false;
        if (envi.Count() == 1 && pivoting != 0 && pivoting->Count() == 2) {
          force_restraints = olx_tetrahedron_volume_n(envi.GetBase().crd(),
            envi.GetCrd(0),
            CreatedAtoms[0]->GetParent()->Orthogonalise(CreatedAtoms[0]->ccrd()),
            CreatedAtoms[0]->GetParent()->Orthogonalise(CreatedAtoms[1]->ccrd())
          ) < 1e-3;
        }
        if (IsUseRestrains()) {
          sr = &RefMod.rDFIX.AddNew();
          sr->SetValue(dis);
          sr->AddAtomPair(envi.GetBase().CAtom(), 0, *CreatedAtoms[0], 0);
          sr->AddAtomPair(envi.GetBase().CAtom(), 0, *CreatedAtoms[1], 0);
          sr = &RefMod.rDANG.AddNew();
          sr->SetValue(olx_round(dis*sqrt(2 - 2 * cos(104.5*M_PI / 180)), 100));
          sr->AddAtomPair(*CreatedAtoms[0], 0, *CreatedAtoms[1], 0);
          if (envi.Count() == 1 &&
            XElementLib::IsMetal(envi.GetCAtom(0).GetType()))
          {
            sr = &RefMod.rSADI.AddNew();
            sr->AddAtomPair(envi.GetCAtom(0), 0, *CreatedAtoms[0], 0);
            sr->AddAtomPair(envi.GetCAtom(0), 0, *CreatedAtoms[1], 0);
          }
        }
        else if (envi.Count() == 1 &&
          XElementLib::IsMetal(envi.GetCAtom(0).GetType()))
        {
          if (force_restraints) {
            sr = &RefMod.rSADI.AddNew();
            sr->AddAtomPair(envi.GetCAtom(0), 0, *CreatedAtoms[0], 0);
            sr->AddAtomPair(envi.GetCAtom(0), 0, *CreatedAtoms[1], 0);
            afix = 6;
          }
          else {
            afix = 7;
          }
        }
        else {
          afix = 6;
        }
      }
      else if (CreatedAtoms.Count() == 1 &&
        envi.GetBase().CAtom().GetDegeneracy() == 2)
      {
        sr = &RefMod.rDFIX.AddNew();
        sr->SetEsd(0.01);
        sr->SetValue(dis);
        sr->AddAtomPair(envi.GetBase().CAtom(), 0, *CreatedAtoms[0], 0);
        sr->AddAtomPair(envi.GetBase().CAtom(), 0, *CreatedAtoms[0],
          &envi.GetBase().CAtom().GetEquiv(0));

        sr = &RefMod.rDANG.AddNew();
        sr->SetEsd(0.02);
        sr->SetValue(dis*sqrt(2 - 2 * cos(109.4*M_PI / 180)));
        sr->AddAtomPair(*CreatedAtoms[0], &envi.GetBase().CAtom().GetEquiv(0),
          *CreatedAtoms[0], 0);

        if (envi.Count() == 1) {
          sr = &RefMod.rSADI.AddNew();
          sr->SetEsd(0.02);
          sr->AddAtomPair(envi.GetCAtom(0), 0, *CreatedAtoms[0], 0);
          sr->AddAtomPair(envi.GetCAtom(0), 0, *CreatedAtoms[0],
            &envi.GetBase().CAtom().GetEquiv(0));
        }
        occu_mult = 2;
      }
      break;
    case fgSH1:
    case fgOH1:
      if (envi.Count() == 3) {
        afix = 13;
      }
      else if (bondex_cnt == 1) {
        afix = 147;
      }
      else if (bondex_cnt == 2 && CreatedAtoms.Count() == 1) {
        const double d1 = envi.GetCrd(0).DistanceTo(envi.GetBase().crd());
        const double d2 = envi.GetCrd(1).DistanceTo(envi.GetBase().crd());
        //afix = 3; // possible...
        dis = Distances.Get(GenId(fgOH2, 0));
        sr = &RefMod.rDFIX.AddNew();
        sr->SetEsd(0.01);
        sr->SetValue(dis);
        sr->AddAtomPair(envi.GetBase().CAtom(), 0, *CreatedAtoms[0], 0);
        const double _d1 = (d1 < 1.8 ? d1 : d2);
        // if this is not applied, the refinement may never converge
        sr = &RefMod.rDANG.AddNew();
        sr->SetEsd(0.02);
        sr->SetValue(sqrt(_d1*_d1 + dis*dis - 2 * dis*_d1*cos(109.4*M_PI / 180)));
        sr->AddAtomPair(envi.GetCAtom(d1 < 1.8 ? 0 : 1), 0, *CreatedAtoms[0], 0);
        // this is optional
        sr = &RefMod.rDANG.AddNew();
        sr->SetEsd(0.02);
        const double _d2 = (d1 < 1.8 ? d2 : d1);
        sr->SetValue(sqrt(_d2*_d2 + dis*dis - 2 * dis*_d2*
            cos((360.0 - 109.4 - olx_angle(envi.GetCrd(0),
          envi.GetBase().crd(), envi.GetCrd(1)))*M_PI / 180)));
        sr->AddAtomPair(envi.GetCAtom(d1 > 1.8 ? 0 : 1), 0, *CreatedAtoms[0], 0);
      }
      break;
    case fgNH4:
      break;
    case fgNH2:
      if (envi.Count() == 1) {
        if (pivoting != 0) {
          afix = 93;
        }
        else  if (CreatedAtoms.Count() == 2) {
          afix = 7;
        }
      }
      else if (envi.Count() == 2) {
        afix = 23;
      }
      break;
    case fgNH1:
      if (envi.Count() == 3) {
        afix = 13;
      }
      else if (envi.Count() == 2) {
        afix = 43;
      }
      break;
    case fgNH1t:
      if (envi.Count() == 2) {
        afix = 3;
      }
      break;
    case fgBH1:
      if (envi.Count() == 3) {
        afix = 13;
      }
      else if (envi.Count() == 4 || envi.Count() == 5) {
        afix = 153;
      }
      break;
    }
    if (afix != 0) {
      TAfixGroup& ag = RefMod.AfixGroups.New(&envi.GetBase().CAtom(), afix);
      for (size_t i = 0; i < CreatedAtoms.Count(); i++) {
        ag.AddDependent(*CreatedAtoms[i]);
      }
    }
    XVarReference *ovr = envi.GetBase().CAtom().GetVarRef(catom_var_name_Sof);
    XVar *var = 0;
    short relation = relation_AsVar;
    if (ovr == 0) {
      var = &RefMod.Vars.NewVar(envi.GetBase().CAtom().GetOccu());
      RefMod.Vars.AddVarRef(*var, envi.GetBase().CAtom(), catom_var_name_Sof,
        relation, 1.0);
    }
    else {
      var = &ovr->Parent;
      relation = ovr->relation_type;
      if (relation != relation_None) {
        occu_mult *= ovr->coefficient;
      }
    }
    for (size_t i = 0; i < CreatedAtoms.Count(); i++) {
      CreatedAtoms[i]->SetOccu(envi.GetBase().CAtom().GetOccu());
      if (negative_part) {
        CreatedAtoms[i]->SetPart(-1);
      }
      else {
        CreatedAtoms[i]->SetPart(envi.GetBase().CAtom().GetPart());
      }
      CreatedAtoms[i]->SetUisoOwner(&envi.GetBase().CAtom());
      if (envi.GetBase().GetType() == iOxygenZ || Group == fgCH3) {
        CreatedAtoms[i]->SetUisoScale(1.5);
      }
      else {
        CreatedAtoms[i]->SetUisoScale(1.2);
      }
      CreatedAtoms[i]->SetUiso(4 * caDefIso*caDefIso);
      
      RefMod.Vars.AddVarRef(*var, *CreatedAtoms[i], catom_var_name_Sof,
        relation, occu_mult);
      CreatedAtoms[i]->SetOccu(CreatedAtoms[i]->GetOccu()*occu_mult);
      if (generated != 0) {
        generated->Add(CreatedAtoms[i]);
      }
    }
    if (Group == fgCH3x2 && CreatedAtoms.Count() == 6) {
      XVar &v = RefMod.Vars.NewVar();
      for (int i = 0; i < 3; i++) {
        RefMod.Vars.AddVarRef(v, *CreatedAtoms[i], catom_var_name_Sof,
          relation_AsVar, 1);
        RefMod.Vars.AddVarRef(v, *CreatedAtoms[3 + i], catom_var_name_Sof,
          relation_AsOneMinusVar, 1);
      }
    }
  }
  catch (TExceptionBase &)
  {}
  return false;
}
//..............................................................................
int TXlConGen::AfixFromQEnvironment(const TSAtom &a) {
  TAtomEnvi envi = a.GetParent().GetUnitCell().GetAtomEnviList(a, true);
  switch (envi.GetBase().GetType().z) {
  case iNitrogenZ:
  {
    size_t q_cnt = envi.CountZ(iQPeakZ);
    if (envi.Count() == 3 && q_cnt == 1) {
      return 43;
    }
    if (envi.Count() == 4 && q_cnt == 1) {
      return 13;
    }
  }
  break;
  }
  return 0;
}
//..............................................................................
bool TXlConGen::NormaliseEnvironment(TSAtom &a) {
  return false;
}
//..............................................................................
int TXlConGen::OlexToShelx(short code, const TAtomEnvi& envi,
  TAtomEnvi* pivot)
{
  switch (code) {
  case fgNH3:
  case fgCH3:
    return 137;
  case fgCH3x2:
    return 127;
  case fgCH2:
    if (envi.Count() == 1) {
      return 93;
    }
    else if (envi.Count() == 2) {
      return 23;
    }
    break;
  case fgCH1:
    if (envi.Count() == 1) {
      return 163;
    }
    else if (envi.Count() == 2) {
      return 43;
    }
    else if (envi.Count() == 3) {
      return 13;
    }
    break;
  case fgSiH1:
    if (envi.Count() == 3) {
      return 13;
    }
  case fgSH1:
  case fgOH1:
    if (envi.Count() == 3) {
      return 13;
    }
    return 147;
    break;
  case fgOH2:
    if (envi.Count() == 2) {
      return 23;
    }
    break;
  case fgNH2:
    if (envi.Count() == 1 && pivot == 0) {
      return 93;
    }
    else if (envi.Count() == 2) {
      return 23;
    }
    break;
  case fgNH1:
    if (envi.Count() == 2) {
      return 43;
    }
    else if (envi.Count() == 3) {
      return 13;
    }
    break;
  case fgBH1:
    if (envi.Count() == 4 || envi.Count() == 5) {
      return 153;
    }
    break;
  }
  return -1;
}
//..............................................................................
short TXlConGen::ShelxToOlex(int shelx_code, const TAtomEnvi& envi) {
  const int m = TAfixGroup::GetM(shelx_code);
  switch (m) {
  case 1:
    if (envi.Count() == 3) {
      if (envi.GetBase().GetType() == iCarbonZ) {
        return fgCH1;
      }
      else if (envi.GetBase().GetType() == iNitrogenZ) {
        return fgNH1;
      }
      else if (envi.GetBase().GetType() == iSiliconZ) {
        return fgSiH1;
      }
      else if (envi.GetBase().GetType() == iOxygenZ) {
        return fgOH1;
      }
    }
    break;
  case 2:
  case 9:
    if (envi.Count() == 2 || envi.Count() == 1) {
      if (envi.GetBase().GetType() == iCarbonZ) {
        return fgCH2;
      }
      else if (envi.GetBase().GetType() == iNitrogenZ) {
        return fgNH2;
      }
      else if (envi.GetBase().GetType() == iOxygenZ) {
        return fgOH2;
      }
    }
    break;
  case 3:
  case 13:
    if (envi.Count() == 1 && envi.GetBase().GetType() == iCarbonZ) {
      return fgCH3;
    }
    else if (envi.Count() == 1 && envi.GetBase().GetType() == iNitrogenZ) {
      return fgNH3;
    }
    break;
  case 12:
    if (envi.Count() == 1 && envi.GetBase().GetType() == iCarbonZ) {
      return fgCH3x2;
    }
    break;
  case 4:
    if (envi.Count() == 2) {
      if (envi.GetBase().GetType() == iCarbonZ) {
        return fgCH1;
      }
      else if (envi.GetBase().GetType() == iNitrogenZ) {
        return fgNH1;
      }
    }
    break;
  case 8:
  case 14:
    if (envi.Count() == 1) {
      if (envi.GetBase().GetType() == iOxygenZ) {
        return fgOH1;
      }
      else if (envi.GetBase().GetType() == iSulphurZ) {
        return fgSH1;
      }
    }
    break;
  case 15:
    if (envi.GetBase().GetType() == iBoronZ &&
      (envi.Count() == 4 || envi.Count() == 5))
    {
      return fgBH1;
    }
    break;
  case 16:
    if (envi.Count() == 1 && envi.GetBase().GetType() == iCarbonZ) {
      return fgCH1;
    }
    break;
  }
  return -1;
};
