/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/
#include <cstdarg>
#include "ostring.h"
#include "../exception.h"

const olxcstr &esdl::CEmptyString()  {
  static olxcstr rv("");
  return rv;
}
const olxcstr &esdl::CTrueString()  {
  static olxcstr rv("true");
  return rv;
}
const olxcstr &esdl::CFalseString()  {
  static olxcstr rv("false");
  return rv;
}

const olxwstr &esdl::WEmptyString()  {
  static olxwstr rv(L"");
  return rv;
}
const olxwstr &esdl::WTrueString()  {
  static olxwstr rv(L"true");
  return rv;
}
const olxwstr &esdl::WFalseString()  {
  static olxwstr rv("false");
  return rv;
}

#ifdef _UNICODE
const olxstr &esdl::EmptyString()  {  return WEmptyString();  }
const olxstr &esdl::TrueString()  {  return WTrueString();  }
const olxstr &esdl::FalseString()  { return WFalseString();  }
#else
const olxstr &esdl::EmptyString()  {  return CEmptyString();  }
const olxstr &esdl::TrueString()  {  return CTrueString();  }
const olxstr &esdl::FalseString()  { return CFalseString();  }
#endif

olxstr esdl::olx_print(const char *format_, ...) {
  va_list argptr;
  va_start(argptr, format_);
  const olxstr format = format_;
  olxstr_buf rv;
  size_t str_st = 0;
  try {
    for (size_t i=0; i < format.Length(); i++) {
      size_t p_cnt=0;
      while (i < format.Length() && format.CharAt(i) == '%') {
        p_cnt++;
        i++;
      }
      if (i >= format.Length()) {
        rv << format.SubString(str_st, i-str_st-p_cnt/2);
        str_st = i; // finished here
        break;
      }
      if ((p_cnt&1) != 0) {
        rv << format.SubString(str_st, i-str_st-p_cnt/2-1);
        switch (format.CharAt(i)) {
        case 'l': {  // ll d/u l d/u/c/s
          if (++i < format.Length()) {
              switch (format.CharAt(i)) {
              case 'l': { // ll d/u
                if (++i < format.Length()) {
                  if (format.CharAt(i) == 'd')
                    rv <<  va_arg(argptr, long long int);
                  else if (format.CharAt(i) == 'u')
                    rv << va_arg(argptr, unsigned long long int);
                  else {
                    throw TInvalidArgumentException(__OlxSourceInfo,
                      olxstr("argument for ll: '") << format.CharAt(i) << '\'');
                  }
                }
              }
              break;
              case 'f':
                rv << va_arg(argptr, double);
                break;
              case 'i':
              case 'd':
                rv << va_arg(argptr, long int);
                break;
              case 'u':
                rv << va_arg(argptr, unsigned long int);
                break;
              case 'c':
#ifdef __GNUC__
                rv << va_arg(argptr, int);
#else
                rv << va_arg(argptr, wchar_t);
#endif
                break;
              case 's':
                rv << va_arg(argptr, const wchar_t*);
                break;
              default:
                throw TInvalidArgumentException(__OlxSourceInfo,
                  olxstr("argument for l: '") << format.CharAt(i) << '\'');
              } // switch
            }
          }
          break;
        case 'h': // h d/u
          if (++i < format.Length()) {
            if (format.CharAt(i) == 'd')
#ifdef __GNUC__
              rv << va_arg(argptr, int);
#else
              rv << va_arg(argptr, short int);
#endif
            else if (format.CharAt(i) == 'u')
#ifdef __GNUC__
              rv << va_arg(argptr, unsigned int);
#else
              rv << va_arg(argptr, unsigned short int);
#endif
            else
              throw TInvalidArgumentException(__OlxSourceInfo,
                olxstr("argument for h: '") << format.CharAt(i) << '\'');
          }
          break;
        case 'L': // L
          if (++i < format.Length()) {
            if (format.CharAt(i) == 'f')
              rv << va_arg(argptr, long double);
            else
              throw TInvalidArgumentException(__OlxSourceInfo,
                olxstr("argument for h: '") << format.CharAt(i) << '\'');
          }
          break;
        case 'f':
#ifdef __GNUC__
          rv << va_arg(argptr, double);
#else
          rv << va_arg(argptr, float);
#endif
          break;
        case 'i':
        case 'd':
          rv << va_arg(argptr, int);
          break;
        case 'u':
          rv << va_arg(argptr, unsigned int);
          break;
        case 'c':
#ifdef __GNUC__
          rv << va_arg(argptr, int);
#else
          rv << va_arg(argptr, char);
#endif
          break;
        case 's':
          rv << va_arg(argptr, const char*);
          break;
        case 'w':
          rv << *va_arg(argptr, olxstr*);
          break;
        default:
          throw TInvalidArgumentException(__OlxSourceInfo,
            olxstr("argument: '") << format.CharAt(i) << '\'');
        } // swicth
        str_st = i+1;
      }
      else if (p_cnt != 0) { // p_cnt is even
        rv << format.SubString(str_st, i-str_st-p_cnt/2);
        str_st = i; // finished here
      }
    }
  }
  catch (const TExceptionBase &e) {
    va_end(argptr);
    throw TFunctionFailedException(__OlxSourceInfo, e);
  }
  va_end(argptr);
  if (str_st < format.Length())
    rv << format.SubStringFrom(str_st);
  return olxstr(rv);
}
