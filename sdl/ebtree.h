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
#include "ellist.h"

BeginEsdlNamespace()

template <typename key_tt>
struct TreeSetEntry {
  typedef key_tt key_t;
  typedef key_tt value_t;

  key_t key;
  
  TreeSetEntry(const key_t& k)
    : key(k)
  {}

  template <class cmp_t>
  int cmp(const key_t& k, const cmp_t& comparator) const {
    return comparator.Compare(key, k);
  }
  const value_t& get_value() const {
    return key;
  }
};

template <typename key_tt, typename value_tt>
struct TreeMapEntry {
  typedef key_tt key_t;
  typedef value_tt value_t;

  key_t key;
  value_t value;
  TreeMapEntry(const key_t& k, const value_t& v)
    : key(k), value(v)
  {}

  template <class cmp_t>
  int cmp(const key_t& k, const cmp_t& comparator) const {
    return comparator.Compare(key, k);
  }
  const value_t& get_value() const {
    return value;
  }
};

template <typename value_t>
struct DuplicateTreeEntry {
  value_t data;
  DuplicateTreeEntry* next;
  DuplicateTreeEntry(const value_t& d)
    : data(d), next(0)
  {}
};

template <class actual, typename value_tt>
struct ABTreeEntry {
  typedef value_tt value_t;
  typedef typename value_t::value_t val_t;
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

  virtual bool AddSame(const val_t& v) {
    return false;
  }

  void copy_from(const ABTreeEntry& e) {
    value = e.value;
    left = e.left;
    right = e.right;
  }

  const val_t& get_value() const {
    return value.get_value();
  }

  IIterator<val_t>* iterate() const {
    return 0;
  }

  virtual size_t Count() const { return 1; }
};

template <class entry_tt, class Comparator=TComparableComparator>
class BTree_ :
public AIterable<typename entry_tt::val_t>
{
protected:
  entry_tt *Root;
  size_t _Count, _LeafCount;
  Comparator cmp;
public:
  typedef entry_tt entry_t;
  typedef typename entry_t::value_t value_t;
  typedef typename entry_t::key_t key_t;
  typedef typename entry_t::val_t val_t;

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

  entry_t* leftmost(entry_t *e) {
    if (e->left == 0) {
      return e;
    }
    while (e->left != 0) {
      e = e->left;
    }
    return e;
  }

  bool Add(const value_t &e) {
    return insert_(e);
  }

  entry_t* Find(const key_t& key) const {
    if (_Count == 0) {
      return 0;
    }
    olx_pair_t<entry_t*, int> r = find_node(key);
    if (r.b == 0) {
      return r.a;
    }
    return 0;
  }

  bool Contains(const key_t& key) const {
    return Find(key) != 0;
  }

  bool Remove(const key_t& key) {
    if (_Count == 0) {
      return false;
    }
    return delete_(key);
  }

  // for sets will have keys, for maps - values
  typename TUDTypeList<val_t>::const_list_type ToValueList() const {
    TUDTypeList<val_t> l;
    if (Root == 0) {
      return l;
    }
    fill_value_list(Root, l);
    return l;
  }

  // identical for sets, for maps will have (key, value)
  typename TUDTypeList<value_t>::const_list_type ToEntryList() const {
    TUDTypeList<value_t> l;
    if (Root == 0) {
      return l;
    }
    fill_entry_list(Root, l);
    return l;
  }

  struct ValueIterator : public IIterator<val_t> {
  private:
    TUDTypeList<val_t> list;
    typename TLinkedList<val_t>::Iterator itr;
  public:
    ValueIterator(const BTree_&p)
      : list(p.ToValueList())
    {
      itr = list.GetIterator();
    }
    bool HasNext() const { return itr.HasNext(); }
    const val_t& Next() { return itr.Next(); }
    const val_t& Lookup() const { return itr.Lookup(); }
    size_t Count() const { return itr.Count(); }
  };

  ValueIterator GetValueIterator() const {
    return ValueIterator(*this);
  }

  IIterator<val_t>* iterate() const {
    return new ValueIterator(*this);
  }

protected:
  void fill_value_list(const entry_t* e, TUDTypeList<val_t>& l) const {
    if (e->left != 0) {
      fill_value_list(e->left, l);
    }
    l.Add(e->get_value());
    IIterator<val_t> *itr =  e->iterate();
    if (itr != 0) {
      while (itr->HasNext()) {
        l.Add(itr->Next());
      }
      delete itr;
    }
    if (e->right != 0) {
      fill_value_list(e->right, l);
    }
  }

  void fill_entry_list(const entry_t* e, TUDTypeList<value_t>& l) const {
    if (e->left != 0) {
      fill_entry_list(e->left, l);
    }
    l.Add(e->value);
    if (e->right != 0) {
      fill_entry_list(e->right, l);
    }
  }

  olx_pair_t<entry_t*, int> find_node(const key_t& key) const {
    entry_t* tmp = Root;
    int cmp_v = 0;
    while (tmp != 0) {
      cmp_v = tmp->cmp(key, this->cmp);
      if (cmp_v < 0) {
        if (tmp->left == 0) {
          break;
        }
        else {
          tmp = tmp->left;
        }
      }
      else if (cmp_v == 0) {
        break;
      }
      else {
        if (tmp->right == 0) {
          break;
        }
        else {
          tmp = tmp->right;
        }
      }
    }
    return olx_pair::make(tmp, cmp_v);
  }

  virtual bool insert_(const value_t& e) = 0;
  // must update _Count!
  virtual bool delete_(const key_t& e) = 0;

  public:
  static TBTreeTraverser< BTree_<entry_t, Comparator> > Traverser;
};

