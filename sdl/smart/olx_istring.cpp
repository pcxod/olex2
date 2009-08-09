#include "ebase.h"

template <class T, typename TC>
CString esdl::TTSString<T,TC>::WStr2CStr(const wchar_t* wstr, size_t len)  {
  const size_t sz = (len == ~0 ? wcslen(wstr) : len);
  if( sz == 0 )
    return CEmptyString;
  const int res = wcstombs(NULL, wstr, sz);
  if( res == -1 )
    TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "could not convert wcs to mbs");
  CString str;
  str.Allocate(res);
  wcstombs(str.raw_str(), wstr, sz);
  return str;
}
template <class T, typename TC>
CString esdl::TTSString<T,TC>::WStr2CStr(const WString& str)  {  return WStr2CStr(str.wc_str(), str.Length());  }

template <class T, typename TC>
WString esdl::TTSString<T,TC>::CStr2WStr(const char* mbs, size_t len)  {
  const size_t sz = (len == ~0 ? strlen(mbs) : len);
  if( sz == 0 )
    return WEmptyString;
  const int res = mbstowcs(NULL, mbs, sz);
  if( res == -1 )
    TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "could not convert mbs to wcs");
  WString str;
  str.Allocate(res);
  mbstowcs(str.raw_str(), mbs, sz);
  return str;
}
template <class T, typename TC>
WString esdl::TTSString<T,TC>::CStr2WStr(const CString& str)  {  return CStr2WStr(str.c_str(), str.Length());  }

template class esdl::TTSString<TCString, char>;
template class esdl::TTSString<TWString, wchar_t>;

