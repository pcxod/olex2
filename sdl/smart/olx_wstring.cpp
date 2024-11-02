/******************************************************************************
* Copyright (c) 2004-2024 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include <locale.h>
#include "../ebase.h"
#include "olx_wstring.h"
#include "olx_cstring.h"
#include "../egc.h"


TWString::TWString() {
  SData = 0;
  _Start = _Length = 0;
  _Increment = 8;
}
//..............................................................................
TWString::TWString(const bool& v)
  : TTIString<wchar_t>(v ? WTrueString() : WFalseString())
{}
//..............................................................................
void TWString::init(const char* str, size_t len) {
  _Start = 0;
  _Increment = 8;
  _Length = len == InvalidIndex ? strlen(str) : len;
  SData = new Buffer(_Length + _Increment);
  for (size_t i = 0; i < _Length; i++) {
    SData->Data[i] = str[i];
  }
}
//..............................................................................
TWString::TWString(const char* str) {
  init(str);
}
TWString::TWString(char* const& str) {
  init(str);
}
TWString::TWString(const TCString& str) {
  init(str.c_str(), str.Length());
}
TWString::TWString(const TTIString<char>& str) {
  init(str.raw_str(), str.Length());
}
// primitive Type constructor
TWString::TWString(const char& v) {
  _Start = 0;
  _Increment = 8;
  _Length = 1;  // one entity!!
  SData = new Buffer(_Increment);
  SData->Data[0] = v;
}

void TWString::OnCopy(const TWString&) {
}

const char* TWString::c_str() const {
  return TEGC::Add(new TCString(*this)).c_str();
}

TWString& TWString::operator << (const CharW& v) {
  checkBufferForModification(_Length + 1);
  SData->Data[_Length] = v.GetValue();
  _Length++;
  return *this;
}

TWString& TWString::operator << (const char& v) {
  checkBufferForModification(_Length + 1);
  SData->Data[_Length] = v;
  _Length++;
  return *this;
}

TWString& TWString::AssignCharStr(const char* str, size_t len) {
  _Start = 0;
  _Increment = 5;
  _Length = ((len == InvalidSize) ? strlen(str) : len);
  if (SData != 0) {
    if (SData->RefCnt == 1)  // owed by this object
      SData->SetCapacity(_Length);
    else {
      SData->RefCnt--;
      SData = 0;
    }
  }
  if (SData == 0) {
    SData = new Buffer(_Length + _Increment);
  }
  for (size_t i = 0; i < _Length; i++) {
    SData->Data[i] = str[i];
  }
  return *this;
}

TWString& TWString::operator = (const char& ch) {
  _Start = 0;
  _Increment = 8;
  _Length = 1;
  if (SData != 0) {
    if (SData->RefCnt == 1) { // owed by this object
      SData->SetCapacity(_Length);
    }
    else {
      SData->RefCnt--;
      SData = 0;
    }
  }
  if (SData == 0) {
    SData = new Buffer(_Increment);
  }
  SData->Data[0] = ch;
  return *this;
}
//..............................................................................
TWString& TWString::operator = (const TCString& astr) {
  return AssignCharStr(astr.raw_str(), astr.Length());
}
//..............................................................................
#ifdef _UNICODE
TIString TWString::ToString() const { return *this; }
#else
TIString TWString::ToString() const { return TCString(*this); }
#endif
