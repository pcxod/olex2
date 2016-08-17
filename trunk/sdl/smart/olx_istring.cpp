/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "../ebase.h"
#include "../eutf8.h"
namespace esdl {

  template<> olxcstr olxcstr::FromCStr(
    const wchar_t* wstr, size_t len)
  {
    const size_t sz = (len == InvalidSize ? wcslen(wstr) : len);
    if (sz == 0) {
      return CEmptyString();
    }
    const size_t res = wcstombs(NULL, wstr, 0);
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

  template<> olxwstr olxwstr::FromCStr(
    const char* mbs, size_t len)
  {
    const size_t sz = (len == InvalidSize ? strlen(mbs) : len);
    if (sz == 0) {
      return WEmptyString();
    }
    const size_t res = mbstowcs(NULL, mbs, 0);
    if (res == InvalidSize) {
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo,
        "could not convert mbs to wcs");
    }
    olx_array_ptr<wchar_t> out = olx_malloc<wchar_t>(res + 1);
    mbstowcs(out(), mbs, res);
    return olxwstr::FromExternal(out.release(), len, len+1);
  }

  template<> olxwstr olxwstr::FromCStr(
    const wchar_t* str, size_t len)
  {
    return olxwstr(str, len);
  }

  template<> olxstr olxcstr::FromUTF8(
    const char* str, size_t len)
  {
    return TUtf8::Decode(str, len);
  }

  template<> olxstr olxwstr::FromUTF8(
    const char* str, size_t len)
  {
    return TUtf8::Decode(str, len);
  }

  template<> olxcstr olxwstr::ToMBStr() const {
    return olxcstr::FromCStr(this->wc_str(), this->Length());
  }

  template<> olxcstr olxwstr::ToUTF8() const {
    return TUtf8::Encode(this->raw_str(), this->Length());
  }

  template<> olxwstr olxwstr::ToWCStr() const {
    return *this;
  }

  template<> olxcstr TTSString<TCString, char>::ToMBStr() const {
    return *this;
  }

  template<> olxwstr olxcstr::ToWCStr() const {
    if (IsUTF8()) {
      return TUtf8::Decode(*this);
    }
    return olxwstr::FromCStr(this->c_str(), this->Length());
  }

  template<> olxcstr olxcstr::ToUTF8() const {
    return TUtf8::Encode(*this);
  }

  template class TTSString<TCString, char>;
  template class TTSString<TWString, wchar_t>;
}
