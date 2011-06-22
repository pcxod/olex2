/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef olx_sdl_strlist_H
#define olx_sdl_strlist_H
#include "typelist.h"
#include "estrlist.h"
#include "talist.h"
#include "egc.h"
#include "tptrlist.h"
#include "datastream.h"
#undef GetObject

BeginEsdlNamespace()

class TEFile;
template <class SC> struct TSingleStringWrapper  {
  SC String;
  TSingleStringWrapper() {  }
  template <class T> TSingleStringWrapper(const T& str) : String(str)  {  }
  virtual ~TSingleStringWrapper() {  ;  }
};

template <class T, bool CaseInsensetive>  class TStringWrapperComparator  {
public:
  static inline int Compare(const T* A, const T* B )  {
   return (CaseInsensetive) ? A->String.Comparei( B->String ) :
                              A->String.Compare( B->String );
  }
};
// string class, string container class
template <class SC, class T> class TTStrList : public IEObject {
protected:
  TPtrList<T> Strings;
  template <class StrClass> size_t FindIndexOf(const StrClass& Str, bool CI) const  {
    if( CI )  {
      for( size_t i=0; i < Count(); i++ )
        if( Strings[i]->String.Equalsi(Str) )
          return i;
    }
    else  {
      for( size_t i=0; i < Count(); i++ )
        if( Strings[i]->String.Equals(Str) )
          return i;
    }
    return InvalidIndex;
  }
public:
  TTStrList()  { }

  template <class SC1, class T1> TTStrList(const TTStrList<SC1,T1>& list)  {
    Strings.SetCapacity( list.Count() );
    for( size_t i=0; i < list.Count(); i++ )
      Add(list[i]);
  }
  TTStrList(const TTStrList& list)  {
    Strings.SetCapacity( list.Count() );
    for( size_t i=0; i < list.Count(); i++ )
      Add(list[i]);
  }
  TTStrList(size_t count)  {
    Strings.SetCapacity( count );
    for( size_t i=0; i < count; i++ )
      Add(EmptyString());
  }
  // creates a list with strtok entries in it
  TTStrList(const SC& string, const SC& sep)  {
    Strtok(string, sep);
  }

  TTStrList(const SC& string, olxch sep, bool skip_sequences=true)  {
    Strtok(string, sep, skip_sequences);
  }
  virtual ~TTStrList()  {  Clear();  }
  TTStrList& SetCount(size_t nc)  {
    if( nc < Strings.Count() )  {
      for( size_t i=nc; i < Strings.Count(); i++ )
        delete Strings[i];
      Strings.SetCount(nc);
    }
    else  {
      while( Strings.Count() < nc )
        Add(EmptyString());
    }
    return *this;
  }

  inline void SetCapacity(size_t cap)   {  Strings.SetCapacity(cap);  }
  inline SC& operator [] (size_t i) const {  return Strings[i]->String;   }
  inline SC& GetString(size_t i)  const {  return Strings[i]->String;   }
  inline T& GetLast() const {  return *Strings.GetLast();   }
  inline SC& GetLastString() const {  return Strings.GetLast()->String;   }
  inline size_t Count() const {  return Strings.Count();  }
  inline bool IsEmpty() const {  return Strings.IsEmpty();  }
  inline SC& Add()  {  return Strings.Add(new T)->String;  }
  inline SC& Add(const SC& str)  {  return Strings.Add(new T(str))->String;  }
  inline SC& Add(const char* str)  {  return Strings.Add(new T(str))->String;  }
  inline SC& Add(const wchar_t* str)  {  return Strings.Add(new T(str))->String;  }

  inline TTStrList& operator << (const SC& str)  {  Strings.Add(new T(str));  return *this;  }
  inline TTStrList& operator << (const char* str)  {  Strings.Add(new T(str));  return *this;  }
  inline TTStrList& operator << (const wchar_t* str)  {  Strings.Add(new T(str));  return *this;  }
  template <class SC1, class T1> 
  TTStrList& operator << (const TTStrList<SC1,T1>& list)  {
    Strings.SetCapacity( Count() + list.Count() );
    for( size_t i=0; i < list.Count(); i++ )
      Add( list[i] );
    return *this;
  }

  inline SC& Insert(size_t i)  {  return Strings.Insert(i, new T)->String;  }
  inline SC& Insert(size_t i, const SC& S)  {  return Strings.Insert(i, new T(S))->String;  }
  inline SC& Insert(size_t i, const char* S)  {  return Strings.Insert(i, new T(S))->String;  }
  inline SC& Insert(size_t i, const wchar_t* S)  {  return Strings.Insert(i, new T(S))->String;  }
  template <class SC1, class T1> 
  inline void Insert(size_t i, const TTStrList<SC1,T1>& list)      {
    if( list.IsEmpty() )  return;
    Strings.Insert(i, list.Count() );
    for( size_t j=0; j < list.Count(); j++ )
      Strings[i+j] = new T(list[j]);
  }
  inline void Swap(size_t i, size_t j)  {  Strings.Swap(i, j);  }
  inline void Move(size_t from, size_t to)  {  Strings.Move(from, to);  }
  void Rearrange(const TSizeList& indexes)  {  Strings.Rearrange(indexes);  }
  void Clear()  {
    for( size_t i=0; i < Strings.Count(); i++ )
      delete Strings[i];
    Strings.Clear();
  }
  inline void Delete(size_t i)  {
    delete Strings[i];
    Strings.Delete(i);
  }
  void DeleteRange(size_t from, size_t count)  {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, from, 0, Strings.Count());
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, from+count, 0, Strings.Count()+1);
#endif
    for( size_t i=0; i < count; i++ )
      delete Strings[from+i];
    Strings.DeleteRange(from, count);
  }

  TTStrList& SubList(size_t from, size_t count, TTStrList& SL) const {
#ifdef _DEBUG
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, from, 0, Strings.Count()+1);
    TIndexOutOfRangeException::ValidateRange(__POlxSourceInfo, from+count, 0, Strings.Count()+1);
