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

struct ItemTagStack {
  struct ACIList {
    TArrayList<index_t> tags;
    ACIList(size_t sz) : tags(sz) {}
    virtual const void* ptr() const = 0;
    virtual void pop() = 0;
  };

  olx_pdict<const void*, struct ACIList*> store;

  ~ItemTagStack() {
    pop_all();
  }

  template <class list_t, class accessor_t>
  ItemTagStack& push(const list_t& l, const accessor_t& acc) {
    ACIList* al = new CIList<list_t, accessor_t>(l, acc);
    store.Add(al->ptr(), al);
    return *this;
  }

  template <class list_t>
  ItemTagStack& push(const list_t& l) { return push(l, DummyAccessor()); }

  template <class list_t>
  ItemTagStack& pop(const list_t& l) {
    size_t i = store.IndexOf((const void*)&l);
    if (i != InvalidIndex) {
      store.GetValue(i)->pop();
      delete store.GetValue(i);
      store.Delete(i);
    }
    return *this;
  }

  void pop_all() {
    for (size_t i = 0; i < store.Count(); i++) {
      store.GetValue(i)->pop();
      delete store.GetValue(i);
    }
    store.Clear();
  }

  // empties store without restoring tags
  void clear() {
    for (size_t i = 0; i < store.Count(); i++) {
      delete store.GetValue(i);
    }
    store.Clear();
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
    void pop() {
      if (list.Count() != this->tags.Count()) {
        throw TFunctionFailedException(__OlxSrcInfo, "assert");
      }
      for (size_t i = 0; i < list.Count(); i++) {
        olx_ref::get(acc(list[i])).SetTag(this->tags[i]);
      }
    }
    const void* ptr() const {
      return (const void*)&list;
    }
  };
};
EndEsdlNamespace()
