/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_wstr_H
#define __olx_sdl_wstr_H
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef __GNUC__
#include <wchar.h>
#endif

#include "../ebase.h"
#include "../linked_operators.h"
BeginEsdlNamespace()

class TWString : public TTIString<wchar_t>, public IOlxObject {
public:
  typedef TTIString<wchar_t> parent_t;
  class CharW : public linked_operators<wchar_t, CharW, char> {
    size_t Index;
    TWString* Instance;
  public:
    CharW(size_t index, TWString* inst) {
      Index = index;
      Instance = inst;
    }
    inline wchar_t GetValue() const { return Instance->CharAt(Index); }
    void SetValue(wchar_t v) { Instance->Set(Index, v); }
    CharW& operator = (wchar_t v) { Instance->Set(Index, v);  return *this; }
    CharW& operator = (int v) { Instance->Set(Index, wchar_t(v));  return *this; }
    CharW& operator = (char v) { Instance->Set(Index, v);  return *this; }
    CharW& operator = (const CharW& v) {
      Instance->Set(Index, v.GetValue());
      return *this;
    }
  };
protected:
  //..............................................................................
  TWString& AssignCharStr(const char* str, size_t len = ~0);
  //..............................................................................
  template <class T> inline TWString& writeType(const wchar_t* format, T v) {
    olx_array_ptr<wchar_t> bf(80);
#if defined(_MSC_VER)
    swprintf_s(bf, 80, format, v);
#elif defined(__GNUC__) && !defined(__WIN32__)
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
  template <class T> inline void setTypeValue(const wchar_t* format, T v) {
    _Start = 0;
    _Increment = 8;
    olx_array_ptr<wchar_t> bf(80);
#if defined(_MSC_VER)
    swprintf_s(bf, 80, format, v);
#elif defined(__GNUC__) && !defined(__WIN32__)
    swprintf(bf, 80, format, v);
#else
    swprintf(bf, format, v);
#endif
    _Length = wcslen(bf);
    SData = new Buffer(_Length + _Increment, bf, _Length);
  }
  //..............................................................................
  template <class T> inline TWString& assignTypeValue(const wchar_t* format, T v) {
    olx_array_ptr<wchar_t> bf(80);
#if defined(_MSC_VER)
    swprintf_s(bf, 80, format, v);
#elif defined(__GNUC__) &&!defined(__WIN32__)
    swprintf(*bf, 80, format, v);
#else
    swprintf(*bf, format, v);
#endif
    _Start = 0;
    _Increment = 8;
    _Length = wcslen(bf);
    if (SData != 0) {
      if (SData->RefCnt == 1) { // owed by this object
        SData->SetCapacity(_Length);
        olx_memcpy(SData->Data, bf, _Length);
      }
      else {
        SData->RefCnt--;
        SData = 0;
      }
    }
    if (SData == 0) {
      SData = new Buffer(_Length + _Increment, bf, _Length);
    }
    return *this;
  }
  void OnCopy(const TWString&);
  void init(const char* str, size_t len = InvalidIndex);
public:
  TWString();
  // simple convertion constructors
  TWString(const bool& v);
  TWString(const char& v);
  TWString(const char* str);
  TWString(char* const& str);
  //..........................................................................................
  TWString(const class TCString& astr);
  //..........................................................................................
  TWString(const TTIString<char>& str);
  // primitive Type constructors
  template <typename T> TWString(const T& v) { setTypeValue(printFormat(v), v); }
  // float numbers need trimming of the 0000
  TWString(const float& v) {
    setTypeValue(printFormat(v), v);
    parent_t::TrimFloat();
  }
  TWString(const double& v) {
    setTypeValue(printFormat(v), v);
    parent_t::TrimFloat();
  }

  virtual ~TWString() {}

  TWString& operator << (const CharW& v);
  TWString& operator << (const char& v);
  TWString& operator << (const char* v) { return Append(v, strlen(v)); }
  TWString& operator << (char* const& v) { return Append(v, strlen(v)); }
  template <typename T> inline TWString& operator << (const T& v) {
    return writeType(printFormat(v), v);
  }
  TWString& operator << (const float& v) {
    writeType(printFormat(v), v);
    parent_t::TrimFloat();
    return *this;
  }
  TWString& operator << (const double& v) {
    writeType(printFormat(v), v);
    parent_t::TrimFloat();
    return *this;
  }
  /* there is just no way with borland to put it TTIString as it would swear about
    [C++ Error] olx_istring.h(112): E2034 Cannot convert 'const wchar_t *' to 'const char *' */
  TWString& Append(const wchar_t* data, size_t len) {
    checkBufferForModification(_Length + len);
    olx_memcpy(&SData->Data[_Start + _Length], data, len);
    _Length += len;
    return *this;
  }

  TWString& Append(const char* data, size_t len) {
    checkBufferForModification(_Length + len);
    for (size_t i = 0; i < len; i++) {
      SData->Data[_Start + _Length + i] = data[i];
    }
    _Length += len;
    return *this;
  }
  //..........................................................................................
  TWString& operator = (const TCString& astr); // cannot make it inline - forward reference...
  TWString& operator = (const char* str) { return AssignCharStr(str); }
  TWString& operator = (char* const& str) { return AssignCharStr(str); }
  //..........................................................................................
  TWString& operator = (const char& ch);
  template <typename T> inline TWString& operator = (const T& v) {
    return assignTypeValue(printFormat(v), v);
  }
  TWString& operator = (const float& v) {
    assignTypeValue(printFormat(v), v);
    parent_t::TrimFloat();
    return *this;
  }
  TWString& operator = (const double& v) {
    assignTypeValue(printFormat(v), v);
    parent_t::TrimFloat();
    return *this;
  }
protected:
  inline wchar_t* Data() const { return ((SData == 0) ? 0 : &SData->Data[_Start]); }
public:
  const char* c_str() const;
  inline const wchar_t* u_str() const {
    return ((SData == 0) ? L"" : TTIString<wchar_t>::u_str());
  }
  inline const wchar_t* wc_str() const { return u_str(); }
  inline CharW operator[] (size_t i) {
#ifdef _DEBUG
    if (i >= _Length)
      TExceptionBase::ThrowIndexOutOfRange(__POlxSourceInfo, i, 0, _Length);
#endif
    return CharW(i, this);
  }
  // very bizzare compilation errors occur if it is not redefined here
  inline wchar_t operator[] (size_t i) const {
#ifdef _DEBUG
    if (i >= _Length) {
      TExceptionBase::ThrowIndexOutOfRange(__POlxSourceInfo, i, 0, _Length);
    }
#endif
    return SData->Data[_Start + i];
  }
  inline void Set(size_t i, wchar_t v) {
#ifdef _DEBUG
    if (i >= _Length) {
      TExceptionBase::ThrowIndexOutOfRange(__POlxSourceInfo, i, 0, _Length);
    }
#endif
    checkBufferForModification(_Length);
    SData->Data[_Start + i] = v;
  }
  //............................................................................
  virtual TIString ToString() const;
  //............................................................................
  static const wchar_t* printFormat(const char)                   { return L"%c"; }
  static const wchar_t* printFormat(const short int)              { return L"%hd"; }
  static const wchar_t* printFormat(const unsigned short int)     { return L"%hu"; }
  static const wchar_t* printFormat(const int)                    { return L"%d"; }
  static const wchar_t* printFormat(const unsigned int)           { return L"%u"; }
  static const wchar_t* printFormat(const long int)               { return L"%ld"; }
  static const wchar_t* printFormat(const unsigned long int)      { return L"%lu"; }
  static const wchar_t* printFormat(const long long int)          { return L"%lld"; }
  static const wchar_t* printFormat(const unsigned long long int) { return L"%llu"; }
  static const wchar_t* printFormat(const float)                  { return L"%f"; }
  static const wchar_t* printFormat(const double)                 { return L"%lf"; }
  static const wchar_t* printFormat(const long double)            { return L"%Lf"; }
};

#include "strbuf.h"

EndEsdlNamespace()
#endif
