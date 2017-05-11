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

struct olx_list_caster  {
  template <class To> struct SimpleCast  {
    template <class From> static To OnItem(From o)  {
      return (To)o;
    }
  };
  struct CopyCast  {
    template <class From, class To> static To& OnItem(From o)  {
      return *(new To(o));
    }
  };
  template <class To> struct AssignCast  {
    template <class From> static To OnItem(From o)  {
      return *(new To) = o;
    }
  };
  template <typename To, class accessor> struct AccessorCast  {
    template <class From> static To OnItem(From o)  {
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

struct olx_list_filter  {
  template <class analyser_t, class collector_t>
  struct ResultCollector_ {
    const analyser_t &analyser;
    const collector_t &collector;
    ResultCollector_(const analyser_t &analyser_,
      const collector_t &collector_)
      : analyser(analyser_), collector(collector_)
    {}
    template <class item_t>
    void OnItem(item_t &i, size_t idx) const {
      if (analyser.OnItem(i, idx))
        collector.OnItem(i, idx);
    }
    template <class item_t>
    void OnItem(const item_t &i, size_t idx) const {
      if (analyser.OnItem(i, idx))
        collector.OnItem(i, idx);
    }
  };
  template <class list_t> struct ItemCollector_ {
    list_t &dest;
    ItemCollector_(list_t &dest_)
      : dest(dest_)
    {}
    template <class item_t>
    void OnItem(item_t &i, size_t) const { dest.Add(i); }
    template <class item_t>
    void OnItem(const item_t &i, size_t) const { dest.Add(i); }
  };
  template <class list_t> struct IndexCollector_ {
    list_t &dest;
    IndexCollector_(list_t &dest_)
      : dest(dest_)
    {}
    template <class item_t>
    void OnItem(const item_t &, size_t i) const { dest.Add(i); }
  };

  template <class SrcList, class ResList, class Condition>
  static ResList& Filter(const SrcList& src, ResList& dest,
    const Condition& cond)
  {
    src.ForEach(ResultCollector_<Condition, ItemCollector_<ResList> >(
      cond, dest));
    return dest;
  }
  template <class SrcList, class ResList, class Condition>
  static ResList& FilterIndices(const SrcList& src, ResList& dest,
    const Condition& cond)
  {
    src.ForEach(ResultCollector_<Condition, IndexCollector_<ResList> >(
      cond, dest));
    return dest;
  }
};

struct olx_list_init {
protected:
  struct zero_ {
    template <typename item_t>
    void OnItem(item_t &item, size_t) const { item = 0; }
  };

  template <typename item_t>
  struct value_ {
    item_t val;
    value_(item_t v) : val(v) {}
    void OnItem(item_t &item, size_t) const { item = val; }
  };

  struct index_ {
    template <typename item_t>
    void OnItem(item_t &item, size_t i) const { item = item_t(i); }
  };
public:
  // list item to a value initialiser
  template <typename item_t>
  static value_<item_t> value(item_t v) {
    return value_<item_t>(v);
  }
  // list item to it's position in the list initialiser
  static index_ index() { return index_(); }
  // list item to zero initialiser
  static zero_ zero() { return zero_(); }
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

struct olx_list_reverse {
  template <class data_list_t>
  struct ReverseList_  {
    typedef typename data_list_t::list_item_type return_type;
    data_list_t &data;
    ReverseList_(data_list_t &data_) : data(data_) {}
    return_type& operator [](size_t& i) const {
      return data[data.Count()-i-1];
    }
    return_type& operator ()(size_t& i) const {
      return data[data.Count()-i-1];
    }
    size_t Count() const { return data.Count(); }
  };

  template <class data_list_t>
  struct ReverseConstList_  {
    typedef typename data_list_t::list_item_type return_type;
    const data_list_t &data;
    ReverseConstList_(const data_list_t &data_) : data(data_) {}
    const return_type& operator [](size_t& i) const {
      return data[data.Count()-i-1];
    }
    const return_type& operator ()(size_t& i) const {
      return data[data.Count()-i-1];
    }
    size_t Count() const { return data.Count(); }
  };

  template <class list_t>
  static ReverseList_<list_t> Make(list_t &data) {
    return ReverseList_<list_t>(data);
  }

  template <class list_t>
  static ReverseConstList_<list_t> MakeConst(const list_t &data) {
    return ReverseConstList_<list_t>(data);
  }
};

template <class list_t, typename func_t>
bool olx_list_and(const list_t &l, func_t f, bool if_empty=true) {
  if (l.IsEmpty()) return if_empty;
  for (size_t i=0; i < l.Count(); i++) {
    if (!(olx_ref::get(l[i]).*f)())
      return false;
  }
  return true;
};
template <class list_t, typename func_t>
bool olx_list_and_st(const list_t &l, func_t f, bool if_empty=true) {
  if (l.IsEmpty()) return if_empty;
  for (size_t i=0; i < l.Count(); i++) {
    if (!(*f)(olx_ref::get(l[i])))
      return false;
  }
  return true;
};

template <class list_t, typename func_t>
bool list_or(const list_t &l, func_t f, bool if_empty=false) {
  if (l.IsEmpty()) return if_empty;
  for (size_t i=0; i < l.Count(); i++) {
    if ((olx_ref::get(l[i]).*f)())
      return true;
  }
  return false;
};
template <class list_t, typename func_t>
bool olx_list_or_st(const list_t &l, func_t f, bool if_empty=false) {
  if (l.IsEmpty()) return if_empty;
  for (size_t i=0; i < l.Count(); i++) {
    if ((*f)(olx_ref::get(l[i])))
      return true;
  }
  return false;
};

template <class list_t, typename func_t>
void olx_list_call(const list_t &l, func_t f) {
  for (size_t i=0; i < l.Count(); i++)
    (olx_ref::get(l[i]).*f)();
};
template <class list_t, typename func_t>
void olx_list_call_st(const list_t &l, func_t f) {
  for (size_t i=0; i < l.Count(); i++)
    (*f)(olx_ref::get(l[i]));
};
#endif