#endif
    SL.Strings.SetCapacity(SL.Count()+count);
    for( size_t i=0; i < count; i++ )
      SL.Add(GetString(i+from));
    return SL;
  }

  TTStrList SubListFrom(size_t offset) const {
    TTStrList SL;
    return SubList(offset, Strings.Count()-offset, SL);
  }

  TTStrList SubListTo(size_t to) const {
    TTStrList SL;
    return SubList(0, to, SL);
  }

  template <class SC1, class T1> TTStrList& Assign(const TTStrList<SC1,T1>& S)  {
      Clear();
      for( size_t i=0; i < S.Count(); i++ )
        Add(S.GetString(i));
      return *this;
    }

  TTStrList& AddList(const TTStrList& S)  {
    for( size_t i=0; i < S.Count(); i++ )
      Add(S.GetString(i));
    return *this;
  }

  TTStrList& TrimWhiteCharStrings(bool leading=true, bool trailing=true)  {
    if( IsEmpty() )  return *this;
    size_t start = 0, end = Count()-1;
    if( trailing )  {
      while( start < end && GetString(start).IsWhiteCharString() ) start++;
      if( start >= Count() )  {  Clear();  return *this;  }
      for( size_t i=0; i < start; i++ )  {
        delete Strings[i];
        Strings[i] = NULL;
      }
    }
    if( trailing )  {
      while( end > start && GetString(end).IsWhiteCharString() )  end--;
      for( size_t i=end+1; i < Count(); i++ )  {
        delete Strings[i];
        Strings[i] = NULL;
      }
    }
    if( start == 0 && end == Count()-1 )  return *this;
    Strings.Pack();
    return *this;
  }

  TTStrList& CombineLines(const SC& LineContinuationDel)  {
    for( size_t i=0; i < Count(); i++ )  {
      if( GetString(i).EndsWith(LineContinuationDel) )  {
        GetString(i).Delete( GetString(i).Length()-LineContinuationDel.Length(), LineContinuationDel.Length());
        if( (i+1) < Count() )  {
          GetString(i) << GetString(i+1);
          Delete(i+1);
          i--;
          continue;
        }
      }
    }
    return *this;
  }
  // this removes empty strings from the list
  TTStrList& Pack()  {
    for( size_t i=0; i < Count(); i++ )  {
      if( Strings[i]->String.IsEmpty() )  {
        delete Strings[i];
        Strings[i] = NULL;
      }
    }
    Strings.Pack();
    return *this;
  }
  template <class StrClass>
  inline size_t IndexOf(const StrClass& C) const {  return FindIndexOf(C, false);  }
  template <class StrClass>
  inline size_t IndexOfi(const StrClass& C) const {  return FindIndexOf(C, true);  }
  
  template <class StrClass>
  size_t FindIndexes(const StrClass& C, TSizeList& rv, bool CI) const {
    size_t cc = rv.Count();
    for( size_t i=0; i < Count(); i++ )
      if( Strings[i]->String.Compare(C, CI) == 0 )
          rv.Add(i);
    return rv.Count() - cc;
  }
  SC Text(const SC& Sep, size_t start=InvalidIndex, size_t end = InvalidIndex) const  {
    if( start == InvalidIndex )  start = 0;
    if( end == InvalidIndex )    end = Strings.Count();
    size_t tc=1, slen = Sep.Length();
    for( size_t i=start; i < end; i++ )  {
      tc += Strings[i]->String.Length();
      tc += slen;
    }
    SC E(EmptyString(), tc);
    for( size_t i=start; i < end; i++ )  {
      E << Strings[i]->String;
      if( i < end-1 ) // skip for the last line
        E << Sep;
    }
    return E;
  }
  // convenience methods
  TTStrList& LoadFromTextArray(char *bf, size_t bf_sz, bool take_ownership)  {
    Clear();
    const olxcstr str = take_ownership ? olxcstr(olxcstr::FromExternal(bf, bf_sz))
      : olxcstr((const char*)bf, bf_sz);
    Strtok(str, '\n', false); // must preserve the new lines on Linux!!! 2008.08.17
    for( size_t i=0; i < Count(); i++ )
      if( GetString(i).EndsWith('\r') )  
        GetString(i).SetLength(GetString(i).Length()-1);
    return *this;
  }
  TTStrList& LoadFromTextStream(IInputStream& io)  {
    Clear();
    size_t fl = io.GetAvailableSizeT();
    if( fl == 0 )  return *this;
    char *bf = olx_malloc<char>(fl+1);
    io.Read(bf, fl);
    return LoadFromTextArray(bf, fl, true);
  }
  TTStrList& LoadFromFile(const olxstr& fileName)  {
    TEFile file(fileName, "rb");
    return LoadFromTextStream(file);
  }
  void operator >> (IDataOutputStream &Stream) const {
    Stream << (uint32_t)Count();
    for( size_t i=0; i < Count(); i++ )
      Stream << Strings[i]->String;
  }
  TTStrList& operator << (IDataInputStream &Stream)  {
    Clear();
    uint32_t size;
    Stream >> size;
    Strings.SetCapacity(size);
    for( uint32_t i=0; i < size; i++ )
      Stream >> Add();
    return *this;
  }
  // convinience method
  const TTStrList& SaveToTextStream(IDataOutputStream& os) const {
    if( IsEmpty() )  return *this;
    for( size_t i=0; i < Count(); i++ )
      os.Writeln(Strings[i]->String);
    return *this;
  }
  const TTStrList& SaveToFile(const olxstr& fileName) const {
    TEFile file(fileName, "wb+");
    return SaveToTextStream(file);
  }

  virtual TIString ToString() const  {   return Text(NewLineSequence()).ToString();  }

  TTStrList& operator = (const TTStrList& list)  {  return Assign(list);  }

  template <class comparator>
    void BubleSort()  {  TPtrList<T>::BubleSorter.template Sort<comparator>(Strings);  }
  void BubleSortSF(int (*f)(const T* a, const T* b) )  {
    TPtrList<T>::BubleSorter.template SortSF(Strings, f);
  }
  template <class BaseClass>
    void BubleSortMF(BaseClass& baseClassInstance, int (BaseClass::*f)(const T* a, const T* b) )  {
      TPtrList<T>::BubleSorter.template SortMF<BaseClass>(Strings, baseClassInstance, f);
    }

  template <class Comparator>
    void QuickSort()  {  TPtrList<T>::QuickSorter.template Sort<Comparator>(Strings);  }
  void QuickSortSF(int (*f)(const T* a, const T* b) )  {
    TPtrList<T>::QuickSorter.template SortSF(Strings, f);
  }
  template <class BaseClass>
    void QuickSortMF(BaseClass& baseClassInstance, int (BaseClass::*f)(const T* a, const T* b) )  {
      TPtrList<T>::QuickSorter.template SortMF<BaseClass>(Strings, baseClassInstance, f);
    }

  void QSort(bool ci)  {
    if( ci )  TPtrList<T>::QuickSorter.template Sort<TStringWrapperComparator<T,true> >(Strings);
    else      TPtrList<T>::QuickSorter.template Sort<TStringWrapperComparator<T,false> >(Strings);
  }

  size_t StrtokF(const SC& Str, const TSizeList& indexes)  {
    if( indexes.IsEmpty() )  return 0;
    size_t fLength = 0, cnt = 0;
    for( size_t i=0; i < indexes.Count(); i++ )  {
      if( indexes[i] > (Str.Length()-fLength) )
        break;
      Add(Str.SubString(fLength, indexes[i]));
      cnt++;
      fLength += indexes[i];
      if( fLength >= Str.Length() )  return cnt;
    }
    if( fLength < Str.Length() )  {
      Add(Str.SubStringFrom(fLength));
      cnt++;
    }
    return cnt;
  }
  TTStrList& Strtok(const SC& Str, olxch Sep, bool SkipSequences = true)  {
    SC Tmp(Str);
    size_t ind = Tmp.IndexOf(Sep);
    while( ind != InvalidIndex )  {
      if( ind != 0 )  // skip sequences of separators
        Add(Tmp.SubStringTo(ind));
      else if( !SkipSequences )
        Add();
      if( ind+1 >= Tmp.Length() )  {
        Tmp.SetLength(0);
        break;
      }
      while( ++ind < Tmp.Length() && Tmp.CharAt(ind) == Sep ) 
        if( !SkipSequences )
          Add(EmptyString());
      if( ind >= Tmp.Length() )  {
        Tmp.SetLength(0);
        break;
      }
      Tmp = Tmp.SubStringFrom(ind);
      ind = Tmp.IndexOf(Sep);
    }
    if( !Tmp.IsEmpty() ) // add last bit
      Add(Tmp);
    return *this;
  }
  // similar to previous implementation, the list is passed as one of the parameters
  // takes a string as a separator
  TTStrList& Strtok(const SC& Str, const SC& Sep)  {
    SC Tmp(Str);
    const size_t sepl = Sep.Length();
    size_t ind = Tmp.IndexOf(Sep);
    while( ind != InvalidIndex )  {
      Add(Tmp.SubStringTo(ind));
      if( (ind+sepl) >= Tmp.Length() )  {
        Tmp.SetLength(0);
        break;
      }
      Tmp = Tmp.SubStringFrom(ind+Sep.Length());
      ind = Tmp.IndexOf(Sep);
    }
    if( !Tmp.IsEmpty() ) // add last bit
      Add(Tmp);
    return *this;
  }

  TTStrList& Hyphenate(const SC& String, size_t Width, bool Force=true)  {
    if( Width == 0 )
      throw TInvalidArgumentException(__OlxSourceInfo, "width");
    SC Str(String);
    while( Str.Length() > Width )  {
      size_t spi = Str.LastIndexOf(' ', Width);
      if( spi != InvalidIndex && spi > 0 )  {
        Add( Str.SubStringTo(spi) );
        Str.Delete(0, spi+1); // remove the space
      }
      else  {
        if( Force )  {
          Add( Str.SubStringTo(Width) );
          Str.Delete(0, Width); // remove the space
        }
      }
    }
    if( !Str.IsEmpty() )  
      Add(Str);
    return *this;
  }
};


