/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "../ebase.h"

template <class T, typename TC>
olxcstr esdl::TTSString<T,TC>::WStr2CStr(const wchar_t* wstr, size_t len)  {
  const size_t sz = (len == InvalidSize ? wcslen(wstr) : len);
  if (sz == 0) {
    return CEmptyString();
  }
  const size_t res = wcstombs(NULL, wstr, sz);
  if (res == (size_t)-1) {
    TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo,
      "could not convert wcs to mbs");
  }
  olxcstr str;
  str.Allocate(res, true);
  wcstombs(str.raw_str(), wstr, res);
  return str;
}
template <class T, typename TC>
olxcstr esdl::TTSString<T,TC>::WStr2CStr(const olxwstr& str)  {
  return WStr2CStr(str.wc_str(), str.Length());
}

template <class T, typename TC>
olxwstr esdl::TTSString<T,TC>::CStr2WStr(const char* mbs, size_t len)  {
  const size_t sz = (len == InvalidSize ? strlen(mbs) : len);
  if (sz == 0) {
    return WEmptyString();
  }
  const size_t res = mbstowcs(NULL, mbs, sz);
  if (res == (size_t)-1) {
    TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo,
      "could not convert mbs to wcs");
  }
  olxwstr str;
  str.Allocate(res, true);
  mbstowcs(str.raw_str(), mbs, res);
  return str;
}
template <class T, typename TC>
olxwstr esdl::TTSString<T, TC>::CStr2WStr(const olxcstr& str) {
  return CStr2WStr(str.c_str(), str.Length());
}

template <class T, typename TC>
olxcstr esdl::TTSString<T, TC>::WStr2CStr(const char* wstr, size_t len = ~0) {
  return olxcstr(wstr, len == InvalidIndex ? olxstr::o_strlen(wstr) : len);
}


template <class T, typename TC>
olxcstr esdl::TTSString<T, TC>::ToCStr() const {
  return WStr2CStr(*this);
}
template <class T, typename TC>
olxwstr esdl::TTSString<T, TC>::CStr2WStr(const wchar_t* mbs, size_t len) {
  return olxwstr(mbs, len == InvalidIndex ? olxstr::o_strlen(mbs) : len);
}

template <class T, typename TC>
olxwstr esdl::TTSString<T, TC>::ToWStr() const {
  return CStr2WStr(*this);
}


template class esdl::TTSString<TCString, char>;
template class esdl::TTSString<TWString, wchar_t>;
