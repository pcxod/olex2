/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __OLX__BTREE
#define __OLX__BTREE
#include "etraverse.h"
BeginEsdlNamespace()

template <typename C, typename O>  class BTree  {
public:
  struct Entry  {
    C key;
    O val;
    Entry* next, *last;  // linked list of items with equal key
    Entry* left, *right;
    Entry()  {
      next = last = left = right = NULL;
    }
    Entry(const C& key, const O& val)  {
      this->key = key;
      this->val = val;
      next = last = left = right = NULL;
    }
    ~Entry()  {  Clear();  }
    inline void Clear()  {
      if( next )  delete next;
      next = NULL;
      if( right )  {
        right->Clear();
        delete right;
        right = NULL;
      }
      if( left )  {
        left->Clear();
        delete left;
        left = NULL;
      }
    }
  };
protected:
  Entry Root;
  int _Count;
public:
  BTree() : _Count(0)  {  }

  ~BTree()  {  Root.Clear();  }

  void Clear()  {  Root.Clear();  _Count = 0;  }

  inline const Entry* GetRoot() const {  return (_Count == 0) ? NULL : &Root;  }
  inline int Count()            const {  return _Count;  }

  O& Add(const C& key, const O& val, Entry* insertPoint=NULL)  {
    Entry* rv = NULL;
    if( _Count == 0 )  {
      Root.key = key;
      Root.val = val;
      rv = &Root;
    }
    else  {
      Entry* en = (insertPoint == NULL) ? &Root : insertPoint;
      while( en->key < key )  {
        if( en->right && en->right->key <= key )
          en = en->right;
        else
          break;
      }
      while( en->key > key )  {
        if( en->left && en->left->key >= key )
          en = en->left;
        else
          break;
      }
      if( en->key == key )  {
        if( en->last )  {
          rv = en->last->next = new Entry(key, val);
          en->last = en->last->next;
        }
        else
          rv = en->last = en->next = new Entry(key, val);
      }
      else if( en->key < key )
        rv = en->right = new Entry(key, val);
      else if( en->key > key )
        rv = en->left = new Entry(key, val);
    }
    _Count++;
    return rv->val;
  }
  /* find an entry by the key, if entry is not found and closest is provided,
  closest is initlised with the entry to accept the key for the next add operation */
  O* Find(const C& key, Entry** insertPoint=NULL)  {
    if( _Count == 0 )  {  
      if( insertPoint ) *insertPoint = &Root;
      return NULL;
    }
    Entry* en = &Root;
    while( en->key < key )  {
      if( en->right && en->right->key <= key )
        en = en->right;
      else
        break;
    }
    while( en->key > key )  {
      if( en->left && en->left->key >= key )
        en = en->left;
      else
        break;
    }
    if( insertPoint != NULL )  *insertPoint = en;
    return (en->key == key) ? &en->val : NULL;
  }

  static TBTreeTraverser< BTree<C,O> > Traverser;
};

template <typename C, typename O>
  TBTreeTraverser< BTree<C,O> > BTree<C,O>::Traverser;