template <class SC, typename OC> 
struct TPrimitiveStrListData : public TSingleStringWrapper<SC>  {
  OC Object;
  TPrimitiveStrListData() : Object(0) {  }
  template <class T> TPrimitiveStrListData(const T& str, const OC& obj = 0)
    : TSingleStringWrapper<SC>(str), Object(obj) {  }
  virtual ~TPrimitiveStrListData()  {  }
};

template <class SC, typename OC> struct TObjectStrListData : public TSingleStringWrapper<SC>  {
  OC Object;
  TObjectStrListData()  {  }
  template <class S> TObjectStrListData(const S& str ) : TSingleStringWrapper<SC>(str)  {  }
  template <class S> TObjectStrListData(const S& str, const OC& obj )
    : TSingleStringWrapper<SC>(str), Object(obj)  {  }
  virtual ~TObjectStrListData()  {  }
};


template <class SC, class OC, class GC> class TTOStringList: public TTStrList<SC,GC>  {
  typedef TTStrList<SC,GC> PList;
public:
  // creates empty list
  TTOStringList()  {}
  TTOStringList(size_t count) : TTStrList<SC,GC>(count)  {}
  // copy constructor
  template <class SC1, class T1>
    TTOStringList(const TTStrList<SC1,T1>& list)  {
      PList::Strings.SetCapacity(list.Count());
      for( size_t i=0; i < list.Count(); i++ )
        Add(list[i]);
    }

  TTOStringList(const TTOStringList& list)  {
    PList::Strings.SetCapacity( list.Count() );
    for( size_t i=0; i < list.Count(); i++ )
      Add(list[i], list.GetObject(i));
  }
  // creates a list with strtok entries in it
  TTOStringList(const SC& string, const SC& sep, TTypeList<OC>* objects = NULL) :
    TTStrList<SC,GC>(string, sep)
  {
    if( objects != NULL )  {
      for( size_t i=0; i < objects->Count(); i++ )  {
        if( (i+1) >= PList::Count() )  break;
        GetObject(i) = objects->GetItem(i);
      }
    }
  }
  // creates a list with strtok entries in it
  TTOStringList(const SC& string, char sep, TTypeList<OC>* objects = NULL) :
    TTStrList<SC,GC>(string, sep)
  {
    if( objects != NULL )  {
      for( size_t i=0; i < objects->Count(); i++ )  {
        if( (i-1) > PList::Count() )  break;
        GetObject(i) = objects->GetItem(i);
      }
    }
  }
  virtual ~TTOStringList()  {}

  TTOStringList& SubList(size_t offset, size_t count, TTOStringList& SL) const {
    for( size_t i=offset; i < offset+count; i++ )
      SL.Add(PList::GetString(i), GetObject(i));
    return SL;
  }

  TTOStringList SubListFrom(size_t offset) const {
    TTOStringList SL;
    return SubList(offset, PList::Count()-offset, SL);
  }

  TTOStringList SubListTo(size_t to) const {
    TTOStringList SL;
    return SubList(0, to, SL);
  }

  template <class SC1, class T1> TTOStringList& Assign(const TTStrList<SC1,T1>& S)  {
      PList::Clear();
      PList::SetCapacity(S.Count());
      for( size_t i=0; i < S.Count(); i++ )
        Add(S[i]);
      return *this;
    }

   TTOStringList& Assign(const TTOStringList& S)  {
    PList::Clear();
    PList::AddList(S);
    return *this;
  }

  TTOStringList AddList(const TTOStringList& S)  {
    for( size_t i=0; i < S.Count(); i++ )
      Add(S[i], S.GetObject(i));
    return *this;
  }

  GC& Add()  {  return *PList::Strings.Add(new GC);  }
  GC& Add(const SC& S)  {  return *PList::Strings.Add(new GC(S));  }
  GC& Add(const SC& S, const OC& Object)  {  return *PList::Strings.Add(new GC(S,Object));  }
  GC& Insert(size_t i, const SC& S)  {  return *PList::Strings.Insert(i, new GC(S));  }
  GC& Insert(size_t i, const SC& S, const OC& O)  {
    return *PList::Strings.Insert(i, new GC(S,O) );
  }
  inline GC& Set(size_t i, const SC& S, const OC& O)  {  
    delete PList::Strings[i];
    return *(PList::Strings[i] = new GC(S,O));
  }

  inline OC& GetObject(size_t i) const { return PList::Strings[i]->Object;  }

  TTOStringList& operator = (const TTOStringList& list)  {  return Assign(list);  }

  size_t IndexOfObject(const OC& C) const {
    for( size_t i=0; i < PList::Count(); i++ )
      if( PList::Strings[i]->Object == C )
        return i;
    return InvalidIndex;
  }
  // the find function with this signature work only for objects; for pointers it
  // causes a lot of trouble
  template <class StrClass> const OC& FindObject(const StrClass& Name) const {
    size_t in = PList::IndexOf(Name);
    return (in != InvalidIndex) ? PList::Strings[in]->Object : *(OC*)NULL;
  }

  template <class StrClass> OC* FindObjecti(const StrClass& Name) const {
    size_t in = PList::IndexOfi(Name);
    return (in != InvalidIndex) ? &PList::Strings[in]->Object : (OC*)NULL;
  }
};

