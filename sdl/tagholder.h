/******************************************************************************
* Copyright (c) 2004-2024 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/
#pragma once

#include "talist.h"
#include "edict.h"

/* Use this in complex code when tags may be modified by one of the above functions
* The lists passed to the stack must outlive it or poped out before getting destructed.
They also must not get modified by themselves
*/
BeginEsdlNamespace()

struct ItemTagHolder {
  struct ACIList {
    TArrayList<index_t> tags;
    ACIList(size_t sz) : tags(sz) {}
    virtual const void* ptr() const = 0;
    virtual void restore() = 0;
    virtual bool have_changed() const = 0;
  };

  olx_pdict<const void*, struct ACIList*> data;

  ItemTagHolder() {}

  template <class list_t>
  ItemTagHolder(const list_t& l) {
    store(l);
  }

  template <class list_t, class accessor_t>
  ItemTagHolder(const list_t& l, const accessor_t& acc) {
    store(l, acc);
  }

  ~ItemTagHolder() {
    restore_all();
  }

  template <class list_t, class accessor_t>
  ItemTagHolder& store(const list_t& l, const accessor_t& acc) {
    ACIList* al = new CIList<list_t, accessor_t>(l, acc);
    data.Add(al->ptr(), al);
    return *this;
  }

  template <class list_t>
  ItemTagHolder& store(const list_t& l) { return store(l, DummyAccessor()); }

  template <class list_t>
  ItemTagHolder& restore(const list_t& l, bool clear=true) {
    size_t i = data.IndexOf((const void*)&l);
    if (i != InvalidIndex) {
      data.GetValue(i)->restore();
      if (clear) {
        delete data.GetValue(i);
        data.Delete(i);
      }
    }
    else {
      throw TFunctionFailedException(__OlxSrcInfo, "assert");
    }
    return *this;
  }

  template <class list_t>
  bool have_changed(const list_t& l) {
    size_t i = data.IndexOf((const void*)&l);
    if (i != InvalidIndex) {
      return data.GetValue(i)->have_changed();
    }
    else {
      throw TFunctionFailedException(__OlxSrcInfo, "assert");
    }
  }

  template <class list_t, class accessor_t>
  ItemTagHolder& copy_to(const list_t& src, list_t& dest, const accessor_t &acc) {
    size_t i = data.IndexOf((const void*)&src);
    if (i != InvalidIndex) {
      ACIList& tl = *data.GetValue(i);
      if (dest.Count() != tl.tags.Count()) {
        throw TFunctionFailedException(__OlxSrcInfo, "assert");
      }
      for (size_t i = 0; i < tl.tags.Count(); i++) {
        olx_ref::get(acc(dest[i])).SetTag(tl.tags[i]);
      }
    }
    else {
      throw TFunctionFailedException(__OlxSrcInfo, "assert");
    }
    return *this;
  }

  template <class list_t>
  ItemTagHolder& copy_to(const list_t& src, list_t& dest) {
    return copy_to(src, dest, DummyAccessor());
  }

  void restore_all() {
    const int dcount = data.Count();
    for (size_t i = 0; i < dcount; i++) {
      data.GetValue(i)->restore();
      data.Delete(i);
    }
    data.Clear();
  }

  // empties store without restoring tags
  void clear() {
    for (size_t i = 0; i < data.Count(); i++) {
      data.Delete(i);
    }
    data.Clear();
  }

  template <class list_t, class accessor_t>
  struct CIList : public ACIList {
    const list_t& list;
    accessor_t acc;
    CIList(const list_t& l, const accessor_t& acc)
      : ACIList(l.Count()), list(l), acc(acc)
    {
      for (size_t i = 0; i < l.Count(); i++) {
        this->tags[i] = olx_ref::get(acc(l[i])).GetTag();
      }
    }
    void restore() {
      if (list.Count() != this->tags.Count()) {
        throw TFunctionFailedException(__OlxSrcInfo, "assert");
      }
      for (size_t i = 0; i < list.Count(); i++) {
        olx_ref::get(acc(list[i])).SetTag(this->tags[i]);
      }
    }
    bool have_changed() const {
      if (list.Count() != this->tags.Count()) {
        throw TFunctionFailedException(__OlxSrcInfo, "assert");
      }
      for (size_t i = 0; i < list.Count(); i++) {
        if (olx_ref::get(acc(list[i])).GetTag() != this->tags[i]) {
          return true;
        }
      }
      return false;
    }
    const void* ptr() const {
      return (const void*)&list;
    }
  };
};
EndEsdlNamespace()
