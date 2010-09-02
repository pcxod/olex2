#ifndef __olx_sdl_listAlg_H
#define __olx_sdl_listAlg_H

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
    template <class Item> inline void OnItem(Item& o) const {
      if( condition.OnItem(o) )
        results.Add(o);
    }
  };
  template <class ResList, class Condition> struct _FilterEx  {
    ResList& results;
    const Condition& condition;
    _FilterEx(ResList& _results, const Condition& _condition) :
      results(_results), condition(_condition)  {}
    template <class Item> inline void OnItem(Item& o, size_t i) const {
      if( condition.OnItem(o, i) )
        results.Add(o);
    }
  };
  template <class SrcList, class ResList, class Condition>
  static void Filter(const SrcList& src, ResList& dest, const Condition& cond)  {
    src.ForEach(_Filter<ResList, Condition>(dest, cond));
  }
  template <class SrcList, class ResList, class Condition>
  static void FilterEx(const SrcList& src, ResList& dest, const Condition& cond)  {
    src.ForEachEx(_FilterEx<ResList, Condition>(dest, cond));
  }
};

#endif
