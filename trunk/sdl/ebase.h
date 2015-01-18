/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_base_H
#define __olx_sdl_base_H
#include <typeinfo>
#include <string.h>
#include <stdlib.h>

#define EsdlClassNameT(class)  typeid(class).name()
#define EsdlObjectNameT(object)  typeid(object).name()
#define EsdlClassName(class)  olxstr(typeid(class).name())
#define EsdlObjectName(object)  olxstr(typeid(object).name())

#define EsdlInstanceOf( class, className )  (typeid(class) == typeid(className))

// defines a primitive type property
#define DefPropP(Type, Name) \
public:\
  Type Get##Name() const {  return Name;  }\
  void Set##Name(Type MaCV) {  Name = MaCV;  }
// defines a boolean type property as Is/Set
#define DefPropBIsSet(Name) \
public:\
  bool Is##Name() const {  return Name;  }\
  void Set##Name(bool MaCV) {  Name = MaCV;  }
// defines a boolean type property as Is/Set
#define DefPropBHasSet(Name) \
public:\
  bool Has##Name() const {  return Name;  }\
  void Set##Name(bool MaCV) {  Name = MaCV;  }
// defines a boolean type property as a bit mask as Is/Set
#define DefPropBFIsSet(Name, VarName, BitMask) \
public:\
  bool Is##Name() const {  return (VarName & BitMask) != 0;  }\
  void Set##Name(bool v)   { \
      if( v )  VarName |= BitMask; \
      else     VarName &= ~BitMask;  }
// defines a boolean type property as a bit mask as Has/Set
#define DefPropBFHasSet(Name, VarName, BitMask) \
public:\
  bool Has##Name() const {  return (VarName & BitMask) != 0;  }\
  void Set##Name(bool v)   { \
      if( v )  VarName |= BitMask; \
      else     VarName &= ~BitMask;  }
// defines a complex (class) type property
#define DefPropC(Type, Name) \
public:\
  const Type& Get##Name()    const {  return Name;  }\
  void Set##Name(const Type& MaCV) {  Name = MaCV;  }

#define BeginEsdlNamespace()  namespace esdl {
#define EndEsdlNamespace()  };\
  using namespace esdl;
#define UseEsdlNamespace()  using namespace esdl;
#define GlobalEsdlFunction( fun )     esdl::fun
#define EsdlObject( obj )     esdl::obj

#include "defs.h"

#ifdef __WIN32__
  #if !defined(_WIN32_WINNT) && _MSC_VER < 1500
    #define _WIN32_WINNT 0x400
  #endif
  #include <WinSock2.h>
  #include <windows.h>
#endif
// there is a mistery how it manages to disapper!!!!
#ifdef __BORLANDC__
  #if !defined(__MT__)
    #define __MT__
  #endif
#endif

BeginEsdlNamespace()
#include "olxptr.h"

static const size_t InvalidIndex = size_t(~0);
static const size_t InvalidSize = size_t(~0);
// validates if unsigned number is valid... since the move to size_t etc...
template <typename int_t> static bool olx_is_valid_index(const int_t& v)  {
  return v != int_t(~0);
}
template <typename int_t> static bool olx_is_valid_size(const int_t& v)  {
  return v != int_t(~0);
}
// wrap memory management functions
extern void *olx_malloc_(size_t sz);  // throws TOutOfMemoryException
extern void *olx_realloc_(void * a, size_t sz);  // throws TOutOfMemoryException
template <typename T> T *olx_malloc(size_t sz) {
  return (T*)olx_malloc_(sz*sizeof(T));
}
template <typename T> T *olx_realloc(T *a, size_t sz) {
  return (T*)olx_realloc_(a, sz*sizeof(T));
}
template <typename T> void olx_free(T *a) {  if( a != NULL )  free(a);  }
template <typename T> void olx_del_obj(T *a) { if (a != NULL) delete a;  }
template <typename T> void olx_del_arr(T *a) { if (a != NULL) delete [] a; }
template <typename T> T *olx_memcpy(T *dest, const T *src, size_t sz) {
  return (T *)memcpy(dest, src, sz*sizeof(T));
}
template <typename T> T *olx_memmove(T *dest, const T *src, size_t sz) {
  return (T *)memmove(dest, src, sz*sizeof(T));
}

