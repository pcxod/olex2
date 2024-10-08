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
#include <cstdarg>
#include "olx_istring.h"
#include "../type_splitter.h"
BeginEsdlNamespace()

template <bool CaseInsensetive> class olxstrComparator {
public:
  olxstrComparator() {}
  template <class S1, class S2>
  inline int Compare(const S1& A, const S2& B) const {
    return CaseInsensetive ? A.Comparei(B): A.Compare(B);
  }
};

// this throws exceptions, manages va_end
extern olxstr olx_print_(const char* format, va_list args);
extern olxstr olx_print(const char* format, ...);
extern olxstr olx_print(olxcstr format, ...);

static const char* olx_print_format(const char) { return "c"; }
static const char* olx_print_format(const short int) { return "d"; }
static const char* olx_print_format(const wchar_t) { return "c"; }
static const char* olx_print_format(const unsigned short int) { return "u"; }
static const char* olx_print_format(const int) { return "d"; }
static const char* olx_print_format(const unsigned int) { return "u"; }
static const char* olx_print_format(const long int) { return "d"; }
static const char* olx_print_format(const unsigned long int) { return "u"; }
static const char* olx_print_format(const long long int) { return "d"; }
static const char* olx_print_format(const unsigned long long int) { return "u"; }
static const char* olx_print_format(const float) { return "f"; }
static const char* olx_print_format(const double) { return "f"; }
static const char* olx_print_format(const long double) { return "f"; }

static const char* olx_format_modifier(const char) { return ""; }
static const char* olx_format_modifier(const wchar_t) { return "c"; }
static const char* olx_format_modifier(const short int) { return "h"; }
static const char* olx_format_modifier(const unsigned short int) { return "h"; }
static const char* olx_format_modifier(const int) { return ""; }
static const char* olx_format_modifier(const unsigned int) { return ""; }
static const char* olx_format_modifier(const long int) { return "l"; }
static const char* olx_format_modifier(const unsigned long int) { return "l"; }
static const char* olx_format_modifier(const long long int) { return "ll"; }
static const char* olx_format_modifier(const unsigned long long int) { return "ll"; }
static const char* olx_format_modifier(const float) { return ""; }
static const char* olx_format_modifier(const double) { return "l"; }
static const char* olx_format_modifier(const long double) { return "L"; }

template <typename T>
const T& olx_get_primitive_type(const T& n) {
  return n;
}

template <typename T>
olxstr olx2str_ext(T n, const olxcstr &f) {
  return olx_print(f, n);
}

template <typename T>
olxstr strof(T n) {
  return olxstr(n);
}

static olxstr strof(const char* str) {
  return olxstr(str);
}

static olxstr strof(const wchar_t* str) {
  return olxstr(str);
}

template <typename ch_t>
olxstr strof(const TTIString<ch_t> &str) {
  return olxstr(str);
}

EndEsdlNamespace()
#endif
