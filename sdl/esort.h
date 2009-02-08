#ifndef eSortH
#define eSortH
#include "ebase.h"

BeginEsdlNamespace()
/* a comparator for primitive types, or object having < and > operators only
   the comparison might call both operators, so use the TComparableComparator
   for whose objects which implement Compare function to improve the speed

   Note that a pointer lists should be treated differently - as the Item(i)
   method returns a reference to the location and considering the implementation
   of QS that location will get changed as Swap operations used!
*/

class TPrimitiveComparator  {
public:
  template <class Comparable>
  static inline int Compare(const Comparable& A, const Comparable& B )  {
    if( A < B )  return -1;
    if( A > B )  return 1;
    return 0;
  }
  template <class Comparable>
  static inline int Compare(const Comparable* A, const Comparable* B )  {
    if( *A < *B )  return -1;
    if( *A > *B )  return 1;
    return 0;
  }
};

class TPointerComparator  {
public:
  template <class Comparable>
  static inline int Compare(const Comparable& A, const Comparable& B )  {
    if( &A < &B )  return -1;
    if( &A > &B )  return 1;
    return 0;
  }
  template <class Comparable>
  static inline int Compare(const Comparable* A, const Comparable* B )  {
    if( A < B )  return -1;
    if( A > B )  return 1;
    return 0;
  }
};

template <class Base, typename ItemClass> struct Sort_ConstMemberFunctionWrapper  {
  Base& Instance;
  int (Base::*Func)(ItemClass a, ItemClass b) const;
  Sort_ConstMemberFunctionWrapper(Base& instance, int (Base::*func)(ItemClass a, ItemClass b) const) :
  Instance(instance), Func(func) {  }
  inline int Compare(ItemClass a, ItemClass b) const {
    return (Instance.*Func)(a, b);
  }
};

template <class Base, typename ItemClass> struct Sort_MemberFunctionWrapper  {
  Base& Instance;
  int (Base::*Func)(ItemClass a, ItemClass b);
  Sort_MemberFunctionWrapper(Base& instance, int (Base::*func)(ItemClass a, ItemClass b)) :
  Instance(instance), Func(func) {  }
  inline int Compare(ItemClass a, ItemClass b) const {
    return (Instance.*Func)(a, b);
  }
};

template <class Comparator, class ItemClass> struct Sort_ComparatorWrapper  {
  inline int Compare(ItemClass a, ItemClass b) const {
    return Comparator::Compare(a, b);
  }
};

template <typename ItemClass> struct Sort_StaticFunctionWrapper  {
  int (*Func)(ItemClass a, ItemClass b);
  Sort_StaticFunctionWrapper(int (*func)(ItemClass a, ItemClass b)) :
  Func(func) {  }
  inline int Compare(ItemClass a, ItemClass b) const {
    return (*Func)(a, b);
  }
};

