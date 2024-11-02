/******************************************************************************
* Copyright (c) 2004-2024 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_collectionItem_H
#define __olx_sdl_collectionItem_H
/* Check tagstack.h when performing embedded operations on tags */

enum {
  OVERLAP_NONE = 0,
  OVERLAP_OVERLAP = 1,
  OVERLAP_SUBGROUP = 3,
  OVERLAP_SUPERGROUP = 5,
  // this does not take order into account!
  OVERLAP_SAME = OVERLAP_SUBGROUP | OVERLAP_SUPERGROUP
};

// implements data for collection item
class ACollectionItem : public virtual IOlxObject {
  index_t CollectionItemTag;
public:
  ACollectionItem() : CollectionItemTag(-1) {}
  virtual ~ACollectionItem()  {}
  index_t GetTag() const {  return CollectionItemTag;  }
  template <class item_t>
  static index_t GetTag(const item_t& i) { return olx_ref::get(i).GetTag(); }
  template <class item_t>
  static void SetTag(item_t& i, index_t t) { olx_ref::get(i).SetTag(t); }
  void SetTag(index_t v) { CollectionItemTag = v;  }
  index_t IncTag()  {  return ++CollectionItemTag;  }
  index_t DecTag()  {  return --CollectionItemTag;  }
  // for extended functionality of containers
  template <class Accessor> struct TagAnalyser_  {
    const Accessor &accessor;
    const index_t ref_tag;
    TagAnalyser_(const Accessor &accessor_, index_t _ref_tag)
      : accessor(accessor_), ref_tag(_ref_tag) {}
    template <class Item>
    bool OnItem(const Item& o, size_t) const {
      return olx_ref::get(accessor(o)).GetTag() == ref_tag;
    }
  };
  template <class Accessor>
  static TagAnalyser_<Accessor>
  TagAnalyser(const Accessor &acc, index_t ref_tag) {
    return TagAnalyser_<Accessor>(acc, ref_tag);
  }
  static TagAnalyser_<DummyAccessor>
  TagAnalyser(index_t ref_tag) {
    return TagAnalyser(DummyAccessor(), ref_tag);
  }

  template <class Accessor> struct IndexTagAnalyser_ {
    const Accessor &accessor;
    IndexTagAnalyser_(const Accessor &accessor_)
      : accessor(accessor_) {}
    template <class Item>
    bool OnItem(const Item& o, size_t i) const {
      return (size_t)GetTag(accessor(o)) == i;
    }
  };
  template <class Accessor>
  static IndexTagAnalyser_<Accessor>
  IndexTagAnalyser(const Accessor &acc) {
    return IndexTagAnalyser_<Accessor>(acc);
  }
  static IndexTagAnalyser_<DummyAccessor>
  IndexTagAnalyser() { return IndexTagAnalyser(DummyAccessor()); }

  template <class Accessor> struct TagSetter_ {
    const Accessor &accessor;
    const index_t ref_tag;
    TagSetter_(const Accessor &accessor_, index_t _ref_tag)
      : accessor(accessor_), ref_tag(_ref_tag)  {}
    template <class Item>
    void OnItem(Item& o, size_t) const { SetTag(accessor(o), ref_tag); }
  };
  template <class Accessor>
  static TagSetter_<Accessor>
  TagSetter(const Accessor &acc, index_t tag) {
    return TagSetter_<Accessor>(acc, tag);
  }
  static TagSetter_<DummyAccessor>
  TagSetter(index_t tag) { return TagSetter(DummyAccessor(), tag); }

  template <class Accessor> struct IndexTagSetter_ {
    const Accessor &accessor;
    IndexTagSetter_(const Accessor &accessor_)
      : accessor(accessor_) {}
    template <class Item>
    void OnItem(Item& o, size_t i) const { SetTag(accessor(o), i); }
  };
  template <class Accessor>
  static IndexTagSetter_<Accessor>
  IndexTagSetter(const Accessor &acc) {
    return IndexTagSetter_<Accessor>(acc);
  }
  static IndexTagSetter_<DummyAccessor>
  IndexTagSetter() { return IndexTagSetter(DummyAccessor()); }

  template <class Accessor> struct TagAccessor_ {
    const Accessor &accessor;
    TagAccessor_(const Accessor &accessor_)
      : accessor(accessor_) {}
    template <class Item>
    index_t operator() (const Item& o) const { return GetTag(accessor(o)); }
  };
  template <class Accessor>
  static TagAccessor_<Accessor>
  TagAccessor(const Accessor &acc) { return TagAccessor_<Accessor>(acc); }
  static TagAccessor_<DummyAccessor>
  TagAccessor() { return TagAccessor(DummyAccessor()); }

  template <class Accessor> struct TagComparator_ {
    const Accessor &accessor;
    TagComparator_(const Accessor &accessor_)
      : accessor(accessor_) {}
    template <class Item>
    int Compare(const Item& o1, const Item& o2) const {
      return olx_cmp(GetTag(accessor(o1)), GetTag(accessor(o2)));
    }
  };
  template <class Accessor>
  static TagComparator_<Accessor>
  TagComparator(const Accessor &acc) { return TagComparator_<Accessor>(acc); }
  static TagComparator_<DummyAccessor>
  TagComparator() { return TagComparator(DummyAccessor()); }

