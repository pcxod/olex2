/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/
#include "ostring.h"
#include "../exception.h"

const olxcstr &esdl::CEmptyString()  {
  static olxcstr rv("");
  return rv;
}
//..............................................................................
const olxcstr &esdl::CTrueString()  {
  static olxcstr rv("true");
  return rv;
}
//..............................................................................
const olxcstr &esdl::CFalseString()  {
  static olxcstr rv("false");
  return rv;
}
//..............................................................................
const olxwstr &esdl::WEmptyString()  {
  static olxwstr rv(L"");
  return rv;
}
//..............................................................................
const olxwstr &esdl::WTrueString()  {
  static olxwstr rv(L"true");
  return rv;
}
//..............................................................................
const olxwstr &esdl::WFalseString()  {
  static olxwstr rv("false");
  return rv;
}
//..............................................................................
#ifdef _UNICODE
const olxstr &esdl::EmptyString()  {  return WEmptyString();  }
const olxstr &esdl::TrueString()  {  return WTrueString();  }
const olxstr &esdl::FalseString()  { return WFalseString();  }
#else
const olxstr &esdl::EmptyString()  {  return CEmptyString();  }
const olxstr &esdl::TrueString()  {  return CTrueString();  }
const olxstr &esdl::FalseString()  { return CFalseString();  }
#endif
//..............................................................................
struct olx_print_i_cont {
  virtual ~olx_print_i_cont() = default;
  virtual olxstr ToString() = 0;
};
//..............................................................................
template <typename T>
struct olx_print_cont : public olx_print_i_cont {
  const T value;
  olx_print_cont(const T &v)
    : value(v)
  {}
  virtual olxstr ToString() {
    return olxstr(value);
  }
};
//..............................................................................
template <typename T>
struct olx_print_cont_f : public olx_print_i_cont {
  const T value;
  int fp_cnt;
  bool expf, trim;
  olx_print_cont_f(const T &v, int fp_cnt, bool expf, bool trim)
    : value(v),
    fp_cnt(fp_cnt),
    expf(expf),
    trim(trim)
  {}
  virtual olxstr ToString() {
    olxstr rv;
    if (fp_cnt != 0) {
      rv = olxstr::FormatFloat(fp_cnt, value, expf);
    }
    else {
      rv = olxstr(value);
    }
    return trim ?rv.TrimFloat() : rv;
  }
};
//..............................................................................
template <typename T>
olx_print_i_cont *olx_print_makec(const T &v) {
  return new olx_print_cont<T>(v);
}
//..............................................................................
template <typename T>
olx_print_i_cont *olx_print_makec(const T &v, int fp_cnt,
  bool expf, bool trim)
{
  return new olx_print_cont_f<T>(v, fp_cnt, expf, trim);
}
//..............................................................................
bool olx_print_check_next(const olxstr &format, olxch what, size_t &idx) {
  if (idx + 1 < format.Length() && format.CharAt(idx + 1) == what) {
    idx++;
    return true;
  }
  return false;
}
//..............................................................................
olxstr esdl::olx_print_(const char *format_, va_list argptr) {
  const olxstr format = format_;
  olxstr_buf rv;
  size_t str_st = 0, f_width = 0;
  int fp_cnt = 0;
  try {
    olx_object_ptr<olx_print_i_cont> val;
    for (size_t i=0; i < format.Length(); i++) {
      if (val.ok()) {
        olxstr val_str = val->ToString();
        if (f_width != 0) {
          if (val_str.Length() < f_width) {
            val_str.Padding(f_width, ' ', false, false);
          }
        }
        rv << val_str;
        val = 0;
      }
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
        f_width = 0;
        fp_cnt = 0;
        // extract field width
        if (olxstr::o_isdigit(format.CharAt(i))) {
          size_t j = i-1;
          while (++j < format.Length() && olxstr::o_isdigit(format.CharAt(j)))
            ;
          if (j == format.Length()) {
            break;
          }
          f_width = format.SubString(i, j - i).ToInt();
          i = j;
        }
        if (format.CharAt(i) == '.') {
          i++;
          size_t j = i - 1;
          while (++j < format.Length() &&
            (olxstr::o_isdigit(format.CharAt(j)) ||
            (j == i && format.CharAt(j) == '-')))
          {
            ;
          }
          if (j == format.Length()) {
            break;
          }
          if (j != i) {
            fp_cnt = format.SubString(i, j - i).ToInt();
            i = j;
          }
        }
        switch (format.CharAt(i)) {
        case 'l': {  // ll d/u l d/u/c/s
          if (++i < format.Length()) {
              switch (format.CharAt(i)) {
              case 'l': { // ll d/u
                if (++i < format.Length()) {
                  if (format.CharAt(i) == 'd') {
                    val = olx_print_makec(va_arg(argptr, long long int));
                  }
                  else if (format.CharAt(i) == 'u') {
                    val = olx_print_makec(va_arg(argptr, unsigned long long int));
                  }
                  else {
                    throw TInvalidArgumentException(__OlxSourceInfo,
                      olxstr("argument for ll: '") << format.CharAt(i) << '\'');
                  }
                }
              }
              break;
              case 'e':
              case 'f':
                val = olx_print_makec(va_arg(argptr, double), fp_cnt,
                  format.CharAt(i) == 'e', olx_print_check_next(format, 't', i));
                break;
              case 'i':
              case 'd':
                val = olx_print_makec(va_arg(argptr, long int));
                break;
              case 'u':
                val = olx_print_makec(va_arg(argptr, unsigned long int));
                break;
              case 'c':
#ifdef __GNUC__
                val = olx_print_makec(va_arg(argptr, int));
#else
                val = olx_print_makec(va_arg(argptr, wchar_t));
#endif
                break;
              case 's':
                val = olx_print_makec(va_arg(argptr, const wchar_t*));
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
              val = olx_print_makec(va_arg(argptr, int));
#else
              val = olx_print_makec (va_arg(argptr, short int));
#endif
            else if (format.CharAt(i) == 'u')
#ifdef __GNUC__
              val = olx_print_makec(va_arg(argptr, unsigned int));
#else
              val = olx_print_makec(va_arg(argptr, unsigned short int));
#endif
            else
              throw TInvalidArgumentException(__OlxSourceInfo,
                olxstr("argument for h: '") << format.CharAt(i) << '\'');
          }
          break;
        case 'L': // L
          if (++i < format.Length()) {
            if (format.CharAt(i) == 'f' || format.CharAt(i) == 'e') {
              val = olx_print_makec(va_arg(argptr, long double), fp_cnt,
                format.CharAt(i) == 'e', olx_print_check_next(format, 't', i));
            }
            else
              throw TInvalidArgumentException(__OlxSourceInfo,
                olxstr("argument for h: '") << format.CharAt(i) << '\'');
          }
          break;
        case 'e':
        case 'f':
#ifdef __GNUC__
          val = olx_print_makec(va_arg(argptr, double), fp_cnt,
            format.CharAt(i) == 'e', olx_print_check_next(format, 't', i));
#else
          val = olx_print_makec(va_arg(argptr, float), fp_cnt,
            format.CharAt(i) == 'e', olx_print_check_next(format, 't', i));
#endif
          break;
        case 'i':
        case 'd':
          val = olx_print_makec(va_arg(argptr, int));
          break;
        case 'u':
          val = olx_print_makec(va_arg(argptr, unsigned int));
          break;
        case 'c':
#ifdef __GNUC__
          val = olx_print_makec(va_arg(argptr, int));
#else
          val = olx_print_makec(va_arg(argptr, char));
#endif
          break;
        case 's':
          val = olx_print_makec(va_arg(argptr, const char*));
          break;
        case 'z':
          val = olx_print_makec(va_arg(argptr, size_t));
          break;
        case 'w':
          val = olx_print_makec(*va_arg(argptr, olxstr*));
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
    if (val.ok()) {
      olxstr val_str = val->ToString();
      if (f_width != 0) {
        if (val_str.Length() < f_width) {
          val_str.Padding(f_width, ' ', false, false);
        }
      }
      rv << val_str;
    }
  }
  catch (const TExceptionBase &e) {
    va_end(argptr);
    throw TFunctionFailedException(__OlxSourceInfo, e);
  }
  va_end(argptr);
  if (str_st < format.Length()) {
    rv << format.SubStringFrom(str_st);
  }
  return olxstr(rv);
}
//..............................................................................
olxstr esdl::olx_print(const char* format, ...) {
  va_list argptr;
  va_start(argptr, format);
  return olx_print_(format, argptr);
}
//..............................................................................
olxstr esdl::olx_print(olxcstr format, ...) {
  va_list argptr;
  va_start(argptr, format);
  return olx_print_(format.c_str(), argptr);
}
