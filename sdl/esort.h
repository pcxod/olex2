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
class TPrimitivePtrComparator  {
public:
template <class Comparable>
  static inline int Compare(const Comparable* A, const Comparable* B )  {
    if( *A < *B )  return -1;
    if( *A > *B )  return 1;
    return 0;
  }
};

class TComparablePtrComparator  {
public:
template <class Comparable>
  static inline int Compare(const Comparable* A, const Comparable* B )  {
    return A->Compare(*B);
  }
};

class TPrimitiveComparator  {
public:
template <class Comparable>
  static inline int Compare(const Comparable& A, const Comparable& B )  {
    if( A < B )  return -1;
    if( A > B )  return 1;
    return 0;
  }
};

class TComparableComparator  {
public:
template <class Comparable>
  static inline int Compare(const Comparable& A, const Comparable& B )  {
    return A.Compare(B);
  }
};

template <class ListClass, class ItemClass >  class TQuickPtrSorter  {
  // static comparator sort
  template <class Comparator>
    static void QuickSortSC(int lo0, int hi0, ListClass& list)  {
      int lo = lo0;
      int hi = hi0;
      if( hi0 > lo0)  {
        ItemClass* mid = list.Item( ( lo0 + hi0 ) / 2 );
        while( lo <= hi )  {
          while( ( lo < hi0 ) && ( Comparator::Compare(list.Item(lo), mid) < 0) )   lo++;
          while( ( hi > lo0 ) && ( Comparator::Compare(list.Item(hi), mid) > 0) ) hi--;
          if( lo <= hi )  {
            list.Swap(lo, hi);
            lo++;
            hi--;
          }
        }
        if( lo0 < hi )  QuickSortSC<Comparator>(lo0, hi, list);
        if( lo < hi0 )  QuickSortSC<Comparator>(lo, hi0, list);
      }
    }
    //synchronious sort
  template <class Comparator, class ListClassA>
    static void SQuickSortSC(int lo0, int hi0, ListClass& list, ListClassA& list1)  {
      int lo = lo0;
      int hi = hi0;
      if( hi0 > lo0)  {
        ItemClass* mid = list.Item( ( lo0 + hi0 ) / 2 );
        while( lo <= hi )  {
          while( ( lo < hi0 ) && ( Comparator::Compare(list.Item(lo), mid) < 0) )   lo++;
          while( ( hi > lo0 ) && ( Comparator::Compare(list.Item(hi), mid) > 0) ) hi--;
          if( lo <= hi )  {
            list.Swap(lo, hi);
            list1.Swap(lom hi);
            lo++;
            hi--;
          }
        }
        if( lo0 < hi )  SQuickSortSC<Comparator,ListClassA>(lo0, hi, list, list1);
        if( lo < hi0 )  SQuickSortSC<Comparator,ListClassA>(lo, hi0, list, list1);
      }
    }
    // static comparison function
    static void QuickSortSF(int lo0, int hi0, ListClass& list,
                              int (*f)(const ItemClass* a, const ItemClass* b) )  {
      int lo = lo0;
      int hi = hi0;
      if( hi0 > lo0)  {
        ItemClass* mid = list.Item( ( lo0 + hi0 ) / 2 );
        while( lo <= hi )  {
          while( ( lo < hi0 ) && ( f(list.Item(lo), mid) < 0) )   lo++;
          while( ( hi > lo0 ) && ( f(list.Item(hi), mid) > 0) ) hi--;
          if( lo <= hi )  {
            list.Swap(lo, hi);
            lo++;
            hi--;
          }
        }
        if( lo0 < hi )  QuickSortSF(lo0, hi, list, f);
        if( lo < hi0 )  QuickSortSF(lo, hi0, list, f);
      }
    }
    template <class ListClassA>
    static void SQuickSortSF(int lo0, int hi0, ListClass& list, ListClassA& list1,
                              int (*f)(const ItemClass* a, const ItemClass* b) )  {
      int lo = lo0;
      int hi = hi0;
      if( hi0 > lo0)  {
        ItemClass* mid = list.Item( ( lo0 + hi0 ) / 2 );
        while( lo <= hi )  {
          while( ( lo < hi0 ) && ( f(list.Item(lo), mid) < 0) )   lo++;
          while( ( hi > lo0 ) && ( f(list.Item(hi), mid) > 0) ) hi--;
          if( lo <= hi )  {
            list.Swap(lo, hi);
            list1.Swap(lo, hi);
            lo++;
            hi--;
          }
        }
        if( lo0 < hi )  SQuickSortSF<ListClassA>(lo0, hi, list, list1, f);
        if( lo < hi0 )  SQuickSortSF<ListClassA>(lo, hi0, list, list1, f);
      }
    }
    // class member comparison function
  template <class BaseClass>
    static void QuickSortMF(int lo0, int hi0, ListClass& list, BaseClass& baseClassInstance,
                              int (BaseClass::*f)(const ItemClass* a, const ItemClass* b) )  {
      int lo = lo0;
      int hi = hi0;
      if( hi0 > lo0)  {
        ItemClass* mid = list.Item( ( lo0 + hi0 ) / 2 );
        while( lo <= hi )  {
          while( ( lo < hi0 ) && ( (baseClassInstance.*f)(list.Item(lo), mid) < 0) )   lo++;
          while( ( hi > lo0 ) && ( (baseClassInstance.*f)(list.Item(hi), mid) > 0) ) hi--;
          if( lo <= hi )  {
            list.Swap(lo, hi);
            lo++;
            hi--;
          }
        }
        if( lo0 < hi )  QuickSortMF<BaseClass>(lo0, hi, list, baseClassInstance, f);
        if( lo < hi0 )  QuickSortMF<BaseClass>(lo, hi0, list, baseClassInstance, f);
      }
    }
  template <class BaseClass, class ListClassA>
    static void SQuickSortMF(int lo0, int hi0, ListClass& list, ListClassA& list1, 
        BaseClass& baseClassInstance, 
        int (BaseClass::*f)(const ItemClass* a, const ItemClass* b) )  {
      int lo = lo0;
      int hi = hi0;
      if( hi0 > lo0)  {
        ItemClass* mid = list.Item( ( lo0 + hi0 ) / 2 );
        while( lo <= hi )  {
          while( ( lo < hi0 ) && ( (baseClassInstance.*f)(list.Item(lo), mid) < 0) )   lo++;
          while( ( hi > lo0 ) && ( (baseClassInstance.*f)(list.Item(hi), mid) > 0) ) hi--;
          if( lo <= hi )  {
            list.Swap(lo, hi);
            list1.Swap(lo, hi);
            lo++;
            hi--;
          }
        }
        if( lo0 < hi )  SQuickSortMF<BaseClass,ListClassA>(lo0, hi, list, list1, baseClassInstance, f);
        if( lo < hi0 )  SQuickSortMF<BaseClass,ListClassA>(lo, hi0, list, list1, baseClassInstance, f);
      }
    }

public:
  template <class Comparator>
    static void Sort(ListClass& list)  {
      QuickSortSC<Comparator>(0, list.Count()-1, list);
    }
  template <class Comparator, class ListClassA>
    static void SyncSort(ListClass& list, ListClassA& list1)  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      SQuickSortSC<Comparator,ListClassA>(0, list.Count()-1, list, list1);
    }
  static void SortSF(ListClass& list, int (*f)(const ItemClass* a, const ItemClass* b) )  {
      QuickSortSF(0, list.Count()-1, list, f);
  }
  template <class ListClassA>
    static void SyncSortSF(ListClass& list, ListClassA& list1, int (*f)(const ItemClass* a, const ItemClass* b) )  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      SQuickSortSF<ListClassA>(0, list.Count()-1, list, list1, f);
    }
  template <class BaseClass>
    static void SortMF(ListClass& list, BaseClass& baseClassInstance,
                       int (BaseClass::*f)(const ItemClass* a, const ItemClass* b) )  {
      QuickSortMF<BaseClass>(0, list.Count()-1, list, baseClassInstance, f);
    }
  template <class BaseClass, class ListClassA>
    static void SortMF(ListClass& list, ListClassA& list1, BaseClass& baseClassInstance,
                       int (BaseClass::*f)(const ItemClass* a, const ItemClass* b) )  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      SQuickSortMF<BaseClass,ListClassA>(0, list.Count()-1, list, list1, baseClassInstance, f);
    }
};

