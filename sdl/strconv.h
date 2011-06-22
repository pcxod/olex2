/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_strconv_H
#define __olx_sdl_strconv_H
#include "ebase.h"
BeginEsdlNamespace()

static olxcstr WC2MB(const olxwstr& wstr)  {
  olxcstr cstr(CEmptyString(), MB_CUR_MAX*wstr.Length());
  size_t n=0;
  if( (n=wcstombs( cstr.raw_str(), wstr.raw_str(), cstr.Length() )) == -1 )
    TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "could not convert the sequence");
  cstr.SetLength(n);
  return cstr;
}

static olxwstr MB2WC(const olxcstr& cstr)  {
  olxwstr wstr(WEmptyString(), cstr.Length());  // better more than less ... /MB_CUR_MAX not sure if gonna work
  size_t n=0;
  if( (n=mbstowcs(wstr.raw_str(), cstr.raw_str(), cstr.Length())) == -1 )
    TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "could not convert the sequence");
  wstr.SetLength(n);
  return wstr;
}

EndEsdlNamespace()