/*
* inspired by
* https://en.wikipedia.org/wiki/AVL_tree
* https://www.geeksforgeeks.org/dsa/insertion-in-an-avl-tree/
* https://www.geeksforgeeks.org/dsa/deletion-in-an-avl-tree/
*/

template <class actual, class value_tt>
struct AVLBTEntry_ : public ABTreeEntry<actual, value_tt > {
  typedef ABTreeEntry<actual, value_tt> parent_t;
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

  actual* rotate_left() {
    actual* r = this->right;
    this->right = r->left;
    r->left = (actual*)(this);
    return update(r);
  }

  actual* rotate_right() {
    actual* l = this->left;
    this->left = l->right;
    l->right = (actual*)(this);
    return update(l);
  }

  actual* update(actual* e) {
    this->update_height();
    e->update_height();
    return e;
  }

  void copy_from(const AVLBTEntry_& e) {
    parent_t::copy_from(e);
    height = e.height;
  }

  void clear_siblings() {
    this->left = this->right = 0;
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


template <class value_tt>
struct AVLTreeEntryEx :
  public AVLBTEntry_<AVLTreeEntryEx<value_tt>, value_tt>,
  public AIterable<typename value_tt::value_t>
{
  typedef AVLBTEntry_<AVLTreeEntryEx<value_tt>, value_tt > parent_t;
  typedef AVLTreeEntryEx<value_tt> actual;
  typedef value_tt value_t;
  typedef typename value_t::key_t key_t;
  typedef typename value_t::value_t val_t;
  typedef DuplicateTreeEntry<val_t> duplicate_value_t;

  duplicate_value_t* next, * last;

  AVLTreeEntryEx()
    : next(0), last(0)
  {}

  AVLTreeEntryEx(const value_t& value)
    : parent_t(value),
    next(0), last(0)
  {}

  ~AVLTreeEntryEx() {
    olx_del_obj(next);
  }

  void Clear() {
    parent_t::Clear();
    if (next != 0) {
      delete next;
      next = 0;
    }
  }
  void copy_from(const AVLTreeEntryEx& e) {
    parent_t::copy_from(e);
    next = e.next;
    last = e.last;
  }

  virtual bool AddSame(const val_t& v) {
    if (last != 0) {
      last->next = new duplicate_value_t(v);
      last = last->next;
    }
    else {
      last = next = new duplicate_value_t(v);
    }
    return true;
  }

  struct Itr : public IIterator<val_t> {
    duplicate_value_t* nxt;
    Itr(duplicate_value_t* nxt)
      : nxt(nxt)
    {}

    bool HasNext() const { return nxt != 0; }

    const val_t& Next() {
      if (nxt == 0) {
        return 0;
      }
      duplicate_value_t* rv = nxt;
      nxt = nxt->next;
      return rv->data;
    }
  };

  IIterator<val_t>* iterate() const {
    return new Itr(next);
  }

  size_t Count() const { 
    size_t cnt = 1;
    duplicate_value_t* nxt = next;
    while (nxt != 0) {
      cnt++;
      nxt = nxt->next;
    }
    return cnt;
  }
};

template <class entry_tt, class Comparator = TComparableComparator>
class AVLTree : public BTree_<entry_tt, Comparator> {
public:
  typedef entry_tt entry_t;
  typedef typename entry_t::value_t value_t;
  typedef typename entry_t::key_t key_t;
  typedef BTree_<entry_tt, Comparator> parent_t;
  AVLTree(const Comparator& cmp = Comparator())
    : BTree_<entry_tt, Comparator>(cmp)
  {}

  entry_t* rebalance_i(entry_t *node, const value_t& e) {
    int bf = node->get_bf();
    if (bf > 1 || bf < -1) {
      int l_cmp;
      if (bf > 1 && (l_cmp = node->left->cmp(e, this->cmp)) < 0) {
        return node->rotate_right();
      }
      int r_cmp;
      if (bf < -1 && (r_cmp = node->right->cmp(e, this->cmp)) > 0) {
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

  entry_t* rebalance_d(entry_t* node) {
    int bf = node->get_bf();
    if (bf > 1) {
      if (node->left->get_bf() < 0) {
        node->left = node->left->rotate_left();
      }
      return node->rotate_right();
    }
    if (bf < -1) {
      if (node->right->get_bf() > 0) {
        node->right = node->right->rotate_right();
      }
      return node->rotate_left();
    }
    return node;
  }

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
      if (node->AddSame(e.get_value())) {
        this->_Count++;
      }
      return node;
    }
    node->update_height();
    return rebalance_i(node, e);
  }
  
  entry_t* delete_recursive(entry_t* node, const key_t& key) {
    if (node == 0) {
      return 0;
    }
    int cmp_v = node->cmp(key, this->cmp);
    if (cmp_v < 0) {
      node->left = delete_recursive(node->left, key);
    }
    else if (cmp_v > 0) {
      node->right = delete_recursive(node->right, key);
    }
    else {
      if (node->left == 0 || node->right == 0) {
        entry_t* e = node->left == 0 ? node->right : node->left;
        size_t rc;
        if (e == 0) {
          e = node;
          rc = e->Count();
          node = 0;
        }
        else {
          rc = e->Count();
          node->copy_from(*e);
          e->clear_siblings();
        }
        delete e;
        this->_LeafCount--;
        this->_Count -= rc;
      }
      else {
        entry_t* e = parent_t::leftmost(node->right);
        node->value.key = e->value.key;
        node->right = delete_recursive(node->right, node->value.key);
      }
      return node;
    }
    if (node == 0) {
      return 0;
    }
    node->update_height();
    return rebalance_d(node);
  }

  virtual bool insert_(const value_t& e) {
    size_t lc = this->_LeafCount;
    this->Root = insert_recursive(this->Root, e);
    return this->_LeafCount > lc;
  }

  virtual bool delete_(const key_t& key) {
    size_t lc = this->_LeafCount;
    this->Root = delete_recursive(this->Root, key);
    return this->_LeafCount < lc;
  }
};

/*
* inspired by
https://www.geeksforgeeks.org/dsa/insertion-in-red-black-tree/
https://www.geeksforgeeks.org/dsa/deletion-in-red-black-tree/
*/

/******************************************************************************/
/********************RB TREE***************************************************/
/******************************************************************************/
template <class actual, typename value_tt>
struct ARBTreeEntry_ : public ABTreeEntry <actual, value_tt > {
  typedef ABTreeEntry <actual, value_tt> parent_t;
  typedef value_tt value_t;
  typedef typename value_t::key_t key_t;
  actual* parent;
  char color;

  ARBTreeEntry_()
    : parent(0), color('R')
  {}

  ARBTreeEntry_(const value_t& value)
    : parent(0), color('R'),
    parent_t(value)
  {}

  actual* rotate_left() {
    if (parent != 0) {
      if (parent->left == (actual*)this) {
        parent->left = this->right;
      }
      else {
        parent->right = this->right;
      }
    }
    this->right->parent = parent;
    this->parent = this->right;
    actual* r = this->right;
    if ((this->right = r->left) != 0) {
      this->right->parent = (actual*)(this);
    }
    r->left = (actual*)(this);
    return this->parent;
  }

  actual* rotate_right() {
    if (parent != 0) {
      if (parent->left == (actual*)this) {
        parent->left = this->left;
      }
      else {
        parent->right = this->left;
      }
    }
    this->left->parent = parent;
    this->parent = this->left;
    actual* l = this->left;
    if ((this->left = l->right) != 0) {
      this->left->parent = (actual*)(this);
    }
    l->right = (actual*)(this);
    return this->parent;
  }

  actual* successor() {
    actual* tmp = (actual*)this;
    while (tmp->left != 0) {
      tmp = tmp->left;
    }
    return tmp;
  }

  actual* replacement() {
    if (this->left != 0 && this->right != 0) {
      return this->right->successor();
    }

    if (this->left == 0 && this->right == 0) {
      return 0;
    }

    if (this->left != 0) {
      return this->left;
    }
    else {
      return this->right;
    }
  }

  bool is_left() const {
    return parent != 0 && parent->left == (actual*)this;
  }

  actual* uncle() const {
    if (parent == 0 || parent->parent == 0) {
      return 0;
    }
    if (parent->is_left()) {
      return parent->parent->right;
    }
    return parent->parent->left;
  }

  actual* sibling() const {
    if (parent == 0) {
      return 0;
    }
    return is_left() ? parent->right : parent->left;
  }

  bool has_reds() const {
    return (this->left != 0 && this->left->color == 'R') ||
      (this->right != 0 && this->right->color == 'R');
  }

};

template <class actual, class value_tt>
struct RBTreeEntry_ : public ARBTreeEntry_<actual, value_tt> {
  typedef ARBTreeEntry_<actual, value_tt> parent_t;
  RBTreeEntry_() {}

  RBTreeEntry_(const value_tt& value)
    : parent_t(value)
  {}

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
struct RBTreeEntryEx :
  public RBTreeEntry_<RBTreeEntryEx<value_tt>, value_tt>,
  public AIterable<typename value_tt::value_t> 
{
  typedef RBTreeEntry_<RBTreeEntryEx<value_tt>, value_tt > parent_t;
  typedef RBTreeEntryEx<value_tt> actual;
  typedef value_tt value_t;
  typedef typename value_t::key_t key_t;
  typedef typename value_t::value_t val_t;
  typedef DuplicateTreeEntry<val_t> duplicate_value_t;

  duplicate_value_t* next, * last;

  RBTreeEntryEx()
    : next(0), last(0)
  {}

  RBTreeEntryEx(const value_t& value)
    : parent_t(value),
    next(0), last(0)
  {}

  ~RBTreeEntryEx() {
    olx_del_obj(next);
  }

  virtual bool AddSame(const val_t& v) {
    if (last != 0) {
      last->next = new duplicate_value_t(v);
      last = last->next;
    }
    else {
      last = next = new duplicate_value_t(v);
    }
    return true;
  }

  struct Itr : public IIterator<val_t> {
    duplicate_value_t* nxt;
    Itr(duplicate_value_t* nxt)
      : nxt(nxt)
    {}

    bool HasNext() const { return nxt != 0; }

    const val_t& Next() {
#ifdef OLX_DEBUG
      if (nxt == 0) {
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "no more data");
      }
#endif
      duplicate_value_t* rv = nxt;
      nxt = nxt->next;
      return rv->data;
    }
  };

  IIterator<val_t>* iterate() const {
    return new Itr(next);
  }

  size_t Count() const {
    size_t cnt = 1;
    duplicate_value_t* nxt = next;
    while (nxt != 0) {
      cnt++;
      nxt = nxt->next;
    }
    return cnt;
  }

  bool replace(const val_t & o, const val_t& n) {
    if (o == this->value.value) {
      this->value.value = n;
      return true;
    }
    duplicate_value_t* nxt = next;
    while (nxt != 0) {
      if (nxt->data == o) {
        nxt->data = n;
        return true;
      }
      nxt = nxt->next;
    }
    return false;
  }
};

template <class entry_tt, class Comparator = TComparableComparator>
class RBTree : public BTree_<entry_tt, Comparator> {
public:
  typedef entry_tt entry_t;
  typedef typename entry_t::value_t value_t;
  typedef typename entry_t::key_t key_t;
  typedef BTree_<entry_tt, Comparator> parent_t;

  RBTree(const Comparator& cmp = Comparator())
    : BTree_<entry_tt, Comparator>(cmp)
  {}

  void delete_node(entry_t* node) {
    entry_t* u = node->replacement();
    // True when u and v are both black
    bool uvBlack = ((u == 0 || u->color == 'B') && (node->color == 'B'));
    if (u == 0) {
      // u is NULL therefore v is leaf
      if (node == this->Root) {
        // v is root, making root null
        this->_Count = this->_LeafCount = 0;
        this->Root = 0;
      }
      else {
        if (uvBlack) {
          fix_bb(node);
        }
        else {
          // u or v is red
          if (node->sibling() != 0) {
            // sibling is not null, make it red"
            node->sibling()->color = 'R';
          }
        }
        // delete v from the tree
        entry_t* parent = node->parent;
        if (parent->left == node) {
          parent->left = 0;
        }
        else {
          parent->right = 0;
        }
      }
      delete node;
      return;
    }

    if (node->left == 0 || node->right == 0) {
      // ndde has 1 child
      if (node == this->Root) {
        // node is root, assign the value of u to v, and delete u
        node->value = u->value;
        node->left = node->right = 0;
        delete u;
      }
      else {
        // Detach v from tree and move u up
        entry_t* parent = node->parent;
        if (parent->left == node) {
          parent->left = u;
        }
        else {
          parent->right = u;
        }
        node->left = node->right = 0;
        delete node;
        u->parent = parent;
        if (uvBlack) {
          // u and v both black, fix double black at u
          fix_bb(u);
        }
        else {
          // u or v red, color u black
          u->color = 'B';
        }
      }
      return;
    }

    // node has 2 children, swap values with successor and recurse
    olx_swap(u->value, node->value);
    delete_node(u);
  }

  bool insert_value(const value_t& v) {
    entry_t* nn = new entry_t(v);
    if (this->Root == 0) {
      nn->color = 'B';
      this->Root = nn;
      this->_Count = this->_LeafCount = 1;
      return true;
    }
    olx_pair_t<entry_t*, int> tmp = parent_t::find_node(v.key);

    if (tmp.b == 0) {
      if (tmp.a->AddSame(v.get_value())) {
        this->_Count++;
        return true;
      }
      return false;
    }
    nn->parent = tmp.a;
    if (tmp.b < 0) {
      tmp.a->left = nn;
    }
    else {
      tmp.a->right = nn;
    }
    fix_rr(nn);
    this->_Count++;
    this->_LeafCount++;
    return true;
  }

  virtual bool insert_(const value_t& e) {
    return this->insert_value(e);
  }

  virtual bool delete_(const key_t& key) {
    olx_pair_t<entry_t*, int> n = parent_t::find_node(key);
    if (n.b != 0) {
      return false;
    }
    this->_Count -= n.a->Count();
    this->_LeafCount -= 1;
    delete_node(n.a);
    return true;
  }
  private:
    void rt_l(entry_t* node) {
      if (node == this->Root) {
        this->Root = node->right;
      }
      node->rotate_left();
    }
    void rt_r(entry_t* node) {
      if (node == this->Root) {
        this->Root = node->left;
      }
      node->rotate_right();
    }


    void fix_rr(entry_t* node) {
      if (node == this->Root) {
        node->color = 'B';
        return;
      }
      if (node->parent->color != 'B') {
        entry_t* uncle = node->uncle(),
          * parent = node->parent,
          * grandp = parent->parent;
        if (uncle != 0 && uncle->color == 'R') {
          // uncle red, perform recoloring and recurse
          parent->color = 'B';
          uncle->color = 'B';
          grandp->color = 'R';
          fix_rr(grandp);
        }
        else {
          // Else perform LR, LL, RL, RR
          if (parent->is_left()) {
            if (node->is_left()) {
              // for left right
              olx_swap(parent->color, grandp->color);
            }
            else {
              this->rt_l(parent);
              olx_swap(node->color, grandp->color);
            }
            // for left left and left right
            this->rt_r(grandp);
          }
          else {
            if (node->is_left()) {
              // for right left
              this->rt_r(parent);
              olx_swap(node->color, grandp->color);
            }
            else {
              olx_swap(parent->color, grandp->color);
            }
            // for right right and right left
            this->rt_l(grandp);
          }
        }
      }
    }

    void fix_bb(entry_t* node) {
      if (node == this->Root) {
        return;
      }

      entry_t* sibling = node->sibling(), *parent = node->parent;
      if (sibling == 0) {
        // No sibling, double black pushed up
        fix_bb(parent);
      }
      else {
        if (sibling->color == 'R') {
          // Sibling red
          parent->color = 'R';
          sibling->color = 'B';
          if (sibling->is_left()) {
            // left case
            this->rt_r(parent);
          }
          else {
            // right case
            this->rt_l(parent);
          }
          this->fix_bb(node);
        }
        else {
          // Sibling black
          if (sibling->has_reds()) {
            // at least 1 red children
            if (sibling->left != 0 && sibling->left->color == 'R') {
              if (sibling->is_left()) {
                // left left
                sibling->left->color = sibling->color;
                sibling->color = parent->color;
                this->rt_r(parent);
              }
              else {
                // right left
                sibling->left->color = parent->color;
                this->rt_r(sibling);
                this->rt_l(parent);
              }
            }
            else {
              if (sibling->is_left()) {
                // left right
                sibling->right->color = parent->color;
                this->rt_l(sibling);
                this->rt_r(parent);
              }
              else {
                // right right
                sibling->right->color = sibling->color;
                sibling->color = parent->color;
                this->rt_l(parent);
              }
            }
            parent->color = 'B';
          }
          else {
            // 2 black children
            sibling->color = 'R';
            if (parent->color == 'B') {
              this->fix_bb(parent);
            }
            else {
              parent->color = 'B';
            }
          }
        }
      }
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