template <class ListClass, class ItemClass >  class TBubblePtrSorter  {
  // static comparator sort
  template <class Comparator>
    static void BubbleSortSC(ListClass& list)  {
      bool changes = true;
      while( changes )  {
        changes = false;
        for( int i=1; i < list.Count(); i++ )  {
          if( Comparator::Compare( list.Item(i-1), list.Item(i) ) == 1 )  {
            list.Swap( i-1, i);
            changes = true;
          }
        }
      }
    }
  template <class Comparator, class ListClassA>
    static void SBubbleSortSC(ListClass& list, ListClassA& list1)  {
      bool changes = true;
      while( changes )  {
        changes = false;
        for( int i=1; i < list.Count(); i++ )  {
          if( Comparator::Compare( list.Item(i-1), list.Item(i) ) == 1 )  {
            list.Swap( i-1, i );
            list1.Swap( i-1, i );
            changes = true;
          }
        }
      }
    }
    // static comparison function
    static void BubbleSortSF(ListClass& list,
                              int (*f)(const ItemClass* a, const ItemClass* b) )  {
      bool changes = true;
      while( changes )  {
        changes = false;
        for( int i=1; i < list.Count(); i++ )  {
          if( f( list.Item(i-1), list.Item(i) ) == 1 )  {
            list.Swap( i-1, i);
            changes = true;
          }
        }
      }
    }
    template <class ListClassA>
    static void SBubbleSortSF(ListClass& list, ListClassA& list1,
                              int (*f)(const ItemClass* a, const ItemClass* b) )  {
      bool changes = true;
      while( changes )  {
        changes = false;
        for( int i=1; i < list.Count(); i++ )  {
          if( f( list.Item(i-1), list.Item(i) ) == 1 )  {
            list.Swap( i-1, i );
            list1.Swap( i-1, i );
            changes = true;
          }
        }
      }
    }
    // class member comparison function
  template <class BaseClass>
    static void BubbleSortMF(ListClass& list, BaseClass& baseClassInstance,
                              int (BaseClass::*f)(const ItemClass* a, const ItemClass* b) )  {
      bool changes = true;
      while( changes )  {
        changes = false;
        for( int i=1; i < list.Count(); i++ )  {
          if( (baseClassInstance.*f)( list.Item(i-1), list.Item(i) ) == 1 )  {
            list.Swap( i-1, i);
            changes = true;
          }
        }
      }
    }
  template <class BaseClass, class ListClassA>
    static void BubbleSortMF(ListClass& list, ListClassA& list1, BaseClass& baseClassInstance,
                              int (BaseClass::*f)(const ItemClass* a, const ItemClass* b) )  {
      bool changes = true;
      while( changes )  {
        changes = false;
        for( int i=1; i < list.Count(); i++ )  {
          if( (baseClassInstance.*f)( list.Item(i-1), list.Item(i) ) == 1 )  {
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
      BubbleSortSC<Comparator>(list);
    }
  template <class Comparator, class ListClassA>
    static void SynchSort(ListClass& list, ListClassA& list1)  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      SBubbleSortSC<Comparator,ListClassA>(list, list1);
    }
  static void SortSF(ListClass& list, int (*f)(const ItemClass* a, const ItemClass* b) )  {
      BubbleSortSF(list, f);
  }
  template <class ListClassA>
    static void SyncSortSF(ListClass& list, ListClassA& list1, int (*f)(const ItemClass* a, const ItemClass* b) )  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      SBubbleSortSF<ListClassA>(list, list1, f);
    }
  template <class BaseClass>
    static void SortMF(ListClass& list, BaseClass& baseClassInstance,
                       int (BaseClass::*f)(const ItemClass* a, const ItemClass* b) )  {
      BubbleSortMF<BaseClass>(list, baseClassInstance, f);
    }
  template <class BaseClass, class ListClassA>
    static void SynchSortMF(ListClass& list, ListClassA& list1, BaseClass& baseClassInstance,
                       int (BaseClass::*f)(const ItemClass* a, const ItemClass* b) )  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      SBubbleSortMF<BaseClass,ListClassA>(list, list1, baseClassInstance, f);
    }
};

