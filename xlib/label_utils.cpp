/******************************************************************************
* Copyright (c) 2004-2025 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "label_utils.h"
#include "residue.h"
#include "asymmunit.h"
#include "unitcell.h"
#include "lattice.h"
#include "refmodel.h"

AtomLabelInfo::AtomLabelInfo(const olxstr& Label)
  : part(no_part),
  resi_number(TResidue::NoResidue),
  chain_id(TResidue::NoChainId())
{
  label = Label;
  size_t p_idx = Label.IndexOf('^');
  if (p_idx != InvalidIndex) {
    olxstr sfx = Label.SubStringFrom(p_idx + 1);
    if (sfx.Length() == 1) {
      part = olxstr::o_tolower(sfx.CharAt(0)) - 'a' + 1;
    }
    label = Label.SubStringTo(p_idx);
  }
  size_t us_ind = label.IndexOf('_');
  if (us_ind != InvalidIndex && ++us_ind < label.Length()) {
    olxstr sfx = label.SubStringFrom(us_ind);
    size_t c_idx = sfx.IndexOf(':');
    if (c_idx <= 1) {  // residue number?
      if (c_idx == 1) {
        chain_id = sfx.CharAt(0);
      }
      sfx = sfx.SubStringFrom(c_idx + 1);
      if (sfx.IsNumber()) {
        resi_number = sfx.ToInt();
      }
      else {
        resi_class = sfx;
      }
    }
    // some old shelxl compatibility
    else {
      if (part == no_part &&
        sfx.Length() == 1 && olxstr::o_isalpha(sfx.CharAt(0)))
      {
        part = olxstr::o_tolower(label.CharAt(us_ind)) - 'a' + 1;
      }
      else {
        if (sfx.IsNumber()) {
          resi_number = sfx.ToInt();
        }
        else {
          resi_class = sfx;
        }
      }
    }
    label = label.SubStringTo(us_ind - 1);
    if (resi_class.StartsFrom('$')) {
      equiv_id = resi_class;
      resi_class.SetLength(0);
    }
  }
}
//.............................................................................
bool AtomLabelInfo::DoesMatch(const TCAtom& a, bool match_label) const {
  if (match_label && !a.GetLabel().Equalsi(label)) {
    return false;
  }
  if (part != no_part) {
    if (part != olx_abs(a.GetPart())) {
      return false;
    }
  }
  if (resi_number != TResidue::NoResidue) {
    const TResidue& r = a.GetParent()->GetResidue(a.GetResiId());
    if (resi_number == r.GetNumber()) {
      if (chain_id == TResidue::NoChainId() || chain_id == r.GetChainId()) {
        return true;
      }
    }
  }
  else if (!resi_class.IsEmpty()) {
    const TResidue& r = a.GetParent()->GetResidue(a.GetResiId());
    if (chain_id != TResidue::NoChainId() && chain_id == r.GetChainId()) {
      if (resi_class == '*' || r.GetClassName().Equalsi(resi_class)) {
        return true;
      }
    }
    else if (r.GetClassName().Equalsi(resi_class)) {
      return true;
    }
  }
  else {
    return true;
  }
  return false;
}
//.............................................................................
bool AtomLabelInfo::DoesMatch(const TSAtom& a, bool match_label) const {
  if (!DoesMatch(a.CAtom(), match_label)) {
    return false;
  }
  if (equiv_id.IsEmpty()) {
    return true;
  }
  RefinementModel* rm = a.GetParent().GetAsymmUnit().GetRefMod();
  if (rm == 0) {
    return true;
  }
  if (equiv_id == "$0") {
    return a.GetMatrix().IsFirst();
  }
  const smatd* eq = rm->FindUsedSymm(equiv_id);
  if (eq == 0) {
    return false;
  }
  if (a.GetMatrix().GetId() == eq->GetId()) {
    return true;
  }
  for (size_t i = 0; i < a.CAtom().EquivCount(); i++) {
    uint32_t mi = a.GetParent().GetUnitCell().MulMatrixId(
      a.GetMatrix(),
      a.CAtom().GetEquiv(i));
    if (mi == eq->GetId()) {
      return true;
    }
  }
  return false;
}
//.............................................................................
