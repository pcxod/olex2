//---------------------------------------------------------------------------
#ifndef eSTListH
#define eSTListH
#include "tptrlist.h"

#undef GetObject

BeginEsdlNamespace()
//---------------------------------------------------------------------------

template <class ComparableClass, class ObjectClass, class Comparator>
  struct TSortedListEntry {
    ComparableClass Comparable;
    mutable ObjectClass Object;
    TSortedListEntry(const ComparableClass& c, const ObjectClass& o ) :
      Comparable(c), 
      Object(o) { }
    TSortedListEntry(const TSortedListEntry& entry) :
      Comparable(entry.Comparable),
      Object(entry.Object)  { }
    virtual ~TSortedListEntry()  {  }

    inline TSortedListEntry& operator = (const TSortedListEntry& entry)  {
      Comparable = entry.Comparable;
      Object = entry.Object;
      return *this;
    }
    inline int Compare(TSortedListEntry& entry) const {
      return Comparator::Compare(Comparable, entry.Comparable );  
    }

    template <class T>
    inline int Compare(const T& key ) const {
      return Comparator::Compare(Comparable, key);  }

};
//---------------------------------------------------------------------------
template <class A, class B, class ComparatorType>
  class TSTypeList : public IEObject  {
    // no arraylist - inserts are too 'heavy' 
    typedef TSortedListEntry<A,B,ComparatorType> EntryType;
    TPtrList<EntryType> Data;
  protected:
    template <class T>
    int FindInsertIndex(const T& key, int from=-1, int to=-1) {
      if( from == -1 ) from = 0;
      if( to == -1 )   to = Count()-1;
      if( to == from ) return to;
      if( (to-from) == 1 )  return from;
      int resfrom = Data[from]->Compare(key),
          resto   = Data[to]->Compare(key);
      if( resfrom == 0 )  return from;
      if( resto == 0 )    return to;
      if( resfrom < 0 && resto > 0 )  {
        int index = (to+from)/2;
        int res = Data[index]->Compare(key);
        if( res < 0 )  return FindInsertIndex(key, index, to);
        if( res > 0 )  return FindInsertIndex(key, from, index);
        if( res == 0 ) return index;
      }
      return -1;
    }
    template <class T>
    int FindIndexOf(const T& key) const {
      if( Data.IsEmpty() )  
        return -1;
      if( Data.Count() == 1 )  
        return (!Data[0]->Compare(key)) ? 0 : -1;
      int from = 0, to = Count()-1;
      if( !Data[from]->Compare(key) )  return from;
      if( !Data[to]->Compare(key)   )  return to;
      while( true ) {
        int index = (to+from)/2;
        int res = Data[index]->Compare(key);
        if( index == from || index == to)  
          return -1;
        if( res < 0 )  from = index;
        else
          if( res > 0 )  to  = index;
          else
            if( res == 0 )  return index;
      }
      return -1;
    }
    //inline void SetCount(int count)  {  Data.SetCount(count);  }
    // used in Replicate, increments the Reference of the added entry
    //inline void SetEntry(int index,  TSortedListEntry<A,B,ComparatorType>*entry)  {
    //  Data[index] = entry;  entry->IncRef();
    //}

//..............................................................................
public:
  TSTypeList()  {  }
//..............................................................................
  /* copy constructor */
  TSTypeList(const TSTypeList& list)  {
    Data.SetCount( list.Count() );
    for( int i=0; i < list.Data.Count(); i++ )
      Data[i] = new EntryType(*list.Data[i]);
  }

  virtual ~TSTypeList()  {  Clear();  }
//..............................................................................
  inline void Clear()  {
    for( int i=0; i < Data.Count(); i++ )
      delete Data[i]; 
    Data.Clear();
  }
//..............................................................................
  template <class T>
  int IndexOfComparable(const T& cmpbl) const {  
    return FindIndexOf(cmpbl);  
  }
//..............................................................................
  int IndexOfObject(const B& v) const  {
    for( int i=0; i < Data.Count(); i++ )
      if( Data[i]->Object == v )  
        return i;
    return -1;
  }
//..............................................................................
  // retrives indexes of all entries with same key and returns the number of added entries
  template <class T>
  int GetIndexes(const T& key, TIntList& il)  {
    if( Data.IsEmpty() )  return 0;
    if( Data.Count() == 1 )  {
      if( Data[0]->Compare(key) != 0 )  return 0;
      il.Add( 0 );
      return 1;
    }

    int index =  IndexOfComparable(key);
    if( index == -1 )  return 0;
    il.Add( index );
    int i = index+1, addedc = 1;
    // go forward
    while( i < Data.Count() && (Data[i]->Compare(key) == 0) )  {
      il.Add( i );
      i++;
      addedc++;
    }
    // go backwards
    i = index-1;
    while( i >= 0 && (Data[i]->Compare(key) == 0) )  {
      il.Add( i );
      i--;
      addedc++;
    }
    return addedc;
  }
//..............................................................................
  inline void NullItem(int i)  {
    if( Data[i] != NULL )  {
      delete Data[i];
      Data[i] = NULL;
    }
  }
//..............................................................................
  inline void Pack()  {  Data.Pack();  }
//..............................................................................
  inline void Remove(size_t i)   {  
    delete Data[i];  
    Data.Delete(i);  
  }
//..............................................................................
  inline int Count()    const {  return Data.Count(); }
//..............................................................................
  inline bool IsEmpty() const {  return Data.IsEmpty();  }
//..............................................................................
  inline const A& GetComparable(int index) const {  return Data[index]->Comparable;  }
//..............................................................................
  inline B& GetObject(int index) const {  return Data[index]->Object;  }
//..............................................................................
//..............................................................................
  inline const EntryType& Last() const {  return *Data.Last();  }
//..............................................................................
  inline void SetCapacity(size_t v)  {  Data.SetCapacity(v);  }
//..............................................................................
  inline void SetIncrement(size_t v) {  Data.SetIncrement(v);  }
//..............................................................................
  template <class T>
  inline B& operator [] (const T& key) const   {
    int ind = IndexOfComparable(key);
    if( ind >= 0 )  return Data[ind]->Object;
    throw TFunctionFailedException(__OlxSourceInfo, "no object at specified location" );
  }
//..............................................................................
  const EntryType& Add(const A& key, const B& Object )  {
    EntryType *entry = new EntryType(key, Object);
    if( Data.IsEmpty() )
      Data.Add( entry);
    else if( Data.Count() == 1 )  {
      if(  Data[0]->Compare(*entry) < 0 )  
        Data.Add(entry);
      else                                 
        Data.Insert(0, entry);
    }
    else  {
      if( Data[0]->Compare(*entry) >= 0 ) // smaller than the first
        Data.Insert(0, entry);
      else if( Data.Last()->Compare(*entry) <= 0 ) // larger than the last 
        Data.Add(entry);  
      else if( Data.Count() == 2 ) // an easy case then with two items 
        Data.Insert(1, entry);
      else  {
        const int pos = FindInsertIndex(key);
        if( pos == -1 )  {
          delete entry;
          throw TIndexOutOfRangeException(__OlxSourceInfo, pos, 0, Count()-1);
        }
        Data.Insert(pos+1, entry);
      }
    }
    return *entry;
  }
};
//..............................................................................
//..............................................................................
//..............................................................................
  // to be used with objects, having Compare operator
  template <typename ComparableClass, typename ObjectClass>
    class TCSTypeList : public TSTypeList<ComparableClass, ObjectClass, TComparableComparator>
    {   };
