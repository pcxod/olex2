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

template <class actual, typename value_tt>
struct ABTreeEntry {
  typedef value_tt value_t;
  typedef typename value_t::key_t key_t;
  value_t value;
  actual* left, * right;

  ABTreeEntry()
    : left(0), right(0)
  {}

  ABTreeEntry(const value_t& value)
    : value(value),
    left(0), right(0)
  {}

  virtual ~ABTreeEntry() {
    olx_del_obj(right);
    olx_del_obj(left);
  }

  virtual void Clear() {
    if (right != 0) {
      delete right;
      right = 0;
    }
    if (left != 0) {
      delete left;
      left = 0;
    }
  }

  template <class cmp_t>
  int cmp(const value_t& e, const cmp_t& comparator) const {
    return value.cmp(e.key, comparator);
  }

  template <class cmp_t>
  int cmp(const key_t& k, const cmp_t& comparator) const {
    return value.cmp(k, comparator);
  }

  virtual actual* rotate_left() {
    actual* r = right;
    right = r->left;
    r->left = (actual*)(this);
    return update(r);
  }

  virtual actual* rotate_right() {
    actual* l = left;
    left = l->right;
    l->right = (actual*)(this);
    return update(l);
  }

  virtual bool AddSame(const value_t& v) {
    return false;
  }

  virtual actual* update(actual* e) = 0;
};

template <class actual, typename value_tt>
struct ARBTreeEntry_ : public ABTreeEntry <actual, value_tt >{
  typedef ABTreeEntry <actual, value_tt> parent_t;
  typedef value_tt value_t;
  typedef typename value_t::key_t key_t;
  actual* parent;
  char color;

  ARBTreeEntry_() : parent(0), color('R')
  {}

  ARBTreeEntry_(const value_t& value)
    : parent(0), color('R'),
    parent_t(value)
  {}

  virtual actual* rotate_left() {
    parent = this->right;
    if (this->left != 0) {
      this->left->parent = this->right;
    }
    return parent_t::rotate_left();
  }

  virtual actual* rotate_right() {
    parent = this->left;
    if (this->right != 0) {
      this->right->parent = this->left;
    }
    return parent_t::rotate_right();
  }
};

template <class actual, class value_tt>
struct AVLBTEntry_ : public ABTreeEntry<actual, value_tt > {
  typedef ABTreeEntry<actual, value_tt > parent_t;
  typedef value_tt value_t;
  typedef typename value_t::key_t key_t;
  size_t height;

  AVLBTEntry_()
    : height(1)
  {}

  AVLBTEntry_(const value_t& value)
    : parent_t(value), height(1)
  {}

  static size_t get_height(const actual* e) {
    return e == 0 ? 0 : e->height;
  }

  void update_height() {
    height = olx_max(get_height(this->left), get_height(this->right)) + 1;
  }

  int get_bf() const {
    return (int)get_height(this->left) - (int)get_height(this->right);
  }

  virtual actual* update(actual* e) {
    this->update_height();
    e->update_height();
    return e;
  }
};
template <class value_tt>
struct AVLTreeEntry : public AVLBTEntry_<AVLTreeEntry<value_tt>, value_tt> {
  typedef AVLBTEntry_<AVLTreeEntry<value_tt>, value_tt> parent_t;
  AVLTreeEntry()
  {}

  AVLTreeEntry(const value_tt& value)
    : parent_t(value)
  {}
};

template <class actual, class value_tt>
struct RBTreeEntry_ : public ARBTreeEntry_<actual, value_tt> {
  typedef ARBTreeEntry_<actual, value_tt> parent_t;
  RBTreeEntry_() {}

  RBTreeEntry_(const value_tt& value)
    : parent_t(value)
  {}

  virtual actual* update(actual* e) {
    return e;
  }
};

template <class value_tt>
struct RBTreeEntry : public RBTreeEntry_<RBTreeEntry<value_tt>, value_tt> {
  typedef value_tt value_t;
  typedef RBTreeEntry_<RBTreeEntry<value_tt>, value_tt> parent_t;
  RBTreeEntry()
  {}

  RBTreeEntry(const value_t& value)
    : parent_t(value)
  {}
};


template <class value_tt>
struct AVLEntryEx : public AVLBTEntry_<AVLEntryEx<value_tt>, value_tt> {
  typedef AVLBTEntry_<AVLEntryEx<value_tt>, value_tt > parent_t;
  typedef AVLEntryEx<value_tt> actual;
  typedef value_tt value_t;
  typedef typename value_t::key_t key_t;
  AVLEntryEx* next, * last;  // linked list of items with equal key

  AVLEntryEx()
    : next(0), last(0)
  {}

  AVLEntryEx(const value_t& value)
    : parent_t(value),
    next(0), last(0)
  {}

  ~AVLEntryEx() {
    olx_del_obj(next);
  }

  void Clear() {
    parent_t::Clear();
    if (next != 0) {
      delete next;
      next = 0;
    }
  }
};

template <class entry_tt, class Comparator=TComparableComparator>
class BTree_ {
protected:
  entry_tt *Root;
  size_t _Count, _LeafCount;
  Comparator cmp;
public:
  typedef entry_tt entry_t;
  typedef typename entry_t::value_t value_t;
  typedef typename entry_t::key_t key_t;
  BTree_(const Comparator &cmp= Comparator())
    : Root(0), _Count(0), _LeafCount(0), cmp(cmp)
  {}

  ~BTree_() { olx_del_obj(Root); }

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
    int cmp_v = this->Root->cmp(key, cmp);
    if (cmp_v < 0) {
      if (this->Root->left != 0) {
        return go_left(this->Root->left, key);
      }
    }
    else if (cmp_v > 0) {
      if (this->Root->right != 0) {
        return go_right(this->Root->right, key);
      }
      return this->Root;
    }
    return olx_pair_t<entry_t*, int>(this->Root, 0);
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

