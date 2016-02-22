/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "../ebase.h"

namespace esdl {

  template<> olxcstr olxcstr::FromCStr(
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
    str.Allocate(res + 1, false);
    wcstombs(str.raw_str(), wstr, res);
    str.SetLength(res);
    return str;
  }

  template<> olxcstr olxcstr::FromCStr(
    const char* str, size_t len)
  {
    return olxcstr(str, len);
  }

  template<> olxcstr TTSString<TCString, char>::ToMBStr() const {
    return *this;
  }

  template<> olxwstr olxwstr::FromCStr(
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
    str.Allocate(res+1, false);
    mbstowcs(str.raw_str(), mbs, res);
    str.SetLength(res);
    return str;
  }

  template<> olxwstr olxwstr::FromCStr(
    const wchar_t* str, size_t len)
  {
    return olxwstr(str, len);
  }

  template<> olxcstr olxwstr::ToMBStr() const {
    // as experience shown - need the '0'
    return olxcstr::FromCStr(this->u_str(), this->Length());
  }

  template<> olxwstr olxwstr::ToWCStr() const {
    return *this;
  }

  template<> olxwstr olxcstr::ToWCStr() const {
    return olxwstr::FromCStr(this->raw_str(), this->Length());
  }

  template class TTSString<TCString, char>;
  template class TTSString<TWString, wchar_t>;
}