// string base
template <class T> class TTIString {
public:
  struct Buffer  {
    T *Data;
    unsigned int RefCnt;
    size_t Length;
    Buffer(size_t len, const T *data=NULL, size_t tocopy=0)  {
      Data = ((len != 0) ? olx_malloc<T>(len) : (T*)NULL);
      if( data != NULL )
        olx_memcpy(Data, data, tocopy);
      RefCnt = 1;
      Length = len;
    }
    // creates object from external array, must be created with malloc
    Buffer(T *data, size_t len)  {
      Data = data;
      RefCnt = 1;
      Length = len;
    }
    ~Buffer()  {  olx_free(Data);  }
    void SetCapacity(size_t newlen)  {
      if( newlen > Length )  {
        Data = olx_realloc(Data, newlen);
        Length = newlen;
      }
    }
  };
protected:
  mutable Buffer *SData;  // do not have much choice with c_str ...
  void IncLength(size_t len)  {  _Length += len;  }
  void DecLength(size_t len)  {  _Length -= len;  }
  size_t GetCapacity()  const {  return SData == NULL ? 0 : SData->Length;  }
  size_t _Increment, _Length;
  mutable size_t _Start;
  TTIString() {}
  void checkBufferForModification(size_t newSize) const {
    if( SData == NULL )
      SData = new Buffer(newSize + _Increment);
    else if( SData->RefCnt > 1 )  {
      SData->RefCnt--;
      Buffer *newData = new Buffer(newSize + _Increment, &SData->Data[_Start],
        olx_min(_Length, newSize));
      SData = newData;
      _Start = 0;
    }
    else if( SData->RefCnt == 1 && _Start != 0 )  {
      if( _Length != 0 )
        memmove(SData->Data, &SData->Data[_Start], _Length*sizeof(T));
      _Start = 0;
    }
    if( SData->Length < newSize )
      SData->SetCapacity((size_t)(newSize*1.5)+_Increment);
  }
public:
  TTIString(const TTIString& str) {
    SData = str.SData;
    if( SData != NULL )  SData->RefCnt++;
    _Length = str._Length;
    _Start = str._Start;
    _Increment = 8;
  }
  virtual ~TTIString() {
    if (SData != NULL && --SData->RefCnt == 0) {
      delete SData;
      SData = NULL;
      _Length = 0;
    }
  }
  // might not have '\0' char, to be used with Length or RawLen (char count)
  T * raw_str() const { return ((SData == NULL) ? NULL : &SData->Data[_Start]);  }
  // length in bytes
  size_t RawLen() const {  return _Length*sizeof(T);  }
  // length in items
  size_t Length() const { return _Length;  }
  // for internal: use with caution!
  inline Buffer *Data_() const {  return SData;  }
  // for internal stuff
  size_t Start_() const { return _Start;  }
  /* standard api requires terminating '\0'; the use of raw_str and Length() is
 preferable
 */
  const T * u_str() const {
    if( SData == NULL ) return NULL;
    if( (SData->Length == (_Start+_Length)) ||
        (SData->Data[_Start+_Length] != '\0') )
    {
      checkBufferForModification(_Length + 1);
      SData->Data[_Start+_Length] = '\0';
    }
    return &SData->Data[_Start];
  }
  bool IsEmpty() const { return _Length == 0;  }
  T CharAt(size_t i) const {
#ifdef _DEBUG
    if( i >= _Length )
      TExceptionBase::ThrowIndexOutOfRange(__POlxSourceInfo, i, 0, _Length);
#endif
    return SData->Data[_Start + i];
  }
  T operator[] (size_t i) const {
#ifdef _DEBUG
    if( i >= _Length )
      TExceptionBase::ThrowIndexOutOfRange(__POlxSourceInfo, i, 0, _Length);
#endif
    return SData->Data[_Start + i];
  }
  /* reads content of the string to external buffer, which must be able to
 accommodate string length + 1 for the end of string char
  */
  T *Read(T *v)  {
    if( SData == NULL ) return NULL;
    olx_memcpy(v, &SData->Data[_Start], _Length);
    v[_Length] = L'\0';
    return v;
  }
  typedef T CharT;
  // this does not help in GCC,
  template <typename,typename> friend class TTSString;
};

