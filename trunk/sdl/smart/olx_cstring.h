#ifndef __SMART_C_STR
#define __SMART_C_STR

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef __BORLANDC__
  #include <mem.h>
#endif
#include <math.h>

#include "../ebase.h"

BeginEsdlNamespace()

//template <class,typename> class TTSString;

/* this class uses reference counting to reduce number of memory reallocations*/
class TCString : public TTIString<char>, public IEObject{
public:
  class CharW  {
    size_t Index;
    TCString *Instance;
  public:
    CharW(size_t index, TCString *inst) {
      Index = index;
      Instance = inst;
    }
    inline char GetValue() const {  return Instance->CharAt(Index);  }
    inline operator char () const {  return Instance->CharAt(Index);  }
    inline void operator = (char v)  {  Instance->Set(Index, v);  }
  };
protected:
//..............................................................................
  TCString& AssignWCharStr(const wchar_t* str, size_t len=~0);
//..............................................................................
  template <class T> inline TCString& writeType(const char *format, T v)  {
    char bf[80];
#if defined(_MSC_VER)
    sprintf_s(bf, 80, format, v);
#else
    sprintf(bf, format, v);
#endif
    size_t len = strlen(bf);
    checkBufferForModification(_Length + len);
    olx_memcpy(&SData->Data[_Length], bf, len);
    _Length += len;
    return *this;
  }
//..............................................................................
  template <class T> inline void setTypeValue(const char *format, T v)  {
    _Start = 0;
    _Increment = 8;
    char bf[80]; // we could use dynamic memory with TTBuffer<T>::Alloc instead
#if defined(_MSC_VER)
    sprintf_s(bf, 80, format, v);
#else
    sprintf(bf, format, v);
#endif
    _Length = strlen(bf);
    SData = new Buffer(_Length +_Increment, bf, _Length);
  }
//..............................................................................
  template <class T> inline TCString& assignTypeValue(const char *format, T v)  {
    char bf[80]; // we could use dynamic memory with TTBuffer<T>::Alloc instead
#if defined(_MSC_VER)
    sprintf_s(bf, 80, format, v);
#else
    sprintf(bf, format, v);
#endif
    _Start = 0;
    _Increment = 8;
    _Length = strlen(bf);
    if( SData != NULL )  {
      if( SData->RefCnt == 1 )  { // owed by this object
        SData->SetCapacity(_Length);
        memcpy(SData->Data, bf, _Length*CharSize);
      }
      else  {
        SData->RefCnt--;
        SData = NULL;
      }
    }
    if( SData == NULL )  SData = new Buffer(_Length +_Increment, bf, _Length);
    return *this;
  }
  inline const char* printFormat(const char)                   const {  return "%c";  }
  inline const char* printFormat(const short int)              const {  return "%hd";  }
  inline const char* printFormat(const unsigned short int)     const {  return "%hu";  }
  inline const char* printFormat(const int)                    const {  return "%d";  }
  inline const char* printFormat(const unsigned int)           const {  return "%u";  }
  inline const char* printFormat(const long int)               const {  return "%ld";  }
  inline const char* printFormat(const unsigned long int)      const {  return "%lu";  }
  inline const char* printFormat(const long long int)          const {  return "%Ld";  }
  inline const char* printFormat(const unsigned long long int) const {  return "%Lu";  }
  inline const char* printFormat(const float)                  const {  return "%f";  }
  inline const char* printFormat(const double)                 const {  return "%lf";  }
  inline const char* printFormat(const long double)            const {  return "%Lf";  }
public:
  TCString();
  TCString(const wchar_t *wstr );
//..........................................................................................
  TCString(const class TWString& astr);
//..........................................................................................
  TCString(const bool& v);
  TCString(const TTIString<wchar_t>& wstr );
  template <typename T> TCString(const T& v)  { setTypeValue( printFormat(v), v);  }
  // float numbers need trimming of the 0000
  TCString(const float& v)  {
    setTypeValue(printFormat(v), v);
    TrimFloat();
  }
  TCString(const double& v)  {
    setTypeValue(printFormat(v), v);
    TrimFloat();
  }
  virtual ~TCString()  {}
  TCString& operator << (const CharW &v);
  template <typename T> inline TCString& operator << (const T &v) {
    return writeType(printFormat(v), v);
  }
  TCString& TrimFloat()  {
    while( _Length > 1 && CharAt(_Length-1) == '0' )  _Length--;
    if( _Length > 0 && CharAt(_Length-1) == '.'  )  _Length--;
    return *this;
  }
  inline TCString& operator << (const float &v) {
    writeType(printFormat(v), v);
    TrimFloat();
    return *this;
  }
  inline TCString& operator << (const double &v) {
    writeType(printFormat(v), v);
    TrimFloat();
    return *this;
  }
  /* there is just no way with borland to put it TTIString as it would swear about
    [C++ Error] olx_istring.h(112): E2034 Cannot convert 'const wchar_t *' to 'const char *' */
  inline TCString& Append(const char *data, size_t len)  {
    checkBufferForModification(_Length + len);
    olx_memcpy( &SData->Data[_Start+_Length], data, len);
    _Length += len;
    return *this;
  }
  inline TCString& Append(const wchar_t *data, size_t len)  {
    checkBufferForModification(_Length + len);
    for( size_t i=0; i < len; i++ )
      SData->Data[_Start+_Length+i] = (char)data[i];
    _Length += len;
    return *this;
  }
//..........................................................................................
  TCString& operator = (const TWString& astr);  // cannot make it inle - froward reference
  inline TCString& operator = (const wchar_t* v)  {  return AssignWCharStr(v);  }
  inline TCString& operator = (wchar_t* const& v) { return AssignWCharStr(v);  }
//..........................................................................................
  template <typename T> inline TCString& operator = (const T& v) {
    return assignTypeValue(printFormat(v), v);
  }
  inline TCString& operator = (const float& v) {
    assignTypeValue(printFormat(v), v);
    TrimFloat();
    return *this;
  }
  inline TCString& operator = (const double& v) {
    assignTypeValue(printFormat(v), v);
    TrimFloat();
    return *this;
  }
//..........................................................................................
protected:
  inline char * Data()     const {  return ((SData==NULL) ? NULL :&SData->Data[_Start]);  }
public:
  const wchar_t * wc_str() const;
  inline const char *u_str() const { return ((SData==NULL) ? "" : TTIString<char>::u_str());  }
  inline const char * c_str()  const {  return u_str();  }
  inline CharW operator[] (size_t i)  {
#ifdef _DEBUG
    if( i >= _Length )
      TExceptionBase::ThrowIndexOutOfRange(__POlxSourceInfo, i, 0, _Length);
#endif
    return CharW(i, this);
  }
  // very bizzare compilation errors occur if it is not redefined here
  inline char operator[] (size_t i) const {
#ifdef _DEBUG
    if( i >= _Length )
      TExceptionBase::ThrowIndexOutOfRange(__POlxSourceInfo, i, 0, _Length);
#endif
    return SData->Data[_Start + i];
  }
  inline void Set(size_t i, char v)  {
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
  friend class TCStrBuffer;
};


/* string buffer, reuses memory allocated by any posted TCString entry */
class TCStrBuffer {
  struct Entry {
    TCString::Buffer* Data;
    size_t Start, Length;
    Entry* Next;
  };
  size_t Length;
  Entry *Head, *Tail;
public:
  TCStrBuffer()  {
    Tail = Head = NULL;
    Length = 0;
  }
  TCStrBuffer(const TCString& v)  {
    Tail = Head = new Entry;
    Tail->Data = v.SData;
    Tail->Start = v._Start;
    Length = Tail->Length = v._Length;
    Tail->Next = NULL;
    v.SData->RefCnt++;
  }
  virtual ~TCStrBuffer()  {
    Entry* en = Head;
    while( en != NULL )  {
      Head = en->Next;
      if( --en->Data->RefCnt == 0 )
        delete en->Data;
      delete en;
      en = Head;
    }
  }

  TCStrBuffer& operator << (const TCString& v)  {
    if( Head == NULL )
      Tail = Head = new Entry;
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

  char *Read(char *v)  {
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