//..............................................................................
//..............................................................................
  // to be used with objects, having >, < operators, or primitive types
  template <typename ComparableClass, typename ObjectClass>
    class TPSTypeList : public TSTypeList<ComparableClass, ObjectClass, TPrimitiveComparator>
    {   };
//..............................................................................
//..............................................................................
  // string specialisation ... special overriding for [] operator - returns NULL if no
  // specified comparable exist, beware it returns '0' for primitive types
  template <class SC, typename ObjectClass, bool caseinsensitive>
    class TSStrPObjList : public TSTypeList<SC, ObjectClass, olxstrComparator<caseinsensitive> >  {
      typedef TSTypeList<SC,ObjectClass, olxstrComparator<caseinsensitive> > PList;
      typedef TSortedListEntry<SC,ObjectClass,olxstrComparator<caseinsensitive> > PListEntry;
    public:
      inline const olxstr& GetString(int i) const {  return PList::GetComparable(i);  }
      inline const PListEntry& Add(const SC& s, const ObjectClass& v = *(ObjectClass*)NULL )  {
        return PList::Add(s, v);
      }
      inline void Delete(int i)  {  PList::Remove(i);  }
      inline int IndexOf(const SC& v)  const  {  return PList::IndexOfComparable(v);  }
      template <class T>
      inline ObjectClass operator [] (const T& key) const {
        int ind = PList::IndexOfComparable(key);
        return (ind >= 0)  ? PList::GetObject(ind) : NULL;
      }
    };
//..............................................................................
//..............................................................................
  // just a string to obj specialisation
  template <class SC, typename ObjectClass, bool caseinsensitive>
    class TSStrObjList : public TSTypeList<SC, ObjectClass, olxstrComparator<caseinsensitive> >  {
      typedef TSTypeList<SC, ObjectClass, olxstrComparator<caseinsensitive> > PList;
      typedef TSortedListEntry<SC,ObjectClass,olxstrComparator<caseinsensitive> > PListEntry;
    public:
      inline const SC& GetString(int i) const {  return PList::GetComparable(i);  }
      inline const PListEntry& Add(const SC& s, const ObjectClass& v = *(ObjectClass*)NULL )  {
        return PList::Add(s, v);
      }
      inline void Delete(int i)  { PList::Remove(i);  }
      template <class T> inline int IndexOf(const T& v) const {  
        return PList::IndexOfComparable(v);  
      }
    };
//..............................................................................
//..............................................................................
  // string - string map specialisation ...
  template <class SC, bool caseinsensitive>
    class TSStrStrList : public TSTypeList<SC, SC, olxstrComparator<caseinsensitive> >  {
      typedef TSTypeList<SC, SC, olxstrComparator<caseinsensitive> > PList;
      typedef TSortedListEntry<SC,SC,olxstrComparator<caseinsensitive> > PListEntry;
    public:
      inline const SC& GetString(int i) const {  return PList::GetComparable(i);  }
      template <class T>
      inline const PListEntry& Add(const T& key, const SC& v = EmptyString )  {
        return PList::Add(key, v);
      }
      inline void Delete(int i)  {  PList::Remove(i);  }
      template <class T>  inline int IndexOf(const T& v) const {  
        return PList::IndexOfComparable(v);
      }
    };

EndEsdlNamespace()
#endif
