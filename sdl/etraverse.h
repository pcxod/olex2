/******************************************************************************
* Copyright (c) 2004-2025 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_traverse_H
#define __olx_sdl_traverse_H
#include "ebase.h"

BeginEsdlNamespace()

/* this class can be used as a base for any object one of the methods of which
  should be called on every item of some collection. This approach allows hiding
  details about that collection implementation and therefore make the code more
  portable
*/
template <class ItemClass> class ATraverser {
public:
  virtual ~ATraverser() {}
  // if the function return false - the iteration stops
  virtual bool OnItem(ItemClass& item) const = 0;
};

template <class ItemClass> class ITraversable {
public:
  virtual ~ITraversable() {}
  virtual bool Traverse(const ATraverser<ItemClass>& tr) = 0;
};

/*for performance reasons virtual functions can be replaced with templates
*/
template <class ListToTraverseClass> class TListTraverser {
public:
  template <class Traverser>
  static bool Traverse(const ListToTraverseClass& list, Traverser& traverser) {
    for (size_t i = 0; i < list.Count(); i++) {
      if (!traverser.OnItem(list.Item(i))) {
        return false;
      }
    }
    return true;
  }
};

/*
  Count and Item functions must be in place
*/
template <class GraphToTraverseClass> class TGraphTraverser {
public:
  template <class Traverser>
  static bool Traverse(const GraphToTraverseClass& graph, Traverser& traverser) {
    for (size_t i = 0; i < graph.Count(); i++) {
      if (!traverser.OnItem(graph.Item(i))) {
        return false;
      }
      if (!Traverse(graph.Item(i), traverser)) {
        return false;
      }
    }
    return true;
  }
  template <class Traverser>
  static bool LevelTraverse(const GraphToTraverseClass& graph, Traverser& traverser) {
    for (size_t i = 0; i < graph.Count(); i++) {
      if (!traverser.OnItem(graph.Item(i))) {
        return false;
      }
    }
    for (size_t i = 0; i < graph.Count(); i++) {
      if (!LevelTraverse(graph.Item(i), traverser)) {
        return false;
      }
    }
    return true;
  }
};

template <class TreeToTraverseClass> class TBTreeTraverser {
  template <class Traverser>
  static bool TraverseLeft(const typename TreeToTraverseClass::entry_t* en, Traverser& traverser) {
    if (!traverser.OnItem(en)) {
      return false;
    }
    const typename TreeToTraverseClass::entry_t* nxt = en->next;
    while (nxt != 0) {
      if (!traverser.OnItem(nxt)) {
        return false;
      }
      nxt = nxt->next;
    }
    while (en) {
      if (en->right != 0) {
        TraverseLeft(en->right, traverser);
      }
      if (en->left != 0) {
        if (!traverser.OnItem(en->left)) {
          return false;
        }
        nxt = en->left->next;
        while (nxt != 0) {
          if (!traverser.OnItem(nxt)) {
            return false;
          }
          nxt = nxt->next;
        }
        en = en->left;
      }
      else {
        return true;
      }
    }
    return true;
  }
public:
  template <class Traverser>
  static bool Traverse(const TreeToTraverseClass& tree, Traverser& traverser) {
    if (tree.Count() == 0) {
      return true;
    }
    return TraverseLeft(tree.GetRoot(), traverser);
  }

};


template <typename value_t>
struct IIterator {
  virtual ~IIterator() {}
  virtual bool HasNext() const = 0;
  virtual const value_t& Next() = 0;
};


template <typename value_t>
struct AIterable {
  typedef olx_object_ptr<IIterator<value_t> > iterator_t;
  virtual IIterator<value_t>* iterate() const = 0;
  iterator_t Iterate() const {
    return iterate();
  }
  template <typename functor_t>
  void ForEach(const functor_t &f) {
    iterator_t i = iterate();
    while (i->HasNext()) {
      f(i->Next());
    }
  }
};

template <class container_t, class accessor_t, typename value_t>
struct IndexableIterator : public IIterator<value_t> {
  size_t idx;
  const container_t* cont;

  IndexableIterator(const container_t* b)
    : idx(InvalidIndex), cont(b)
  {}

  bool HasNext() const {
    return (idx + 1) < olx_ref::get(cont).Count();
  }

  const value_t& Next() {
    idx++;
    size_t cnt = olx_ref::get(cont).Count();
    if (idx >= cnt) {
      TExceptionBase::ThrowIndexOutOfRange(__POlxSourceInfo, idx, 0, cnt);
    }
    return accessor_t::get(olx_ref::get(cont), idx);
  }
};

template <class value_t>
struct IteratorJ : public IIterator<value_t> {
  typedef olx_object_ptr<IIterator<value_t> > iterator_t;
  const value_t* first;
  iterator_t itr;
  IteratorJ(const value_t* first, iterator_t itr)
    : first(first), itr(itr)
  {}

  bool HasNext() const { return first != 0 || itr->HasNext(); }
  const value_t& Next() {
    if (first != 0) {
      const value_t* rv = first;
      first = 0;
      return *rv;
    }
    return itr->Next();
  }
};

EndEsdlNamespace()
#endif
