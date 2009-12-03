#ifndef __OLX_STRCONV__
#define __OLX_STRCONV__
#include "ebase.h"

BeginEsdlNamespace()

static olxcstr WC2MB(const olxwstr& wstr)  {
  olxcstr cstr(CEmptyString, MB_CUR_MAX*wstr.Length());
  size_t n=0;
  if( (n=wcstombs( cstr.raw_str(), wstr.raw_str(), cstr.Length() )) == -1 )
    TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "could not convert the sequence");
  cstr.SetLength(n);
  return cstr;
}
  //............................................................................
static olxwstr MB2WC(const olxcstr& cstr)  {
  olxwstr wstr(WEmptyString, cstr.Length());  // better more than less ... /MB_CUR_MAX not sure if gonna work
  size_t n=0;
  if( (n=mbstowcs(wstr.raw_str(), cstr.raw_str(), cstr.Length())) == -1 )
    TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "could not convert the sequence");
  wstr.SetLength(n);
  return wstr;
}

EndEsdlNamespace()
