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

static bool olx_is_float(const float&) { return true; }
static bool olx_is_float(const double&) { return true; }
static bool olx_is_float(const long double&) { return true; }
template <typename T>
static bool olx_is_float(const T&) { return false; }

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
olxstr olx_to_str(T n, const char *fmt=0) {
  if (fmt == 0) {
    return olx_print(olxcstr::printFormat(n), n);
  }
  return olx_print(fmt, n);
}

EndEsdlNamespace()
#endif
