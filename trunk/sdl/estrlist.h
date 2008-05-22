//----------------------------------------------------------------------------//
// namespace TEObjects: lists
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#ifndef estrlistH
#define estrlistH
//---------------------------------------------------------------------------
#include "typelist.h"
#include "estrlist.h"
#include "talist.h"
#include "egc.h"
#include "tptrlist.h"
#include "datastream.h"

BeginEsdlNamespace()
  class TEFile;
template <class SC> class TSingleStringWrapper  {
  SC ThisString;
public:
  TSingleStringWrapper() {  ;  }
  virtual ~TSingleStringWrapper() {  ;  }
  TSingleStringWrapper(const SC& str)      {  ThisString = str;  }
  TSingleStringWrapper(const char* str)    {  ThisString = str;  }
  TSingleStringWrapper(const wchar_t* str) {  ThisString = str;  }
  inline SC& String() {  return ThisString;  }
  inline const SC& GetString()       const {  return ThisString;  }
};

template <class T, bool CaseInsensetive>  class TStringWrapperComparator  {
public:
  static inline int Compare(const T* A, const T* B )  {
   return (CaseInsensetive) ? A->GetString().Comparei( B->GetString() ) :
                              A->GetString().Compare( B->GetString() );
  }
};
// string class, string container class
template <class SC, class T> class TTStrList : public IEObject {
protected:
  TPtrList<T> Strings;
  int FindIndexOf(const SC &Str, bool CI) const  {
    if( CI )  {
      for( int i=0; i < Count(); i++ )
        if( Strings[i]->GetString().Comparei(Str) == 0 )
          return i;
    }
    else  {
      for( int i=0; i < Count(); i++ )
        if( Strings[i]->GetString().Compare(Str) == 0 )
          return i;
    }
    return -1;
  }
public:
  TTStrList()  { }

  template <class SC1, class T1>  TTStrList(const TTStrList<SC1,T1>& list)  {
    Strings.SetCapacity( list.Count() );
    for( int i=0; i < list.Count(); i++ )
      Add( list[i] );
  }
  TTStrList(const TTStrList& list)  {
    Strings.SetCapacity( list.Count() );
    for( int i=0; i < list.Count(); i++ )
      Add( list[i] );
  }
  // creates a list with strtok entries in it
  TTStrList(const SC& string, const SC& sep)  {
    Strtok(string, sep);
  }

  TTStrList(const SC& string, char sep)  {
    Strtok(string, sep);
  }
  virtual ~TTStrList()  {
     Clear();
  }
  void SetCount(int nc)  {
    if( nc < Count() )
      ;//  Strings.Shrink(nc);
    else  {
      int oldCount = Strings.Count();
      for( int i=0; i < nc-oldCount; i++ )
        Add(EmptyString);
    }
  }

  inline void SetCapacity(int cap)      {  Strings.SetCapacity(cap);  }
  inline SC& operator [] (int i)  const {  return Strings[i]->String();   }
  inline SC& String(int i)        const {  return Strings[i]->String();   }
  inline T& Last()                const {  return *Strings.Last();   }
  inline SC& LastStr()            const {  return Strings.Last()->String();   }
  inline int Count()              const {  return Strings.Count();  }
  inline bool IsEmpty()           const {  return Strings.IsEmpty();  }
  inline SC& Add(const SC& str)         {  return Strings.Add( new T(str) )->String();  }
  inline SC& Add(const char* str)       {  return Strings.Add( new T(str) )->String();  }
  inline SC& Add(const wchar_t* str)    {  return Strings.Add( new T(str) )->String();  }

  inline TTStrList<SC,T>& operator << (const SC& str)      {  Strings.Add( new T(str) );  return *this;  }
  inline TTStrList<SC,T>& operator << (const char* str)    {  Strings.Add( new T(str) );  return *this;  }
  inline TTStrList<SC,T>& operator << (const wchar_t* str) {  Strings.Add( new T(str) );  return *this;  }
  template <class SC1, class T1> 
  TTStrList<SC,T>& operator << (const TTStrList<SC1,T1>& list)  {
    Strings.SetCapacity( Count() + list.Count() );
    for( int i=0; i < list.Count(); i++ )
      Add( list[i] );
    return *this;
  }

  inline SC& Insert(int i, const SC& S)      {  return Strings.Insert(i, new T(S))->String();  }
  inline SC& Insert(int i, const char* S)    {  return Strings.Insert(i, new T(S))->String();  }
  inline SC& Insert(int i, const wchar_t* S) {  return Strings.Insert(i, new T(S))->String();  }
  template <class SC1, class T1> 
  inline void Insert(int i, const TTStrList<SC1,T1>& list)      {
    if( list.Count() == 0 )  return;
    Strings.Insert(i, list.Count() );
    for( int j=0; j < list.Count(); j++ )
      Strings[i+j] = new T(list[j]);
  }
  inline void Swap( int i, int j)               {  Strings.Swap(i, j);  }
  inline void Move( int from, int to)           {  Strings.Move(from, to);  }
  void Rearrange(const TIntList& indexes)       {  Strings.Rearrange(indexes);  }
  void Clear()                           {
    for( int i=0; i < Strings.Count(); i++ )
      delete Strings[i];
    Strings.Clear();
  }
  inline void Delete(int i)                     {
    delete Strings[i];
    Strings.Delete(i);
  }
  inline void DeleteRange(int From, int To)     {
    for( int i=From; i < To; i++ )
      delete Strings[i];
    Strings.DeleteRange(From, To);
  }

  void SubList(int offset, int count, TTStrList& SL) const  {
    for(int i=offset; i < offset+count; i++ )
      SL.Add( String(i) );
  }

  template <class SC1, class T1>
    void Assign(const TTStrList<SC1,T1>& S)  {
      Clear();
      for( int i=0; i < S.Count(); i++ )
        Add( S.String(i) );
    }

  void AddList(const TTStrList& S)  {
    for( int i=0; i < S.Count(); i++ )
      Add( S.String(i) );
  }

  void CombineLines(const SC& LineContinuationDel)  {
    for(int i=0; i < Count(); i++ )  {
      if( String(i).EndsWith(LineContinuationDel) )  {
        String(i).Delete( String(i).Length()-LineContinuationDel.Length(), LineContinuationDel.Length());
        if( (i+1) < Count() )  {
          String(i) << String(i+1);
          Delete( i+1 );
          i--;
          continue;
        }
      }
    }
  }
  // this removes empty strings from the list
  void Pack()  {
    for( int i=0; i < Count(); i++ )  {
      if( Strings[i]->GetString().IsEmpty() )  {
        delete Strings[i];
        Strings[i] = NULL;
      }
    }
    Strings.Pack();
  }

  inline int IndexOf(const SC& C) const     {  return FindIndexOf(C, false);  }
  inline int CIIndexOf(const SC& C) const   {  return FindIndexOf(C, true);  }

  int FindIndexes(const SC& C, TIntList& rv, bool CI) const  {
    int cc = rv.Count();
    for( int i=0; i < Count(); i++ )
      if( Strings[i]->GetString().Compare(C, CI) == 0 )
          rv.Add(i);
    return rv.Count() - cc;
  }

  SC Text(const SC &Sep, int start=-1, int end = -1) const  {
    if( start == -1 )  start = 0;
    if( end == -1 )    end = Strings.Count();
    size_t tc=1, slen = Sep.Length();
    for( int i=start; i < end; i++ )  {
      tc += Strings[i]->String().Length();
      tc += slen;
    }
    SC E(EmptyString, tc);
    for( int i=start; i < end; i++ )  {
      E << Strings[i]->String();
      if( i < Count()-1 ) // skip for the last line
        E << Sep;
    }
    return E;
  }
  // convinience method
  void LoadFromTextStream(IInputStream& io)  {
    int fl = io.GetSize() - io.GetPosition();
    if( fl <= 0 )  return;
    char * const&bf = new char [fl+1];
    io.Read(bf, fl);
    bf[fl] = '\0';
    Clear();
    Strtok(CString(bf, fl), '\n');
    for(int i=0; i < Count(); i++ )
      if( String(i).EndsWith('\r') )  String(i).SetLength( String(i).Length() -1 );
    delete [] bf;
  }

  void LoadFromFile(const olxstr& fileName)  {
    TEFile file(fileName, "rb");
    LoadFromTextStream(file);
  }

  void operator >> (IDataOutputStream &Stream) const  {
    Stream << (uint32_t)Count();
    for( int i=0; i < Count(); i++ )
      Stream << Strings[i]->GetString();
  }
  void operator << (IDataInputStream &Stream) {
    Clear();
    uint32_t size;
    Stream >> size;
    Strings.SetCapacity(size);
    for( size_t i=0; i < size; i++ )
      Stream >> Add(EmptyString);
  }
  // convinience method
  void SaveToTextStream(IDataOutputStream& os) const {
    for( int i=0; i < Count()-1; i++ )
      os.Writenl( Strings[i]->GetString().raw_str(), Strings[i]->GetString().RawLen());
    if( Count() > 0 )
      os.Writenl( Strings.Last()->GetString().raw_str(), Strings.Last()->GetString().RawLen());
  }
  void SaveToFile(const olxstr& fileName) const {
    TEFile file(fileName, "wb+");
    SaveToTextStream(file);
  }

  virtual TIString ToString() const  {   return Text(NewLineSequence).ToString();  }

  const TTStrList& operator = (const TTStrList& list)  {
    Assign( list );
    return list;
  }

  template <class comparator>
    void BubleSort()  {  TPtrList<T>::BubleSorter.template Sort<comparator>(Strings);  }
  void BubleSortSF(int (*f)(const T* a, const T* b) )  {
    TPtrList<T>::BubleSorter.template SortSF(Strings, f);
  }
  template <class BaseClass>
    void BubleSortMF(BaseClass& baseClassInstance,
                       int (BaseClass::*f)(const T* a, const T* b) )  {
      TPtrList<T>::BubleSorter.template SortMF<BaseClass>(Strings, baseClassInstance, f);
    }

  template <class Comparator>
    void QuickSort()  {  TPtrList<T>::QuickSorter.template Sort<Comparator>(Strings);  }
  void QuickSortSF(int (*f)(const T* a, const T* b) )  {
    TPtrList<T>::QuickSorter.template SortSF(Strings, f);
  }
  template <class BaseClass>
    void QuickSortMF(BaseClass& baseClassInstance,
                       int (BaseClass::*f)(const T* a, const T* b) )  {
      TPtrList<T>::QuickSorter.template SortMF<BaseClass>(Strings, baseClassInstance, f);
    }


  void QSort(bool ci)  {
    if( ci )  TPtrList<T>::QuickSorter.template Sort< TStringWrapperComparator<T,true> >(Strings);
    else      TPtrList<T>::QuickSorter.template Sort< TStringWrapperComparator<T,false> >(Strings);
  }

  void StrtokF( const SC &Str, const TIntList& indexes )  {
    if( indexes.Count() < 2 )
      throw TInvalidArgumentException(__OlxSourceInfo, "at least two numbers are required");
    int fLength = 0;
    for( int i=0; i < indexes.Count(); i++ )
      fLength += indexes[i];

    fLength=0; // it wil be recalculated again
    for( int i=0; i < indexes.Count(); i++ )  {
      int valid_end = olx_min( Str.Length()-fLength, indexes[i] );
      Add( Str.SubString(fLength, indexes[i]) );
      fLength += indexes[i];
      if( fLength >= Str.Length() )  return;
    }
    if( fLength < Str.Length() )  Add( Str.SubStringFrom(fLength) );
  }
  void Strtok( const SC &Str, char Sep )  {
    olxstr Tmp(Str);
    int ind = Tmp.IndexOf(Sep);
    while( ind != -1 )  {
      if( ind != 0 )  // skip sequences of separators
        Add(Tmp.SubStringTo(ind));
      if( (ind+1) >= Tmp.Length() )  {
        Tmp = EmptyString;
        break;
      }
      while( ++ind < Tmp.Length() && Tmp.CharAt(ind) == Sep ) ;
      if( ind >= Tmp.Length() )  {
        Tmp = EmptyString;
        break;
      }
      Tmp = Tmp.SubStringFrom(ind);
      ind = Tmp.IndexOf(Sep);
    }
    if( !Tmp.IsEmpty() ) // add last bit
      Add(Tmp);
  }
  // similar to previous implementation, the list is passed as one of the parameters
  // takes a string as a separator
  void Strtok( const SC &Str, const SC &Sep )  {
    SC Tmp(Str);
    int ind = Tmp.IndexOf(Sep);
    while( ind != -1 )  {
      Add(Tmp.SubStringTo(ind));
      if( (ind+Sep.Length()) >= Tmp.Length() )  {
        Tmp = EmptyString;
        break;
      }
      Tmp = Tmp.SubStringFrom(ind+Sep.Length());
      ind = Tmp.IndexOf(Sep);
    }
    if( !Tmp.IsEmpty() ) // add last bit
      Add(Tmp);
  }

  void Hypernate(const SC& String, int Width, bool Force=true)  {
   SC Str(String);
    while( Str.Length() > Width )  {
      int spi = Str.LastIndexOf(' ', Width);
      if( spi > 0 )  {
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
    if( !Str.IsEmpty() )  Add(Str);
  }
};


template <class SC, typename T> class TPrimitiveStrListData : public TSingleStringWrapper<SC>  {
  T ThisObject;
public:
  TPrimitiveStrListData() : TSingleStringWrapper<SC>()  { ThisObject =  0;  }
  virtual ~TPrimitiveStrListData()  {  ;  }
  TPrimitiveStrListData(const SC& str, const T& obj = 0 ) : TSingleStringWrapper<SC>(str)  {
    ThisObject =  obj;
  }
  TPrimitiveStrListData(const char* str, const T& obj = 0 ) : TSingleStringWrapper<SC>(str)  {
    ThisObject =  obj;
  }
  TPrimitiveStrListData(const wchar_t* str, const T& obj = 0 ) : TSingleStringWrapper<SC>(str)  {
    ThisObject =  obj;
  }
  inline T& Object()                  {  return ThisObject;  }
  inline const T& GetObject()  const  {  return ThisObject;  }
};

template <class SC, typename T> class TObjectStrListData : public TSingleStringWrapper<SC>  {
  T ThisObject;
public:
  TObjectStrListData() : TSingleStringWrapper<SC>()  {  ;  }
  virtual ~TObjectStrListData()  {  ;  }
  TObjectStrListData(const olxstr str ) : TSingleStringWrapper<SC>(str)  {  }
  TObjectStrListData(const char* str ) : TSingleStringWrapper<SC>(str)  {  }
  TObjectStrListData(const wchar_t* str ) : TSingleStringWrapper<SC>(str)  {  }
  TObjectStrListData(const SC& str, const T& obj ) : TSingleStringWrapper<SC>(str)  {
    ThisObject =  obj;
  }
  TObjectStrListData(const char* str, const T& obj ) : TSingleStringWrapper<SC>(str)  {
    ThisObject =  obj;
  }
  TObjectStrListData(const wchar_t* str, const T& obj ) : TSingleStringWrapper<SC>(str)  {
    ThisObject =  obj;
  }
  inline T& Object()                  {  return ThisObject;  }
  inline const T& GetObject()  const  {  return ThisObject;  }
};


template <class SC, class OC, class GC>  class TTOStringList:  public TTStrList<SC,GC>  {
  //TTypeList< TEStrListData<T> > Strings;
public:
  // creates nempty list
  TTOStringList()  {  }
  // copy constructor
  template <class SC1, class T1>
    TTOStringList(const TTStrList<SC1,T1>& list)  {
      TTStrList<SC,GC>::Strings.SetCapacity( list.Count() );
      for( int i=0; i < list.Count(); i++ )
        Add( list.String(i) );
    }

  TTOStringList(const TTOStringList& list)  {
    TTStrList<SC,GC>::Strings.SetCapacity( list.Count() );
    for( int i=0; i < list.Count(); i++ )
      Add( list.String(i), list.Object(i) );
  }
  // creates a list with strtok entries in it
  TTOStringList(const SC& string, const SC& sep, TTypeList<OC>* objects = NULL) :
                          TTStrList<SC,GC>(string, sep)  {
    if( objects != NULL )  {
      for( int i=0; i < objects->Count(); i++ )  {
        if( (i-1) > TTStrList<SC,GC>::Count() )  break;
        TTStrList<SC,GC>::Object(i) = objects->Item(i);
      }
    }
  }
  // creates a list with strtok entries in it
  TTOStringList(const SC& string, char sep, TTypeList<OC>* objects = NULL) :
                          TTStrList<SC,GC>(string, sep)  {
    if( objects != NULL )  {
      for( int i=0; i < objects->Count(); i++ )  {
        if( (i-1) > TTStrList<SC,GC>::Count() )  break;
        Object(i) = objects->Item(i);
      }
    }
  }
  virtual ~TTOStringList()  {  }

  void SubList(int offset, int count, TTOStringList& SL) const  {
    for(int i=offset; i < offset+count; i++ )
      SL.Add( TTStrList<SC,GC>::String(i), Object(i));
  }

  template <class SC1, class T1>
    void Assign(const TTStrList<SC1,T1>& S)  {
      TTStrList<SC,GC>::Clear();
      for( int i=0; i < S.Count(); i++ )
        Add( S.String(i) );
    }

  inline void Assign(const TTOStringList& S)  {
    TTStrList<SC,GC>::Clear();
    TTStrList<SC,GC>::AddList(S);
  }

  void AddList(const TTOStringList& S)  {
    for( int i=0; i < S.Count(); i++ )
      Add(S.String(i), S.Object(i));
  }

  inline void Add(const SC& S)  {  TTStrList<SC,GC>::Strings.Add( new GC(S) );  }
  inline void Add(const SC& S, const OC& Object)  {  TTStrList<SC,GC>::Strings.Add( new GC(S,Object) );  }
  inline void Insert(int i, const SC& S)         {  TTStrList<SC,GC>::Strings.Insert(i, new GC(S) );  }
  inline void Insert(int i, const SC& S, const OC& O)  {  TTStrList<SC,GC>::Strings.Insert(i, new GC(S,O) );  }

  // access functions
  inline OC& Object(int i)                const { return TTStrList<SC,GC>::Strings[i]->Object();  }
  //inline const OC& GetObject(int i)  const { return Strings[i].Object();  }

  const TTOStringList& operator = (const TTOStringList& list)  {
    TTStrList<SC,GC>::Clear();
    TTStrList<SC,GC>::Strings.SetCapacity( list.Count() );
    for( int i=0; i < list.Count(); i++ )
      Add( list.String(i), list.Object(i) );
    return list;
  }

  int IndexOfObject(const OC& C ) const  {
    for( int i=0; i < TTStrList<SC,GC>::Count(); i++ )
      if( TTStrList<SC,GC>::Strings[i]->GetObject() == C )
        return i;
    return -1;
  }
  // the find function with this signature work only for objects; for pointers it
  // causes a lot of trouble
  const OC & FindObject(const SC &Name) const  {
    int in = TTStrList<SC,GC>::IndexOf(Name);
    return (in != -1) ? TTStrList<SC,GC>::Strings[in].GetObject() : *(OC*)NULL;
  }

  OC* FindObjecti(const SC &Name) const  {
    int in = TTStrList<SC,GC>::CIIndexOf(Name);
    return (in != -1) ? &TTStrList<SC,GC>::Strings[in].GetObject() : *(OC*)NULL;
  }
};

template <class SC, typename OC>  class TStrPObjList: 
   public TTOStringList<SC,OC,TPrimitiveStrListData<SC,OC> >  {
public:
  TStrPObjList()  {}

  template <class SC1, class T1> TStrPObjList(const TTStrList<SC1,T1>& list) : 
    TTOStringList<SC,OC,TPrimitiveStrListData<SC,OC> >(list)  {}

  TStrPObjList(const TTOStringList<SC,OC,TPrimitiveStrListData<SC,OC> >& list) :
    TTOStringList<SC,OC,TPrimitiveStrListData<SC,OC> >(list)  {}

  TStrPObjList(const SC& string, const olxstr& sep, TTypeList<OC>* objects = NULL) :
    TTOStringList<SC,OC,TPrimitiveStrListData<SC,OC> >(string, sep, objects)  {}

  TStrPObjList(const SC& string, char sep, TTypeList<OC>* objects = NULL) :
    TTOStringList<SC,OC,TPrimitiveStrListData<SC,OC> >(string, sep, objects)  {}

  inline OC FindObject(const SC &Name) const  {
    int in = TTOStringList<SC,OC,TPrimitiveStrListData<SC,OC> >::IndexOf(Name);
    return (in != -1) ? TTOStringList<SC,OC,TPrimitiveStrListData<SC,OC> >::Strings[in]->GetObject() : NULL;
  }

  OC FindObjectCI(const SC &Name) const  {
    int in = TTOStringList<SC,OC,TPrimitiveStrListData<SC,OC> >::CIIndexOf(Name);
    return (in != -1) ? TTOStringList<SC,OC,TPrimitiveStrListData<SC,OC> >::Strings[in]->GetObject() : NULL;
  }
};


typedef TStrPObjList<olxstr, IEObject*> TStrObjList;
typedef TStrPObjList<CString, IEObject*> TCStrObjList;
typedef TStrPObjList<WString, IEObject*> TWStrObjList;

typedef TTOStringList<olxstr, olxstr, TObjectStrListData<olxstr,olxstr> > TStrStrList;
typedef TTOStringList<CString, CString, TObjectStrListData<CString,CString> > TCStrCStrList;
typedef TTOStringList<WString, WString, TObjectStrListData<WString,WString> > TWStrWStrList;

typedef TTStrList<olxstr, TSingleStringWrapper<olxstr> > TStrList;
typedef TTStrList<CString, TSingleStringWrapper<CString> > TCStrList;
typedef TTStrList<WString, TSingleStringWrapper<WString> > TWStrList;

//template <class T>  class TStrPObj

EndEsdlNamespace()
#endif