template <class ListClass, class ItemClass > class TQuickSorter  {
  // static comparator sort
  template <class Comparator>
    static void QuickSortSC(int lo0, int hi0, ListClass& list)  {
      int lo = lo0;
      int hi = hi0;
      if( hi0 > lo0)  {
        const ItemClass& mid = list.Item( ( lo0 + hi0 ) / 2 );
        while( lo <= hi )  {
          while( ( lo < hi0 ) && ( Comparator::Compare(list.Item(lo), mid) < 0) )   lo++;
          while( ( hi > lo0 ) && ( Comparator::Compare(list.Item(hi), mid) > 0) ) hi--;
          if( lo <= hi )  {
            list.Swap(lo, hi);
            lo++;
            hi--;
          }
        }
        if( lo0 < hi )  QuickSortSC<Comparator>(lo0, hi, list);
        if( lo < hi0 )  QuickSortSC<Comparator>(lo, hi0, list);
      }
    }
  template <class Comparator, class ListClassA>
    static void SQuickSortSC(int lo0, int hi0, ListClass& list, ListClassA& list1)  {
      int lo = lo0;
      int hi = hi0;
      if( hi0 > lo0)  {
        const ItemClass& mid = list.Item( ( lo0 + hi0 ) / 2 );
        while( lo <= hi )  {
          while( ( lo < hi0 ) && ( Comparator::Compare(list.Item(lo), mid) < 0) )   lo++;
          while( ( hi > lo0 ) && ( Comparator::Compare(list.Item(hi), mid) > 0) ) hi--;
          if( lo <= hi )  {
            list.Swap(lo, hi);
            list1.Swap(lo, hi);
            lo++;
            hi--;
          }
        }
        if( lo0 < hi )  SQuickSortSC<Comparator,ListClassA>(lo0, hi, list, list1);
        if( lo < hi0 )  SQuickSortSC<Comparator,ListClassA>(lo, hi0, list, list1);
      }
    }
    // static comparison function
    static void QuickSortSF(int lo0, int hi0, ListClass& list,
                              int (*f)(const ItemClass& a, const ItemClass&b) )  {
      int lo = lo0;
      int hi = hi0;
      if( hi0 > lo0)  {
        const ItemClass& mid = list.Item( ( lo0 + hi0 ) / 2 );
        while( lo <= hi )  {
          while( ( lo < hi0 ) && ( f(list.Item(lo), mid) < 0) )   lo++;
          while( ( hi > lo0 ) && ( f(list.Item(hi), mid) > 0) ) hi--;
          if( lo <= hi )  {
            list.Swap(lo, hi);
            lo++;
            hi--;
          }
        }
        if( lo0 < hi )  QuickSortSF(lo0, hi, list, f);
        if( lo < hi0 )  QuickSortSF(lo, hi0, list, f);
      }
    }
    template <class ListClassA>
    static void SQuickSortSF(int lo0, int hi0, ListClass& list, ListClassA& list1,
                              int (*f)(const ItemClass& a, const ItemClass&b) )  {
      int lo = lo0;
      int hi = hi0;
      if( hi0 > lo0)  {
        const ItemClass& mid = list.Item( ( lo0 + hi0 ) / 2 );
        while( lo <= hi )  {
          while( ( lo < hi0 ) && ( f(list.Item(lo), mid) < 0) )   lo++;
          while( ( hi > lo0 ) && ( f(list.Item(hi), mid) > 0) ) hi--;
          if( lo <= hi )  {
            list.Swap(lo, hi);
            list1.Swap(lo, hi);
            lo++;
            hi--;
          }
        }
        if( lo0 < hi )  SQuickSortSF<ListClassA>(lo0, hi, list, list1, f);
        if( lo < hi0 )  SQuickSortSF<ListClassA>(lo, hi0, list, list1, f);
      }
    }
    // class member comparison function
  template <class BaseClass>
    static void QuickSortMF(int lo0, int hi0, ListClass& list, BaseClass& baseClassInstance,
                              int (BaseClass::*f)(const ItemClass& a, const ItemClass& b) )  {
      int lo = lo0;
      int hi = hi0;
      if( hi0 > lo0)  {
        const ItemClass& mid = list.Item( ( lo0 + hi0 ) / 2 );
        while( lo <= hi )  {
          while( ( lo < hi0 ) && ( (baseClassInstance.*f)(list.Item(lo), mid) < 0) )   lo++;
          while( ( hi > lo0 ) && ( (baseClassInstance.*f)(list.Item(hi), mid) > 0) ) hi--;
          if( lo <= hi )  {
            list.Swap(lo, hi);
            lo++;
            hi--;
          }
        }
        if( lo0 < hi )  QuickSortMF<BaseClass>(lo0, hi, list, baseClassInstance, f);
        if( lo < hi0 )  QuickSortMF<BaseClass>(lo, hi0, list, baseClassInstance, f);
      }
    }
  template <class BaseClass, class ListClassA>
    static void SQuickSortMF(int lo0, int hi0, ListClass& list, ListClassA& list1, BaseClass& baseClassInstance,
                              int (BaseClass::*f)(const ItemClass& a, const ItemClass& b) )  {
      int lo = lo0;
      int hi = hi0;
      if( hi0 > lo0)  {
        const ItemClass& mid = list.Item( ( lo0 + hi0 ) / 2 );
        while( lo <= hi )  {
          while( ( lo < hi0 ) && ( (baseClassInstance.*f)(list.Item(lo), mid) < 0) )   lo++;
          while( ( hi > lo0 ) && ( (baseClassInstance.*f)(list.Item(hi), mid) > 0) ) hi--;
          if( lo <= hi )  {
            list.Swap(lo, hi);
            lis1.Swap(lo, hi);
            lo++;
            hi--;
          }
        }
        if( lo0 < hi )  SQuickSortMF<BaseClass,ListClassA>(lo0, hi, list, list1, baseClassInstance, f);
        if( lo < hi0 )  SQuickSortMF<BaseClass,ListClassA>(lo, hi0, list, list1, baseClassInstance, f);
      }
    }

public:
  template <class Comparator>
    static void Sort(ListClass& list)  {
      QuickSortSC<Comparator>(0, list.Count()-1, list);
    }
  template <class Comparator, class ListClassA>
    static void SyncSort(ListClass& list, ListClassA& list1)  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      SQuickSortSC<Comparator,ListClassA>(0, list.Count()-1, list, list1);
    }
  static void SortSF(ListClass& list, int (*f)(const ItemClass& a, const ItemClass& b) )  {
      QuickSortSF(0, list.Count()-1, list, f);
  }
  template <class ListClassA>
    static void SyncSortSF(ListClass& list, ListClassA& list1, int (*f)(const ItemClass& a, const ItemClass& b) )  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      SQuickSortSF<ListClassA>(0, list.Count()-1, list, list1, f);
    }
  template <class BaseClass>
    static void SortMF(ListClass& list, BaseClass& baseClassInstance,
                       int (BaseClass::*f)(const ItemClass& a, const ItemClass& b) )  {
      QuickSortMF<BaseClass>(0, list.Count()-1, list, baseClassInstance, f);
    }
  template <class BaseClass, class ListClassA>
    static void SyncSortMF(ListClass& list, ListClassA& list1, BaseClass& baseClassInstance,
                       int (BaseClass::*f)(const ItemClass& a, const ItemClass& b) )  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      SQuickSortMF<BaseClass,ListClassA>(0, list.Count()-1, list, list1, baseClassInstance, f);
    }
};

