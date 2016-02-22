/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "../ebase.h"

template<> olxstr esdl::TTSString<TCString, char>::FromCStr(
  const wchar_t* wstr, size_t len)
{
  const size_t sz = (len == InvalidSize ? wcslen(wstr) : len);
  if (sz == 0) {
    return CEmptyString();
  }
  const size_t res = wcstombs(NULL, wstr, sz);
  if (res == InvalidSize) {
    TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo,
      "could not convert wcs to mbs");
  }
  olxcstr str;
  str.Allocate(res+1, false);
  wcstombs(str.raw_str(), wstr, res);
  str.SetLength(res);
  return str;
}

template<> olxstr esdl::TTSString<TCString, char>::FromCStr(
  const char* str, size_t len)
{
  return olxcstr(str, len);
}

template<> olxcstr esdl::TTSString<TCString, char>::ToMBStr() const {
  return *this;
}

template<> olxwstr esdl::TTSString<TCString, char>::ToWCStr() const {
  return olxwstr::FromCStr(this->raw_str(), this->Length());
}

template<> olxstr esdl::TTSString<TWString, wchar_t >::FromCStr(
  const char* mbs, size_t len)
{
  const size_t sz = (len == InvalidSize ? strlen(mbs) : len);
  if (sz == 0) {
    return WEmptyString();
  }
  const size_t res = mbstowcs(NULL, mbs, sz);
  if (res == InvalidSize) {
    TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo,
      "could not convert mbs to wcs");
  }
  olxwstr str;
  str.Allocate(res, true);
  mbstowcs(str.raw_str(), mbs, res);
  return str;
}

template<> olxstr esdl::TTSString<TWString, wchar_t >::FromCStr(
  const wchar_t* str, size_t len)
{
  return olxwstr(str, len);
}

template<> olxcstr esdl::TTSString<TWString, wchar_t>::ToMBStr() const {
  return olxcstr::FromCStr(this->raw_str(), this->Length());
}

template<> olxwstr esdl::TTSString<TWString, wchar_t>::ToWCStr() const {
  return *this;
}


template class esdl::TTSString<TCString, char>;
template class esdl::TTSString<TWString, wchar_t>;
