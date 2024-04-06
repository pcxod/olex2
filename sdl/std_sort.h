#pragma once
/******************************************************************************
* Copyright (c) 2004-2024 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

/* sorting based on STD algs - sort and stable_sort */

#include "esort.h"
#include "talist.h"
#include <algorithm>

BeginEsdlNamespace()

struct std_sorter {
  template <class item_t, class comparator_t>
  struct idx_value {
    item_t item;
    const comparator_t* cmp;
    size_t index;

    idx_value() {}
    idx_value(const item_t& item, const comparator_t& cmp, size_t index)
      : item(item), cmp(&cmp), index(index)
    {}
    bool operator < (const idx_value& other) const {
      return cmp->Compare(this->item, other.item) < 0;
    }
  };

  template <bool stable, class list_t, class comparator_t, class listener_t>
  struct StdSorter_ {
    StdSorter_(list_t& list_, const comparator_t& cmp_,
      const listener_t& listener_)
      : list(list_), cmp(cmp_), listener(listener_)
    {}
    void Sort() {
      size_t lc = list.list.Count();
      if (lc < 2) {
        return;
      }
      typedef typename list_t::InternalAccessor::list_item_type item_t;
      typedef idx_value<item_t, comparator_t> idx_value_t;
      TArrayList<idx_value_t> data(lc);
      for (size_t i = 0; i < lc; i++) {
        data[i] = idx_value_t(list[i], cmp, i);
      }
      idx_value_t* last = &data.GetLast();
      if (stable) {
        std::stable_sort(&data[0], ++last);
      }
      else {
        std::sort(&data[0], ++last);
      }
      //https://medium.com/@kevingxyz/permutation-in-place-8528581a5553
      for (size_t i = 0; i < lc; i++) {
        size_t tsw = data[i].index;
        while (tsw < i) {
          tsw = data[tsw].index;
        }
        list.list.Swap(i, tsw);
        listener.OnSwap(i, tsw);
      }
    }
  protected:
    typename list_t::InternalAccessor list;
    const comparator_t& cmp;
    const listener_t& listener;
  };
  template <bool stable, class list_t, class comparator_t, class listener_t>
  static StdSorter_<stable, list_t, comparator_t, listener_t>
    Make(list_t& list, const comparator_t& cmp, const listener_t& listener) {
    return StdSorter_<stable, list_t, comparator_t, listener_t>(
      list, cmp, listener);
  }
};

struct StdSorter : public SortInterface<StdSorter> {
  template <class list_t, class comparator_t, class listener_t>
  static std_sorter::StdSorter_<false, list_t, comparator_t, listener_t>
    Make(list_t& list, const comparator_t& cmp, const listener_t& listener) {
    return std_sorter::Make<false, list_t, comparator_t, listener_t>(
      list, cmp, listener);
  }
};

struct StdStableSorter : public SortInterface<StdStableSorter> {
  template <class list_t, class comparator_t, class listener_t>
  static std_sorter::StdSorter_<true, list_t, comparator_t, listener_t>
    Make(list_t& list, const comparator_t& cmp, const listener_t& listener) {
    return std_sorter::Make<true, list_t, comparator_t, listener_t>(
      list, cmp, listener);
  }
};

EndEsdlNamespace()