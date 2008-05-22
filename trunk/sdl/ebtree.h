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
#ifndef __BORLANDC__
  template <typename C, typename O>
TBTreeTraverser< BTree<C,O> > BTree<C,O>::Traverser;
#endif

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
    YEntry* insertPX = NULL;
    XEntry* insertPY = NULL;
    XTree** rx = Tree.Find(x, &insertPX);
    if( rx == NULL )  rx = &Tree.Add(x, new XTree(), insertPX );
    O* ry = (*rx)->Find(y, &insertPY);
    if( ry == NULL )  ry = &(*rx)->Add(y, val, insertPY);
    _Count++;
    return *ry;
  }
  O* Find(const C& x, const C& y) {
    XTree** rx = Tree.Find(x);
    return (rx == NULL) ? NULL : (*rx)->Find(y);
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
  public:
  template <class Trav>
    bool Traverse(const BTree2<C,O>& tree, Trav& traverser)  {
#ifdef __BORLANDC__
       return true;
#else
      InternalTraverser<Trav> t(traverser);
      return tree.Tree.Traverser.Traverse(tree.Tree, t);
#endif
    }
  };
  static Tree2Traverser Traverser;
};

#ifndef __BORLANDC__
template <typename C, typename O>
  typename BTree2<C,O>::Tree2Traverser BTree2<C,O>::Traverser;
#endif

EndEsdlNamespace()
#endif
