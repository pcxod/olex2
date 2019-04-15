/******************************************************************************
* Copyright (c) 2004-2019 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xlib_label_utils_H
#define __olx_xlib_label_utils_H
#include "asymmunit.h"
#include "residue.h"
#include "talist.h"

BeginXlibNamespace()

struct AtomLabelInfo {
  enum {
    no_part = -10000
  };

  olxstr label;
  int part;
  int resi_number;
  olxch chain_id;
  olxstr resi_class;

  AtomLabelInfo(const olxstr &Label)
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
    }
  }

  bool DoesMatch(const TCAtom &a, bool match_label) const {
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
};

EndXlibNamespace()
#endif

