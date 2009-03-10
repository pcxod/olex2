//---------------------------------------------------------------------------
#ifndef eSTListH
#define eSTListH
#include "tptrlist.h"

#undef GetObject

BeginEsdlNamespace()
//---------------------------------------------------------------------------

template <class ComparableClass, class ObjectClass, class Comparator>
  class TSortedListEntry {
  ComparableClass FComparable;
  ObjectClass FObject;
  public:
    TSortedListEntry(const ComparableClass& c, const ObjectClass& o )  {
      FComparable = c;
      FObject = o;
    }
    TSortedListEntry(const TSortedListEntry& entry)  {
      FComparable = entry.FComparable;
      FObject = entry.FObject;
    }
    TSortedListEntry()  {  }
    virtual ~TSortedListEntry()  {  }

    inline const TSortedListEntry& operator = (const TSortedListEntry& entry)  {
      FComparable = entry.FComparable;
      FObject = entry.FObject;
      return entry;
    }
    inline ComparableClass& Comparable()       {  return FComparable;  }
    inline ObjectClass& Object()               {  return FObject;  }

    inline int Compare( TSortedListEntry& entry ) const {
      return Comparator::template Compare<ComparableClass>(FComparable, entry.FComparable );  }

    inline int Compare( const ComparableClass& entity ) const {
      return Comparator::template Compare<ComparableClass>(FComparable, entity );  }

};
//---------------------------------------------------------------------------
template <class A, class B, class ComparatorType>
  class TSTypeList : public IEObject  {
    // no arraylist - inserts are too 'heavy' 
    TPtrList< TSortedListEntry<A,B,ComparatorType> > Data;
  protected:
    int FindInsertIndex(const A& entity, int from=-1, int to=-1) {
      if( from == -1 ) from = 0;
      if( to == -1 )   to = Count()-1;
      if( to == from ) return to;
      if( (to-from) == 1 )  return from;
      int resfrom = Data[from]->Compare(entity),
          resto   = Data[to]->Compare(entity);
      if( resfrom == 0 )  return from;
      if( resto == 0 )    return to;
      if( resfrom < 0 && resto > 0 )  {
        int index = (to+from)/2;
        int res = Data[index]->Compare(entity);
        if( res < 0 )  return FindInsertIndex(entity, index, to);
        if( res > 0 )  return FindInsertIndex(entity, from, index);
        if( res == 0 ) return index;
      }
      return -1;
    }

    int FindIndexOf(const A& entity) const {
      if( Data.Count() == 0 )  return -1;
      if( Data.Count() == 1 )  return (!Data[0]->Compare(entity)) ? 0 : -1;
      int from = 0, to = Count()-1;
      if( !Data[from]->Compare(entity) )  return from;
      if( !Data[to]->Compare(entity)   )  return to;
      while( true ) {
        int index = (to+from)/2;
        int res = Data[index]->Compare(entity);
        if( index == from || index == to)  return -1;
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
      Data[i] = new TSortedListEntry<A,B,ComparatorType>(*list.Data[i]);
  }

  virtual ~TSTypeList()  {  Clear();  }
//..............................................................................
  inline void Clear()  {
    for( int i=0; i < Data.Count(); i++ )
      delete Data[i]; 
    Data.Clear();
  }
//..............................................................................
  int IndexOfComparable(const A& cmpbl) const {  return FindIndexOf(cmpbl);  }
//..............................................................................
  int IndexOfObject(const B& v)  const  {
    for( int i=0; i < Data.Count(); i++ )
      if( Data[i]->Object() == v )  return i;
    return -1;
  }
//..............................................................................
  // retrives indexes of all entries with same key and returns the number of added entries
  int GetIndexes(const A& cmpbl, TIntList& il)  {
    if( Data.IsEmpty() )  return 0;
    if( Data.Count() == 1 )  {
      if( Data[0]->Compare(cmpbl) != 0 )  return 0;
      il.Add( 0 );
      return 1;
    }

    int index =  IndexOfComparable(cmpbl);
    if( index == -1 )  return 0;
    il.Add( index );
    int i = index+1, addedc = 1;
    // go forward
    while( i < Data.Count() && (Data[i]->Compare(cmpbl) == 0) )  {
      il.Add( i );
      i++;
      addedc++;
    }
    // go backwards
    i = index-1;
    while( i >= 0 && (Data[i]->Compare(cmpbl) == 0) )  {
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
  inline void Remove(size_t i)   {  delete Data[i];  Data.Delete(i);  }
//..............................................................................
  inline int Count()    const {  return Data.Count(); }
//..............................................................................
  inline bool IsEmpty() const {  return Data.IsEmpty();  }
//..............................................................................
  // TODO: a check has to be done if the value changed -> Resort
  inline A&  GetComparable(int index)   const {  return Data[index]->Comparable();  }
//..............................................................................
  inline const B&  GetObject(int index) const {  return Data[index]->Object();  }
//..............................................................................
  inline B&  Object(int index)                {  return Data[index]->Object();  }
//..............................................................................
  inline TSortedListEntry<A,B,ComparatorType>&  Last() {  return *Data.Last();  }
//..............................................................................
  inline void SetCapacity(size_t v)              {  Data.SetCapacity(v);  }
//..............................................................................
  inline void SetIncrement(size_t v)             {  Data.SetIncrement(v);  }
//..............................................................................
  inline B&  operator [] (const A& Comparable) const   {
    int ind = IndexOfComparable(Comparable);
    if( ind >= 0 )  return Data[ind]->Object();
    throw TFunctionFailedException(__OlxSourceInfo, "no object at specified location" );
  }
//..............................................................................
  inline B&  Item(const A& Comparable) const   {
    int ind = IndexOfComparable(Comparable);
    if( ind >= 0 )  return Data[ind]->Object();
    throw TFunctionFailedException(__OlxSourceInfo, "no object at specified location" );
  }
//..............................................................................
  TSortedListEntry<A,B,ComparatorType>& Add( const A& Comparable, const B& Object )  {
    TSortedListEntry<A,B,ComparatorType> *entry = new TSortedListEntry<A,B,ComparatorType>(Comparable, Object);
    int pos;
    if( Data.IsEmpty() )  {
      Data.Add( entry);
      return *entry;
    }
    if( Data.Count() == 1 )  {
      if(  Data[0]->Compare(*entry) < 0 )  Data.Add(entry);
      else                                 Data.Insert(0, entry);
      return *entry;
    }
    // smaller than the first
    if( Data[0]->Compare(*entry) >= 0 )     {  Data.Insert(0, entry);  return *entry; }
    // larger than the last
    if( Data.Last()->Compare(*entry) <= 0 )  {  Data.Add(entry);  return *entry; }
    // an easy case then with two items
    if( Data.Count() == 2 )  {  Data.Insert(1, entry);  return *entry; }
    pos = FindInsertIndex( Comparable );
    if( pos == -1 )
      throw TIndexOutOfRangeException(__OlxSourceInfo, pos, 0, Count()-1);
    Data.Insert(pos+1, entry);
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
    public:
      inline const olxstr& GetString(int i)    const {
        return TSTypeList<SC, ObjectClass, olxstrComparator<caseinsensitive> >::GetComparable(i);
      }
      inline SC& String(int i)                   {
        return TSTypeList<SC, ObjectClass, olxstrComparator<caseinsensitive> >::GetComparable(i);
      }

      inline void Add( const SC& s, const ObjectClass& v = *(ObjectClass*)NULL )  {
        TSTypeList<SC, ObjectClass, olxstrComparator<caseinsensitive> >::Add(s, v);
      }
      inline void Delete(int i)  {
        TSTypeList<SC, ObjectClass, olxstrComparator<caseinsensitive> >::Remove(i);
      }
      inline int IndexOf(const SC& v)  const  {
        return TSTypeList<SC, ObjectClass, olxstrComparator<caseinsensitive> >::IndexOfComparable(v);
      }

      inline ObjectClass  operator [] (const olxstr& Comparable) const   {
        int ind = TSTypeList<SC, ObjectClass, olxstrComparator<caseinsensitive> >::IndexOfComparable(Comparable);
        return (ind >= 0)  ? TSTypeList<SC, ObjectClass, olxstrComparator<caseinsensitive> >::GetObject(ind) : NULL;
      }
    };
//..............................................................................
//..............................................................................
  // just a string to obj specialisation
  template <class SC, typename ObjectClass, bool caseinsensitive>
    class TSStrObjList : public TSTypeList<SC, ObjectClass, olxstrComparator<caseinsensitive> >  {
    public:
      inline const SC& GetString(int i)    const {
        return TSTypeList<SC, ObjectClass, olxstrComparator<caseinsensitive> >::GetComparable(i);
      }
      inline olxstr& String(int i)                   {
        return TSTypeList<SC, ObjectClass, olxstrComparator<caseinsensitive> >::GetComparable(i);
      }

      inline TSortedListEntry<SC,ObjectClass,olxstrComparator<caseinsensitive> >& 
        Add( const SC& s, const ObjectClass& v = *(ObjectClass*)NULL )  {
          return TSTypeList<SC, ObjectClass, olxstrComparator<caseinsensitive> >::Add(s, v);
      }
      inline void Delete(int i)  {
        TSTypeList<SC, ObjectClass, olxstrComparator<caseinsensitive> >::Remove(i);
      }
      inline int IndexOf(const SC& v)  const  {
        return TSTypeList<SC, ObjectClass, olxstrComparator<caseinsensitive> >::IndexOfComparable(v);
      }
    };
//..............................................................................
//..............................................................................
  // string - string map specialisation ...
  template <class SC, bool caseinsensitive>
    class TSStrStrList : public TSTypeList<SC, SC, olxstrComparator<caseinsensitive> >  {
    public:
      inline const olxstr& GetString(int i)    const {
        return TSTypeList<SC, SC, olxstrComparator<caseinsensitive> >::GetComparable(i);
      }
      inline SC& String(int i)                   {
        return TSTypeList<SC, SC, olxstrComparator<caseinsensitive> >::GetComparable(i);
      }

      inline void Add( const SC& s, const olxstr & v = EmptyString )  {
        TSTypeList<SC, SC, olxstrComparator<caseinsensitive> >::Add(s, v);
      }
      inline void Delete(int i)  {
        TSTypeList<SC, SC, olxstrComparator<caseinsensitive> >::Remove(i);
      }
      inline int IndexOf(const SC& v)  const  {
        return TSTypeList<SC, SC, olxstrComparator<caseinsensitive> >::IndexOfComparable(v);
      }
    };

EndEsdlNamespace()
#endif