typedef TTIString<olxch> TIString;
typedef TTIString<char> TICString;
typedef TTIString<wchar_t> TIWString;

extern const TIString& NewLineSequence();
extern const TICString& CNewLineSequence();
extern const TIWString& WNewLineSequence();


struct ADestructionObserver {
  ADestructionObserver *next;
  ADestructionObserver() : next(0)
  {}
  virtual ~ADestructionObserver() {}
  virtual void call(class APerishable* obj) const = 0;
  virtual bool operator == (const ADestructionObserver *) const = 0;
  virtual ADestructionObserver *clone() const = 0;
};

// class base
class IOlxObject {
public:
#ifdef _DEBUG
  IOlxObject();
  virtual ~IOlxObject();
#else
  virtual ~IOlxObject() {}
#endif
  // throws an exception if not implemented
  virtual TIString ToString() const;
  // throws an exception if not implemented
  virtual IOlxObject* Replicate() const;
};

#include "olxvptr.h"
#include "destruction_obs.h"

class APerishable : public virtual IOlxObject {
  bool HasDObserver(ADestructionObserver *dh) const;
protected:
  ADestructionObserver *dsh_head, *dsh_tail;
public:
  APerishable() : dsh_head(0), dsh_tail(0)
  {}
  virtual ~APerishable();
  // must be created with new
  bool AddDestructionObserver(ADestructionObserver &);
  void RemoveDestructionObserver(const ADestructionObserver &);
};


// an interface for a referencible object
class AReferencible : public virtual IOlxObject {
  int This_RefCount;
public:
  AReferencible() : This_RefCount(0)
  {}
  virtual ~AReferencible();
  int GetRefCount() const { return This_RefCount; }
  int DecRef() { return --This_RefCount; }
  int IncRef() { return ++This_RefCount; }
};

// we need this class to throw exceptions from string with gcc ...
class TExceptionBase : public IOlxObject {
protected:
  static bool &AutoLog() {
    static bool v = false;
    return v;
  }
  /* to prevent creation this class directly. All instances must be of the
 TBasicExceptionClass defined in exception.h
 */
  virtual void CreationProtection() = 0;
public:
  static void ThrowFunctionFailed(const char* file, const char* function,
    int line, const char* msg);
  static void ThrowIndexOutOfRange(const char* file, const char* function,
    int line, size_t index, size_t min_ind, size_t max_ind);
  static void ThrowInvalidUnsignedFormat(const char* file, const char* function,
    int line, const char* src, size_t src_len);
  static void ThrowInvalidUnsignedFormat(const char* file, const char* function,
    int line, const wchar_t* src, size_t src_len);
  static void ThrowInvalidIntegerFormat(const char* file, const char* function,
    int line, const char* src, size_t src_len);
  static void ThrowInvalidIntegerFormat(const char* file, const char* function,
    int line, const wchar_t* src, size_t src_len);
  static void ThrowInvalidFloatFormat(const char* file, const char* function,
    int line, const char* src, size_t src_len);
  static void ThrowInvalidFloatFormat(const char* file, const char* function,
  int line, const wchar_t* src, size_t src_len);
  static void ThrowInvalidBoolFormat(const char* file, const char* function,
    int line, const char* src, size_t src_len);
  static void ThrowInvalidBoolFormat(const char* file, const char* function,
    int line, const wchar_t* src, size_t src_len);
  static TIString FormatSrc(const char* file, const char* func, int line);
  static void SetAutoLogging(bool v)  {  AutoLog() = v;  }
  static bool GetAutoLogging()  {  return AutoLog();  }
  // returns recasted this, or throws exception if dynamic_cast fails
  const class TBasicException* GetException() const;
};

#include "eaccessor.h"