  virtual entry_t* insert_recursive(entry_t* node, const value_t& e) = 0;

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

  static TBTreeTraverser< BTree_<entry_t, Comparator> > Traverser;
};

template <class entry_tt, class Comparator = TComparableComparator>
class AVLTree : public BTree_<entry_tt, Comparator> {
public:
  typedef entry_tt entry_t;
  typedef typename entry_t::value_t value_t;
  typedef typename entry_t::key_t key_t;
  AVLTree(const Comparator& cmp = Comparator())
    : BTree_<entry_tt, Comparator>(cmp)
  {}

  entry_t* insert_recursive(entry_t* node, const value_t &e) {
    if (node == 0) {
      node = new entry_t(e);
      this->_LeafCount++;
      this->_Count++;
      if (this->Root == 0) {
        this->Root = node;
      }
      return node;
    }
    int cmp_v = node->cmp(e, this->cmp);
    if (cmp_v < 0) {
      node->left = insert_recursive(node->left, e);
    }
    else if (cmp_v > 0) {
      node->right = insert_recursive(node->right, e);
    }
    else {
      if (node->AddSame(e)) {
        this->_Count++;
      }
      return node;
    }
    node->update_height();
    int bf = node->get_bf();
    if (bf > 1 || bf < -1) {
      int l_cmp;
      if (bf > 1 && (l_cmp = node->left->cmp(e, this->cmp)) < 0) {
        return node->rotate_right();
      }
      int r_cmp;
      if (bf < -1 && (r_cmp= node->right->cmp(e, this->cmp)) > 0) {
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

};

/*
https://www.geeksforgeeks.org/dsa/insertion-in-red-black-tree/
*/

template <class entry_tt, class Comparator = TComparableComparator>
class RBTree : public BTree_<entry_tt, Comparator> {
public:
  typedef entry_tt entry_t;
  typedef typename entry_t::value_t value_t;
  typedef typename entry_t::key_t key_t;
  bool ll, lr, rr, rl;

  RBTree() {
    ll= lr = rr = rl = false;
  }

  entry_t* insert_recursive(entry_t* node, const value_t& v) {
    bool check = false;
    if (node == 0) {
      this->_Count++;
      this->_LeafCount++;
      node = new entry_t(v);
      if (this->Root == 0) {
        this->Root = node;
      }
      return node;
    }
    int cmp_v = node->cmp(v, this->cmp);
    if (cmp_v < 0) {
      node->left = insert_recursive(node->left, v);
      node->left->parent = node;
      if (node != this->Root) {
        if (node->color == 'R' && node->left->color == 'R') {
          check = true;
        }
      }
    }
    else if (cmp_v > 0) {
      node->right = insert_recursive(node->right, v);
      node->right->parent = node;
      if (node != this->Root) {
        if (node->color == 'R' && node->right->color == 'R') {
          check = true;
        }
      }
    }
    else {
      if (node->AddSame(v)) {
        this->_Count++;
      }
      return node;
    }

    // Perform rotations
    if (ll) {
      node = node->rotate_left();
      node->color = 'B';
      node->left->color = 'R';
      ll = false;
    }
    else if (rr) {
      node = node->rotate_right();
      node->color = 'B';
      node->right->color = 'R';
      rr = false;
    }
    else if (rl) {
      node->right = node->right->rotate_right();
      node->right->parent = node;
      node = node->rotate_left();
      node->color = 'B';
      node->left->color = 'R';
      rl = false;
    }
    else if (lr) {
      node->left = node->left->rotate_left();
      node->left->parent = node;
      node = node->rotate_right();
      node->color = 'B';
      node->right->color = 'R';
      lr = false;
    }

    // Handle RED-RED conflicts
    if (check) {
      if (node->parent->right == node) {
        if (node->parent->left == 0 || node->parent->left->color == 'B') {
          if (node->left != 0 && node->left->color == 'R') {
            rl = true;
          }
          else if (node->right != 0 && node->right->color == 'R') {
            ll = true;
          }
        }
        else {
          node->parent->left->color = 'B';
          node->color = 'B';
          if (node->parent != this->Root) {
            node->parent->color = 'R';
          }
        }
      }
      else {
        if (node->parent->right == 0 || node->parent->right->color == 'B') {
          if (node->left != 0 && node->left->color == 'R') {
            rr = true;
          }
          else if (node->right != 0 && node->right->color == 'R') {
            lr = true;
          }
        }
        else {
          node->parent->right->color = 'B';
          node->color = 'B';
          if (node->parent != this->Root) {
            node->parent->color = 'R';
          }
        }
      }
    }
    return node;
  }

};

template <class entry_t, class Comparator>
  TBTreeTraverser<BTree_<entry_t, Comparator> >
    BTree_<entry_t, Comparator>::Traverser;

template <typename C, typename O>  class BTree2 {
public:
  typedef TreeMapEntry<C, O> entry_t;
  typedef AVLTree<entry_t, TPrimitiveComparator> XTree;
  typedef TreeMapEntry<C, XTree*> entry_t1;
  typedef AVLTree<entry_t1, TPrimitiveComparator> YTree;
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

template <class entry_t, class cmp_t>
typename BTree2<entry_t, cmp_t>::Tree2Traverser BTree2<entry_t, cmp_t>::Traverser;

template <typename C, typename O>  class BTree3 {
public:
  typedef BTree2<C, O> XYTree;
  typedef TreeMapEntry<C, XYTree*> entry_t2;
  typedef AVLTree<entry_t2, TPrimitiveComparator> ZTree;
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
