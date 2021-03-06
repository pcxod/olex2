/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include <string>
#include "../ebase.h"
#include "olx_cstring.h"
#include "olx_wstring.h"
#include "../egc.h"

TCString::TCString()
  : utf8(false)
{
  SData = 0;
  _Start = _Length = 0;
  _Increment = 8;
  utf8 = false;
}

TCString::TCString(const bool& v)
  : TTIString<char>(v ? CTrueString(): CFalseString()), utf8(false)
{}

void TCString::init(const wchar_t* wstr, size_t l) {
  _Start = 0;
  _Increment = 8;
  _Length = l == InvalidIndex ? wcslen(wstr) : l;
  SData = new Buffer(_Length + _Increment);
  for (size_t i = 0; i < _Length; i++) {
    if (((unsigned)wstr[i]) > 255) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        "Char out of range for MBStr");
    }
    SData->Data[i] = (char)wstr[i];
  }
}

TCString::TCString(const wchar_t *wstr)
  : utf8(false)
{
  init(wstr);
}

TCString::TCString(const TWString& wstr)
  : utf8(false)
{
  init(wstr.u_str(), wstr.Length());
}

TCString::TCString(const TTIString<wchar_t>& wstr)
  : utf8(false)
{
  init(wstr.u_str(), wstr.Length());
}

void TCString::OnCopy(const TCString& cstr) {
  utf8 = cstr.IsUTF8();
}

TCString& TCString::AssignWCharStr(const wchar_t* wstr, size_t len) {
  _Start = 0;
  _Increment = 8;
  _Length = ((len == InvalidSize) ? wcslen(wstr) : len);
  if (SData != 0) {
    if (SData->RefCnt == 1) { // owed by this object
      SData->SetCapacity(_Length);
    }
    else {
      SData->RefCnt--;
      SData = 0;
    }
  }
  if (SData == 0) { // make sure enough space for terminating \0
    SData = new Buffer(_Length + 1);
  }
  for (size_t i = 0; i < _Length; i++) {
    if (((unsigned)wstr[i]) > 255) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        "Char out of range for MBStr");
    }
    SData->Data[i] = (char)wstr[i];
  }
  return *this;
}

TCString& TCString::operator = (const TWString& astr)  {
  return AssignWCharStr(astr.raw_str(), astr.Length());
}

const wchar_t * TCString::wc_str() const {
  return TEGC::Add(new TWString(*this)).wc_str();
}

TCString& TCString::operator << (const CharW &v)  {
  checkBufferForModification(_Length + 1);
  SData->Data[_Length] = v.GetValue();
  _Length ++;
  return *this;
}

#ifdef _UNICODE
  TIString TCString::ToString() const {  return TWString(*this);  }
#else
  TIString TCString::ToString() const {  return *this;  }
#endif
