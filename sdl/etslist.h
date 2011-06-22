/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_tslist_H
#define __olx_sdl_tslist_H
#include "talist.h"
#undef GetObject
BeginEsdlNamespace()

template <class ComparableClass, class Comparator>
class TSimpleSortedListEntry {
  ComparableClass _Comparable;
public:
  TSortedListEntry(const ComparableClass& c) : _Comparable(c) {  }
  TSortedListEntry(const TSortedListEntry& entry) :
    _Comparable(entry._Comparable )  {  }
  TSortedListEntry()  {  }
  virtual ~TSortedListEntry()  {  }
  inline TSortedListEntry& operator = (const TSortedListEntry& entry)  {
    _Comparable = entry._Comparable;
    return *this;
  }
  inline const ComparableClass& GetComparable()   {  return _Comparable;  }
  inline int Compare( TSortedListEntry& entry ) const {
    return Comparator::template Compare<ComparableClass>(_Comparable, entry._Comparable );  }
  inline int Compare(const ComparableClass& entity) const {
    return Comparator::template Compare<ComparableClass>(_Comparable, entity );
  }
};

template <class T>
class TSGTSList : public IEObject  {  // simple generic template sorted list
  TArrayList<T> Data;
protected:
  int FindInsertIndex(const T& entity, int from=-1, int to=-1) {
    if( from == -1 ) from = 0;
    if( to == -1 )   to = Count()-1;
    if( to == from ) return to;
    if( (to-from) == 1 )  return from;
    int resfrom = Data[from].Compare(entity),
        resto   = Data[to].Compare(entity);
    if( resfrom == 0 )  return from;
    if( resto == 0 )    return to;
    if( resfrom < 0 && resto > 0 )  {
      int index = (to+from)/2;
      int res = Data[index].Compare(entity);
      if( res < 0 )  return FindInsertIndex(entity, index, to);
      if( res > 0 )  return FindInsertIndex(entity, from, index);
      if( res == 0 ) return index;
    }
    return -1;
  }

  int FindIndexOf(const T& entity) const {
    if( Data.Count() == 0 )  return -1;
    if( Data.Count() == 1 )  return (!Data[0].Compare(entity)) ? 0 : -1;
    int from = 0, to = Count()-1;
    if( !Data[from].Compare(entity) )  return from;
    if( !Data[to].Compare(entity)   )  return to;
    while( true ) {
      int index = (to+from)/2;
      int res = Data[index].Compare(entity);
      if( index == from || index == to)  return -1;
      if( res < 0 )  from = index;
      else
        if( res > 0 )  to  = index;
        else
          if( res == 0 )  return index;
    }
    return -1;
  }
public:
  TTSList()  {}
  TTSList(const TTSList& list)  {
    Data.SetCount( list.Count() );
    for( int i=0; i < list.Data.Count(); i++ )
      Data[i] = list.Data[i];
  }

  virtual ~TTSList()  {  }
  inline void Clear()  {  Data.Clear();  }
  int IndexOf(const T& cmpbl) const {  return FindIndexOf(cmpbl);  }
  // retrives indexes of all entries with same key and returns the number of added entries
  int GetIndexes(const T& cmpbl, TIntList& il)  {
    if( Data.IsEmpty() )  return 0;
    if( Data.Count() == 1 )  {
      if( Data[0].Compare(cmpbl) != 0 )  return 0;
      il.Add( 0 );
      return 1;
    }

    int index =  IndexOf(cmpbl);
    if( index == -1 )  return 0;
    il.Add( index );
    int i = index+1, addedc = 1;
    // go forward
    while( i < Data.Count() && (Data[i].Compare(cmpbl) == 0) )  {
      il.Add( i );
      i++;
      addedc++;
    }
    // go backwards
    i = index-1;
    while( i >= 0 && (Data[i].Compare(cmpbl) == 0) )  {
      il.Add( i );
      i--;
      addedc++;
    }
    return addedc;
  }
//..............................................................................
  inline void Remove(int i)                 {  Data.Delete(i);  }
  inline int Count()                  const {  return Data.Count(); }
  inline bool IsEmpty()               const {  return Data.IsEmpty();  }
  inline const T& GetItem(int index)  const {  return Data[index].GetComparable();  }
  inline T& Last()                    const {  return Data.Last();  }
  inline void SetCapacity(int v)            {  Data.SetCapacity(v);  }
  inline void SetIncrement(int v)           {  Data.SetIncrement(v);  }
  inline const T& operator [] (int i) const { return Data[index].GetComparable();  }
//..............................................................................
  void Add(const T& Cmpbl)  {
    if( Data.IsEmpty() )  {  Data.Add(Cmpbl);  return;  }
    if( Data.Count() == 1 )  {
      if(  Data[0].Compare(Cmpbl) < 0 )
        Data.Add(Cmpbl);
      else
        Data.Insert(0, Cmpbl);
      return;
    }
    // smaller than the first
    if( Data[0].Compare(Cmpbl) >= 0 )    {  Data.Insert(0, Cmpbl);  return; }
    // larger than the last
    if( Data[Data.Count()-1].Compare(Cmpbl) <= 0 )  {  Data.Add(Cmpbl);  return; }
    // an easy case then with two items
    if( Data.Count() == 2 )  {  Data.Insert(1, Cmpbl);  return; }
    int pos = FindInsertIndex( Cmpbl );
    if( pos == -1 )
      throw TIndexOutOfRangeException(__OlxSourceInfo, pos, 0, Count()-1);
    Data.Insert(pos+1, Cmpbl);
  }
};
//..............................................................................
template <class CmpblC, class CmprC, typename ObjT>
class TObjectSortedListEntry : public TSortedListEntry<CmpblC,CmprC>  {
  ObjC _Object;
public:
  TObjectSortedListEntry(const CmpblC &cmpbl, const ObjT &obj) :
    TSortedListEntry<CmpblC,CmprC>(cmpbl), Obj(obj)  {}
  TObjectSortedListEntry(const TObjectSortedListEntry &entry) :
    TSortedListEntry<CmpblC,CmprC>(entry), Obj(entry._Object)  {}
  inline ObjT& Object()  {  rteurn _Object;  }
  inline const ObjT& GetObject()  const  {  return _Object;  }
};
//..............................................................................
template <class CmpblC, class CmprC, typename ObjT>  // object generic template sorted list 
class TOGTSList : public TSGTSList< TObjectSortedListEntry<CmpblC, CmprC, ObjT> >  {
public:
  TOGTSList(const TOGTSList &toCopy) :
    TSGTSList< TObjectSortedListEntry<CmpblC, CmprC, ObjT> >(toCopy)  {  }