struct olx_alg {
protected:
// logical NOT operator for an analyser
  template <class Analyser> struct not_ {
    const Analyser& analyser;
    not_(const Analyser& _analyser)
      : analyser(_analyser)
    {}
    template <class Item> bool OnItem(const Item& o) const {
      return !analyser.OnItem(o);
    }
    template <class Item> bool OnItem(const Item& o, size_t i) const {
      return !analyser.OnItem(o, i);
    }
  };
  // logical AND operator for two analysers
  template <class AnalyserA, class AnalyserB> struct and_ {
    const AnalyserA& analyserA;
    const AnalyserB& analyserB;
    and_(const AnalyserA& _analyserA, const AnalyserB& _analyserB)
      : analyserA(_analyserA), analyserB(_analyserB)
    {}
    template <class Item> bool OnItem(const Item& o) const {
      return analyserA.OnItem(o) && analyserB.OnItem(o);
    }
    template <class Item> bool OnItem(const Item& o, size_t i) const {
      return analyserA.OnItem(o, i) && analyserB.OnItem(o, i);
    }
  };
  // logical OR operator for two analysers
  template <class AnalyserA, class AnalyserB> struct or_ {
    const AnalyserA& analyserA;
    const AnalyserB& analyserB;
    or_(const AnalyserA& _analyserA, const AnalyserB& _analyserB)
      : analyserA(_analyserA), analyserB(_analyserB)
    {}
    template <class Item> bool OnItem(const Item& o) const {
      return analyserA.OnItem(o) || analyserB.OnItem(o);
    }
    template <class Item> bool OnItem(const Item& o, size_t i) const {
      return analyserA.OnItem(o, i) || analyserB.OnItem(o, i);
    }
  };

  template <class Accessor> struct chsig_ {
    const Accessor &accessor;
    chsig_(const Accessor &accessor_) : accessor(accessor_) {}
    template <class Item>
    typename Accessor::return_type OnItem(const Item& o) const {
      return -accessor(o);
    }
    template <class Item>
    typename Accessor::return_type OnItem(const Item& o, size_t) const {
      return -accessor(o);
    }
    template <class Item>
    typename Accessor::return_type operator () (const Item& o) const {
      return -accessor(o);
    }
  };

  struct op_eq {
    template <typename item_to, typename item_t>
    static bool op(const item_to &to, const item_t &t) {
      return t == to;
    }
  };
  struct op_neq {
    template <typename item_to, typename item_t>
    static bool op(const item_to &to, const item_t &t) {
      return t != to;
    }
  };
  struct op_lt {
    template <typename item_to, typename item_t>
    static bool op(const item_to &to, const item_t &t) {
      return t < to;
    }
  };
  struct op_le {
    template <typename item_to, typename item_t>
    static bool op(const item_to &to, const item_t &t) {
      return t <= to;
    }
  };
  struct op_gt {
    template <typename item_to, typename item_t>
    static bool op(const item_to &to, const item_t &t) {
      return t > to;
    }
  };
  struct op_ge {
    template <typename item_to, typename item_t>
    static bool op(const item_to &to, const item_t &t) {
      return t >= to;
    }
  };

