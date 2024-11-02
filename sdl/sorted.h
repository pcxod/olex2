/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_sorted_H
#define __olx_sdl_sorted_H
#include "ebase.h"
BeginEsdlNamespace()

namespace sorted {

  /* The function assumes that there are at least two elements in the list and
  that the given entity is greater than the first and smaller than the last
  entry. Returns index of the first entry that is greater than the reference.
  */
  template <class ListClass, class Comparator, typename TypeClass>
  size_t FindInsertIndex(const ListClass &list, const Comparator &cmp,
    const TypeClass &entity)
  {
    size_t from = 0, to = list.Count()-1;
    while ((to - from) != 1) {
      const size_t index = from + (to - from) / 2;
      const int cr = cmp.Compare(list[index], entity);
      if (cr < 0)
        from = index;
      else if (cr > 0)
        to  = index;
      else
        return index;
    }
    return to;
  }

  /* The function assumes that there are at least two elements in the list and
  that the given entity is greater than the first ans smaller than the last
  entry. Returns index of the first entry that is greater or equal and true/false
  if the value in the list (equal).
  */
  template <class ListClass, class Comparator, typename TypeClass>
  olx_pair_t<size_t, bool> FindInsertIndexEx(const ListClass &list,
    const Comparator &cmp, const TypeClass &entity)
  {
    size_t from = 0, to = list.Count() - 1;
    while ((to - from) != 1) {
      const size_t index = from + (to - from) / 2;
      const int cr = cmp.Compare(list[index], entity);
      if (cr < 0)
        from = index;
      else if (cr > 0)
        to  = index;
      else {
        return olx_pair::make(index, true);
      }
    }
    if (cmp.Compare(list[from], entity) == 0) {
      return olx_pair::make(from, true);
    }
    else if (cmp.Compare(list[to], entity) == 0) {
      return olx_pair::make(to, true);
    }
    return olx_pair::make(to, false);
  }

  template <class ListClass, class Comparator, typename KeyC>
  size_t FindIndexOf(const ListClass &list, const Comparator &cmp,
    const KeyC &entity)
  {
    const size_t lc = list.Count();
    if (lc == 0) return InvalidIndex;
    if (lc == 1)
      return cmp.Compare(list[0], entity) == 0 ? 0 : InvalidIndex;
    size_t from = 0, to = lc-1;
    const int from_cr = cmp.Compare(list[from], entity);
    if (from_cr == 0) return from;
    if (from_cr > 0) return InvalidIndex;
    const int to_cr = cmp.Compare(list[to], entity);
    if (to_cr == 0) return to;
    if (to_cr < 0) return InvalidIndex;
    while((to-from) > 1) {
      const size_t index = from + (to - from) / 2;
      const int cr = cmp.Compare(list[index], entity);
      if (cr < 0)
        from = index;
      else if (cr > 0)
        to  = index;
      else
        return index;
    }
    return InvalidIndex;
  }

  template <class ListClass, class Comparator, typename TypeClass>
  size_t Add(ListClass &list, const Comparator &cmp, TypeClass entry) {
    const size_t lc = list.Count();
    if (lc == 0) {
      list.Add(entry);
      return 0;
    }
    const int cmp_val0 = cmp.Compare(list[0], entry);
    if (lc == 1) {
      if (cmp_val0 < 0) {
        list.Add(entry);
        return 1;
      }
      else if (cmp_val0 > 0) {
        list.Insert(0, entry);
        return 0;
      }
      else {
        list.Add(entry);
        return 1;
      }
    }
    // smaller than the first
    if (cmp_val0 >= 0) {
      list.Insert(0, entry);
      return 0;
    }
    // larger than the last
    if (cmp.Compare(list.GetLast(), entry) <=0) {
      list.Add(entry);
      return lc;
    }
    // an easy case then with two items
    if (lc == 2) {
      list.Insert(1, entry);
      return 1;
    }
    const size_t pos = FindInsertIndex(list, cmp, entry);
    list.Insert(pos, entry);
    return pos;
  }

  template <class ListClass, class Comparator, typename TypeClass>
  olx_pair_t<size_t, bool> AddUnique(ListClass &list, const Comparator &cmp,
    TypeClass entry)
  {
    const size_t lc = list.Count();
    if (lc == 0) {
      list.Add(entry);
      return olx_pair::make(0, true);
    }
    const int cmp_val0 = cmp.Compare(list[0], entry);
    if (lc == 1) {
      if (cmp_val0 < 0) {
        list.Add(entry);
        return olx_pair::make(1, true);
      }
      else if (cmp_val0 > 0) {
        list.Insert(0, entry);
        return olx_pair::make(0, true);
      }
      else {
        return olx_pair::make(0, false);
      }
    }
    if (cmp_val0 > 0) {  // smaller than the first
      list.Insert(0, entry);
      return olx_pair::make(0, true);
    }
    // larger than the last
    if (cmp.Compare(list.GetLast(), entry) < 0) {
      list.Add(entry);
      return olx_pair::make(lc, true);
    }
    olx_pair_t<size_t, bool> ps = FindInsertIndexEx(list, cmp, entry);
    if (!(ps.b = !ps.b)) {
      return ps;
    }
    list.Insert(ps.a, entry);
    return ps;
  }
}  // end namespace sorted

EndEsdlNamespace()
#endif