  inline ObjT& operator [] (const CmpblC &key)  {
    int i = IndexOf(key);
    if( key == -1 )  ;
    return
  }
};
//..............................................................................
  // to be used with objects, having Compare operator
  template <typename ComparableClass>
    class TTSList : public TGTSt<ComparableClass, TComparableComparator>
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
  template <typename ObjectClass, bool caseinsensitive>
    class TSStrPObjList : public TSTypeList<TEString, ObjectClass, TEStringComparator<caseinsensitive> >  {
    public:
      inline const TEString& GetString(int i)    const {
        return TSTypeList<TEString, ObjectClass, TEStringComparator<caseinsensitive> >::GetComparable(i);
      }
      inline TEString& String(int i)                   {
        return TSTypeList<TEString, ObjectClass, TEStringComparator<caseinsensitive> >::GetComparable(i);
      }

      inline void Add( const TEString& s, const ObjectClass& v = *(ObjectClass*)NULL )  {
        TSTypeList<TEString, ObjectClass, TEStringComparator<caseinsensitive> >::Add(s, v);
      }
      inline void Delete(int i)  {
        TSTypeList<TEString, ObjectClass, TEStringComparator<caseinsensitive> >::Remove(i);
      }
      inline int IndexOf(const TEString& v)  const  {
        return TSTypeList<TEString, ObjectClass, TEStringComparator<caseinsensitive> >::IndexOfComparable(v);
      }

      inline ObjectClass  operator [] (const TEString& Comparable) const   {
        int ind = TSTypeList<TEString, ObjectClass, TEStringComparator<caseinsensitive> >::IndexOfComparable(Comparable);
        return (ind >= 0)  ? TSTypeList<TEString, ObjectClass, TEStringComparator<caseinsensitive> >::GetObject(ind) : NULL;
      }
    };
//..............................................................................
//..............................................................................
  // just a string to obj specialisation
  template <typename ObjectClass, bool caseinsensitive>
    class TSStrObjList : public TSTypeList<TEString, ObjectClass, TEStringComparator<caseinsensitive> >  {
    public:
      inline const TEString& GetString(int i)    const {
        return TSTypeList<TEString, ObjectClass, TEStringComparator<caseinsensitive> >::GetComparable(i);
      }
      inline TEString& String(int i)                   {
        return TSTypeList<TEString, ObjectClass, TEStringComparator<caseinsensitive> >::GetComparable(i);
      }

      inline void Add( const TEString& s, const ObjectClass& v = *(ObjectClass*)NULL )  {
        TSTypeList<TEString, ObjectClass, TEStringComparator<caseinsensitive> >::Add(s, v);
      }
      inline void Delete(int i)  {
        TSTypeList<TEString, ObjectClass, TEStringComparator<caseinsensitive> >::Remove(i);
      }
      inline int IndexOf(const TEString& v)  const  {
        return TSTypeList<TEString, ObjectClass, TEStringComparator<caseinsensitive> >::IndexOfComparable(v);
      }
    };
//..............................................................................
//..............................................................................
  // string - string map specialisation ...
  template <bool caseinsensitive>
    class TSStrStrList : public TSTypeList<TEString, TEString, TEStringComparator<caseinsensitive> >  {
    public:
      inline const TEString& GetString(int i)    const {
        return TSTypeList<TEString, TEString, TEStringComparator<caseinsensitive> >::GetComparable(i);
      }
      inline TEString& String(int i)                   {
        return TSTypeList<TEString, TEString, TEStringComparator<caseinsensitive> >::GetComparable(i);
      }

      inline void Add( const TEString& s, const TEString & v = EmptyString )  {
        TSTypeList<TEString, TEString, TEStringComparator<caseinsensitive> >::Add(s, v);
      }
      inline void Delete(int i)  {
        TSTypeList<TEString, TEString, TEStringComparator<caseinsensitive> >::Remove(i);
      }
      inline int IndexOf(const TEString& v)  const  {
        return TSTypeList<TEString, TEString, TEStringComparator<caseinsensitive> >::IndexOfComparable(v);
      }
    };

EndEsdlNamespace()
#endif
