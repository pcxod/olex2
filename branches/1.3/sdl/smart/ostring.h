/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_sdl_ostr_H
#define __olx_sdl_ostr_H
#include "olx_istring.h"
BeginEsdlNamespace()

#ifdef __BORLANDC__

typedef TTSString<TCString, char> olxcstr;
typedef TTSString<TWString, wchar_t> olxwstr;

#ifdef _UNICODE
  typedef TTSString<TWString, wchar_t> olxstr;
#else
  typedef TTSString<TCString, char > olxstr;
#endif

extern const olxstr &EmptyString();
extern const olxstr &FalseString();
extern const olxstr &TrueString();

extern const olxcstr &CEmptyString();
extern const olxcstr &CFalseString();
extern const olxcstr &CTrueString();

extern const olxwstr &WEmptyString();
extern const olxwstr &WFalseString();
extern const olxwstr &WTrueString();
#endif

template <bool CaseInsensetive> class olxstrComparator {
public:
  olxstrComparator() {}
  template <class S1, class S2>
  inline int Compare(const S1& A, const S2& B) const {
    return CaseInsensetive ? A.Comparei(B): A.Compare(B);
  }
};

// this throws exceptions
extern olxstr olx_print(const char *format, ...);
EndEsdlNamespace()
#endif