  template <typename to_t, class Accessor, class op_t> struct op_ {
    to_t to;
    const Accessor &accessor;
    op_(const to_t &t, const Accessor &accessor_) : to(t), accessor(accessor_)
    {}
    template <class Item> bool OnItem(const Item& o) const {
      return op_t::op(to, accessor(o));
    }
    template <class Item> bool OnItem(const Item& o, size_t) const {
      return op_t::op(to, accessor(o));
    }
  };
public:
  /* creates a new not logical operator */
  template <class Analyser>
  static not_<Analyser> olx_not(const Analyser& a) {
    return not_<Analyser>(a);
  }
  /* creates a new and logical operator */
  template <class AnalyserA, class AnalyserB>
  static and_<AnalyserA, AnalyserB> olx_and(
    const AnalyserA& a, const AnalyserB& b)
  {
    return and_<AnalyserA, AnalyserB>(a, b);
  }
  /* creates a new or logical operator */
  template <class AnalyserA, class AnalyserB>
  static or_<AnalyserA, AnalyserB> olx_or(
    const AnalyserA& a, const AnalyserB& b) {
    return or_<AnalyserA, AnalyserB>(a, b);
  }
  /* creates a new chsig arithmetic functor/accessor */
  template <class Accessor>
  static chsig_<Accessor> olx_chsig(const Accessor& a) {
    return chsig_<Accessor>(a);
  }
  /* creates a new equality checker */
  template <typename to_t>
  static op_<to_t, DummyAccessor, op_eq> olx_eq(const to_t &to) {
    return op_<to_t, DummyAccessor, op_eq>(to, DummyAccessor());
  }
  template <typename to_t, class Accessor>
  static op_<to_t, Accessor, op_eq> olx_eq(const to_t &to, const Accessor& a) {
    return op_<to_t, Accessor, op_eq>(to, a);
  }
  /* creates a new non-equality checker */
  template <typename to_t>
  static op_<to_t, DummyAccessor, op_neq> olx_neq(const to_t &to) {
    return op_<to_t, DummyAccessor, op_neq>(to, DummyAccessor());
  }
  template <typename to_t, class Accessor>
  static op_<to_t, Accessor, op_neq> olx_neq(const to_t &to, const Accessor& a) {
    return op_<to_t, Accessor, op_neq>(to, a);
  }
  /* creates a LT checker */
  template <typename to_t>
  static op_<to_t, DummyAccessor, op_lt> olx_lt(const to_t &to) {
    return op_<to_t, DummyAccessor, op_lt>(to, DummyAccessor());
  }
  template <typename to_t, class Accessor>
  static op_<to_t, Accessor, op_lt> olx_lt(const to_t &to, const Accessor& a) {
    return op_<to_t, Accessor, op_lt>(to, a);
  }
  /* creates a LE checker */
  template <typename to_t>
  static op_<to_t, DummyAccessor, op_le> olx_le(const to_t &to) {
    return op_<to_t, DummyAccessor, op_le>(to, DummyAccessor());
  }
  template <typename to_t, class Accessor>
  static op_<to_t, Accessor, op_le> olx_le(const to_t &to, const Accessor& a) {
    return op_<to_t, Accessor, op_le>(to, a);
  }
  /* creates a GT checker */
  template <typename to_t>
  static op_<to_t, DummyAccessor, op_gt> olx_gt(const to_t &to) {
    return op_<to_t, DummyAccessor, op_gt>(to, DummyAccessor());
  }
  template <typename to_t, class Accessor>
  static op_<to_t, Accessor, op_gt> olx_gt(const to_t &to, const Accessor& a) {
    return op_<to_t, Accessor, op_gt>(to, a);
  }
  /* creates a GE checker */
  template <typename to_t>
  static op_<to_t, DummyAccessor, op_ge> olx_ge(const to_t &to) {
    return op_<to_t, DummyAccessor, op_ge>(to, DummyAccessor());
  }
  template <typename to_t, class Accessor>
  static op_<to_t, Accessor, op_ge> olx_ge(const to_t &to, const Accessor& a) {
    return op_<to_t, Accessor, op_ge>(to, a);
  }
};

/* swaps two objects using a temporary variable (copy constructor must be
 available for complex types) */
template <typename obj> inline void olx_swap(obj& o1, obj& o2) {
  obj tmp = o1;
  o1 = o2;
  o2 = tmp;
}
// returns 10^val, cannot put it to emath due to dependencies...
template <typename FT> FT olx_pow10(size_t val)  {
  if( val == 0 )  return 1;
  FT rv = 10;
  while( --val > 0 ) rv *=10;
  return rv;
}
/* comparison function (useful for the size_t on Win64, where size_t=uint64_t
 *  and int is int32_t) */
template <typename T1, typename T2> inline
int olx_cmp(T1 a, T2 b) { return a < b ? -1 : (a > b ? 1 : 0); }

template <typename T, typename T1> bool olx_is(const T1 &v) {
  return typeid(T) == typeid(olx_ref::get(v));
};

#include "association.h"
#include "listalg.h"
#include "citem.h"

EndEsdlNamespace()

#include "istream.h"
#include "smart/ostring.h"
#endif