  // common algorithms
  // creates a list of unique items
  template <class List, class Accessor>
  static List& Unify(List& list, const Accessor &acc)  {
    list.ForEach(IndexTagSetter(acc));
    return list.Pack(olx_alg::olx_not(IndexTagAnalyser(acc)));
  }
  template <class List>
  static List& Unify(List& list)  { return Unify(list, DummyAccessor()); }
  template <class List, class Accessor>
  static typename List::const_list_type Unique(const List &list,
    const Accessor &acc)
  {
    list.ForEach(IndexTagSetter(acc));
    return list.Filter(IndexTagAnalyser(acc));
  }
  template <class List>
  static typename List::const_list_type Unique(const List &list) {
    return Unique(list, DummyAccessor());
  }
  // subtracts a set of items from a list of items
  template <class ListA, class AccessorA, class ListB, class AccessorB>
  static ListA& Subtract(
    ListA& from, const AccessorA &aa,
    const ListB& set, const AccessorB &ab)
  {
    from.ForEach(TagSetter(aa, 0));
    set.ForEach(TagSetter(ab, 1));
    from.Pack(TagAnalyser(aa, 1));
    return from;
  }
  template <class ListA, class ListB>
  static ListA& Exclude(ListA& from, const ListB& set) {
    return Subtract(from, DummyAccessor(), set, DummyAccessor());
  }
  // synonym for Exclude
  template <class ListA, class ListB, class Accessor>
  static ListA& Subtract(ListA& from, const ListB& set, const Accessor& acc) {
    return Subtract(from, acc, set, acc);
  }
  template <class ListA, class ListB>
  static ListA& Subtract(ListA& from, const ListB& set) {
    return Subtract(from, DummyAccessor(), set, DummyAccessor());
  }

  // A wil have the common subset 
  template <class ListA, class AccessorA, class ListB, class AccessorB>
  static ListA& Intersect(
    ListA& A, const AccessorA& aa,
    const ListB& B, const AccessorB& ab)
  {
    A.ForEach(TagSetter(aa, 0));
    B.ForEach(TagSetter(ab, 1));
    A.Pack(TagAnalyser(aa, 0));
    return A;
  }
  template <class ListA, class ListB, class Accessor>
  static ListA& Intersect(ListA& A, const ListB& B, const Accessor& acc) {
    return Intersect(A, acc, B, acc);
  }
  template <class ListA, class ListB>
  static ListA& Intersect(ListA& A, const ListB& B) {
    return Intersect(A, DummyAccessor(), B, DummyAccessor());
  }

  /* OVERLAP_SAME returned independent of the items' order!
  Use IsTheSame if order matters.
  */
  template <class ListA, class AccessorA, class ListB, class AccessorB>
  static int AnalyseOverlap(
    const ListA& A, const AccessorA& aa,
    const ListB& B, const AccessorB& ab)
  {
    A.ForEach(TagSetter(aa, 0));
    B.ForEach(TagSetter(ab, 1));
    bool overlap = false;
    for (size_t i = 0; i < A.Count(); i++) {
      if (aa(A[i]).GetTag() != 0) {
        overlap = true;
        break;
      }
    }
    if (!overlap) {
      return OVERLAP_NONE;
    }
    int rv = OVERLAP_OVERLAP;
    bool subgroup = true;
    for (size_t i = 0; i < A.Count(); i++) {
      if (aa(A[i]).GetTag() != 1) {
        subgroup = false;
        break;
      }
    }
    if (subgroup) {
      rv |= OVERLAP_SUBGROUP;
    }
    bool supergroup = true;
    A.ForEach(TagSetter(aa, 0));

    for (size_t i = 0; i < B.Count(); i++) {
      if (ab(B[i]).GetTag() != 0) {
        supergroup = false;
        break;
      }
    }
    if (supergroup) {
      rv |= OVERLAP_SUPERGROUP;
    }
    return rv;
  }
  template <class ListA, class ListB, class Accessor>
  static int AnalyseOverlap(const ListA& A, const ListB& B, const Accessor& acc) {
    return AnalyseOverlap(A, acc, B, acc);
  }
  template <class ListA, class ListB>
  static int AnalyseOverlap(const ListA& A, const ListB& B) {
    return AnalyseOverlap(A, DummyAccessor(), B, DummyAccessor());
  }
  
  /* Could be used to check if the lists contain items in the same order.
  Checking size allows to check the list contains identical items in the same order.
  */
  template <class ListA, class AccessorA, class ListB, class AccessorB>
  static bool IsTheSameOrder(
    const ListA& A, const AccessorA& aa,
    const ListB& B, const AccessorB& ab, bool check_size=false)
  {
    if (check_size && A.Count() != B.Count()) {
      return false;
    }
    A.ForEach(TagSetter(aa, 0));
    B.ForEach(IndexTagSetter(ab));
    for (size_t i = 0; i < A.Count(); i++) {
      if (aa(A[i]).GetTag() != i) {
        return false;
      }
    }
    return false;
  }
  template <class ListA, class ListB, class Accessor>
  static int IsTheSameOrder(const ListA& A, const ListB& B, const Accessor& acc,
    bool check_size=false)
  {
    return IsTheSameOrder(A, acc, B, acc, check_size);
  }
  template <class ListA, class ListB>
  static int IsTheSameOrder(const ListA& A, const ListB& B, bool check_size) {
    return IsTheSameOrder(A, DummyAccessor(), B, DummyAccessor(), check_size);
  }
  template <class ListA, class AccessorA, class ListB, class AccessorB>
  static bool IsTheSame(
    const ListA& A, const AccessorA& aa,
    const ListB& B, const AccessorB& ab)
  {
    return IsTheSameOrder(A, aa, B, ab, true);
  }
  template <class ListA, class ListB, class Accessor>
  static int IsTheSame(const ListA& A, const ListB& B, const Accessor& acc)
  {
    return IsTheSameOrder(A, acc, B, acc, true);
  }
  template <class ListA, class ListB>
  static int IsTheSame(const ListA& A, const ListB& B) {
    return IsTheSameOrder(A, DummyAccessor(), B, DummyAccessor(), true);
  }
};
#endif