template <typename C, typename O>  class BTree2 {
public:
  typedef BTree<C, O> XTree;
  typedef BTree<C,XTree*> YTree;
  typedef typename YTree::Entry YEntry;
  typedef typename XTree::Entry XEntry;
private:
  int _Count;
protected:
  YTree Tree;
public:
  BTree2() : _Count(0) {  }
  ~BTree2()  {  Clear();  }
//___________________________________________________________________
  bool OnItem(const YEntry* item)  {
    delete const_cast<YEntry*>(item)->val;
    return true;
  }
//___________________________________________________________________
  void Clear()  {
    Tree.Traverser.Traverse(Tree, *this);
    Tree.Clear();
    _Count = 0;
  }
  
  inline const YEntry* GetRoot() const {  return Tree.GetRoot();  }
  inline int Count()            const {  return _Count;  }

  O& Add(const C& x, const C& y, const O& val)  {
    YEntry* insertPY = NULL;
    XEntry* insertPX = NULL;
    XTree** rx = Tree.Find(y, &insertPY);
    if( rx == NULL )  rx = &Tree.Add(y, new XTree(), insertPY );
    O* ry = (*rx)->Find(x, &insertPX);
    if( ry == NULL )  ry = &(*rx)->Add(x, val, insertPX);
    _Count++;
    return *ry;
  }
  O* Find(const C& x, const C& y) {
    XTree** rx = Tree.Find(y);
    return (rx == NULL) ? NULL : (*rx)->Find(x);
  }
  class Tree2Traverser {
    template <class Traverser> class InternalTraverser  {
      Traverser& trav;
    public:
      InternalTraverser(Traverser& t) : trav(t)  {}
      bool OnItem(const YEntry* item)  {
        return item->val->Traverser.Traverse(*item->val, trav);
      }
    };
    template <class Traverser> class InternalFullTraverser  {
      Traverser& trav;
    public:
      InternalFullTraverser(Traverser& t) : trav(t)  {}
      bool OnItem(const YEntry* item)  {
        trav.SetY( item->key );
        return item->val->Traverser.Traverse(*item->val, trav);
      }
    };
  public:
  template <class Trav>
    bool Traverse(const BTree2<C,O>& tree, Trav& traverser)  {
      InternalTraverser<Trav> t(traverser);
      return tree.Tree.Traverser.Traverse(tree.Tree, t);
    }
  template <class Trav>
    bool FullTraverse(const BTree2<C,O>& tree, Trav& traverser)  {
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
  typedef BTree<C,XYTree*> ZTree;
  typedef typename XYTree::YEntry XYEntry;
  typedef typename ZTree::Entry ZEntry;
private:
  int _Count;
protected:
  ZTree Tree;
public:
  BTree3() : _Count(0) {  }
  ~BTree3()  {  Clear();  }
//___________________________________________________________________
  bool OnItem(const ZEntry* item)  {
    delete const_cast<ZEntry*>(item)->val;
    return true;
  }
//___________________________________________________________________
  void Clear()  {
    Tree.Traverser.Traverse(Tree, *this);
    Tree.Clear();
    _Count = 0;
  }
  
  inline const ZEntry* GetRoot() const {  return Tree.GetRoot();  }
  inline int Count()            const {  return _Count;  }

  O& Add(const C& x, const C& y, const C& z, const O& val)  {
    ZEntry* insertPZ = NULL;
    XYTree** xy = Tree.Find(z, &insertPZ);
    if( xy == NULL )  xy = &Tree.Add(z, new XYTree(), insertPZ );
    _Count++;
    return (*xy)->Add(x, y, val);
  }
  O* Find(const C& x, const C& y, const C& z) {
    XYTree** xy = Tree.Find(z);
    return (xy == NULL) ? NULL : (*xy)->Find(x, y);
  }
  class Tree3Traverser {
    template <class Traverser> class InternalTraverser  {
      Traverser& trav;
    public:
      InternalTraverser(Traverser& t) : trav(t)  {}
      bool OnItem(const ZEntry* item)  {
        return item->val->Traverser.Traverse(*item->val, trav);
      }
    };
    template <class Traverser> class InternalFullTraverser  {
      Traverser& trav;
    public:
      InternalFullTraverser(Traverser& t) : trav(t)  {}
      bool OnItem(const ZEntry* item)  {
        trav.SetZ(item->key);
        return item->val->Traverser.FullTraverse(*item->val, trav);
      }
    };
  public:
  template <class Trav>
    bool Traverse(const BTree3<C,O>& tree, Trav& traverser)  {
      InternalTraverser<Trav> t(traverser);
      return tree.Tree.Traverser.Traverse(tree.Tree, t);
    }
  template <class Trav>
    bool FullTraverse(const BTree3<C,O>& tree, Trav& traverser)  {
      InternalFullTraverser<Trav> t(traverser);
      return tree.Tree.Traverser.Traverse(tree.Tree, t);
    }
  };
  static Tree3Traverser Traverser;
};

template <typename C, typename O>
  typename BTree3<C,O>::Tree3Traverser BTree3<C,O>::Traverser;

EndEsdlNamespace()
#endif
