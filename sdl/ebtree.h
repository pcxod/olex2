/******************************************************************************
* Copyright (c) 2004-2025 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#pragma once
#include "etraverse.h"
#include "esort.h"

BeginEsdlNamespace()
/*
* inspired by 
* https://en.wikipedia.org/wiki/AVL_tree
* https://www.geeksforgeeks.org/dsa/insertion-in-an-avl-tree/
*/

template <typename key_tt>
struct TreeSetEntry {
  typedef key_tt key_t;

  key_t key;
  
  TreeSetEntry(const key_t& k)
    : key(k)
  {}

  template <class cmp_t>
  int cmp(const key_t& k, const cmp_t& comparator) const {
    return comparator.Compare(key, k);
  }
};

template <typename key_tt, typename value_t>
struct TreeMapEntry {
  typedef key_tt key_t;

  key_t key;
  value_t value;
  TreeMapEntry(const key_t& k, const value_t& v)
    : key(k), value(v)
  {}

  template <class cmp_t>
  int cmp(const key_t& k, const cmp_t& comparator) const {
    return comparator.Compare(key, k);
  }
};

template <class value_tt>
struct BTEntry {
  typedef value_tt value_t;
  typedef typename value_t::key_t key_t;
  value_t value;
  size_t height;
  BTEntry* left, * right;
  BTEntry* next, * last;  // linked list of items with equal key

  BTEntry()
    : height(1), left(0), right(0),
    next(0), last(0)
  {}

  BTEntry(const value_t& value)
    : value(value), height(1),
    left(0), right(0),
    next(0), last(0)
  {}

  ~BTEntry() {
    olx_del_obj(next);
    olx_del_obj(left);
    olx_del_obj(right);
  }

  template <class cmp_t>
  int cmp(const value_t& e, const cmp_t& comparator) const {
    return value.cmp(e.key, comparator);
  }

  template <class cmp_t>
  int cmp(const key_t& k, const cmp_t& comparator) const {
    return value.cmp(k, comparator);
  }

  static size_t get_height(const BTEntry* e) {
    return e == 0 ? 0 : e->height;
  }

  void update_height() {
    height = olx_max(get_height(left), get_height(right)) + 1;
  }

  int get_bf() const {
    return (int)get_height(left) - (int)get_height(right);
  }

  BTEntry* rotate_left() {
    BTEntry* r = right;
    right = r->left;
    r->left = this;
    this->update_height();
    r->update_height();
    return r;
  }

  BTEntry* rotate_right() {
    BTEntry* l = left;
    left = l->right;
    l->right = this;
    this->update_height();
    l->update_height();
    return l;
  }

  void Clear() {
    if (next != 0) {
      delete next;
      next = 0;
    }
    if (right != 0) {
      delete right;
      right = 0;
    }
    if (left != 0) {
      delete left;
      left = 0;
    }
  }
};


template <class entry_tt, class Comparator=TComparableComparator>
class BTree {
protected:
  entry_tt *Root;
  size_t _Count, _LeafCount;
  Comparator cmp;
public:
  typedef entry_tt entry_t;
  typedef typename entry_t::value_t value_t;
  typedef typename entry_t::key_t key_t;
  BTree(const Comparator &cmp= Comparator())
    : Root(0), _Count(0), _LeafCount(0), cmp(cmp)
  {}

  ~BTree() { olx_del_obj(Root); }

  void Clear() {
    if (Root != 0) {
      delete Root;
      _LeafCount = _Count = 0;
      Root = 0;
    }
  }

  const entry_t* GetRoot() const { return Root; }
  entry_t* GetRoot() { return Root; }
  size_t Count() const { return _Count; }
  size_t LeafCount() const { return _LeafCount; }

  olx_pair_t<entry_t*, int> find_semi(const key_t& key) const {
    int cmp_v = Root->cmp(key, cmp);
    if (cmp_v < 0) {
      if (Root->left != 0) {
        return go_left(Root->left, key);
      }
    }
    else if (cmp_v > 0) {
      if (Root->right != 0) {
        return go_right(Root->right, key);
      }
      return Root;
    }
    return olx_pair_t<entry_t*, int>(Root, 0);
  }

  olx_pair_t<entry_t*, int> find_recursive(entry_t*e, const key_t& key) const {
    int cmp_v = e->cmp(key, cmp);
    if (cmp_v < 0) {
      if (e->left != 0) {
        return find_recursive(e->left, key);
      }
    }
    else if (cmp_v > 0) {
      if (e->right != 0) {
        return find_recursive(e->right, key);
      }
    }
    return olx_pair_t<entry_t*, int>(e, cmp_v);
  }