class TComparableComparator  {
public:
  template <class Comparable>
  static inline int Compare(const Comparable& A, const Comparable& B )  {
    return A.Compare(B);
  }
  template <class Comparable>
  static inline int Compare(const Comparable* A, const Comparable* B )  {
    return A->Compare(*B);
  }
};
//.........................................................................................................................
template <class ListClass, class ItemClass >  class TQuickPtrSorter  {
  // static comparator sort
  template <class Comparator>
    static void QuickSort(const Comparator& Cmp, int lo0, int hi0, ListClass& list)  {
      int lo = lo0;
      int hi = hi0;
      if( hi0 > lo0)  {
        ItemClass* mid = list.Item( ( lo0 + hi0 ) / 2 );
        while( lo <= hi )  {
          while( ( lo < hi0 ) && ( Cmp.Compare(list.Item(lo), mid) < 0) )   lo++;
          while( ( hi > lo0 ) && ( Cmp.Compare(list.Item(hi), mid) > 0) ) hi--;
          if( lo <= hi )  {
            list.Swap(lo, hi);
            lo++;
            hi--;
          }
        }
        if( lo0 < hi )  QuickSort(Cmp, lo0, hi, list);
        if( lo < hi0 )  QuickSort(Cmp, lo, hi0, list);
      }
    }
    //synchronious sort
  template <class Comparator, class ListClassA>
    static void QuickSort(const Comparator& Cmp, int lo0, int hi0, ListClass& list, ListClassA& list1)  {
      int lo = lo0;
      int hi = hi0;
      if( hi0 > lo0)  {
        ItemClass* mid = list.Item( ( lo0 + hi0 ) / 2 );
        while( lo <= hi )  {
          while( ( lo < hi0 ) && ( Cmp.Compare(list.Item(lo), mid) < 0) )   lo++;
          while( ( hi > lo0 ) && ( Cmp.Compare(list.Item(hi), mid) > 0) ) hi--;
          if( lo <= hi )  {
            list.Swap(lo, hi);
            list1.Swap(lo, hi);
            lo++;
            hi--;
          }
        }
        if( lo0 < hi )  QuickSort(Cmp, lo0, hi, list, list1);
        if( lo < hi0 )  QuickSort(Cmp, lo, hi0, list, list1);
      }
    }
public:
  template <class Comparator>
    static void Sort(ListClass& list)  {
      Sort_ComparatorWrapper<Comparator, const ItemClass*> cmp;
      QuickSort(cmp, 0, list.Count()-1, list);
    }
  template <class Comparator, class ListClassA>
    static void SyncSort(ListClass& list, ListClassA& list1)  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      Sort_ComparatorWrapper<Comparator, const ItemClass*> cmp;
      QuickSort(cmp, 0, list.Count()-1, list, list1);
    }
  static void SortSF(ListClass& list, int (*f)(const ItemClass* a, const ItemClass* b) )  {
    Sort_StaticFunctionWrapper<const ItemClass*> cmp(f);  
    QuickSort(cmp, 0, list.Count()-1, list);
  }
  template <class ListClassA>
    static void SyncSortSF(ListClass& list, ListClassA& list1, int (*f)(const ItemClass* a, const ItemClass* b) )  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      Sort_StaticFunctionWrapper<const ItemClass*> cmp(f);  
      QuickSort(cmp, 0, list.Count()-1, list, list1);
    }
  template <class BaseClass>
    static void SortMF(ListClass& list, BaseClass& baseClassInstance,
                       int (BaseClass::*f)(const ItemClass* a, const ItemClass* b) )  {
      Sort_MemberFunctionWrapper<BaseClass, const ItemClass*> cmp(baseClassInstance, f);
      QuickSort(cmp, 0, list.Count()-1, list);
    }
  template <class BaseClass>
    static void SortMF(ListClass& list, BaseClass& baseClassInstance,
                       int (BaseClass::*f)(const ItemClass* a, const ItemClass* b) const )  {
      Sort_ConstMemberFunctionWrapper<BaseClass, const ItemClass*> cmp(baseClassInstance, f);
      QuickSort(cmp, 0, list.Count()-1, list);
    }
  template <class BaseClass, class ListClassA>
    static void SortMF(ListClass& list, ListClassA& list1, BaseClass& baseClassInstance,
                       int (BaseClass::*f)(const ItemClass* a, const ItemClass* b) )  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      Sort_MemberFunctionWrapper<BaseClass, const ItemClass*> cmp(baseClassInstance, f);
      QuickSort(cmp, 0, list.Count()-1, list, list1);
    }
  template <class BaseClass, class ListClassA>
    static void SortMF(ListClass& list, ListClassA& list1, BaseClass& baseClassInstance,
                       int (BaseClass::*f)(const ItemClass* a, const ItemClass* b) const )  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      Sort_ConstMemberFunctionWrapper<BaseClass, const ItemClass*> cmp(baseClassInstance, f);
      QuickSort(cmp, 0, list.Count()-1, list, list1);
    }
};
//.........................................................................................................................
template <class ListClass, class ItemClass >  class TBubblePtrSorter  {
  // static comparator sort
  template <class Comparator>
    static void BubbleSort(const Comparator& Cmp, ListClass& list)  {
      bool changes = true;
      const int lc = list.Count();
      while( changes )  {
        changes = false;
        for( int i=1; i < lc; i++ )  {
          if( Cmp.Compare(list.Item(i-1), list.Item(i) ) > 0 )  {
            list.Swap( i-1, i);
            changes = true;
          }
        }
      }
    }
  template <class Comparator, class ListClassA>
    static void BubbleSortSC(const Comparator& Cmp, ListClass& list, ListClassA& list1)  {
      bool changes = true;
      const int lc = list.Count();
      while( changes )  {
        changes = false;
        for( int i=1; i < lc; i++ )  {
          if( Cmp.Compare( list.Item(i-1), list.Item(i) ) > 0 )  {
            list.Swap( i-1, i );
            list1.Swap( i-1, i );
            changes = true;
          }
        }
      }
    }
public:
  template <class Comparator>
    static void Sort(ListClass& list)  {
      Sort_ComparatorWrapper<Comparator, const ItemClass*> cmp;
      BubbleSort(cmp, list);
    }
  template <class Comparator, class ListClassA>
    static void SynchSort(ListClass& list, ListClassA& list1)  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      Sort_ComparatorWrapper<Comparator, const ItemClass*> cmp;
      BubbleSort(cmp, list, list1);
    }
  static void SortSF(ListClass& list, int (*f)(const ItemClass* a, const ItemClass* b) )  {
    Sort_StaticFunctionWrapper<const ItemClass*> cmp(f);  
    BubbleSort(cmp, list);
  }
  template <class ListClassA>
    static void SyncSortSF(ListClass& list, ListClassA& list1, int (*f)(const ItemClass* a, const ItemClass* b) )  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      Sort_StaticFunctionWrapper<const ItemClass*> cmp(f);  
      BubbleSort(cmp, list, list1);
    }
  template <class BaseClass>
    static void SortMF(ListClass& list, BaseClass& baseClassInstance,
                       int (BaseClass::*f)(const ItemClass* a, const ItemClass* b) )  {
      Sort_MemberFunctionWrapper<BaseClass, const ItemClass*> cmp(baseClassInstance, f);
      BubbleSort(cmp, list);
    }
  template <class BaseClass>
    static void SortMF(ListClass& list, BaseClass& baseClassInstance,
                       int (BaseClass::*f)(const ItemClass* a, const ItemClass* b) const )  {
      Sort_ConstMemberFunctionWrapper<BaseClass, const ItemClass*> cmp(baseClassInstance, f);
      BubbleSort(cmp, list);
    }
  template <class BaseClass, class ListClassA>
    static void SynchSortMF(ListClass& list, ListClassA& list1, BaseClass& baseClassInstance,
                       int (BaseClass::*f)(const ItemClass* a, const ItemClass* b) )  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      Sort_MemberFunctionWrapper<BaseClass, const ItemClass*> cmp(baseClassInstance, f);
      BubbleSort(cmp, list, list1);
    }
};
//.........................................................................................................................
template <class ListClass, class ItemClass > class TQuickSorter  {
  // static comparator sort
  template <class Comparator>
    static void QuickSort(const Comparator& Cmp, int lo0, int hi0, ListClass& list)  {
      int lo = lo0;
      int hi = hi0;
      if( hi0 > lo0)  {
        const ItemClass& mid = list.Item( ( lo0 + hi0 ) / 2 );
        while( lo <= hi )  {
          while( ( lo < hi0 ) && ( Cmp.Compare(list.Item(lo), mid) < 0) )  lo++;
          while( ( hi > lo0 ) && ( Cmp.Compare(list.Item(hi), mid) > 0) )  hi--;
          if( lo <= hi )
            list.Swap(lo++, hi--);
        }
        if( lo0 < hi )  QuickSort(Cmp, lo0, hi, list);
        if( lo < hi0 )  QuickSort(Cmp, lo, hi0, list);
      }
    }
  template <class Comparator, class ListClassA>
    static void QuickSort(const Comparator& Cmp, int lo0, int hi0, ListClass& list, ListClassA& list1)  {
      int lo = lo0;
      int hi = hi0;
      if( hi0 > lo0)  {
        const ItemClass& mid = list.Item( ( lo0 + hi0 ) / 2 );
        while( lo <= hi )  {
          while( ( lo < hi0 ) && ( Cmp.Compare(list.Item(lo), mid) < 0) )  lo++;
          while( ( hi > lo0 ) && ( Cmp.Compare(list.Item(hi), mid) > 0) )  hi--;
          if( lo <= hi )  {
            list.Swap(lo, hi);
            list1.Swap(lo++, hi--);
          }
        }
        if( lo0 < hi )  QuickSort(Cmp, lo0, hi, list, list1);
        if( lo < hi0 )  QuickSort(Cmp, lo, hi0, list, list1);
      }
    }
public:
  template <class Comparator>
    static void Sort(ListClass& list)  {
      Sort_ComparatorWrapper<Comparator, const ItemClass&> cmp;
      QuickSort(cmp, 0, list.Count()-1, list);
    }
  template <class Comparator, class ListClassA>
    static void SyncSort(ListClass& list, ListClassA& list1)  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      Sort_ComparatorWrapper<Comparator, const ItemClass&> cmp;
      QuickSort(cmp, 0, list.Count()-1, list, list1);
    }
  static void SortSF(ListClass& list, int (*f)(const ItemClass& a, const ItemClass& b) )  {
    Sort_StaticFunctionWrapper<const ItemClass&> cmp(f);  
    QuickSort(cmp, 0, list.Count()-1, list);
  }
  template <class ListClassA>
    static void SyncSortSF(ListClass& list, ListClassA& list1, int (*f)(const ItemClass& a, const ItemClass& b) )  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      Sort_StaticFunctionWrapper<const ItemClass&> cmp(f);  
      QuickSort(cmp, 0, list.Count()-1, list, list1);
    }
  template <class BaseClass>
    static void SortMF(ListClass& list, BaseClass& baseClassInstance,
                       int (BaseClass::*f)(const ItemClass& a, const ItemClass& b) const )  {
      Sort_ConstMemberFunctionWrapper<BaseClass, const ItemClass&> cmp(baseClassInstance, f);
      QuickSort(cmp, 0, list.Count()-1, list);
    }
  template <class BaseClass>
    static void SortMF(ListClass& list, BaseClass& baseClassInstance,
                       int (BaseClass::*f)(const ItemClass& a, const ItemClass& b) )  {
      Sort_MemberFunctionWrapper<BaseClass, const ItemClass&> cmp(baseClassInstance, f);
      QuickSort(cmp, 0, list.Count()-1, list);
    }
  template <class BaseClass, class ListClassA>
    static void SyncSortMF(ListClass& list, ListClassA& list1, BaseClass& baseClassInstance,
                       int (BaseClass::*f)(const ItemClass& a, const ItemClass& b) const )  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      Sort_ConstMemberFunctionWrapper<BaseClass, const ItemClass&> cmp(baseClassInstance, f);
      QuickSort(cmp, 0, list.Count()-1, list, list1);
    }
  template <class BaseClass, class ListClassA>
    static void SyncSortMF(ListClass& list, ListClassA& list1, BaseClass& baseClassInstance,
                       int (BaseClass::*f)(const ItemClass& a, const ItemClass& b) )  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      Sort_MemberFunctionWrapper<BaseClass, const ItemClass&> cmp(baseClassInstance, f);
      QuickSort(cmp, 0, list.Count()-1, list, list1);
    }
};
//.........................................................................................................................
template <class ListClass, class ItemClass > class TBubbleSorter  {
  // static comparator sort
  template <class Comparator>
    static void BubbleSort(const Comparator& Cmp, ListClass& list)  {
      bool changes = true;
      const int lc = list.Count();
      while( changes )  {
        changes = false;
        for( int i=1; i < lc; i++ )  {
          if( Cmp.Compare( list.Item(i-1), list.Item(i) ) > 1 )  {
            list.Swap( i-1, i);
            changes = true;
          }
        }
      }
    }
  template <class Comparator, class ListClassA>
    static void BubbleSort(const Comparator& Cmp, ListClass& list, ListClassA& list1)  {
      bool changes = true;
      while( changes )  {
        changes = false;
        for( int i=1; i < list.Count(); i++ )  {
          if( Cmp.Compare( list.Item(i-1), list.Item(i) ) > 1 )  {
            list.Swap( i-1, i );
            list1.Swap( i-1, i );
            changes = true;
          }
        }
      }
    }
public:
  template <class Comparator>
    static void Sort(ListClass& list)  {
      Sort_ComparatorWrapper<Comparator, const ItemClass&> cmp;
      BubbleSort(cmp, list);
    }
  template <class Comparator, class ListClassA>
    static void SyncSort(ListClass& list, ListClassA& list1)  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      Sort_ComparatorWrapper<Comparator, const ItemClass&> cmp;
      BubbleSort(cmp, list, list1);
    }
  static void SortSF(ListClass& list, int (*f)(const ItemClass& a, const ItemClass& b) )  {
    Sort_StaticFunctionWrapper<const ItemClass&> cmp(f);
    BubbleSort(cmp, list);
  }
  template <class ListClassA>
    static void SyncSortSF(ListClass& list, ListClassA& list1, int (*f)(const ItemClass& a, const ItemClass& b) )  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      Sort_StaticFunctionWrapper<const ItemClass&> cmp(f);
      BubbleSort(cmp, list, list1);
    }
  template <class BaseClass>
    static void SortMF(ListClass& list, BaseClass& baseClassInstance,
                       int (BaseClass::*f)(const ItemClass& a, const ItemClass& b) const )  {
      Sort_ConstMemberFunctionWrapper<BaseClass, const ItemClass&> cmp(baseClassInstance, f);
      BubbleSort(cmp, list);
    }
  template <class BaseClass>
    static void SortMF(ListClass& list, BaseClass& baseClassInstance,
                       int (BaseClass::*f)(const ItemClass& a, const ItemClass& b) )  {
      Sort_MemberFunctionWrapper<BaseClass, const ItemClass&> cmp(baseClassInstance, f);
      BubbleSort(cmp, list);
    }
  template <class BaseClass, class ListClassA>
    static void SyncSortMF(ListClass& list, ListClassA& list1, BaseClass& baseClassInstance,
                       int (BaseClass::*f)(const ItemClass& a, const ItemClass& b) const )  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      Sort_ConstMemberFunctionWrapper<BaseClass, const ItemClass&> cmp(baseClassInstance, f);
      BubbleSort(cmp, list, list1);
    }
  template <class BaseClass, class ListClassA>
    static void SyncSortMF(ListClass& list, ListClassA& list1, BaseClass& baseClassInstance,
                       int (BaseClass::*f)(const ItemClass& a, const ItemClass& b) )  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      Sort_MemberFunctionWrapper<BaseClass, const ItemClass&> cmp(baseClassInstance, f);
      BubbleSort(cmp, list, list1);
    }
};
//.........................................................................................................................
/* and the last one ... for the ArrayList - same issues as for the pointers -
  we need a copy of the object - not a reference to its location!
*/
template <class ListClass, class ItemClass > class TQuickObjectSorter  {
  // static comparator sort
  template <class Comparator>
    static void QuickSort(const Comparator& Cmp, int lo0, int hi0, ListClass& list)  {
      int lo = lo0;
      int hi = hi0;
      if( hi0 > lo0)  {
        ItemClass mid = list.Item( ( lo0 + hi0 ) / 2 );
        while( lo <= hi )  {
          while( ( lo < hi0 ) && ( Cmp.Compare(list.Item(lo), mid) < 0) )  lo++;
          while( ( hi > lo0 ) && ( Cmp.Compare(list.Item(hi), mid) > 0) )  hi--;
          if( lo <= hi )
            list.Swap(lo++, hi--);
        }
        if( lo0 < hi )  QuickSort(Cmp, lo0, hi, list);
        if( lo < hi0 )  QuickSort(Cmp, lo, hi0, list);
      }
    }
  template <class Comparator, class ListClassA>
    static void QuickSort(const Comparator& Cmp, int lo0, int hi0, ListClass& list, ListClassA& list1)  {
      int lo = lo0;
      int hi = hi0;
      if( hi0 > lo0)  {
        ItemClass mid = list.Item( ( lo0 + hi0 ) / 2 );
        while( lo <= hi )  {
          while( ( lo < hi0 ) && ( Cmp.Compare(list.Item(lo), mid) < 0) )  lo++;
          while( ( hi > lo0 ) && ( Cmp.Compare(list.Item(hi), mid) > 0) )  hi--;
          if( lo <= hi )  {
            list.Swap(lo, hi);
            list1.Swap(lo++, hi--);
          }
        }
        if( lo0 < hi )  QuickSort(Cmp, lo0, hi, list, list1);
        if( lo < hi0 )  QuickSort(Cmp, lo, hi0, list, list1);
      }
    }
public:
  template <class Comparator>
    static void Sort(ListClass& list)  {
      Sort_ComparatorWrapper<Comparator, const ItemClass&> cmp;
      QuickSort(cmp, 0, list.Count()-1, list);
    }
  template <class Comparator, class ListClassA>
    static void SyncSort(ListClass& list, ListClassA& list1)  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      Sort_ComparatorWrapper<Comparator, const ItemClass&> cmp;
      QuickSort(cmp, 0, list.Count()-1, list, list1);
    }
  static void SortSF(ListClass& list, int (*f)(const ItemClass& a, const ItemClass& b) )  {
    Sort_StaticFunctionWrapper<const ItemClass&> cmp(f);
    QuickSort(cmp, 0, list.Count()-1, list);
  }
  template <class ListClassA>
    static void SyncSortSF(ListClass& list, ListClassA& list1, int (*f)(const ItemClass& a, const ItemClass& b) )  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      Sort_StaticFunctionWrapper<const ItemClass&> cmp(f);
      QuickSort(cmp, 0, list.Count()-1, list, list1);
    }
  template <class BaseClass>
    static void SortMF(ListClass& list, BaseClass& baseClassInstance,
                       int (BaseClass::*f)(const ItemClass& a, const ItemClass& b) const )  {
      Sort_ConstMemberFunctionWrapper<BaseClass, const ItemClass&> cmp(baseClassInstance, f);
      QuickSort(cmp, 0, list.Count()-1, list);
    }
  template <class BaseClass>
    static void SortMF(ListClass& list, BaseClass& baseClassInstance,
                       int (BaseClass::*f)(const ItemClass& a, const ItemClass& b) )  {
      Sort_MemberFunctionWrapper<BaseClass, const ItemClass&> cmp(baseClassInstance, f);
      QuickSort(cmp, 0, list.Count()-1, list);
    }
  template <class BaseClass, class ListClassA>
    static void SyncSortMF(ListClass& list, ListClassA& list1, BaseClass& baseClassInstance,
                       int (BaseClass::*f)(const ItemClass& a, const ItemClass& b) const )  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      Sort_ConstMemberFunctionWrapper<BaseClass, const ItemClass&> cmp(baseClassInstance, f);
      QuickSort(cmp, 0, list.Count()-1, list, list1);
    }
  template <class BaseClass, class ListClassA>
    static void SyncSortMF(ListClass& list, ListClassA& list1, BaseClass& baseClassInstance,
                       int (BaseClass::*f)(const ItemClass& a, const ItemClass& b) )  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      Sort_MemberFunctionWrapper<BaseClass, const ItemClass&> cmp(baseClassInstance, f);
      QuickSort(cmp, 0, list.Count()-1, list, list1);
    }
};

