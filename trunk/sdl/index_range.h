/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_index_range_H
#define __olx_sdl_index_range_H
#include "typelist.h"
#include "estrlist.h"
#include "citem.h"
BeginEsdlNamespace()

/* a class to represent a list of (partly) sequential indices with sequences
presented only by the boundary indices.
*/
struct IndexRange  {
  struct RangeItr {
    TStrList toks;
    size_t i, start, end;
    RangeItr(const olxstr& r) : toks(r, ','), i(0), start(1), end(0)  {}
    bool HasNext() const {  return (toks.Count() > i || start <= end);  }
    size_t Next()  {
      if( start <= end )  {  return start++;  }
      const olxstr& s = toks[i++];
      const size_t hp = s.FirstIndexOf('-');
      if( hp != InvalidIndex )  {
        start = s.SubStringTo(hp).ToSizeT();
        end = s.SubStringFrom(hp+1).ToSizeT();
        if( start >= end )
          throw TInvalidArgumentException(__OlxSourceInfo, "start>=end");
        return start++;
      }
      else
        return s.ToSizeT();
    }
    size_t CalcSize() const {
      size_t sz = 0;
      for( size_t _i=0; _i < toks.Count(); _i++ )  {
        const size_t pos = toks[_i].FirstIndexOf('-');
        if( pos != InvalidIndex )
          sz += (toks[_i].SubStringFrom(pos+1).ToSizeT()-toks[_i].SubStringTo(pos).ToSizeT()+1);
        else
          sz++;
      }
      return sz;
    }
  };
  // preferred way - no intermediate index storage is required
  static RangeItr GetIterator(const olxstr& r)  {  return RangeItr(r);  }
  static TSizeList& FromString(const olxstr& range, TSizeList& list)  {
    TStrList toks(range, ',');
    for( size_t i=0; i < toks.Count(); i++ )  {
      const size_t pos = toks[i].FirstIndexOf('-');
      if( pos != InvalidIndex )  {
        const size_t start = toks[i].SubStringTo(pos).ToSizeT();
        const size_t end = toks[i].SubStringFrom(pos+1).ToSizeT();
        list.SetCapacity(list.Count()+end-start+(toks.Count()-i));
        for( size_t j=start; j <= end; j++ )
          list.Add(j);
      }
      else
        list.Add(toks[i].ToSizeT());
    }
    return list;
  }
  static TSizeList::const_list_type FromString(const olxstr& range)  {
    TSizeList l;
    return FromString(range, l);
  }
  static olxstr ToString(const TSizeList& list)  {
    return ToString(list, ListAccessor(list));
  }
  class Builder  {
    olxstr range;
    size_t last;
    bool last_written;
    bool check_last(size_t v)  {
      if( last != InvalidIndex ) {
        if( last+1 == v )  {
          last = v;
          last_written = false;
        }
        else  {
          if( last_written )  {
            if( v != InvalidIndex )
              range << ',' << v;
            last = v;
          }
          else  {
            range << '-' << last;
            if( v != InvalidIndex )
              range << ',' << v;
            last = v;
            last_written = true;
          }
        }
        return true;
      }
      return false;
    }
  public:
    Builder() : last(InvalidIndex), last_written(false)  {}
    Builder& operator << (size_t v)  {
      if( !check_last(v) ) {
        if( !range.IsEmpty() )
          range << ',';
        range << v;
        last = v;
        last_written = true;
      }
      return *this;
    }
    olxstr GetString(bool reset=false)  {
      check_last(InvalidIndex);
      olxstr rv = range;
      if( reset )
        Reset();
      return rv;
    }
    /* resets the object and returns the accumulated result
    */
    olxstr Reset() {
      olxstr rv = range;
      range.SetLength(0);
      last = InvalidIndex;
      last_written = false;
      return rv;
    }
  };
  template <class list_t, class accessor_t>
  static olxstr ToString(const list_t& list, const accessor_t& acc)  {
    olxstr rv;
    for( size_t i=0; i < list.Count(); i++ )  {
      if( i+1 == list.Count() )
        return (rv << acc(list[i]));
      size_t j = 1;
      while( list[i] == (acc(list[i+j])-j) )  {
        j++;
        if( (j + i) >= (list.Count()-1) )  {
          if( i != (j+i-1) )
            return (rv << acc(list[i]) << '-' << acc(list[j+i]));
          else
            return (rv << acc(list[i]));
        }
      }
      if( j == 1 )
        rv << acc(list[i]);
      else  {
        rv << acc(list[i]) <<  '-';
        i += (j - 1);
        rv << acc(list[i]);
      }
      if( i+1 < list.Count() )
        rv << ',';
    }
    return rv;
  }
};

EndEsdlNamespace()
#endif
