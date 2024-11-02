#pragma once
/******************************************************************************
* Copyright (c) 2004-2024 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/
#include "esort.h"
#include "typelist.h"

BeginEsdlNamespace()

template <class sorter_t> struct TStableSorter {
  template <class list_t, class listener_t>
  struct SortListener {
    const listener_t& listener;
    list_t& list;
    SortListener(list_t& list, const listener_t& listener)
      : list(list), listener(listener)
    {}
    void OnSwap(size_t i, size_t j) const {
      list.Swap(i, j);
      listener.OnSwap(i, j);
    }
    void OnMove(size_t i, size_t j) const {
      list.Move(i, j);
      listener.OnMove(i, j);
    }
  };

  template <class item_t, class comparator_t>
  struct idx_value {
    item_t item;
    const comparator_t& cmp;
    size_t index;

    idx_value() {}
    idx_value(const item_t& item, const comparator_t& cmp, size_t index)
      : item(item), cmp(cmp), index(index)
    {}
    int Compare(const idx_value& other) const {
      int r = cmp.Compare(this->item, other.item);
      if (r == 0) {
        return olx_cmp(this->index, other.index);
      }
      return r;
    }
  };
  template <class list_t, class comparator_t, class listener_t>
  struct StableSorter_ {
    StableSorter_(list_t& list, const comparator_t& cmp,
      const listener_t& listener)
      : list(list), cmp(cmp), listener(listener)
    {}
    void Sort() {
      size_t lc = list.list.Count();
      if (lc < 2) {
        return;
      }
      typedef typename list_t::InternalAccessor::list_item_type item_t;
      typedef idx_value<item_t, comparator_t> idx_value_t;
      TTypeList<idx_value_t> data(olx_reserve(lc));
      for (size_t i = 0; i < lc; i++) {
        data.Add(new idx_value_t(list[i], cmp, i));
      }
      sorter_t::Make(data, TComparableComparator(),
        SortListener<list_t, listener_t>(list.list, listener)).Sort();
    }
    typename list_t::InternalAccessor list;
    const comparator_t& cmp;
    const listener_t& listener;
  };
  template <class list_t, class comparator_t, class listener_t>
  static StableSorter_<list_t, comparator_t, listener_t>
    Make(list_t& list, const comparator_t& cmp, const listener_t& listener) {
    return StableSorter_<list_t, comparator_t, listener_t>(
      list, cmp, listener);
  }
};
//.............................................................................
//.............................................................................
//.............................................................................
struct StableQuickSorter : public SortInterface<StableQuickSorter> {
  template <class list_t, class comparator_t, class listener_t>
  static TStableSorter<QuickSorter>::StableSorter_<list_t, comparator_t, listener_t>
    Make(list_t& list, const comparator_t& cmp, const listener_t& listener) {
    return TStableSorter<QuickSorter>::StableSorter_<list_t, comparator_t, listener_t>(
      list, cmp, listener);
  }
};

EndEsdlNamespace()