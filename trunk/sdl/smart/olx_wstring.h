#ifndef __SMART_W_STR
#define __SMART_W_STR

#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef __GNUC__
#include <wchar.h>
#endif

#include "../ebase.h"

BeginEsdlNamespace()

class TWString : public TTIString<wchar_t>, public IEObject {
public:
  class CharW  {
    size_t Index;
    TWString *Instance;
  public:
    CharW(size_t index, TWString *inst) {
      Index = index;
      Instance = inst;
    }
    inline wchar_t GetValue() const {  return Instance->CharAt(Index);  }
    inline operator wchar_t () const {  return Instance->CharAt(Index);  }
    inline void operator = (wchar_t v)  {  Instance->Set(Index, v);  }
  };
protected:
//..............................................................................
  TWString& AssignCharStr(const char*str, size_t len=~0);
//..............................................................................
  template <class T> inline TWString& writeType(const wchar_t *format, T v)  {
    wchar_t bf[80];
#if defined(_MSC_VER)
    swprintf_s(bf, 80, format, v);
#elif defined(__GNUC__) 
    swprintf(bf, 80, format, v);
#else
    swprintf(bf, format, v);
#endif
    size_t len = wcslen(bf);
    checkBufferForModification(_Length + len);
    olx_memcpy(&SData->Data[_Length], bf, len);
    _Length += len;
    return *this;
  }
//..............................................................................
  template <class T> inline void setTypeValue(const wchar_t *format, T v)  {
    _Start = 0;
    _Increment = 8;
    wchar_t bf[80]; // we could use dynamic memory with TTBuffer<T>::Alloc instead
#if defined(_MSC_VER)
    swprintf_s(bf, 80, format, v);
#elif defined(__GNUC__) 
    swprintf(bf, 80, format, v);
#else
    swprintf(bf, format, v);
#endif
    _Length = wcslen(bf);
    SData = new Buffer(_Length +_Increment, bf, _Length);
  }
//..............................................................................
  template <class T> inline TWString& assignTypeValue(const wchar_t *format, T v)  {
    wchar_t bf[80]; // we could use dynamic memory with TTBuffer<T>::Alloc instead
#if defined(_MSC_VER)
    swprintf_s(bf, 80, format, v);
#elif defined(__GNUC__) 
    swprintf(bf, 80, format, v);
#else
    swprintf(bf, format, v);
#endif
    _Start = 0;
    _Increment = 8;
    _Length = wcslen(bf);
    if( SData != NULL )  {
      if( SData->RefCnt == 1 )  { // owed by this object
        SData->SetCapacity(_Length);
        olx_memcpy(SData->Data, bf, _Length);
      }
      else  {
        SData->RefCnt--;
        SData = NULL;
      }
    }
    if( SData == NULL )  SData = new Buffer(_Length +_Increment, bf, _Length);
    return *this;
  }
  inline const wchar_t* printFormat(const char)                   const {  return L"%c";  }
  inline const wchar_t* printFormat(const short int)              const {  return L"%hd";  }
  inline const wchar_t* printFormat(const unsigned short int)     const {  return L"%hu";  }
  inline const wchar_t* printFormat(const int)                    const {  return L"%d";  }
  inline const wchar_t* printFormat(const unsigned int)           const {  return L"%u";  }
  inline const wchar_t* printFormat(const long int)               const {  return L"%ld";  }
  inline const wchar_t* printFormat(const unsigned long int)      const {  return L"%lu";  }
  inline const wchar_t* printFormat(const long long int)          const {  return L"%Ld";  }
  inline const wchar_t* printFormat(const unsigned long long int) const {  return L"%Lu";  }
  inline const wchar_t* printFormat(const float)                  const {  return L"%f";  }
  inline const wchar_t* printFormat(const double)                 const {  return L"%lf";  }
  inline const wchar_t* printFormat(const long double)            const {  return L"%Lf";  }
public:
  TWString();
  // simple convertion constructors
  TWString(const bool& v);
  TWString(const char& v);
  TWString(const char *str);
  TWString(char * const  &str);
//..........................................................................................
  TWString(const class TCString& astr);
//..........................................................................................
  TWString(const TTIString<char>& str);
  // primitive Type constructors
  template <typename T> TWString(const T &v)  {  setTypeValue( printFormat(v), v);  }
  // float numbers need trimming of the 0000
  TWString(const float& v)  {
    setTypeValue( printFormat(v), v);
    TrimFloat();
  }
  TWString(const double& v)  {
    setTypeValue( printFormat(v), v);
    TrimFloat();
  }

  virtual ~TWString()  {}

