/******************************************************************************
* Copyright (c) 2004-2024 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/
#pragma once
#include "typelist.h"

BeginEsdlNamespace()
/*
  For a list of lists finds overlapping lists (considering the individual list
  content to be unique) and joins them.
*/
struct ListJoiner {
  template <typename item_t>
  static void list_add(TTypeList<item_t>& a, const item_t& i) {
    a.AddCopy(i);
  }
  template <typename item_t>
  static void list_add(TPtrList<item_t>& a, item_t* i) {
    a.Add(i);
  }

  template<class list_t>
  static void merge_lists(list_t& a, const list_t& b) {
    size_t a_sz = a.Count();
    for (size_t i = 0; i < b.Count(); i++) {
      bool uniq = true;
      for (size_t j = 0; j < a_sz; j++) {
        if (b[i].Compare(a[j]) == 0) {
          uniq = false;
          break;
        }
      }
      if (uniq) {
        list_add(a, b[i]);
      }
    }
  }

  template <typename item_t, class in_list_t>
  static typename TTypeList<TTypeList<item_t> >::const_list_type
    Join(const in_list_t& lists, size_t min_sz, const item_t& dummy)
  {
    typedef TTypeList<item_t> list_t;
    typedef olx_object_ptr<list_t> list_ptr_t;
    olxdict<item_t, olx_object_ptr<list_t>, TComparableComparator> dest;
    for (size_t i = 0; i < lists.Count(); i++) {
      const typename in_list_t::list_item_type& pl = lists[i];
      list_ptr_t d;
      for (size_t j = 0; j < pl.Count(); j++) {
        size_t idx = dest.IndexOf(pl[j]);
        if (idx != InvalidIndex) {
          if (!d.ok()) {
            d = dest.GetValue(idx);
          }
          else if (d != dest.GetValue(idx)) {
            merge_lists(*d, *dest.GetValue(idx));
            list_ptr_t d_ = dest.GetValue(idx);
            for (size_t k = 0; k < dest.Count(); k++) {
              if (dest.GetValue(k) == d_) {
                dest.GetValue(k) = d;
              }
            }
          }
        }
      }
      if (!d.ok()) {
        d = new list_t();
      }
      list_t& dl = *d;
      for (size_t j = 0; j < pl.Count(); j++) {
        if (dest.HasKey(pl[j])) {
          continue;
        }
        dl.AddCopy(pl[j]);
        dest.Add(pl[j], d);
      }
    }
    TTypeList<list_t> res;
    for (size_t i = 0; i < dest.Count(); i++) {
      if (!dest.GetValue(i).ok() || dest.GetValue(i)->Count() < min_sz) {
        continue;
      }
      res.Add(dest.GetValue(i).release());
    }
    return res;
  }
};

EndEsdlNamespace()

