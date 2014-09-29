/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xlib_label_corrector_H
#define __olx_xlib_label_corrector_H
#include "asymmunit.h"
#include "residue.h"
#include "talist.h"

struct LabelIterator {
  static olxch inc_char(olxch ch) {
    if( ch == '9' )  return 'a';
    if( olxstr::o_isalphanumeric(olxch(ch+1)) )
      return ch+1;
    return 0;
  }
  static olxch inc_num_char(olxch ch) {
    if( olxstr::o_isdigit(olxch(ch+1)) )
      return ch+1;
    return 0;
  }
  olxstr label;
  size_t max_ind;
  LabelIterator(size_t _max_ind=0) : max_ind(_max_ind) {}
  LabelIterator(const LabelIterator& l)
    : label(l.label), max_ind(l.max_ind) {}
  LabelIterator(const olxstr& l, size_t _max_ind)
    : label(l), max_ind(_max_ind)
  {
    if( l.Length() < max_ind )
      throw TInvalidArgumentException(__OlxSourceInfo, "max_ind");
  }
  LabelIterator& inc() {
    if( label.Length() == max_ind )  label << '0';
    else  {
      size_t i = label.Length();
      while( --i > max_ind && inc_char(label.CharAt(i)) == 0 )
        ;
      if( i == max_ind )  { // overflown, create new position
        if( inc_num_char(label.CharAt(i)) == 0 )  {
          label = olxstr(label.SubStringTo(i)) <<
            '1' << olxstr::CharStr('0', label.Length()-i);
        }
        else  {
          label[i] = inc_num_char(label.CharAt(i));
          for( size_t j=i+1; j < label.Length(); j++ )
            label[j] = '0';
        }
      }
      else  {
        label[i] = inc_char(label.CharAt(i));
        for( size_t j=i+1; j < label.Length(); j++ )
          label[j] = '0';
      }
    }
    return *this;
  }
};

struct LabelCorrector  {
  olxstr_dict<TCAtom*, true> uniq_labels;
  olxdict<const cm_Element*, LabelIterator, TPointerComparator>
    labels;
  bool trim;
  LabelCorrector(bool trim=true) : trim(trim)  {}
  LabelCorrector(TAsymmUnit& au, bool trim=true) : trim(trim) {
    uniq_labels.SetCapacity(au.AtomCount());
    for( size_t i=0; i < au.AtomCount(); i++ )  {
      TCAtom& a = au.GetAtom(i);
      if( a.IsDeleted() )  continue;
      if( trim && a.GetLabel().Length() > 4 )
        a.SetLabel(a.GetLabel().SubStringTo(4), false);
      uniq_labels.Add(a.GetResiLabel(), &a);
    }
  }
  void Correct(TCAtom& a) {
    if (a.IsDeleted()) return;
    if (trim && a.GetLabel().Length() > 4)
      a.SetLabel(a.GetLabel().SubStringTo(4), false);
    TCAtom* lo = uniq_labels.Find(a.GetResiLabel(), NULL);
    if (lo != NULL) {
      // is diplicate allowed?
      if (a.GetPart() != lo->GetPart() && a.GetPart() != 0 && lo->GetPart() != 0)
        return;
      LabelIterator *li;
      if (labels.HasKey(&a.GetType()))
        li = &labels.Get(&a.GetType());
      else {
        const size_t off = a.GetType().symbol.Length();
        li = &labels(&a.GetType(),
          LabelIterator(olxstr(a.GetType().symbol) << '1', off));
      }
      while (uniq_labels.IndexOf(li->label) != InvalidIndex)
        li->inc();
      a.SetLabel(li->label, false);
      uniq_labels.Add(a.GetResiLabel(), &a);
      li->inc();
    }
    else
      uniq_labels.Add(a.GetResiLabel(), &a);
  }
  void CorrectAll(TResidue& r) {
    uniq_labels.SetCapacity(r.Count());
    for (size_t i=0; i < r.Count(); i++)
      Correct(r[i]);
  }
  // must be initialised with AsymmUnit!
  void CorrectGlobal(TCAtom& a) {
    if (a.IsDeleted()) return;
    TCAtom* lo = uniq_labels.Find(a.GetResiLabel(), NULL);
    if (lo != &a) {
      LabelIterator *li;
      if (labels.HasKey(&a.GetType()))
        li = &labels.Get(&a.GetType());
      else {
        const size_t off = a.GetType().symbol.Length();
        li = &labels(&a.GetType(),
          LabelIterator(olxstr(a.GetType().symbol) << '1', off));
      }
      while (uniq_labels.IndexOf(li->label) != InvalidIndex)
        li->inc();
      a.SetLabel(li->label, false);
      uniq_labels.Add(li->label);
      li->inc();
    }
    else if (lo == NULL) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "Incorrectly intialised object - use the right constructor");
    }
  }
  bool IsGlobal(const TCAtom& a) const {
    TCAtom* lo = uniq_labels.Find(a.GetResiLabel(), NULL);
    if (lo != &a)
      return false;
    else if (lo == NULL) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "Incorrectly intialised object - use the right constructor");
    }
    return true;
  }
};

#endif
