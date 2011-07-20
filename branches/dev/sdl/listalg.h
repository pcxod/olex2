/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_list_alg_H
#define __olx_sdl_list_alg_H

struct ListCaster  {
  template <class To> struct SimpleCast  {
    template <class From> static inline To OnItem(From o)  {
      return (To)o;
    }
  };
  struct CopyCast  {
    template <class From, class To> static inline To& OnItem(From o)  {
      return *(new To(o));
    }
  };
  template <class To> struct AssignCast  {
    template <class From> static inline To OnItem(From o)  {
      return *(new To) = o;
    }
  };
  template <typename To, class accessor> struct AccessorCast  {
    template <class From> static inline To OnItem(From o)  {
      return accessor::Access(o);
    }
  };

  template <typename ListA, typename ListB, class Caster>
  static void Cast(const ListA& from, ListB& to, const Caster& caster)  {
    to.SetCapacity(to.Count()+from.Count());
    for( size_t i=0; i < from.Count(); i++ )
      to.Add(caster.OnItem(from[i]));
  }
};

struct ListFilter  {
  template <class ResList, class Condition> struct _Filter  {
    ResList& results;
    const Condition& condition;
    _Filter(ResList& _results, const Condition& _condition) :
      results(_results), condition(_condition)  {}
    template <class Item> inline void OnItem(Item& o, size_t i) const {
      if( condition.OnItem(o, i) )
        results.Add(o);
    }
  };
  template <class SrcList, class ResList, class Condition>
  static ResList& Filter(const SrcList& src, ResList& dest, const Condition& cond)  {
    src.ForEach(_Filter<ResList, Condition>(dest, cond));
    return dest;
  }
};

// the items must have copy constructors
template <typename seq_t> static seq_t &olx_reverse(seq_t &seq, size_t sz)  {
  const size_t hsz = sz/2;
  for( size_t i=0; i < hsz; i++ )
    olx_swap(seq[i], seq[sz-1-i]);
  return seq;
}
template <class seq_t> static seq_t &olx_reverse(seq_t &seq)  {
  return olx_reverse(seq, seq.Count());
}

#endif