template <class ListClass, class ItemClass >  class TBubbleSorter  {
  // static comparator sort
  template <class Comparator>
    static void BubbleSortSC(ListClass& list)  {
      bool changes = true;
      while( changes )  {
        changes = false;
        for( int i=1; i < list.Count(); i++ )  {
          if( Comparator::Compare( list.Item(i-1), list.Item(i) ) == 1 )  {
            list.Swap( i-1, i);
            changes = true;
          }
        }
      }
    }
  template <class Comparator, class ListClassA>
    static void SBubbleSortSC(ListClass& list, ListClassA& list1)  {
      bool changes = true;
      while( changes )  {
        changes = false;
        for( int i=1; i < list.Count(); i++ )  {
          if( Comparator::Compare( list.Item(i-1), list.Item(i) ) == 1 )  {
            list.Swap( i-1, i );
            list1.Swap( i-1, i );
            changes = true;
          }
        }
      }
    }
    // static comparison function
    static void BubbleSortSF(ListClass& list,
                              int (*f)(const ItemClass& a, const ItemClass&b) )  {
      bool changes = true;
      while( changes )  {
        changes = false;
        for( int i=1; i < list.Count(); i++ )  {
          if( f( list.Item(i-1), list.Item(i) ) == 1 )  {
            list.Swap( i-1, i);
            changes = true;
          }
        }
      }
    }
    template <class ListClassA>
    static void SBubbleSortSF(ListClass& list, ListClassA& list1, 
                              int (*f)(const ItemClass& a, const ItemClass&b) )  {
      bool changes = true;
      while( changes )  {
        changes = false;
        for( int i=1; i < list.Count(); i++ )  {
          if( f( list.Item(i-1), list.Item(i) ) == 1 )  {
            list.Swap( i-1, i );
            list1.Swap( i-1, i );
            changes = true;
          }
        }
      }
    }
    // class member comparison function
  template <class BaseClass>
    static void BubbleSortMF(ListClass& list, BaseClass& baseClassInstance,
                              int (BaseClass::*f)(const ItemClass& a, const ItemClass& b) )  {
      bool changes = true;
      while( changes )  {
        changes = false;
        for( int i=1; i < list.Count(); i++ )  {
          if( (baseClassInstance.*f)( list.Item(i-1), list.Item(i) ) == 1 )  {
            list.Swap( i-1, i);
            changes = true;
          }
        }
      }
    }
  template <class BaseClass, class ListClassA>
    static void SBubbleSortMF(ListClass& list, ListClassA& list1, BaseClass& baseClassInstance,
                              int (BaseClass::*f)(const ItemClass& a, const ItemClass& b) )  {
      bool changes = true;
      while( changes )  {
        changes = false;
        for( int i=1; i < list.Count(); i++ )  {
          if( (baseClassInstance.*f)( list.Item(i-1), list.Item(i) ) == 1 )  {
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
      BubbleSortSC<Comparator>(list);
    }
  template <class Comparator, class ListClassA>
    static void SyncSort(ListClass& list, ListClassA& list1)  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      SBubbleSortSC<Comparator,ListClassA>(list, list1);
    }
  static void SortSF(ListClass& list, int (*f)(const ItemClass& a, const ItemClass& b) )  {
    BubbleSortSF(list, f);
  }
  template <class ListClassA>
    static void SyncSortSF(ListClass& list, ListClassA& list1, int (*f)(const ItemClass& a, const ItemClass& b) )  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      SBubbleSortSF(list, list1, f);
    }
  template <class BaseClass>
    static void SortMF(ListClass& list, BaseClass& baseClassInstance,
                       int (BaseClass::*f)(const ItemClass& a, const ItemClass& b) )  {
      BubbleSortMF<BaseClass>(list, baseClassInstance, f);
    }
  template <class BaseClass, class ListClassA>
    static void SyncSortMF(ListClass& list, ListClassA& list1, BaseClass& baseClassInstance,
                       int (BaseClass::*f)(const ItemClass& a, const ItemClass& b) )  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      SBubbleSortMF<BaseClass,ListClassA>(list, list1, baseClassInstance, f);
    }
};
/* and the last one ... for the ArrayList - same issues as for the pointers -
  we need a copy of the object - not a reference to its location!
*/
template <class ListClass, class ItemClass > class TQuickObjectSorter  {
  // static comparator sort
  template <class Comparator>
    static void QuickSortSC(int lo0, int hi0, ListClass& list)  {
      int lo = lo0;
      int hi = hi0;
      if( hi0 > lo0)  {
        ItemClass mid = list.Item( ( lo0 + hi0 ) / 2 );
        while( lo <= hi )  {
          while( ( lo < hi0 ) && ( Comparator::Compare(list.Item(lo), mid) < 0) )   lo++;
          while( ( hi > lo0 ) && ( Comparator::Compare(list.Item(hi), mid) > 0) ) hi--;
          if( lo <= hi )  {
            list.Swap(lo, hi);
            lo++;
            hi--;
          }
        }
        if( lo0 < hi )  QuickSortSC<Comparator>(lo0, hi, list);
        if( lo < hi0 )  QuickSortSC<Comparator>(lo, hi0, list);
      }
    }
  template <class Comparator, class ListClassA>
    static void SQuickSortSC(int lo0, int hi0, ListClass& list, ListClassA& list1)  {
      int lo = lo0;
      int hi = hi0;
      if( hi0 > lo0)  {
        ItemClass mid = list.Item( ( lo0 + hi0 ) / 2 );
        while( lo <= hi )  {
          while( ( lo < hi0 ) && ( Comparator::Compare(list.Item(lo), mid) < 0) )   lo++;
          while( ( hi > lo0 ) && ( Comparator::Compare(list.Item(hi), mid) > 0) ) hi--;
          if( lo <= hi )  {
            list.Swap(lo, hi);
            list1.Swap(lo, hi);
            lo++;
            hi--;
          }
        }
        if( lo0 < hi )  SQuickSortSC<Comparator,ListClassA>(lo0, hi, list, list1);
        if( lo < hi0 )  SQuickSortSC<Comparator,ListClassA>(lo, hi0, list, list1);
      }
    }
    // static comparison function
    static void QuickSortSF(int lo0, int hi0, ListClass& list,
                              int (*f)(const ItemClass& a, const ItemClass&b) )  {
      int lo = lo0;
      int hi = hi0;
      if( hi0 > lo0)  {
        ItemClass mid = list.Item( ( lo0 + hi0 ) / 2 );
        while( lo <= hi )  {
          while( ( lo < hi0 ) && ( f(list.Item(lo), mid) < 0) )   lo++;
          while( ( hi > lo0 ) && ( f(list.Item(hi), mid) > 0) ) hi--;
          if( lo <= hi )  {
            list.Swap(lo, hi);
            lo++;
            hi--;
          }
        }
        if( lo0 < hi )  QuickSortSF(lo0, hi, list, f);
        if( lo < hi0 )  QuickSortSF(lo, hi0, list, f);
      }
    }
    template <class ListClassA>
    static void SQuickSortSF(int lo0, int hi0, ListClass& list, ListClassA& list1, 
                              int (*f)(const ItemClass& a, const ItemClass&b) )  {
      int lo = lo0;
      int hi = hi0;
      if( hi0 > lo0)  {
        ItemClass mid = list.Item( ( lo0 + hi0 ) / 2 );
        while( lo <= hi )  {
          while( ( lo < hi0 ) && ( f(list.Item(lo), mid) < 0) )   lo++;
          while( ( hi > lo0 ) && ( f(list.Item(hi), mid) > 0) ) hi--;
          if( lo <= hi )  {
            list.Swap(lo, hi);
            list1.Swap(lo, hi);
            lo++;
            hi--;
          }
        }
        if( lo0 < hi )  SQuickSortSF<ListClassA>(lo0, hi, list, list1, f);
        if( lo < hi0 )  SQuickSortSF<ListClassA>(lo, hi0, list, list1, f);
      }
    }
    // class member comparison function
  template <class BaseClass>
    static void QuickSortMF(int lo0, int hi0, ListClass& list, BaseClass& baseClassInstance,
                              int (BaseClass::*f)(const ItemClass& a, const ItemClass& b) )  {
      int lo = lo0;
      int hi = hi0;
      if( hi0 > lo0)  {
        ItemClass mid = list.Item( ( lo0 + hi0 ) / 2 );
        while( lo <= hi )  {
          while( ( lo < hi0 ) && ( (baseClassInstance.*f)(list.Item(lo), mid) < 0) )   lo++;
          while( ( hi > lo0 ) && ( (baseClassInstance.*f)(list.Item(hi), mid) > 0) ) hi--;
          if( lo <= hi )  {
            list.Swap(lo, hi);
            lo++;
            hi--;
          }
        }
        if( lo0 < hi )  QuickSortMF<BaseClass>(lo0, hi, list, baseClassInstance, f);
        if( lo < hi0 )  QuickSortMF<BaseClass>(lo, hi0, list, baseClassInstance, f);
      }
    }
  template <class BaseClass, class ListClassA>
    static void SQuickSortMF(int lo0, int hi0, ListClass& list, ListClassA& list1, BaseClass& baseClassInstance,
                              int (BaseClass::*f)(const ItemClass& a, const ItemClass& b) )  {
      int lo = lo0;
      int hi = hi0;
      if( hi0 > lo0)  {
        ItemClass mid = list.Item( ( lo0 + hi0 ) / 2 );
        while( lo <= hi )  {
          while( ( lo < hi0 ) && ( (baseClassInstance.*f)(list.Item(lo), mid) < 0) )   lo++;
          while( ( hi > lo0 ) && ( (baseClassInstance.*f)(list.Item(hi), mid) > 0) ) hi--;
          if( lo <= hi )  {
            list.Swap(lo, hi);
            list1.Swap(lo, hi);
            lo++;
            hi--;
          }
        }
        if( lo0 < hi )  SQuickSortMF<BaseClass>(lo0, hi, list, list1, baseClassInstance, f);
        if( lo < hi0 )  SQuickSortMF<BaseClass>(lo, hi0, list, list1, baseClassInstance, f);
      }
    }

public:
  template <class Comparator>
    static void Sort(ListClass& list)  {
      QuickSortSC<Comparator>(0, list.Count()-1, list);
    }
  template <class Comparator, class ListClassA>
    static void SyncSort(ListClass& list, ListClassA& list1)  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      SQuickSortSC<Comparator,ListClassA>(0, list.Count()-1, list, list1);
    }
  static void SortSF(ListClass& list, int (*f)(const ItemClass& a, const ItemClass& b) )  {
    QuickSortSF(0, list.Count()-1, list, f);
  }
  template <class ListClassA>
    static void SyncSortSF(ListClass& list, ListClassA& list1, int (*f)(const ItemClass& a, const ItemClass& b) )  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      SQuickSortSF<ListClassA>(0, list.Count()-1, list, list1, f);
    }
  template <class BaseClass>
    static void SortMF(ListClass& list, BaseClass& baseClassInstance,
                       int (BaseClass::*f)(const ItemClass& a, const ItemClass& b) )  {
      QuickSortMF<BaseClass>(0, list.Count()-1, list, baseClassInstance, f);
    }
  template <class BaseClass, class ListClassA>
    static void SyncSortMF(ListClass& list, ListClassA& list1, BaseClass& baseClassInstance,
                       int (BaseClass::*f)(const ItemClass& a, const ItemClass& b) )  {
      if( list.Count() != list1.Count() )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "list size mismatch");
      SQuickSortMF<BaseClass,ListClassA>(0, list.Count()-1, list, list1, baseClassInstance, f);
    }
};

//examples
/*
      TTypeList<double> AA;
      AA.AddACopy(10.0);
      AA.AddACopy(1.0);
      AA.AddACopy(70.0);
      AA.AddACopy(7.0);

      class TestR  {
      public:
        int Sort(const double& v, const double& v1 )  {  if( v < v1 )  return -1;  if( v1 < v )  return 1;  return 0;  }
        static int SSort(const double& v, const double& v1 )  {  if( v < v1 )  return -1;  if( v1 < v )  return 1;  return 0;  }
      };
      TestR  testR;

      TTypeList<double>::BubbleSorter.Sort(AA, &TestR::SSort);
      TTypeList<double>::BubbleSorter.Sort<TestR>(AA, testR, &TestR::Sort);
      TTypeList<double>::BubbleSorter.Sort<TPrimitiveComparator>(AA);
      TTypeList<double>::QuickSorter.Sort(AA, &TestR::SSort);
      TTypeList<double>::QuickSorter.Sort<TestR>(AA, testR, &TestR::Sort);
      TTypeList<double>::QuickSorter.Sort<TPrimitiveComparator>(AA);
*/
EndEsdlNamespace()
#endif