  TWString& operator << (const CharW &v);
  TWString& operator << (const char &v);
  TWString& operator << (const char *v)  { return Append(v, strlen(v));  }
  TWString& operator << (char * const  &v)  { return Append(v, strlen(v));  }
  template <typename T> inline TWString& operator << (const T &v)  {
    return writeType(printFormat(v), v);
  }
  TWString& TrimFloat()  {
    while( _Length > 1 && CharAt(_Length-1) == L'0' )  _Length--;
    if( _Length > 0 && CharAt(_Length-1) == L'.'  )  _Length--;
    return *this;
  }
  inline TWString& operator << (const float &v) {
    writeType(printFormat(v), v);
    TrimFloat();
    return *this;
  }
  inline TWString& operator << (const double &v) {
    writeType(printFormat(v), v);
    TrimFloat();
    return *this;
  }
  /* there is just no way with borland to put it TTIString as it would swear about
    [C++ Error] olx_istring.h(112): E2034 Cannot convert 'const wchar_t *' to 'const char *' */
  inline TWString& Append(const wchar_t *data, size_t len)  {
    checkBufferForModification(_Length + len);
    olx_memcpy(&SData->Data[_Start+_Length], data, len);
    _Length += len;
    return *this;
  }

  TWString& Append(const char *data, size_t len)  {
    checkBufferForModification(_Length + len);
    for( size_t i=0; i < len; i++ )
      SData->Data[_Start+_Length+i] = data[i];
    _Length += len;
    return *this;
  }
//..........................................................................................
  TWString& operator = (const TCString& astr); // cannot make it inline - forward reference...
  inline TWString& operator = (const char *str)  {  return AssignCharStr(str);  }
  inline TWString& operator = (char * const &str)  {  return AssignCharStr(str);  }
//..........................................................................................
  TWString& operator = (const char &ch);
  template <typename T> inline TWString& operator = (const T& v)  {
    return assignTypeValue(printFormat(v), v);
  }
  inline TWString& operator = (const float& v)  {
    assignTypeValue(printFormat(v), v);
    TrimFloat();
    return *this;
  }
  inline TWString& operator = (const double& v)  {
    assignTypeValue(printFormat(v), v);
    TrimFloat();
    return *this;
  }
protected:
  inline wchar_t * Data() const {  return ((SData==NULL) ? NULL : &SData->Data[_Start]);  }
public:
  const char * c_str() const;
  inline const wchar_t * u_str() const {  return ((SData==NULL) ? L"" : TTIString<wchar_t>::u_str());  }
  inline const wchar_t * wc_str() const {  return u_str();  }
  inline CharW operator[] (size_t i)  {
#ifdef _DEBUG
    if( i >= _Length )
      TExceptionBase::ThrowIndexOutOfRange(__POlxSourceInfo, i, 0, _Length);
#endif
    return CharW(i, this);
  }
  // very bizzare compilation errors occur if it is not redefined here
  inline wchar_t operator[] (size_t i) const {
#ifdef _DEBUG
    if( i >= _Length )
      TExceptionBase::ThrowIndexOutOfRange(__POlxSourceInfo, i, 0, _Length);
#endif
    return SData->Data[_Start + i];
  }
  inline void Set(size_t i, wchar_t v)  {
#ifdef _DEBUG
    if( i >= _Length )
      TExceptionBase::ThrowIndexOutOfRange(__POlxSourceInfo, i, 0, _Length);
#endif
    checkBufferForModification(_Length);
    SData->Data[_Start+i] = v;
  }
  //............................................................................
  virtual TIString ToString() const;
  //............................................................................
  friend class TWStrBuffer;
};


/* string buffer, reuses memory allocated by any posted TCString entry */
class TWStrBuffer {
  struct Entry {
    TWString::Buffer* Data;
    size_t Start, Length;
    Entry* Next;
  };
  size_t Length;
  Entry *Head, *Tail;
public:
  TWStrBuffer()  {
    Tail = Head = NULL;
    Length = 0;
  }
  TWStrBuffer(const TWString& v)  {
    Tail = Head = new Entry;
    Tail->Data = v.SData;
    Tail->Start = v._Start;
    Length = Tail->Length = v._Length;
    Tail->Next = NULL;
    v.SData->RefCnt++;
  }
  virtual ~TWStrBuffer()  {
    Entry* en = Head;
    while( en != NULL )  {
      Head = en->Next;
      if( --en->Data->RefCnt == 0 )
        delete en->Data;
      delete en;
      en = Head;
    }
  }

  TWStrBuffer& operator << (const TWString& v)  {
    if( Head == NULL )  {
      Tail = Head = new Entry;
    }
    else  {
      Tail->Next = new Entry;
      Tail = Tail->Next;
      Tail->Next = NULL;
    }
    Tail->Data = v.SData;
    Tail->Start = v._Start;
    Length += (Tail->Length = v._Length);
    Tail->Next = NULL;
    v.SData->RefCnt++;

    return *this;
  }

  wchar_t *Read(wchar_t *v)  {
    if( Head == NULL )
      v[0] = '\0';
    else  {
      Entry* en = Head;
      size_t read = 0;
      while( en != NULL )  {
        olx_memcpy(&v[read], &en->Data->Data[en->Start], en->Length);
        read += en->Length;
        en = en->Next;
      }
      v[read] = '\0';
    }
    return v;
  }
};
EndEsdlNamespace()

#endif
