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
template <typename T> void olx_free(T *a) {
  if( a != 0 )  free(a);
}
template <typename T> void olx_del_obj(T *a) {
  if (a != 0) delete a;
}
template <typename T> void olx_del_arr(T *a) {
  if (a != 0) delete [] a;
}
template <typename T> T *olx_memcpy(T *dest, const T *src, size_t sz) {
  return (T *)memcpy(dest, src, sz*sizeof(T));
}
template <typename T> T *olx_memmove(T *dest, const T *src, size_t sz) {
  return (T *)memmove(dest, src, sz*sizeof(T));
}
template <typename T, typename T1>
T* olx_memcpy(T* dest, const T1 &src, size_t sz) {
  return (T*)memcpy(dest, src, sz * sizeof(T));
}
template <typename T, typename T1>
T* olx_memmove(T* dest, const T1& src, size_t sz) {
  return (T*)memmove(dest, src, sz * sizeof(T));
}

template <typename A>
struct olx_type {
  template <typename B>
  static bool check(const B &b) {
    return typeid(b) == typeid(A);
  }
};

template <typename A, typename B>
bool olx_type_check(const A &a, const B &b) {
  return typeid(a) == typeid(b);
}

// string base
template <class T> class TTIString {
public:
  struct Buffer {
    T* Data;
    unsigned int RefCnt;
    size_t Length;
    Buffer(size_t len, const T* data = 0, size_t tocopy = 0) {
      Data = ((len != 0) ? olx_malloc<T>(len) : (T*)0);
      if (data != 0) {
        olx_memcpy(Data, data, tocopy);
      }
      RefCnt = 1;
      Length = len;
    }
    // creates object from external array, must be created with malloc
    Buffer(T* data, size_t len) {
      Data = data;
      RefCnt = 1;
      Length = len;
    }
    ~Buffer() { olx_free(Data); }
    void SetCapacity(size_t newlen) {
      if (newlen > Length) {
        Data = olx_realloc(Data, newlen);
        Length = newlen;
      }
    }
  };
protected:
  mutable Buffer* SData;  // do not have much choice with c_str ...
  void IncLength(size_t len) { _Length += len; }
  void DecLength(size_t len) { _Length -= len; }
  size_t GetCapacity()  const { return SData == 0 ? 0 : SData->Length; }
  size_t _Increment, _Length;
  mutable size_t _Start;
  TTIString() {}
  void checkBufferForModification(size_t newSize) const {
    if (SData == 0) {
      SData = new Buffer(newSize + _Increment);
    }
    else if (SData->RefCnt > 1) {
      SData->RefCnt--;
      Buffer* newData = new Buffer(newSize + _Increment, &SData->Data[_Start],
        olx_min(_Length, newSize));
      SData = newData;
      _Start = 0;
    }
    else if (SData->RefCnt == 1 && _Start != 0) {
      if (_Length != 0) {
        memmove(SData->Data, &SData->Data[_Start], _Length * sizeof(T));
      }
      _Start = 0;
    }
    if (SData->Length < newSize) {
      SData->SetCapacity((size_t)(newSize * 1.5) + _Increment);
    }
  }
public:
  TTIString(const TTIString& str) {
    SData = str.SData;
    if (SData != 0) {
      SData->RefCnt++;
    }
    _Length = str._Length;
    _Start = str._Start;
    _Increment = 8;
  }
  virtual ~TTIString() {
    if (SData != 0 && --SData->RefCnt == 0) {
      delete SData;
      SData = 0;
      _Length = 0;
    }
  }
  // might not have '\0' char, to be used with Length or RawLen (char count)
  T* raw_str() const { return ((SData == 0) ? 0 : &SData->Data[_Start]); }
  // length in bytes
  size_t RawLen() const { return _Length * sizeof(T); }
  // length in items
  size_t Length() const { return _Length; }
  // for internal: use with caution!
  inline Buffer* Data_() const { return SData; }
  // for internal stuff
  size_t Start_() const { return _Start; }
  /* standard api requires terminating '\0'; the use of raw_str and Length() is
 preferable
 */
  const T* u_str() const {
    if (SData == 0) {
      return 0;
    }
    if ((SData->Length == (_Start + _Length)) ||
      (SData->Data[_Start + _Length] != '\0'))
    {
      checkBufferForModification(_Length + 1);
      SData->Data[_Start + _Length] = '\0';
    }
    return &SData->Data[_Start];
  }
  bool IsEmpty() const { return _Length == 0; }
  T CharAt(size_t i) const {
#ifdef _DEBUG
    if (i >= _Length) {
      TExceptionBase::ThrowIndexOutOfRange(__POlxSourceInfo, i, 0, _Length);
    }
#endif
    return SData->Data[_Start + i];
  }
  T operator[] (size_t i) const {
#ifdef _DEBUG
    if (i >= _Length) {
      TExceptionBase::ThrowIndexOutOfRange(__POlxSourceInfo, i, 0, _Length);
    }
#endif
    return SData->Data[_Start + i];
  }
  /* reads content of the string to external buffer, which must be able to
 accommodate string length + 1 for the end of string char
  */
  T* Read(T* v) {
    if (SData == 0) {
      return 0;
    }
    olx_memcpy(v, &SData->Data[_Start], _Length);
    v[_Length] = '\0';
    return v;
  }
  void TrimFloat() {
    size_t fp_pos = InvalidIndex, exp_pos = InvalidIndex;
    for (size_t i = 0; i < _Length; i++) {
      if (CharAt(i) == '.') {
        fp_pos = i;
      }
      else if (CharAt(i) == 'e') {
        exp_pos = i;
        break;
      }
    }
    if (fp_pos == InvalidIndex && exp_pos == InvalidIndex) {
      return;
    }
    // check for e+/-000
    if (exp_pos != InvalidIndex) {
      for (size_t i = exp_pos + 2; i < _Length; i++) {
        if (CharAt(i) != '0') {
          return;
        }
      }
      _Length = exp_pos-1;
      if (fp_pos == InvalidIndex) {
        return;
      }
    }
    
    while (_Length > 1 && CharAt(_Length - 1) == '0') {
      _Length--;
    }
    if (_Length > 0 && CharAt(_Length - 1) == '.') {
      _Length--;
    }
  }
  //...........................................................................
  typedef T CharT;
  // this does not help in GCC,
  template <typename, typename> friend class TTSString;
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
  template <class T>
  bool Is() const {
    return olx_type<T>::check(*this);
  }
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

#include "olxpptr.h"
#include "olxalg.h"
#include "olxfunc.h"

#include "association.h"
#include "listalg.h"
#include "citem.h"

EndEsdlNamespace()

#include "istream.h"
#include "smart/ostring.h"
#endif