template <class SC, typename OC> class TStrPObjList: 
   public TTOStringList<SC,OC,TPrimitiveStrListData<SC,OC> >
{
  typedef TTOStringList<SC,OC,TPrimitiveStrListData<SC,OC> > PList;
public:
  TStrPObjList()  {}
  TStrPObjList(size_t count) : PList(count)  { 
    for( size_t i=0; i < PList::Count(); i++ )
      PList::GetObject(i) = NULL;
  }

  template <class SC1, class T1> TStrPObjList(const TTStrList<SC1,T1>& list) : 
    PList(list) {}

  TStrPObjList(const TTOStringList<SC,OC,TPrimitiveStrListData<SC,OC> >& list) :
    PList(list)  {}

  TStrPObjList(const SC& string, const SC& sep, TTypeList<OC>* objects = NULL) :
    PList(string, sep, objects)  {}

  TStrPObjList(const SC& string, char sep, TTypeList<OC>* objects = NULL) :
    PList(string, sep, objects)  {}

  template <class StrClass> inline OC FindObject(const StrClass& Name) const {
    const size_t in = PList::IndexOf(Name);
    return (in != InvalidIndex) ? PList::Strings[in]->Object : NULL;
  }

  template <class StrClass> OC FindObjecti(const StrClass& Name) const {
    const size_t in = PList::IndexOfi(Name);
    return (in != InvalidIndex) ? PList::Strings[in]->Object : NULL;
  }
};

typedef TStrPObjList<olxstr, IEObject*> TStrObjList;
typedef TStrPObjList<olxcstr, IEObject*> TCStrObjList;
typedef TStrPObjList<olxwstr, IEObject*> TWStrObjList;

typedef TTOStringList<olxstr, olxstr, TObjectStrListData<olxstr,olxstr> > TStrStrList;
typedef TTOStringList<olxcstr, olxcstr, TObjectStrListData<olxcstr,olxcstr> > TCStrCStrList;
typedef TTOStringList<olxwstr, olxwstr, TObjectStrListData<olxwstr,olxwstr> > TWStrWStrList;

typedef TTStrList<olxstr, TSingleStringWrapper<olxstr> > TStrList;
typedef TTStrList<olxcstr, TSingleStringWrapper<olxcstr> > TCStrList;
typedef TTStrList<olxwstr, TSingleStringWrapper<olxwstr> > TWStrList;

EndEsdlNamespace()
#endif
