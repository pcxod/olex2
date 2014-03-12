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

  template <class ListClass, class Comparator, typename TypeClass>
  size_t FindInsertIndex(const ListClass &list, const Comparator &cmp,
    const TypeClass &entity)
  {
    size_t from = 0, to = list.Count()-1;
    while( true )  {
      if( (to-from) == 1 )  return to;
      const size_t index = (to+from)/2;
      const int cr = cmp.Compare(list[index], entity);
      if( cr < 0 )
        from = index;
      else  {
        if( cr > 0 )
          to  = index;
        else
          if( cr == 0 )
            return index;
      }
    }
    return InvalidIndex;  // should never happen - infinite loop above!
  }

  template <class ListClass, class Comparator, typename TypeClass>
  size_t FindInsertIndexEx(const ListClass &list, const Comparator &cmp,
    const TypeClass &entity, bool &exists)
  {
    size_t from = 0, to = list.Count()-1;
    while( true )  {
      if( (to-from) == 1 )  {
        if( cmp.Compare(list[from], entity) == 0 )  {
          exists = true;
          return from;
        }
        else if( cmp.Compare(list[to], entity) == 0 )  {
          exists = true;
          return to;
        }
        else
          return to;
      }
      const size_t index = (to+from)/2;
      const int cr = cmp.Compare(list[index], entity);
      if( cr < 0 )
        from = index;
      else  {
        if( cr > 0 )
          to  = index;
        else
          if( cr == 0 )  {
            exists = true;
            return index;
          }
      }
    }
    return InvalidIndex;  // should never happen - infinite loop above!
  }

  template <class ListClass, class Comparator, typename KeyC>
  size_t FindIndexOf(const ListClass &list, const Comparator &cmp,
    const KeyC &entity)
  {
    if( list.Count() == 0 )  return InvalidIndex;
    if( list.Count() == 1 )
      return cmp.Compare(list[0],entity) == 0 ? 0 : InvalidIndex;
    size_t from = 0, to = list.Count()-1;
    const int from_cr = cmp.Compare(list[from], entity);
    if( from_cr == 0 )  return from;
    if( from_cr > 0  )  return InvalidIndex;
    const int to_cr = cmp.Compare(list[to], entity);
    if( to_cr == 0 )  return to;
    if( to_cr < 0  )  return InvalidIndex;
    while( true ) {
      const size_t index = (to+from)/2;
      if( index == from || index == to)
        return InvalidIndex;
      const int cr = cmp.Compare(list[index], entity);
      if( cr < 0 )
        from = index;
      else  {
        if( cr > 0 )
          to  = index;
        else  {
          if( cr == 0 )
            return index;
        }
      }
    }
    return InvalidIndex;
  }

  template <class ListClass, class Comparator, typename TypeClass>
  size_t Add(ListClass &list, const Comparator &cmp, TypeClass entry)  {
    if( list.Count() == 0 )  {  list.Add(entry);  return 0;  }
    if( list.Count() == 1 )  {
      const int cmp_val = cmp.Compare(list[0], entry);
      if(  cmp_val < 0 )     {  list.Add(entry);  return 1;  }
      else if( cmp_val > 0 ) {  list.Insert(0, entry);  return 0;  }
      else                   {  list.Add(entry);  return 1;  }
    }
    // smaller than the first
    if( cmp.Compare(list[0], entry) >=0  )  {
      list.Insert(0, entry);
      return 0;
    }
    // larger than the last
    if( cmp.Compare(list.GetLast(), entry) <=0 )  {
      list.Add(entry);
      return list.Count()-1;
    }
    // an easy case then with two items
    if( list.Count() == 2 )  {  list.Insert(1, entry);  return 1; }
    const size_t pos = FindInsertIndex(list, cmp, entry);
    list.Insert(pos, entry);
    return pos;
  }

  template <class ListClass, class Comparator, typename TypeClass>
  bool AddUnique(ListClass &list, const Comparator &cmp,
    TypeClass entry, size_t *pos=NULL)
  {
    if( list.Count() == 0 )  {
      list.Add(entry);
      if( pos != NULL )  *pos = 0;
      return true;
    }
    if( list.Count() == 1 )  {
      const int cmp_val = cmp.Compare(list[0], entry);
      if(  cmp_val < 0 )  {
        list.Add(entry);
        if( pos != NULL )  *pos = 1;
        return true;
      }
      else if( cmp_val > 0 )  {
        list.Insert(0, entry);
        if( pos != NULL )  *pos = 0;
        return true;
      }
      else  {
        if( pos != NULL )  *pos = 0;
        return false;
      }
    }
    if( cmp.Compare(list[0],entry) > 0 )  {  // smaller than the first
      list.Insert(0, entry);
      if( pos != NULL )  *pos = 0;
      return true;
    }
    if( cmp.Compare(list.GetLast(), entry) < 0 )  {  // larger than the last
      list.Add(entry);
      if( pos != NULL ) *pos = list.Count()-1;
      return true;
    }
    bool exists = false;
    size_t ps = FindInsertIndexEx(list, cmp, entry, exists);
    if( pos != NULL )  *pos = ps;
    if( exists )  return false;
    list.Insert(ps, entry);
    return true;
  }
}  // end namespace sorted

EndEsdlNamespace()
#endif