//examples
/*
class main_TestR  {
public:
  int Sort(const double& v, const double& v1 )   {  return ( v < v1 ) ? -1 : (( v1 > v ) ? 1 : 0);  }
  int Sortp(const double* v, const double* v1 )   {  return (*v < *v1) ? -1 : (( *v1 > *v ) ? 1 : 0);  }
  int PtrSort(const double* v, const double* v1 )   {  return (v < v1) ? -1 : (( v1 > v ) ? 1 : 0);  }
  int CSort(const double& v, const double& v1 ) const  {  return ( v < v1 ) ? -1 : (( v1 > v ) ? 1 : 0);  }
  int CSortp(const double* v, const double* v1 ) const  {  return ( *v < *v1 ) ? -1 : (( *v1 < *v ) ? 1 : 0);  }
  int CPtrSort(const double* v, const double* v1 ) const   {  return (v < v1) ? -1 : (( v1 > v ) ? 1 : 0);  }
  static int SSort(const double& v, const double& v1 )  {  return ( v < v1 ) ? -1 : (( v1 > v ) ? 1 : 0);  }
  static int SSortp(const double* v, const double* v1 )  {  return ( *v < *v1 ) ? -1 : (( *v1 > *v ) ? 1 : 0);  }
  static int SPtrSort(const double* v, const double* v1 )  {  return ( v < v1 ) ? -1 : (( v1 > v ) ? 1 : 0);  }
};
class main_TestR1 {
  double v;
  static int cmp(const main_TestR1& a, const main_TestR1& b)  {
    return (a.v < b.v) ? -1 : ( a.v > b.v ? 1 : 0 );
  }
  static int cmp(const main_TestR1* a, const main_TestR1* b)  {
    return (a->v < b->v) ? -1 : ( a->v > b->v ? 1 : 0 );
  }
public:
  main_TestR1(double _v) : v(_v) {}
  int Sort(const main_TestR1& a, const main_TestR1& b )   {  return cmp(a,b);  }
  int CSort(const main_TestR1& a, const main_TestR1& b ) const  {  return cmp(a,b);  }
  static int SSort(const main_TestR1& a, const main_TestR1& b )  {  return cmp(a,b);  }
  int Sortp(const main_TestR1* a, const main_TestR1* b )   {  return cmp(a,b);  }
  int CSortp(const main_TestR1* a, const main_TestR1* b ) const  {  return cmp(a,b);  }
  static int SSortp(const main_TestR1* a, const main_TestR1* b )  {  return cmp(a,b);  }
  int Compare(const main_TestR1& s) const {  return cmp(*this, s);  }
};

      TTypeList<double> AA;
      AA.AddACopy(10.0);
      AA.AddACopy(1.0);
      AA.AddACopy(70.0);
      AA.AddACopy(7.0);

      main_TestR  testR;
      main_TestR1  testR1(0);

      TTypeList<double>::BubleSorter.SortSF(AA, &main_TestR::SSort);
      TTypeList<double>::BubleSorter.SortMF(AA, testR, &main_TestR::Sort);
      TTypeList<double>::BubleSorter.SortMF(AA, testR, &main_TestR::CSort);
      TTypeList<double>::BubleSorter.Sort<TPrimitiveComparator>(AA);

      TTypeList<double>::QuickSorter.SortSF(AA, &main_TestR::SSort);
      TTypeList<double>::QuickSorter.SortMF(AA, testR, &main_TestR::Sort);
      TTypeList<double>::QuickSorter.SortMF(AA, testR, &main_TestR::CSort);
      TTypeList<double>::QuickSorter.Sort<TPrimitiveComparator>(AA);

      TArrayList<double>  da;
      TArrayList<double>::BubleSorter.SortSF(da, &main_TestR::SSort);
      TArrayList<double>::BubleSorter.SortMF(da, testR, &main_TestR::Sort);
      TArrayList<double>::BubleSorter.SortMF(da, testR, &main_TestR::CSort);
      TArrayList<double>::BubleSorter.Sort<TPrimitiveComparator>(da);

      TArrayList<double>::QuickSorter.SortSF(da, &main_TestR::SSort);
      TArrayList<double>::QuickSorter.SortMF(da, testR, &main_TestR::Sort);
      TArrayList<double>::QuickSorter.SortMF(da, testR, &main_TestR::CSort);
      TArrayList<double>::QuickSorter.Sort<TPrimitiveComparator>(da);

      TTypeList<main_TestR1> r1l;
      r1l.AddNew(10);
      r1l.AddNew(1);
      r1l.AddNew(2);
      r1l.AddNew(5);
      r1l.AddNew(3);
      TTypeList<main_TestR1>::BubleSorter.SortSF(r1l, &main_TestR1::SSort);
      TTypeList<main_TestR1>::BubleSorter.SortMF(r1l, testR1, &main_TestR1::Sort);
      TTypeList<main_TestR1>::BubleSorter.SortMF(r1l, testR1, &main_TestR1::CSort);
      TTypeList<main_TestR1>::BubleSorter.Sort<TComparableComparator>(r1l);

      TTypeList<main_TestR1>::QuickSorter.SortSF(r1l, &main_TestR1::SSort);
      TTypeList<main_TestR1>::QuickSorter.SortMF(r1l, testR1, &main_TestR1::Sort);
      TTypeList<main_TestR1>::QuickSorter.SortMF(r1l, testR1, &main_TestR1::CSort);
      TTypeList<main_TestR1>::QuickSorter.Sort<TComparableComparator>(r1l);

      TPtrList<main_TestR1> ptrl;
      for( int i=0; i < r1l.Count(); i++ )
        ptrl.Add( &r1l[i] );
      TPtrList<main_TestR1>::BubleSorter.SortSF(ptrl, &main_TestR1::SSortp);
      TPtrList<main_TestR1>::BubleSorter.SortMF(ptrl, testR1, &main_TestR1::Sortp);
      TPtrList<main_TestR1>::BubleSorter.SortMF(ptrl, testR1, &main_TestR1::CSortp);
      TPtrList<main_TestR1>::BubleSorter.Sort<TComparableComparator>(ptrl);

      TPtrList<main_TestR1>::QuickSorter.SortSF(ptrl, &main_TestR1::SSortp);
      TPtrList<main_TestR1>::QuickSorter.SortMF(ptrl, testR1, &main_TestR1::Sortp);
      TPtrList<main_TestR1>::QuickSorter.SortMF(ptrl, testR1, &main_TestR1::CSortp);
      TPtrList<main_TestR1>::QuickSorter.Sort<TComparableComparator>(ptrl);

      TPtrList<double> pptrl;
      for( int i=0; i < da.Count(); i++ )
        pptrl.Add( &da[i] );
      TPtrList<double>::BubleSorter.SortSF(pptrl, &main_TestR::SSortp);
      TPtrList<double>::BubleSorter.SortSF(pptrl, &main_TestR::SPtrSort);
      TPtrList<double>::BubleSorter.SortMF(pptrl, testR, &main_TestR::Sortp);
      TPtrList<double>::BubleSorter.SortMF(pptrl, testR, &main_TestR::CSortp);
      TPtrList<double>::BubleSorter.Sort<TPrimitiveComparator>(pptrl);

      TPtrList<double>::QuickSorter.SortSF(pptrl, &main_TestR::SSortp);
      TPtrList<double>::QuickSorter.SortMF(pptrl, testR, &main_TestR::Sortp);
      TPtrList<double>::QuickSorter.SortMF(pptrl, testR, &main_TestR::CSortp);
      // Comparator dereference the pointers
      TPtrList<double>::QuickSorter.Sort<TPrimitiveComparator>(pptrl);
*/
EndEsdlNamespace()
#endif