  olx_pair_t<entry_t*, int> go_left(entry_t*en, const key_t& key) const {
    int cmp_v1;
    while ((cmp_v1 = en->cmp(key, cmp)) < 0) {
      if (en->left == 0) {
        return olx_pair_t<entry_t*, int>(en, cmp_v1);
      }
      en = en->left;
    }
    if (cmp_v1 > 0 && en->right != 0) {
      return go_right(en->right, key);
    }
    return olx_pair_t<entry_t*, int>(en, cmp_v1);
  }

  olx_pair_t<entry_t*, int> go_right(entry_t* en, const key_t& key) const {
    int cmp_v1;
    while ((cmp_v1 = en->cmp(key, cmp)) > 0) {
      if (en->right == 0) {
        return olx_pair_t<entry_t*, int>(en, cmp_v1);
      }
      en = en->right;
    }
    if (cmp_v1 < 0 && en->left != 0) {
      return go_left(en->left, key);
    }
    return olx_pair_t<entry_t*, int>(en, cmp_v1);
  }
  
  entry_t* Add(const value_t &e) {
    return Root = insert_recursive(Root, e);
  }

  entry_t* insert_recursive(entry_t* node, const value_t &e) {
    if (node == 0) {
      node = new entry_t(e);
      _LeafCount++;
      _Count++;
      if (Root == 0) {
        Root = node;
      }
      return node;
    }
    int cmp_v = node->cmp(e, cmp);
    if (cmp_v < 0) {
      node->left = insert_recursive(node->left, e);
    }
    else if (cmp_v > 0) {
      node->right = insert_recursive(node->right, e);
    }
    else {
      if (node->last != 0) {
        node->last->next = new entry_t(e);
        node->last = node->last->next;
      }
      else {
        node->last = node->next = new entry_t(e);
      }
      _Count++;
      return node;
    }
    node->update_height();
    int bf = node->get_bf();
    if (bf > 1 || bf < -1) {
      int l_cmp;
      if (bf > 1 && (l_cmp = node->left->cmp(e, cmp)) < 0) {
        return node->rotate_right();
      }
      int r_cmp;
      if (bf < -1 && (r_cmp= node->right->cmp(e, cmp)) > 0) {
        return node->rotate_left();
      }

      if (bf > 1 && l_cmp > 0) {
        node->left = node->left->rotate_left();
        return node->rotate_right();
      }

      if (bf < -1 && r_cmp < 0) {
        node->right = node->right->rotate_right();
        return node->rotate_left();
      }
    }
    return node;
  }

  entry_t* Find(const key_t& key) const {
    if (_Count == 0) {
      return 0;
    }
    //olx_pair_t<Entry*, int> r = find_recurive(Root, key);
    olx_pair_t<entry_t*, int> r = find_semi(key);
    if (r.b == 0) {
      return r.a;
    }
    return 0;
  }

  bool Contains(const key_t& key) const {
    return Find(key) != 0;
  }

  static TBTreeTraverser< BTree<entry_t, Comparator> > Traverser;
};

template <class entry_t, class Comparator>
  TBTreeTraverser< BTree<entry_t, Comparator> >
    BTree<entry_t, Comparator>::Traverser;

template <typename C, typename O>  class BTree2 {
public:
  typedef TreeMapEntry<C, O> entry_t;
  typedef BTree<entry_t, TPrimitiveComparator> XTree;
  typedef TreeMapEntry<C, XTree*> entry_t1;
  typedef BTree<entry_t1, TPrimitiveComparator> YTree;
  typedef typename YTree::Entry YEntry;
  typedef typename XTree::Entry XEntry;
private:
  size_t _Count;
protected:
  YTree Tree;
public:
  BTree2() : _Count(0) {}
  ~BTree2() { Clear(); }
  //___________________________________________________________________
  bool OnItem(const YEntry* item) {
    delete const_cast<YEntry*>(item)->val;
    return true;
  }
  //___________________________________________________________________
  void Clear() {
    Tree.Traverser.Traverse(Tree, *this);
    Tree.Clear();
    _Count = 0;
  }

  const YEntry* GetRoot() const { return Tree.GetRoot(); }
  size_t Count() const { return _Count; }

  O& Add(const C& x, const C& y, const O& val) {
    YEntry* insertPY = 0;
    XEntry* insertPX = 0;
    XTree** rx = Tree.Find(y, &insertPY);
    if (rx == 0) {
      rx = &Tree.Add(y, new XTree(), insertPY);
    }
    O* ry = (*rx)->Find(x, &insertPX);
    if (ry == 0) {
      ry = &(*rx)->Add(x, val, insertPX);
    }
    _Count++;
    return *ry;
  }
  O* Find(const C& x, const C& y) {
    XTree** rx = Tree.Find(y);
    return (rx == 0) ? 0 : (*rx)->Find(x);
  }
  class Tree2Traverser {
    template <class Traverser> class InternalTraverser {
      Traverser& trav;
    public:
      InternalTraverser(Traverser& t) : trav(t) {}
      bool OnItem(const YEntry* item) {
        return item->val->Traverser.Traverse(*item->val, trav);
      }
    };
    template <class Traverser> class InternalFullTraverser {
      Traverser& trav;
    public:
      InternalFullTraverser(Traverser& t) : trav(t) {}
      bool OnItem(const YEntry* item) {
        trav.SetY(item->key);
        return item->val->Traverser.Traverse(*item->val, trav);
      }
    };
  public:
    template <class Trav>
    bool Traverse(const BTree2<C, O>& tree, Trav& traverser) {
      InternalTraverser<Trav> t(traverser);
      return tree.Tree.Traverser.Traverse(tree.Tree, t);
    }
    template <class Trav>
    bool FullTraverse(const BTree2<C, O>& tree, Trav& traverser) {
      InternalFullTraverser<Trav> t(traverser);
      return tree.Tree.Traverser.Traverse(tree.Tree, t);
    }
  };
  static Tree2Traverser Traverser;
};

template <typename C, typename O>
typename BTree2<C,O>::Tree2Traverser BTree2<C,O>::Traverser;

template <typename C, typename O>  class BTree3 {
public:
  typedef BTree2<C, O> XYTree;
  typedef TreeMapEntry<C, XYTree*> entry_t2;
  typedef BTree<entry_t2, TPrimitiveComparator> ZTree;
  typedef typename XYTree::YEntry XYEntry;
  typedef typename ZTree::Entry ZEntry;
private:
  size_t _Count;
protected:
  ZTree Tree;
public:
  BTree3() : _Count(0) {}
  ~BTree3() { Clear(); }
  //___________________________________________________________________
  bool OnItem(const ZEntry* item) {
    delete const_cast<ZEntry*>(item)->val;
    return true;
  }
  //___________________________________________________________________
  void Clear() {
    Tree.Traverser.Traverse(Tree, *this);
    Tree.Clear();
    _Count = 0;
  }

  const ZEntry* GetRoot() const { return Tree.GetRoot(); }
  size_t Count() const { return _Count; }

  O& Add(const C& x, const C& y, const C& z, const O& val) {
    ZEntry* insertPZ = 0;
    XYTree** xy = Tree.Find(z, &insertPZ);
    if (xy == 0) {
      xy = &Tree.Add(z, new XYTree(), insertPZ);
    }
    _Count++;
    return (*xy)->Add(x, y, val);
  }
  O* Find(const C& x, const C& y, const C& z) {
    XYTree** xy = Tree.Find(z);
    return (xy == 0) ? 0 : (*xy)->Find(x, y);
  }
  class Tree3Traverser {
    template <class Traverser> class InternalTraverser {
      Traverser& trav;
    public:
      InternalTraverser(Traverser& t) : trav(t) {}
      bool OnItem(const ZEntry* item) {
        return item->val->Traverser.Traverse(*item->val, trav);
      }
    };
    template <class Traverser> class InternalFullTraverser {
      Traverser& trav;
    public:
      InternalFullTraverser(Traverser& t) : trav(t) {}
      bool OnItem(const ZEntry* item) {
        trav.SetZ(item->key);
        return item->val->Traverser.FullTraverse(*item->val, trav);
      }
    };
  public:
    template <class Trav>
    bool Traverse(const BTree3<C, O>& tree, Trav& traverser) {
      InternalTraverser<Trav> t(traverser);
      return tree.Tree.Traverser.Traverse(tree.Tree, t);
    }
    template <class Trav>
    bool FullTraverse(const BTree3<C, O>& tree, Trav& traverser) {
      InternalFullTraverser<Trav> t(traverser);
      return tree.Tree.Traverser.Traverse(tree.Tree, t);
    }
  };
  static Tree3Traverser Traverser;
};

template <typename C, typename O>
  typename BTree3<C,O>::Tree3Traverser BTree3<C,O>::Traverser;

EndEsdlNamespace()
