/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef olx_sdl_i_string_H
#define olx_sdl_i_string_H
#include <string.h>
#include <wctype.h>
#include <math.h>
#include <string>

#ifdef __WXWIDGETS__
  #include "wx/wx.h"
  #include "wx/string.h"
#endif
#ifdef _PYTHON
  #undef HAVE_SSIZE_T
  #undef HAVE_WCHAR_H
  #ifdef _DEBUG
    #undef _DEBUG
    #include "Python.h"
    #define _DEBUG
  #else
    #include "Python.h"
  #endif
#endif

#define CharSizeMask 0xE0000000
#define LengthMask   ~0xE0000000
#define CodeLength(CharSize, Length)   ((((size_t)CharSize) << 29) | (Length))
#define ExtractCharSize(Code)   ((Code) >> 29)
#define ExtractLength(Code)   (((Code) << 3) >> 3)

#ifdef _MSC_VER
  #include <memory.h>
  #define olx_strcmpn  strncmp
  #define olx_strcmpni _strnicmp
  #define olx_wcscmpn  wcsncmp
  #define olx_wcscmpni _wcsnicmp
#elif __BORLANDC__
  #define olx_strcmpn  strncmp
  #define olx_strcmpni strncmpi
  #define olx_wcscmpn  wcsncmp
  #define olx_wcscmpni _wcsnicmp
#else  // POSIX
  #include <strings.h>
  #include <wchar.h>
  #include <ctype.h>
  #define olx_strcmpn  strncmp
  #define olx_strcmpni strncasecmp
  #define olx_wcscmpn  wcsncmp
  #if defined(__MAC__)
    static int olx_wcscmpni(const wchar_t* s1, const wchar_t* s2, size_t len)  {
      for( size_t i=0; i < len; i++ )  {
        const int diff = towlower(s1[i]) - towlower(s2[i]);
        if( diff != 0 )  return diff;
      }
      return 0;
    }
  #else
    #define olx_wcscmpni wcsncasecmp
  #endif
#endif

#include "olx_wstring.h"
#include "olx_cstring.h"

BeginEsdlNamespace()

#ifndef __BORLANDC__
template <class,typename> class TTSString;

typedef TTSString<TCString, char> olxcstr;
typedef TTSString<TWString, wchar_t> olxwstr;
typedef TTStrBuffer<wchar_t, olxwstr> olxwstr_buf;
typedef TTStrBuffer<char, olxcstr> olxcstr_buf;
#ifdef _UNICODE
  typedef TTSString<TWString, wchar_t> olxstr;
  typedef olxwstr_buf olxstr_buf;
#else
  typedef TTSString<TCString, char > olxstr;
  typedef olxcstr_buf olxstr_buf;
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

template <class T, typename TC> class TTSString : public T  {
  void InitFromCharStr(const TC* str, size_t len) {
    T::_Length = len == InvalidIndex ? o_strlen(str) : len;
    T::SData = new struct T::Buffer(T::_Length + T::_Increment, str, T::_Length);
  }
  template <class SC> void InitFromString(const SC& str) {
    T::SData = str.Data_();
    T::_Length = str.Length();
    T::_Start = str.Start_();
    if (T::SData != 0) {
      T::SData->RefCnt++;
    }
  }
  TTSString& AssignCharStr(const TC* str, size_t len = ~0) {
    T::_Start = 0;
    T::_Length = ((len == InvalidIndex) ? o_strlen(str) : len);
    if (T::SData != 0) {
      if (T::SData->RefCnt == 1) { // owed by this object
        T::SData->SetCapacity(T::_Length);
        olx_memcpy(T::SData->Data, str, T::_Length);
      }
      else {
        T::SData->RefCnt--;
        T::SData = 0;
      }
    }
    if (T::SData == 0) {
      T::SData = new struct T::Buffer(T::_Length + T::_Increment, str,
        T::_Length);
    }
    return *this;
  }

public:
  TTSString() : T() {}
  TTSString(const olx_capacity_t& cap) {
    T::_Increment = cap.inc;
    T::checkBufferForModification(cap.capacity);
  }
  //...........................................................................
  TTSString(const TTSString& str, size_t start, size_t length) {
    T::SData = str.SData;
    if (T::SData != 0) {
      T::SData->RefCnt++;
    }
    T::_Start = str._Start + start;
    T::_Length = length;
    T::OnCopy(str);
  }
  //...........................................................................
  TTSString(const TTSString& str) {
    InitFromString(str);
    T::OnCopy(str);
  }
  //...........................................................................
  TTSString(const TTSString& str, size_t capacity)  {
    InitFromString(str);
    if (capacity != 0) {
      T::checkBufferForModification(T::_Length + capacity);
    }
    T::OnCopy(str);
  }
  //...........................................................................
  TTSString(const TC* str, size_t len = InvalidIndex) {
    InitFromCharStr(str, len);
  }
  TTSString(TC* const& str, size_t len=InvalidIndex) {
    InitFromCharStr(str, len);
  }
  //...........................................................................
  TTSString(const TC& v) {
    T::_Length = 1;
    T::SData = new struct T::Buffer(T::_Length + T::_Increment);
    T::SData->Data[0] = v;
  }
  //...........................................................................
  TTSString(const TTIString<TC>& str) {
    InitFromString(str);
  }
  //...........................................................................
  /* allocates requested size, if the change_size is false (default), the size
  of the string stays the same and ony the capacity is increased
  */
  TTSString &Allocate(size_t sz, bool change_size=false)  {
    if (T::SData == 0) {
      T::SData = new struct T::Buffer(sz);
    }
    else {
      T::SData->SetCapacity(sz);
    }
    if (change_size) {
      T::_Length = sz;
    }
    return *this;
  }

  template <class T1, typename TC1> TTSString(const TTSString<T1,TC1>& v)
    : T((const T1&)v)
  {}
  //...........................................................................
#if defined(__WXWIDGETS__)
  TTSString(const wxString& v) {
    InitFromCharStr((const TC*)v.c_str(), v.Length());
  }
#endif
  TTSString(const std::string &v) {
    T::Append(v.c_str(), v.length());
  }
  template <typename AC> TTSString(const AC& v)
    : T(v)
  {}
  //...........................................................................
  // creates a string from external array allocated with alloc
  static TTSString FromExternal(TC* data, size_t len,
    size_t buffer_size=InvalidIndex)
  {
    TTSString rv;
    rv._Length = len;
    rv.SData = new struct T::Buffer(data,
      buffer_size==InvalidIndex ? len : buffer_size);
    return rv;
  }
  static TTSString FromExternal(TC* data)  {
    TTSString rv;
    rv._Length = o_strlen(data);
    rv.SData = new struct T::Buffer(data, rv._Length);
    return rv;
  }
  //...........................................................................
  TTSString& operator << (const TIString& v)  {
    T::Append(v.raw_str(), v.Length());
    return *this;
  }
  //...........................................................................
  template <class AC> TTSString& operator << (const AC& v)  {
    T::operator << (v);  return *this;
  }
  //...........................................................................
  TTSString& operator << (const TC& v) {
    T::checkBufferForModification(T::_Length + 1);
    T::SData->Data[T::_Length] = v;
    T::_Length++;
    return *this;
  }
  template <typename AC> TTSString& Append(const AC* data, size_t len)  {
    T::Append(data, len);
    return *this;
  }
#if defined(__WXWIDGETS__)
  TTSString& operator << (const wxString& v)  {
    return Append((const TC*)v.c_str(), v.Length());
  }
#endif
  TTSString& operator << (TC* const& v)  {  return Append(v, o_strlen(v));  }
  TTSString& operator << (const TC* v)  {  return Append(v, o_strlen(v));  }
  TTSString& operator << (const TTSString& v)  {
    return Append(v.Data(), v.Length());
  }
  template <class T1, typename TC1>
  TTSString& operator << (const TTSString<T1,TC1>& v)  {
    return Append(v.raw_str(), v.Length());
  }
  TTSString& operator << (const bool& v)  {
    return operator << (v ? TrueString() : FalseString());
  }
  //...........................................................................
  //...........................................................................
  TTSString& operator = (const TTIString<TC>& v) {
    if (T::SData != 0 && --T::SData->RefCnt == 0) {
      delete T::SData;
    }
    T::_Start = v.Start_();
    T::_Length = v.Length();
    T::SData = v.Data_();
    if (T::SData != 0) {
      T::SData->RefCnt++;
    }
    return *this;
  }
#if defined(__WXWIDGETS__)
 TTSString& operator = (const wxString& v)  {
   return AssignCharStr((const TC*)v.c_str(), v.Length());
 }
#endif
  //...........................................................................
 template <class AC> TTSString& operator = (const AC& v) {
   T::operator = (v);
   return *this;
 }
 //...........................................................................
  TTSString& operator = (TC* const& str)  {  return AssignCharStr(str);  }
  TTSString& operator = (const TC* str)   {  return AssignCharStr(str);  }
  //...........................................................................
  TTSString& operator = (bool v) {
    (*this) = (v ? TrueString() : FalseString());
    return *this;
  }
  //...........................................................................
  TTSString& operator = (const TC& ch) {
    T::_Start = 0;
    T::_Increment = 8;
    T::_Length = 1;
    if (T::SData != 0) {
      if (T::SData->RefCnt == 1) { // owed by this object
        T::SData->SetCapacity(T::_Length);
        T::SData->Data[0] = ch;
      }
      else {
        T::SData->RefCnt--;
        T::SData = 0;
      }
    }
    if (T::SData == 0) {
      T::SData = new struct T::Buffer(T::_Length + T::_Increment);
      T::SData->Data[0] = ch;
    }
    return *this;
  }
  //...........................................................................
  template <class T1, typename TC1> TTSString& operator = (
    const TTSString<T1,TC1>& v)
  {
    T::operator = ((const T1&)v);
    return *this;
  }
  TTSString& operator = (const TTSString& v) {
    if (&v == this) {
      return *this;
    }
    if (T::SData != 0 && --T::SData->RefCnt == 0) {
      delete T::SData;
    }
    T::_Start = v._Start;
    T::_Length = v._Length;
    T::SData = v.SData;
    if (T::SData != 0) {
      T::SData->RefCnt++;
    }
    T::OnCopy(v);
    return *this;
  }
  //...........................................................................
  TTSString SubString(size_t from, size_t count) const {
#ifdef OLX_DEBUG
    if (from > T::_Length) {
      TExceptionBase::ThrowIndexOutOfRange(__POlxSourceInfo, from, 0,
        T::_Length);
    }
    if (from + count > T::_Length) {
      TExceptionBase::ThrowIndexOutOfRange(__POlxSourceInfo, from + count, 0,
        T::_Length);
    }
#endif
    return TTSString<T, TC> (*this, from, count);
  }
  TTSString SubStringFrom(size_t from, size_t indexFromEnd=0) const {
    return SubString(from, T::_Length-from-indexFromEnd);
  }
  TTSString SubStringTo(size_t to, size_t indexFromStart=0) const {
#ifdef OLX_DEBUG
    if (to > T::_Length) {
      TExceptionBase::ThrowIndexOutOfRange(__POlxSourceInfo, to, 0,
        T::_Length);
    }
#endif
    return SubString(indexFromStart, to-indexFromStart);
  }
  //...........................................................................
  // for latin letetrs only
  static char o_ltoupper(char ch)  {
    return (ch >='a' && ch <= 'z') ? (ch + 'A'-'a') : ch;
  }
  // for latin letetrs only
  template <typename ch_t>
  static char o_ltolower(ch_t ch)  {
    return (ch >='A' && ch <= 'Z') ? (ch + 'a'-'A') : ch;
  }
  static char o_toupper(char ch)  {  return toupper(ch);  }
  static char o_tolower(char ch)  {  return tolower(ch);  }
  static wchar_t o_toupper(wchar_t ch)  {  return towupper(ch);  }
  static wchar_t o_tolower(wchar_t ch)  {  return towlower(ch);  }
  static bool o_isdigit(char ch)     {  return (ch >= '0' && ch <= '9');  }
  static bool o_isdigit(wchar_t ch)  {  return (ch >= L'0' && ch <= L'9');  }
  static bool o_ishexdigit(char ch)  {
    return (ch >= '0' && ch <= '9') ||
           (ch >= 'A' && ch <= 'F') ||
           (ch >= 'a' && ch <= 'f');
  }
  static bool o_ishexdigit(wchar_t ch)  {
    return (ch >= L'0' && ch <= L'9') ||
           (ch >= L'A' && ch <= L'F') ||
           (ch >= L'a' && ch <= L'f');
  }
  static bool o_islatin(char ch)  {
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
  }
  static bool o_islatin(wchar_t ch)  {
    return (ch >= L'a' && ch <= L'z') || (ch >= L'A' && ch <= L'Z');
  }
  static bool o_isalpha(char ch)  {  return isalpha(ch) != 0;  }
  static bool o_isalpha(wchar_t ch)  {  return iswalpha(ch) != 0;  }
  static bool o_isalphanumeric(char ch)  {
    return o_isdigit(ch) || o_isalpha(ch);
  }
  static bool o_isalphanumeric(wchar_t ch)  {
    return o_isdigit(ch) || o_isalpha(ch);
  }
  static bool o_iswhitechar(char ch)  {  return (ch == ' ' || ch == '\t');  }
  static bool o_iswhitechar(wchar_t ch)  {
    return (ch == L' ' || ch == L'\t');
  }
  static size_t o_strlen(const char* cstr)  {
    return (cstr==0) ? 0 : strlen(cstr);
  }
  static size_t o_strlen(const wchar_t* wstr)  {
    return (wstr==0) ? 0 : wcslen(wstr);
  }
  static size_t o_strlen(const T &str)  { return str.Length(); }
  static const char* o_data(const char* cstr)  { return cstr; }
  static const wchar_t* o_data(const wchar_t* cstr)  { return cstr; }
  static typename T::CharT* o_data(const T &str)  { return str.raw_str(); }
  static void o_strtup(TC* wht, size_t wht_len)  {
    for( size_t i=0; i < wht_len; i++ )
      wht[i] = o_toupper(wht[i]);
  }
  static void o_strtlw(TC* wht, size_t wht_len) {
    for (size_t i = 0; i < wht_len; i++) {
      wht[i] = o_tolower(wht[i]);
    }
  }
  // returns length of common string
  template <typename OC, typename AC>
  static size_t o_cmnstr(OC* str1, size_t str1_len, AC* str2,
    size_t str2_len)
  {
    size_t mlen = olx_min(str1_len, str2_len);
    for (size_t i = 0; i < mlen; i++) {
      if (str1[i] != str2[i]) {
        return i;
      }
    }
    return mlen;
  }
  //...........................................................................
  /* returns length of common string and initialises start1 - the start in the
  first string */
  template <typename OC, typename AC>
  static size_t o_cmnsubstr(OC* str1, size_t str1_len, AC* str2,
    size_t str2_len, size_t& start1)
  {
    size_t max_l = 0;
    start1 = 0;
    for (size_t i = 0; i < str1_len; i++) {
      const size_t l = olx_min(str1_len - i, str2_len);
      if (l < max_l) { // cannot be longer...
        break;
      }
      bool equal = true;
      for (size_t j = 0; j < l; j++) {
        if (str1[j + i] != str2[j]) {
          if (j > max_l) {
            max_l = j;
            start1 = i;
          }
          equal = false;
          break;
        }
      }
      if (equal) {
        max_l = l;
        start1 = i;
        break;
      }
    }
    return max_l;
  }
  //...........................................................................
  TTSString& UpperCase() {
    T::checkBufferForModification(T::_Length);
    o_strtup(T::Data(), T::_Length);
    return *this;
  }
  //...........................................................................
  TTSString& LowerCase() {
    T::checkBufferForModification(T::_Length);
    o_strtlw(T::Data(), T::_Length);
    return *this;
  }
  //...........................................................................
  TTSString ToUpperCase() const {
    return TTSString<T, TC>(*this).UpperCase();
  }
  //...........................................................................
  TTSString ToLowerCase() const {
    return TTSString<T, TC>(*this).LowerCase();
  }
  //...........................................................................
  TTSString CommonString(const TTSString& str) const {
    return SubStringTo(
      o_cmnstr(T::Data(), T::_Length, str.Data(), str.Length()));
  }
  //...........................................................................
  static TTSString CommonString(const TTSString& str1, const TTSString& str2) {
    return str1.CommonString(str2);
  }
  //...........................................................................
  TTSString CommonSubString(const TTSString& str) const {
    size_t s_ta = 0, s_at = 0;
    const size_t l_ta = o_cmnsubstr(T::Data(), T::_Length, str.Data(),
      str.Length(), s_ta);
    const size_t l_at = o_cmnsubstr(str.Data(), str.Length(), T::Data(),
      T::_Length, s_at);
    if (l_ta > l_at) {
      return SubString(s_ta, l_ta);
    }
    else {
      return str.SubString(s_at, l_at);
    }
  }
  //...........................................................................
  static TTSString CommonSubString(const TTSString& s1, const TTSString& s2) {
    return s1.CommonSubString(s2);
  }
  //...........................................................................
  /* to avoid any potential problems this function should be used for the ALL
  string comparison because the system functions may use locale to do locale
  specific sorting and since some procedures WILL call this function in some
  situations, the sorted search may fail and cause all possible errors */
  template <typename OC, typename AC> static int o_memcmp(const OC* wht,
    const AC* with, size_t len)
  {
    for (size_t i = 0; i < len; i++) {
      if (wht[i] != with[i]) {
        return wht[i] - with[i];
      }
    }
    return 0;
  }
  //static int o_memcmp(const char* wht, const char* with, size_t len) {
  //   return olx_strcmpn(wht, with, len);
  //}
  //static int o_memcmp(const wchar_t* wht, const wchar_t* with, size_t len) {
  //  return olx_wcscmpn(wht, with, len);
  //}
  /* read comment to o_memcmp */
  template <typename OC, typename AC> static int o_memcmpi(const OC* wht,
    const AC* with, size_t len)
  {
    for (size_t i = 0; i < len; i++) {
      const int diff = o_tolower(wht[i]) - o_tolower(with[i]);
      if (diff != 0) {
        return diff;
      }
    }
    return 0;
  }
  //static int o_memcmpi(const char* wht, const char* with, size_t len) {
  //  return olx_strcmpni(wht, with, len);
  //}
  //static int o_memcmpi(const wchar_t* wht, const wchar_t* with, size_t len) {
  //  return olx_wcscmpni(wht, with, len);
  //}
  template <typename OC, typename AC>
  static int o_strcmp(const OC* wht, size_t len_a, const AC* with,
    size_t len_b)
  {
    if (len_a == len_b) {
      if (len_a == 0) {
        return 0;
      }
      return o_memcmp(wht, with, len_a);
    }
    if (len_a == 0) {
      return -1;
    }
    if (len_b == 0) {
      return 1;
    }
    const int res = o_memcmp(wht, with, olx_min(len_a, len_b));
    return (res != 0 ? res : (len_a < len_b ? -1 : 1));
  }
  template <typename OC, typename AC>
  static int o_strcmpi(const OC* wht, size_t len_a, const AC* with,
    size_t len_b)
  {
    if( len_a == len_b )  {
      if (len_a == 0) {
        return 0;
      }
      return o_memcmpi(wht, with, len_a);
    }
    if (len_a == 0) {
      return -1;
    }
    if (len_b == 0) {
      return 1;
    }
    const int res = o_memcmpi(wht, with, olx_min(len_a, len_b));
    return (res != 0 ? res : (len_a < len_b ? -1 : 1));
  }
#if defined(__WXWIDGETS__)
 int Compare(const wxString& v) const {
   return o_strcmp(T::Data(), T::_Length, (const TC*)v.c_str(), v.Length());
 }
#endif
  int Compare(const std::string& v) const {
    return o_strcmp(T::Data(), T::_Length, v.c_str(), v.length());
  }
  int Compare(const TTSString& v) const {
    return o_strcmp(T::Data(), T::_Length, v.Data(), v._Length);
  }
  int Compare(const wchar_t& v) const {
    if (T::_Length == 0) {
      return -1;
    }
    const int df = T::Data()[0] - v;
    return df != 0 ? df : (T::_Length == 1 ? 0 : 1);
  }
  int Compare(const char& v) const {  return Compare((wchar_t)v);  }
  int Compare(const char* v) const {
    return o_strcmp(T::Data(), T::_Length, v, o_strlen(v));
  }
  int Compare(const wchar_t* v) const {
    return o_strcmp(T::Data(), T::_Length, v, o_strlen(v));
  }
#if defined(__WXWIDGETS__)
 int Comparei(const wxString& v) const {
   return o_strcmpi(T::Data(), T::_Length, (const TC*)v.c_str(), v.Length());
 }
#endif
  int Comparei(const std::string& v) const {
    return o_strcmpi(T::Data(), T::_Length, v.c_str(), v.length());
  }
  int Comparei(const TTSString& v) const {
    return o_strcmpi(T::Data(), T::_Length, v.Data(), v._Length);
  }
  int Comparei(const wchar_t& v) const {
    if (T::_Length == 0) {
      return -1;
    }
    int df = 0;
    if (T::Data()[0] != v) {
      if (o_isalpha(T::Data()[0]) && o_isalpha(v)) {
        df = o_toupper(T::Data()[0]) - o_toupper(v);
      }
      else {
        df = T::Data()[0] - v;
      }
    }
    return df != 0 ? df : (T::_Length == 1 ? 0 : 1);
  }
  int Comparei(const char& v) const {  return Comparei((wchar_t)v);  }
  int Comparei(const char* v) const {
    return o_strcmpi(T::Data(), T::_Length, v, o_strlen(v));
  }
  int Comparei(const wchar_t* v) const {
    return o_strcmpi(T::Data(), T::_Length, v, o_strlen(v));
  }
  template <typename AC>
    bool Equals(const AC& v) const {  return Compare(v) == 0;  }
  template <typename AC>
    bool Equalsi(const AC& v) const {  return Comparei(v) == 0;  }

  template <typename AC>
    bool operator == (const AC& v) const { return Equals(v); }
  template <typename AC>
    bool operator != (const AC& v) const { return !Equals(v); }
  template <typename AC>
    bool operator >  (const AC& v) const { return Compare(v) > 0; }
  template <typename AC>
    bool operator >= (const AC& v) const { return Compare(v) >= 0; }
  template <typename AC>
    bool operator <  (const AC& v) const { return Compare(v) < 0; }
  template <typename AC>
    bool operator <= (const AC& v) const { return Compare(v) <= 0; }
  //...........................................................................
  template <typename AC> bool StartsFrom(const AC* v) const {
    size_t len = o_strlen(v);
    return (len > T::_Length) ? false : (o_memcmp(T::Data(), v, len) == 0);
  }
  template <typename AC> bool StartsFromi(const AC* v) const {
    size_t len = o_strlen(v);
    return (len > T::_Length) ? false : (o_memcmpi(T::Data(), v, len) == 0);
  }
  //
  bool StartsFrom(const TTSString& v) const {
    return (v._Length > T::_Length) ? false
      : (o_memcmp(T::Data(), v.Data(), v._Length) == 0);
  }
  bool StartsFromi(const TTSString& v) const {
    return (v._Length > T::_Length) ? false
      : (o_memcmpi(T::Data(), v.Data(), v._Length) == 0);
  }
  //
  template <typename AC> bool StartsFrom(AC v) const {
    return (T::_Length == 0) ? false : T::Data()[0] == v;
  }
  template <typename AC> bool StartsFromi(AC v) const {
    return (T::_Length == 0) ? false : o_tolower(T::Data()[0]) == o_tolower(v);
  }
  //...........................................................................
  template <typename AC> size_t LeadingCharCount(AC v) const {
    size_t cc = 0;
    while( cc < T::_Length && T::CharAt(cc) == v )  cc++;
    return cc;
  }
  //...........................................................................
  template <typename AC> bool EndsWith(const AC* v) const {
    const size_t len = o_strlen(v);
    return (len > T::_Length) ? false
      : (o_memcmp(&T::Data()[T::_Length-len], v, len) == 0);
  }
  template <typename AC> bool EndsWithi(const AC* v) const {
    const size_t len = o_strlen(v);
    return (len > T::_Length) ? false
      : (o_memcmpi(&T::Data()[T::_Length-len], v, len) == 0);
  }
  //
  bool EndsWith(const TTSString& v) const {
    return (v._Length > T::_Length) ? false
      : (o_memcmp(&T::Data()[T::_Length-v._Length], v.Data(), v._Length)==0);
  }
  bool EndsWithi(const TTSString& v) const {
    return (v._Length > T::_Length) ? false
      : (o_memcmpi(&T::Data()[T::_Length-v._Length], v.Data(), v._Length)==0);
  }
  //
  template <typename AC> bool EndsWith(AC ch) const {
    return T::_Length == 0 ? false : T::Data()[T::_Length-1] == ch;
  }
  template <typename AC> bool EndsWithi(AC ch) const {
    return T::_Length == 0 ? false
      : o_tolower(T::Data()[T::_Length-1]) == o_tolower(ch);
  }
  //...........................................................................
  //...........................................................................
  TTSString operator + (const TC* v) const {
    return (TTSString<T,TC>&)TTSString<T,TC>(*this).Append(v, o_strlen(v));
  }
  //...........................................................................
  TTSString operator + (TC v) const { return (TTSString<T,TC>(*this) << v); }
  //...........................................................................
  TTSString operator + (const TTSString& v) const {
    return (TTSString<T,TC>(*this) << v);
  }
  //...........................................................................
  template <typename OC, typename AC>
  static size_t o_strpos(const OC* whr, size_t whr_len, const AC* wht,
    size_t wht_len)
  {
    for (size_t i = 0; i < whr_len; i++) {
      if (i + wht_len > whr_len) {
        return InvalidIndex;
      }
      bool found = true;
      for (size_t j = 0; j < wht_len; j++)
        if (whr[i + j] != wht[j]) {
          found = false;
          break;
        }
      if (found) {
        return i;
      }
    }
    return InvalidIndex;
  }
  //...........................................................................
  template <typename OC, typename AC>
  static bool o_issubstr(const OC* whr, size_t whr_len, size_t pos,
    const AC* wht, size_t wht_len)
  {
    if (pos + wht_len > whr_len) {
      return false;
    }
    for (size_t i = 0; i < wht_len; i++) {
      if (whr[pos + i] != wht[i]) {
        return false;
      }
    }
    return true;
  }
  //...........................................................................
  template <typename OC, typename AC>
  static size_t o_chrpos(const OC* whr, size_t whr_len, AC wht) {
    for (size_t i = 0; i < whr_len; i++) {
      if (whr[i] == wht) {
        return i;
      }
    }
    return InvalidIndex;
  }
  //...........................................................................
  // index of any char in the sequence, linear search
  template <typename OC, typename AC>
  static size_t o_chrseqpos(const OC* whr, size_t whr_len,
    AC* seq, size_t seq_len)
  {
    for (size_t i = 0; i < whr_len; i++) {
      for (size_t j = 0; j < seq_len; j++) {
        if (whr[i] == seq[j]) {
          return i;
        }
      }
    }
    return InvalidIndex;
  }
  //...........................................................................
  template <typename OC, typename AC>
  static size_t o_strposi(const OC* whr, size_t whr_len, const AC* wht,
    size_t wht_len)
  {
    for (size_t i = 0; i < whr_len; i++) {
      if (i + wht_len > whr_len) {
        return InvalidIndex;
      }
      bool found = true;
      for (size_t j = 0; j < wht_len; j++) {
        if (o_toupper(whr[i + j]) != o_toupper(wht[j])) {
          found = false;
          break;
        }
      }
      if (found) {
        return i;
      }
    }
    return InvalidIndex;
  }
  //...........................................................................
  template <typename OC, typename AC>
  static bool o_issubstri(const OC* whr, size_t whr_len, size_t pos,
    const AC* wht, size_t wht_len)
  {
    if (pos + wht_len >= whr_len) {
      return false;
    }
    for (size_t i = 0; i < wht_len; i++) {
      if (o_toupper(whr[pos + i]) != o_toupper(wht[i])) {
        return false;
      }
    }
    return true;
  }
  //...........................................................................
  template <typename OC, typename AC>
  static size_t o_chrposi(const OC* whr, size_t whr_len, AC wht) {
    wht = o_toupper(wht);
    for (size_t i = 0; i < whr_len; i++) {
      if (o_toupper(whr[i]) == wht) {
        return i;
      }
    }
    return InvalidIndex;
  }
  //...........................................................................
  template <typename OC, typename AC>
  static size_t o_strposr(const OC* whr, size_t whr_len, const AC* wht,
    size_t wht_len)
  {
    if (wht_len > whr_len || whr_len == 0 || wht_len == 0) {
      return InvalidIndex;
    }
    for (size_t i = whr_len - 1; i != InvalidIndex; i--) {
      if (i < wht_len) {
        return InvalidIndex;
      }
      bool found = true;
      for (size_t j = wht_len - 1; j != InvalidIndex; j--) {
        if (whr[i - j] != wht[j]) {
          found = false;
          break;
        }
      }
      if (found) {
        return i - 1;
      }
    }
    return InvalidIndex;
  }
  //...........................................................................
  template <typename OC, typename AC>
  static size_t o_chrposr(const OC* whr, size_t whr_len, AC wht) {
    if (whr_len == 0) {
      return InvalidIndex;
    }
    for (size_t i = whr_len - 1; i != InvalidIndex; i--) {
      if (whr[i] == wht) {
        return i;
      }
    }
    return InvalidIndex;
  }
  //...........................................................................
  template <typename OC, typename AC>
  static size_t o_strposri(const OC* whr, size_t whr_len, const AC* wht,
    size_t wht_len)
  {
    if (wht_len > whr_len || whr_len == 0 || wht_len == 0) {
      return InvalidIndex;
    }
    for (size_t i = whr_len - 1; i != InvalidIndex; i--) {
      if (i < wht_len) {
        return InvalidIndex;
      }
      bool found = true;
      for (size_t j = wht_len - 1; j != InvalidIndex; j--) {
        if (o_toupper(whr[i - j]) != o_toupper(wht[j])) {
          found = false;
          break;
        }
      }
      if (found) {
        return i;
      }
    }
    return InvalidIndex;
  }
  //...........................................................................
  template <typename OC, typename AC>
  static size_t o_chrposri(const OC* whr, size_t whr_len, AC wht) {
    if (whr_len == 0) {
      return InvalidIndex;
    }
    wht = o_toupper(wht);
    for (size_t i = whr_len - 1; i != InvalidIndex; i--) {
      if (o_toupper(whr[i]) == wht) {
        return i;
      }
    }
    return InvalidIndex;
  }
  //...........................................................................
  size_t IndexOf(const TTSString& wht) const {
    return o_strpos(T::Data(), T::_Length, wht.Data(), wht._Length);
  }
  size_t IndexOfi(const TTSString& wht) const {
    return o_strposi(T::Data(), T::_Length, wht.Data(), wht._Length);
  }
  size_t FirstIndexOf(const TTSString& wht, size_t from = 0) const {
    const size_t i = o_strpos(&T::Data()[from], T::_Length-from,
      wht.Data(), wht._Length);
    return ((i==InvalidIndex) ? InvalidIndex : (i + from));
  }
  size_t FirstIndexOfi(const TTSString& wht, size_t from = 0) const {
    const size_t i = o_strposi(&T::Data()[from], T::_Length-from, wht.Data(),
      wht._Length);
    return ((i==InvalidIndex) ? InvalidIndex : (i + from));
  }
  bool IsSubStringAt(const TTSString& wht, size_t pos)   const {
    return o_issubstr(T::Data(), T::_Length, pos, wht.Data(), wht._Length);
  }
  bool IsSubStringAti(const TTSString& wht, size_t pos)   const {
    return o_issubstri(T::Data(), T::_Length, pos, wht.Data(), wht._Length);
  }
  size_t LastIndexOf(const TTSString& wht, size_t from = InvalidIndex) const {
    return o_strposr(T::Data(), olx_min(from, T::_Length), wht.Data(),
      wht._Length);
  }
  size_t LastIndexOfi(const TTSString& wt, size_t from = InvalidIndex) const {
    return o_strposri(T::Data(), olx_min(from, T::_Length), wt.Data(),
      wt._Length);
  }
  template <typename AC> size_t IndexOf(const AC* wht) const {
    return o_strpos(T::Data(), T::_Length, wht, o_strlen(wht));
  }
  template <typename AC> size_t IndexOfi(const AC* wht) const {
    return o_strposi(T::Data(), T::_Length, wht, o_strlen(wht));
  }
  template <typename AC> size_t IndexOf(AC wht) const {
    return o_chrpos(T::Data(), T::_Length, wht);
  }
  template <typename AC> size_t IndexOfi(AC wht) const {
    return o_chrposi(T::Data(), T::_Length, wht);
  }

  template <typename ST> bool Contains(const ST &v) const {
    return IndexOf(v) != InvalidIndex;
  }
  template <typename ST> bool Containsi(const ST &v) const {
    return IndexOfi(v) != InvalidIndex;
  }
  template <typename seq_t>
  bool ContainAnyOf(const seq_t &v) const {
    return o_chrseqpos(T::Data(), T::_Length, o_data(v), o_strlen(v)) !=
      InvalidIndex;
  }

  template <typename AC>
  size_t FirstIndexOf(const AC* wht, size_t from=0) const {
    size_t i = o_strpos(&T::Data()[from], T::_Length-from, wht, o_strlen(wht));
    return ((i==InvalidIndex) ? InvalidIndex : (i + from));
  }
  template <typename AC>
  size_t FirstIndexOfi(const AC* wht, size_t from=0) const {
    size_t i = o_strposi(&T::Data()[from], T::_Length-from, wht, o_strlen(wht));
    return ((i==InvalidIndex) ? InvalidIndex : (i + from));
  }
  template <typename AC> size_t FirstIndexOf(AC wht, size_t from=0) const {
    size_t i = o_chrpos(&T::Data()[from], T::_Length-from, wht);
    return ((i==InvalidIndex) ? InvalidIndex : (i + from));
  }
  template <typename AC> size_t FirstIndexOfi(AC wht, size_t from=0) const {
    size_t i = o_chrposi(&T::Data()[from], T::_Length-from, wht);
    return ((i==InvalidIndex) ? InvalidIndex : (i + from));
  }

  template <typename AC> bool IsSubStringAt(const AC* wht, size_t pos) const {
    return o_issubstr(T::Data(), T::_Length, pos, wht, o_strlen(wht));
  }
  template <typename AC> bool IsSubStringAti(const AC* wht, size_t pos) const {
    return o_issubstri(T::Data(), T::_Length, pos, wht, o_strlen(wht));
  }

  template <typename AC> size_t LastIndexOf(const AC* wt,
    size_t from=InvalidIndex) const
  {
    return o_strposr(T::Data(), olx_min(from, T::_Length), wt, o_strlen(wt));
  }
  template <typename AC> size_t LastIndexOfi(const AC* wt,
    size_t from=InvalidIndex) const
  {
    return o_strposri(T::Data(), olx_min(from, T::_Length), wt, o_strlen(wt));
  }
  template <typename AC> size_t LastIndexOf(AC wht,
    size_t from=InvalidIndex) const
  {
    return o_chrposr(T::Data(), olx_min(from, T::_Length), wht);
  }
  template <typename AC> size_t LastIndexOfi(AC wht,
    size_t from=InvalidIndex) const
  {
    return o_chrposri(T::Data(), olx_min(from, T::_Length), wht);
  }
  //...........................................................................
  // function checks for preceding radix encoding
  template <typename IT>
  static IT o_atois(const TC* data, size_t len, bool& negative,
    unsigned short Rad = 10)
  {
    if (len == 0) {
      TExceptionBase::ThrowInvalidIntegerFormat(__POlxSourceInfo, data, len);
    }
    size_t sts = 0; // string start, end
    while (o_iswhitechar(data[sts]) && ++sts < len) {
    }
    if (sts >= len) {
      TExceptionBase::ThrowInvalidIntegerFormat(__POlxSourceInfo, data, len);
    }
    // test for any particluar format specifier, here just '0x', for hexadecimal
    if (len > sts + 1 && data[sts] == '0' &&
      (data[sts + 1] == 'x' || data[sts + 1] == 'X'))
    {
      Rad = 16;
      sts += 2;
    }
    else if (data[sts] == 'o' || data[sts] == 'O') {
      Rad = 8;
      sts++;
    }
    return o_atoi_s<IT>(&data[sts], len - sts, negative, Rad);
  }
  //...........................................................................
  // function checks for preceding radix encoding
  static bool o_isints(const TC* data, size_t len, bool& negative) {
    if (len == 0) {
      return false;
    }
    size_t sts = 0; // string start, end
    while (o_iswhitechar(data[sts]) && ++sts < len) {
    }
    if (sts >= len) {
      return false;
    }
    // test for any particluar format specifier, here just '0x', for hexadecimal
    unsigned short Rad = 10;
    if (data[sts] == '0') {
      negative = false;
      if (len-sts == 1) {
        return true;
      }
      if (data[sts + 1] == 'x' || data[sts + 1] == 'X') {
        if (len-sts == 2) {
          return false;
        }
        Rad = 16;
        sts += 2;
      }
      else if (data[sts] == '0') {
        Rad = 8;
        sts++;
      }
    }
    return o_isint_s(&data[sts], len - sts, negative, Rad);
  }
  //...........................................................................
  template <typename IT> static IT o_atoi_safe(const TC* data, size_t len,
    unsigned short Rad=10)
  {
    bool negative;
    IT val = o_atois<IT>(data, len, negative, Rad);
    return negative ? -val : val;
  }
  //...........................................................................
  template <typename IT> static IT o_atoui_safe(const TC* data, size_t len,
    unsigned short Rad=10)
  {
    bool negative;
    IT val = o_atois<IT>(data, len, negative, Rad);
    if (negative) {
      TExceptionBase::ThrowInvalidUnsignedFormat(__POlxSourceInfo, data, len);
    }
    return val;
  }
  //...........................................................................
  // preceding encoding like 0x ir 0 are treated correctly
  template <typename IT> IT SafeInt(unsigned short Rad=10) const {
    return o_atoi_safe<IT>(T::Data(), T::_Length, Rad);
  }
  //...........................................................................
  // preceding encoding like 0x ir 0 are treated correctly
  template <typename IT> IT SafeUInt(unsigned short Rad=10) const {
    return o_atoui_safe<IT>(T::Data(), T::_Length, Rad);
  }
  //...........................................................................
  /* no '\0' at the end, got to do it ourselves, returns the value without
  applying chsig */
  template <typename IT> static IT o_atoi_s(const TC* data, size_t len,
    bool& negative, unsigned short Rad = 10)
  {
    if (len == 0) {
      TExceptionBase::ThrowInvalidIntegerFormat(__POlxSourceInfo, data, len);
    }
    size_t sts = 0, ste = len; // string start, end
    while (o_iswhitechar(data[sts]) && ++sts < len) {
    }
    while (--ste > sts && o_iswhitechar(data[ste])) {
    }
    if (++ste <= sts) {
      TExceptionBase::ThrowInvalidIntegerFormat(__POlxSourceInfo, data, len);
    }
    IT val = 0;
    negative = false;
    if (data[sts] == '-') {
      negative = true;
      sts++;
    }
    else if (data[sts] == '+') {
      sts++;
    }
    if (sts == ste) {
      TExceptionBase::ThrowInvalidIntegerFormat(__POlxSourceInfo, data, len);
    }
    if (Rad > 10) {
      for (size_t i = sts; i < ste; i++) {
        short pv = 0;
        // the order is important, chars are rearer
        if (data[i] <= '9' && data[i] >= '0') {
          pv = data[i] - '0';
        }
        else if (data[i] <= 'Z' && data[i] >= 'A') {
          pv = data[i] - 'A' + 10;
        }
        else if (data[i] <= 'z' && data[i] >= 'a') {
          pv = data[i] - 'a' + 10;
        }
        else {
          TExceptionBase::ThrowInvalidIntegerFormat(__POlxSourceInfo, data,
            len);
        }
        if (pv >= Rad) {
          TExceptionBase::ThrowInvalidIntegerFormat(__POlxSourceInfo, data,
            len);
        }
        val = val*Rad + pv;
      }
    }
    else {
      for (size_t i = sts; i < ste; i++) {
        if (data[i] <= '9' && data[i] >= '0') {
          const short pv = data[i] - '0';
          if (pv >= Rad) {
            TExceptionBase::ThrowInvalidIntegerFormat(__POlxSourceInfo, data,
              len);
          }
          val = val*Rad + pv;
        }
        else {
          TExceptionBase::ThrowInvalidIntegerFormat(__POlxSourceInfo, data,
            len);
        }
      }
    }
    return val;
  }
  //...........................................................................
  /** no '\0' at the end, got to do it ourselves, returns the value without
  applying chsig */
  static bool o_isint_s(const TC* data, size_t len, bool& negative,
    unsigned short Rad = 10)
  {
    if (len == 0) {
      return false;
    }
    size_t sts = 0, ste = len; // string start, end
    while (o_iswhitechar(data[sts]) && ++sts < len) {
    }
    while (--ste > sts && o_iswhitechar(data[ste])) {
    }
    if (++ste <= sts) {
      return false;
    }
    negative = false;
    if (data[sts] == '-') {
      negative = true;
      sts++;
    }
    else if (data[sts] == '+') {
      sts++;
    }
    if (sts == ste) {
      return false;
    }
    for (size_t i = sts; i < ste; i++) {
      short pv = 0;
      if (data[i] <= '9' && data[i] >= '0') {
        pv = data[i] - '0';
      }
      else if (data[i] <= 'Z' && data[i] >= 'A') {
        pv = data[i] - 'A' + 10;
      }
      else if (data[i] <= 'z' && data[i] >= 'a') {
        pv = data[i] - 'a' + 10;
      }
      else  return false;
      if (pv >= Rad) {
        return false;
      }
    }
    return true;
  }
  //...........................................................................
  template <typename IT> static IT o_atoi(const TC* data, size_t len,
    unsigned short Rad=10)
  {
    bool negative;
    IT val = o_atoi_s<IT>(data, len, negative, Rad);
    return negative ? -val : val;
  }
  //...........................................................................
  static bool o_isint(const TC* data, size_t len) {
    bool negative;
    return o_isints(data, len, negative);
  }
  //...........................................................................
  template <typename IT> static IT o_atoui(const TC* data, size_t len,
    unsigned short Rad=10)
  {
    bool negative;
    IT val = o_atoi_s<IT>(data, len, negative, Rad);
    if (negative) {
      TExceptionBase::ThrowInvalidIntegerFormat(__POlxSourceInfo, data, len);
    }
    return val;
  }
  //...........................................................................
  static bool o_isuint(const TC* data, size_t len) {
    bool negative;
    bool v = o_isints(data, len, negative);
    return v && !negative;
  }
  //...........................................................................
  template <typename IT> IT RadInt(unsigned short Rad=10) const  {
     return o_atoi<IT>(T::Data(), T::_Length, Rad);
  }
  //...........................................................................
  template <typename IT> IT RadUInt(unsigned short Rad=10) const  {
     return o_atoui<IT>(T::Data(), T::_Length, Rad);
  }
  //...........................................................................
  int ToInt() const {  return o_atoi<int>(T::Data(), T::_Length, 10);  }
  bool IsInt() const {  return o_isint(T::Data(), T::_Length);  }
  //...........................................................................
  unsigned int ToUInt() const {
    return o_atoui<unsigned int>(T::Data(), T::_Length, 10);
  }
  bool IsUInt() const {  return o_isuint(T::Data(), T::_Length);  }
  //...........................................................................
  size_t ToSizeT() const {
    return o_atoui<size_t>(T::Data(), T::_Length, 10);
  }
  //...........................................................................
  bool ToBool() const {
    if (Equalsi(TrueString())) {
      return true;
    }
    else if (Equalsi(FalseString())) {
      return false;
    }
    else {
      TExceptionBase::ThrowInvalidBoolFormat(__POlxSourceInfo, T::Data(),
        T::_Length);
    }
    return false; // to avoid compiler warning
  }
  //...........................................................................
  bool ToBool(bool consider_number) const {
    if (consider_number && IsNumber()) {
      return ToDouble() != 0;
    }
    return ToBool();
  }
  //...........................................................................
  bool IsBool() const {
    return Equalsi(TrueString()) || Equalsi(FalseString());
  }
  //...........................................................................
  // no '\0' at the end, got to do it ourselves
  template <class FT> static FT o_atof(const TC* data, size_t len) {
    if (len == 0) {
      TExceptionBase::ThrowInvalidFloatFormat(__POlxSourceInfo, data, len);
    }
    size_t sts = 0, ste = len; // string start, end
    while (o_iswhitechar(data[sts]) && ++sts < len) {
    }
    while (--ste > sts && o_iswhitechar(data[ste])) {
    }
    if (++ste <= sts) {
      TExceptionBase::ThrowInvalidFloatFormat(__POlxSourceInfo, data, len);
    }
    bool negative = false;
    if (data[sts] == '-') {
      negative = true;
      sts++;
    }
    else if (data[sts] == '+') {
      sts++;
    }
    if (sts == ste) {
      TExceptionBase::ThrowInvalidFloatFormat(__POlxSourceInfo, data, len);
    }
    FT bp = 0, ap = 0, apexp = 1;
    size_t exp = 0;
    bool fpfound = false, expfound = false, expneg = false;
    for (size_t i = sts; i < ste; i++) {
      if (data[i] <= '9' && data[i] >= '0') {
        if (expfound) {
          exp = exp * 10 + (data[i] - '0');
        }
        else if (fpfound) {
          ap = ap * 10 + (data[i] - '0');
          apexp *= 10;
        }
        else {
          bp = bp * 10 + (data[i] - '0');
        }
      }
      else if (data[i] == '.') {
        if (fpfound) {
          TExceptionBase::ThrowInvalidFloatFormat(__POlxSourceInfo, data, len);
        }
        fpfound = true;
      }
      else if (data[i] == 'e' || data[i] == 'E') {
        if (expfound) {
          TExceptionBase::ThrowInvalidFloatFormat(__POlxSourceInfo, data, len);
        }
        expfound = true;
        if (++i == len) {
          break;
        }
        if (data[i] == '-') {
          expneg = true;
        }
        else if (data[i] == '+') {
        }
        else if (data[i] >= '0' && data[i] <= '9') { // anonymous positive exp
          i--;
        }
        else { // invalid dddd.ddddE-/+/ddd format
          TExceptionBase::ThrowInvalidFloatFormat(__POlxSourceInfo, data, len);
        }
      }
      else { // invalid char for a number
        TExceptionBase::ThrowInvalidFloatFormat(__POlxSourceInfo, data, len);
      }
    }
    bp = (expneg) ? (bp + ap / apexp) / olx_pow10<FT>(exp)
      : (bp + ap / apexp)*olx_pow10<FT>(exp);
    return negative ? -bp : bp;
  }
  //...........................................................................
  double ToDouble() const { return ToFloatT<double>(); }
  //...........................................................................
  float ToFloat() const { return ToFloatT<float>(); }
  //...........................................................................
  template <typename FT> FT ToFloatT() const {
    return o_atof<FT>(T::Data(), T::_Length);
  }
  //...........................................................................
  int8_t ToNumber(int8_t &b) const {
    return (b=o_atoi<int8_t>(T::Data(), T::_Length, 10));
  }
  uint8_t ToNumber(uint8_t &b) const {
    return (b=o_atoui<uint8_t>(T::Data(), T::_Length, 10));
  }
  int16_t ToNumber(int16_t &b) const {
    return (b=o_atoi<int16_t>(T::Data(), T::_Length, 10));
  }
  uint16_t ToNumber(uint16_t &b) const {
    return (b=o_atoui<uint16_t>(T::Data(), T::_Length, 10));
  }
  int32_t ToNumber(int32_t &b) const {
    return (b=o_atoi<int32_t>(T::Data(), T::_Length, 10));
  }
  uint32_t ToNumber(uint32_t &b) const {
    return (b=o_atoui<uint32_t>(T::Data(), T::_Length, 10));
  }
  int64_t ToNumber(int64_t &b) const {
    return (b=o_atoi<int64_t>(T::Data(), T::_Length, 10));
  }
  uint64_t ToNumber(uint64_t &b) const {
    return (b=o_atoui<uint64_t>(T::Data(), T::_Length, 10));
  }
  float &ToNumber(float &b) const { return (b=ToFloatT<float>()); }
  double &ToNumber(double &b) const { return (b=ToFloatT<double>()); }
  //...........................................................................
  template <typename num_t> num_t ToNumber() const {
    num_t n;
    return ToNumber(n);
  }
  //...........................................................................
  void SetLength(size_t newLen) {
    if (newLen < T::_Length) {
      T::DecLength(T::_Length - newLen);
    }
    else {
      T::checkBufferForModification(newLen);
      T::_Length = newLen;
    }
  }
  //...........................................................................
  TTSString& Delete(size_t from, size_t count) {
    const size_t dv = from + count;
#ifdef OLX_DEBUG
    if (from >= T::_Length) {
      TExceptionBase::ThrowIndexOutOfRange(__POlxSourceInfo, from, 0,
        T::_Length);
    }
    if (dv > T::_Length) {
      TExceptionBase::ThrowIndexOutOfRange(__POlxSourceInfo, dv, 0,
        T::_Length);
    }
#endif
    if (dv == T::_Length) {
      if (from != 0) {
        T::_Length -= count;  // substring to
      }
      else {
        T::_Length = 0;  // empty string ...
      }
      return *this;
    }
    // delete from start - just substring from
    if (from == 0) {
      T::_Start += count;
      T::_Length -= count;
    }
    else {
      T::checkBufferForModification(T::_Length);
      olx_memmove(&T::Data()[from], &T::Data()[from + count],
        T::_Length - from - count);
      T::DecLength(count);
    }
    return *this;
  }
  //...........................................................................
  //replaces sequences of chars with a single char
  template <typename AC> static size_t o_strdcs(TC* whr, size_t whr_len,
    AC wht)
  {
    size_t ni = 0;
    for (size_t i = 0; i < whr_len; i++, ni++) {
      if (whr[i] == wht && ((i + 1) < whr_len && whr[i + 1] == wht)) {
        ni--;
        continue;
      }
      else if (ni != i) {
        whr[ni] = whr[i];
      }
    }
    return whr_len - ni;
  }
  //...........................................................................
  template <typename OC> TTSString& DeleteSequencesOf(OC wht) {
    T::checkBufferForModification(T::_Length);
    T::DecLength(o_strdcs(T::Data(), T::_Length, wht));
    return *this;
  }
  //...........................................................................
  template <typename AC> static TTSString DeleteSequencesOf(
    const TTSString& str, AC wht)
  {
    return TTSString<T,TC>(str).DeleteSequencesOf(wht);
  }
  //...........................................................................
  //removes chars from string and return the number of removed chars
  template <typename AC> static size_t o_strdch(TC* whr, size_t whr_len,
    AC wht)
  {
    size_t rn = 0;
    for (size_t i = 0; i < whr_len; i++) {
      if (whr[i] == wht) {
        rn++;
        continue;
      }
      else {
        whr[i - rn] = whr[i];
      }
    }
    return rn;
  }
  //...........................................................................
  // removes a string from another; returns number of replacements
  template <typename OC, typename AC>
  static size_t o_strdstr(OC* whr, size_t whr_len, const AC* wht,
    size_t wht_len)
  {
    size_t cnt = 0;
    for (size_t i = 0, dest = 0; i < whr_len; i++, dest++) {
      if (i + wht_len > whr_len) {
        if (dest != i) {
          for (;i < whr_len; i++, dest++) {
            whr[dest] = whr[i];
          }
        }
        return cnt;
      }
      bool found = true;
      for (size_t j = 0; j < wht_len; j++) {
        if (whr[i + j] != wht[j]) {
          found = false;
          break;
        }
      }
      if (found) {
        cnt++;
        i += wht_len - 1;
        dest--;
        continue;
      }
      whr[dest] = whr[i];
    }
    return cnt;
  }
  //removes a set of chars from string and return the number of removed chars
  template <typename AC>
  static size_t o_strdchs(TC* whr, size_t whr_len, const AC* wht,
    size_t wht_len)
  {
    size_t rn = 0;
    for (size_t i = 0; i < whr_len; i++) {
      bool found = false;
      for (size_t j = 0; j < wht_len; j++) {
        if (whr[i] == wht[j]) {
          rn++;
          found = true;
          break;
        }
      }
      if (!found) {
        whr[i - rn] = whr[i];
      }
    }
    return rn;
  }
  //...........................................................................
  template <typename AC> TTSString& DeleteChars(AC wht) {
    T::checkBufferForModification(T::_Length);
    T::DecLength(o_strdch(T::Data(), T::_Length, wht));
    return *this;
  }
  //...........................................................................
  TTSString& DeleteStrings(const TTSString& wht) {
    T::checkBufferForModification(T::_Length);
    T::DecLength(o_strdstr(T::Data(), T::_Length, wht.Data(),
      wht._Length)*wht._Length);
    return *this;
  }
  //...........................................................................
  // deletes a set of chars
  TTSString& DeleteCharSet(const TTSString& wht) {
    T::checkBufferForModification(T::_Length);
    T::DecLength(o_strdchs(T::Data(), T::_Length, wht.raw_str(),
      wht.Length()));
    return *this;
  }
  //...........................................................................
  template <typename AC> static TTSString DeleteChars(const TTSString& str,
    AC wht)
  {
    return TTSString<T,TC>(str).DeleteChars(wht);
  }
  //...........................................................................
  template <typename AC> static TTSString DeleteCharSet(const TTSString& str,
    const TTSString& wht)
  {
    return TTSString<T,TC>(str).DeleteCharSet(wht);
  }
  //...........................................................................
  static size_t o_strins(const TC* wht, size_t wht_len, TC* to, size_t to_len,
    size_t at, size_t amount=1)
  {
    size_t toshift = wht_len*amount;
    if (at < to_len) {
      olx_memmove(&to[at + toshift], &to[at], to_len - at);
    }
    for (size_t i = 0; i < amount; i++) {
      olx_memcpy(&to[at + wht_len*i], wht, wht_len);
    }
    return toshift;
  }
  //...........................................................................
  static size_t o_chrins(TC wht, TC* to, size_t to_len, size_t at,
    size_t amount = 1)
  {
    if (at < to_len) {
      olx_memmove(&to[at + amount], &to[at], to_len - at);
    }
    //memset(to, wht, CharSize*amount);
    for (size_t i = 0; i < amount; i++) {
      to[at + i] = wht;
    }
    return amount;
  }
  //...........................................................................
  TTSString& Insert(const TTSString& wht, size_t whr, size_t amount = 1) {
#ifdef OLX_DEBUG
    if (whr > T::_Length) {
      TExceptionBase::ThrowIndexOutOfRange(__POlxSourceInfo, whr, 0,
        T::_Length);
    }
#endif
    T::checkBufferForModification(T::_Length + wht._Length*amount);
    T::IncLength(
      o_strins(wht.Data(), wht._Length, T::Data(), T::_Length, whr, amount));
    return *this;
  }
  //...........................................................................
  TTSString& Insert(const TC* wht, size_t whr, size_t amount = 1) {
#ifdef OLX_DEBUG
    if (whr > T::_Length) {
      TExceptionBase::ThrowIndexOutOfRange(__POlxSourceInfo, whr, 0,
        T::_Length);
    }
#endif
    size_t wht_len = o_strlen(wht);
    T::checkBufferForModification(T::_Length + wht_len*amount);
    T::IncLength(o_strins(wht, wht_len, T::Data(), T::_Length, whr, amount));
    return *this;
  }
  //...........................................................................
  TTSString& Insert(TC wht, size_t whr, size_t amount = 1) {
#ifdef OLX_DEBUG
    if (whr > T::_Length) {
      TExceptionBase::ThrowIndexOutOfRange(__POlxSourceInfo, whr, 0,
        T::_Length);
    }
#endif
    T::checkBufferForModification(T::_Length + amount);
    T::IncLength(o_chrins(wht, T::Data(), T::_Length, whr, amount));
    return *this;
  }
  //...........................................................................
  // counts number of occurences of chars
  template <typename OC>
  static size_t o_chrcnt(OC wht, const TC* whr, size_t whr_len) {
    size_t cnt = 0;
    for (size_t i = 0; i < whr_len; i++) {
      if (whr[i] == wht) {
        cnt++;
      }
    }
    return cnt;
  }
  //...........................................................................
  template <typename AC> size_t CharCount(AC wht) const {
    return o_chrcnt(wht, T::Data(), T::_Length);
  }
  //...........................................................................
  template <typename OC>
  static size_t o_strcnt(const OC* wht, size_t wht_len, const TC* whr,
    size_t whr_len)
  {
    if (wht_len > whr_len) {
      return 0;
    }
    size_t cnt = 0;
    for (size_t i = 0; i < whr_len; i++) {
      if (i + wht_len > whr_len) {
        return cnt;
      }
      bool found = true;
      for (size_t j = 0; j < wht_len; j++) {
        if (whr[i + j] != wht[j]) {
          found = false;
          break;
        }
      }
      if (found) {
        cnt++;
      }
    }
    return cnt;
  }
  // replaces a string with another, returns number of replacements
  template <typename OC, typename AC>
  static size_t o_strrplstr(const OC* wht, size_t wht_len,
    const AC* with, size_t with_len, TC* whr, size_t whr_len,
    size_t final_len=InvalidIndex)
  {
    if (wht_len > whr_len) {
      return 0;
    }
    size_t cnt = 0;
    if (wht_len >= with_len) {
      for (size_t i = 0, dest = 0; i < whr_len; i++, dest++) {
        if (i + wht_len > whr_len) {
          if (dest != i) {
            for (; i < whr_len; i++) {
              whr[dest++] = whr[i];
            }
          }
          return cnt;
        }
        bool found = true;
        for (size_t j = 0; j < wht_len; j++) {
          if (whr[i + j] != wht[j]) {
            found = false;
            break;
          }
        }
        if (found) {
          cnt++;
          for (size_t j = 0; j < with_len; j++) {
            whr[dest++] = with[j];
          }
          i += wht_len;
          i--;
          dest--;
          continue;
        }
        whr[dest] = whr[i];
      }
      return cnt;
    }
    else {
      if (final_len == InvalidIndex || (final_len - whr_len) < with_len) {
        for (size_t i = 0; i < whr_len; i++) {
          if (i + wht_len > whr_len) {
            return cnt;
          }
          bool found = true;
          for (size_t j = 0; j < wht_len; j++) {
            if (whr[i + j] != wht[j]) {
              found = false;
              break;
            }
          }
          if (found) {
            if (with_len != wht_len) {
              olx_memmove(&whr[i + with_len], &whr[i + wht_len],
                whr_len - i - wht_len);
              whr_len -= (wht_len - with_len);
            }
            for (size_t j = 0; j < with_len; j++) {
              whr[i + j] = with[j];
            }
            cnt++;
            i += (with_len - 1);
          }
        }
      }
      else {
        // shift source to the end
        size_t src_i = 0;
        if (whr_len != final_len) {
          src_i = final_len - whr_len;
          olx_memmove(&whr[src_i], &whr[0], whr_len);
        }
        for (size_t i = 0, dest_i = 0; i < whr_len; i++, dest_i++) {
          bool found = true;
          for (size_t j = 0; j < wht_len; j++) {
            if (whr[src_i + i + j] != wht[j]) {
              found = false;
              break;
            }
          }
          if (found) {
            for (size_t j = 0; j < with_len; j++) {
              whr[dest_i + j] = with[j];
            }
            cnt++;
            dest_i += with_len - 1;
            i += wht_len - 1;
          }
          else {
            whr[dest_i] = whr[src_i + i];
          }
        }
      }
    }
    return cnt;
  }
  //...........................................................................
  // replaces a string with a char
  template <typename OC, typename AC>
  static size_t o_strrplch(const OC* wht, size_t wht_len, AC with,
    TC* whr, size_t whr_len)
  {
    if (wht_len > whr_len) {
      return 0;
    }
    size_t cnt = 0;
    for (size_t i = 0, dest = 0; i < whr_len; i++, dest++) {
      if (i + wht_len > whr_len) {
        if (dest != i) {
          for (; i < whr_len; i++, dest++) {
            whr[dest] = whr[i];
          }
        }
        return cnt;
      }
      bool found = true;
      for (size_t j = 0; j < wht_len; j++) {
        if (whr[i + j] != wht[j]) {
          found = false;
          break;
        }
      }
      if (found) {
        cnt++;
        i += wht_len;
        whr[dest] = with;
        i--;
        continue;
      }
      whr[dest] = whr[i];
    }
    return cnt;
  }
  //...........................................................................
  /* replaces a char with a string, provide final length after the replacement
  for optimal performace
  */
  template <typename OC, typename AC>
  static size_t o_chrplstr(OC wht, const AC* with, size_t with_len,
    TC* whr, size_t whr_len, size_t final_len=InvalidIndex)
  {
    size_t cnt = 0;
    if (final_len == InvalidIndex || (final_len - whr_len) < with_len) {
      for (size_t i = 0; i < whr_len; i++) {
        if (whr[i] == wht) {
          olx_memmove(&whr[i + with_len], &whr[i + 1], whr_len - i - 1);
          whr_len -= (1 - with_len);
          for (size_t j = 0; j < with_len; j++) {
            whr[i + j] = with[j];
          }
          cnt++;
          i += (with_len - 1);
        }
      }
    }
    else {
      // shift source to the end
      size_t src_i = 0;
      if (whr_len != final_len) {
        src_i = final_len - whr_len;
        olx_memmove(&whr[src_i], &whr[0], whr_len);
      }
      for (size_t i = 0, dest_i = 0; i < whr_len; i++, dest_i++) {
        if (whr[src_i + i] == wht) {
          for (size_t j = 0; j < with_len; j++) {
            whr[dest_i + j] = with[j];
          }
          cnt++;
          dest_i += (with_len - 1);
        }
        else {
          whr[dest_i] = whr[src_i + i];
        }
      }
    }
    return cnt;
  }
  //...........................................................................
  template <typename OC, typename AC> // replace chars
  static size_t o_chrplch(OC wht, AC with, TC* whr, size_t whr_len) {
    size_t cnt = 0;
    for (size_t i = 0; i < whr_len; i++) {
      if (whr[i] == wht) {
        whr[i] = with;
        cnt++;
      }
    }
    return cnt;
  }
#ifndef __BORLANDC__ // what use of templates when this does not get them???
  //...........................................................................
#if defined(__WXWIDGETS__)
  template <typename OC> TTSString& Replace(OC wht, const wxString &with) {
    return Replace(wht, (const TC*)with.c_str(), with.Length());
  }
  template <typename OC> TTSString& Replace(const OC* wht,
    const wxString &with)
  {
    return Replace(wht, ~0, (const TC*)with.c_str(), with.Length());
  }
  TTSString& Replace(const TTSString &wht, const wxString &with) {
    return Replace(wht.Data(), wht._Length,
      (const TC*)with.c_str(), with.Length());
  }
#endif
  //...........................................................................
  TTSString& Replace(const TTSString& wht, const TTSString& with) {
    return Replace(wht.Data(), wht._Length, with.Data(), with._Length);
  }
  //...........................................................................
  template <typename AC> TTSString& Replace(const TTSString& wht,
    const AC* with)
  {
    return Replace(wht.Data(), wht._Length, with, ~0);
  }
  //...........................................................................
  template <typename AC> TTSString& Replace(const TTSString& wht, AC with) {
    return Replace(wht.Data(), wht._Length, with);
  }
  //...........................................................................
  template <typename OC> TTSString& Replace(const OC* wht,
    const TTSString& with)
  {
    return Replace(wht, ~0, with.Data(), with._Length);
  }
  //...........................................................................
  template <typename OC> TTSString& Replace(OC wht, const TTSString& with) {
    return Replace(wht, with.Data(), with._Length);
  }
  //...........................................................................
  template <typename OC, typename AC> TTSString& Replace(OC wht, AC with) {
    T::checkBufferForModification(T::_Length);
    o_chrplch(wht, with, T::Data(), T::_Length);
    return *this;
  }
  //...........................................................................
  template <typename OC, typename AC>
  TTSString& Replace(const OC* wht, AC with) {
    return Replace(wht, ~0, with);
  }
  //...........................................................................
  template <typename OC, typename AC>
  TTSString& Replace(const OC* wht, size_t wht_l, AC with) {
    size_t wht_len = wht_l == InvalidIndex ? o_strlen(wht) : wht_l;
    T::checkBufferForModification(T::_Length);
    size_t rv = o_strrplch(wht, wht_len, with, T::Data(), T::_Length);
    T::_Length -= (wht_len - 1)*rv;
    return *this;
  }
  //...........................................................................
  template <typename OC, typename AC>
  TTSString& Replace(OC wht, const AC* with, size_t with_l = ~0) {
    size_t extra_len = 0,
      with_len = (with_l == InvalidIndex ? o_strlen(with) : with_l);
    if (with_len == 0) {
      T::_Length -= o_strdch(T::Data(), T::_Length, wht);
      return *this;
    }
    if (with_len == 1) {
      o_chrplch(wht, *with, T::Data(), T::_Length);
      return *this;
    }
    size_t ch_cnt = o_chrcnt(wht, T::Data(), T::_Length);
    if (ch_cnt == 0) {
      return *this;
    }
    extra_len = (with_len - 1) * ch_cnt;
    T::checkBufferForModification(T::_Length + extra_len);
    size_t rv = o_chrplstr(wht, with, with_len, T::Data(), T::_Length,
      ch_cnt > 1 ? T::_Length + extra_len : InvalidIndex);
    T::_Length -= (1 - with_len)*rv;
    return *this;
  }
  //...........................................................................
  template <typename OC, typename AC>
  TTSString& Replace(const OC* wht, const AC* with)  {
    return Replace(wht, ~0, with, ~0);
  }
  //...........................................................................
  template <typename OC, typename AC>
  TTSString& Replace(const OC* wht, size_t wht_l, const AC* with,
    size_t with_l)
  {
    size_t extra_len = 0,
      with_len = (with_l == InvalidIndex ? o_strlen(with) : with_l),
      wht_len = (wht_l == InvalidIndex ? o_strlen(wht) : wht_l);
    if( with_len == 0 ) {
      T::_Length -= o_strdstr(T::Data(), T::_Length, wht, wht_len)*wht_len;
      return *this;
    }
    if( wht_len < with_len )  {
      extra_len = (with_len - wht_len)*
        o_strcnt(wht, wht_len, T::Data(), T::_Length);
      if (extra_len == 0) {
        return *this;
      }
    }
    T::checkBufferForModification(T::_Length+extra_len);
    const size_t rv = o_strrplstr(wht, wht_len, with, with_len, T::Data(),
      T::_Length, T::_Length + extra_len);
    T::_Length -= (wht_len - with_len)*rv;
    return *this;
  }
#else  // just 4 functions will do (str,str), (ch,str), (str,ch) and (ch,ch)
  TTSString& Replace(olxch wht, const TTSString& with)  {
    size_t extra_len = 0;
    if( with._Length > 1 )  {
      extra_len = (with._Length - 1) * o_chrcnt(wht, T::Data(), T::_Length);
      if( extra_len == 0 )  return *this;
    }
    T::checkBufferForModification(T::_Length+extra_len);
    size_t rv = o_chrplstr(wht, with.Data(), with._Length, T::Data(),
      T::_Length);
    T::_Length -= (1 - with._Length)*rv;
    return *this;
  }
  //...........................................................................
  TTSString& Replace(const TTSString& wht, olxch with)  {
    T::checkBufferForModification(T::_Length);
    size_t rv = o_strrplch(wht.Data(), wht._Length, with, T::Data(),
      T::_Length);
    T::_Length -= (wht._Length - 1)*rv;
    return *this;
  }
  //...........................................................................
  TTSString& Replace(olxch wht, olxch with)  {
    T::checkBufferForModification(T::_Length);
    o_chrplch(wht, with, T::Data(), T::_Length);
    return *this;
  }
#endif // __BORLANDC__
  //...........................................................................
  template <typename AC> TTSString& Trim(AC wht) {
    if (T::_Length == 0) {
      return *this;
    }
    size_t start = 0, end = T::_Length;
    while (TTIString<TC>::CharAt(start) == wht && ++start < end) {
    }
    while (--end > start && TTIString<TC>::CharAt(end) == wht) {
    }
    T::_Start += start;
    T::_Length = (end + 1 - start);
    return *this;
  }
  //...........................................................................
  template <typename AC> TTSString& TrimL(AC wht) {
    if (T::_Length == 0) {
      return *this;
    }
    size_t start = 0;
    while (TTIString<TC>::CharAt(start) == wht && ++start < T::_Length) {
    }
    T::_Start += start;
    T::_Length = (T::_Length - start);
    return *this;
  }
  //...........................................................................
  template <typename AC> TTSString& TrimR(AC wht) {
    if (T::_Length == 0) {
      return *this;
    }
    if (T::_Length == 1) {
      if (TTIString<TC>::CharAt(0) == wht) {
        T::_Length = 0;
      }
      return *this;
    }
    size_t end = T::_Length;
    while (--end > 0 && TTIString<TC>::CharAt(end) == wht) {
    }
    T::_Length = (end + 1);
    return *this;
  }
  //...........................................................................
  bool IsWhiteCharString() const {
    const TC* data = T::Data();
    for (size_t i = 0; i < T::_Length; i++) {
      if (!o_iswhitechar(data[i])) {
        return false;
      }
    }
    return true;
  }
  //...........................................................................
  TTSString& TrimWhiteChars(bool leading = true, bool trailing = true) {
    if (T::_Length == 0) {
      return *this;
    }
    size_t start = 0, end = T::_Length;
    if (leading) {
      while (o_iswhitechar(TTIString<TC>::CharAt(start)) && ++start < end) {
      }
    }
    if (trailing) {
      while (--end > start && o_iswhitechar(TTIString<TC>::CharAt(end))) {
      }
    }
    else {
      end--;
    }
    T::_Start += start;
    T::_Length = (end - start + 1);
    return *this;
  }
  //...........................................................................
  TTSString& TrimFloat() {
    T::TrimFloat();
    return *this;
  }
  //...........................................................................
  TTSString& Padding(size_t count, const TTSString& sep, bool right,
    bool ensure_sep = false)
  {
    size_t extra = ((T::_Length >= count) ? 0 : count - T::_Length);
    if (ensure_sep && extra == 0) {
      if (!EndsWith(sep))
        extra = 1;
    }
    if (extra != 0) {
      T::checkBufferForModification(T::_Length + extra*sep._Length);
      Insert(sep, right ? T::_Length : 0, extra);
    }
    return *this;
  }
  //...........................................................................
  template <typename AC>
  TTSString& Padding(size_t count, AC sep, bool right, bool ensure_sep = false) {
    size_t extra = ((T::_Length >= count) ? 0 : count - T::_Length);
    if (ensure_sep && extra == 0) {
      if (!StartsFrom(sep))
        extra = 1;
    }
    if (extra != 0) {
      T::checkBufferForModification(T::_Length + extra);
      Insert(sep, right ? T::_Length : 0, extra);
    }
    return *this;
  }
  //...........................................................................
  template <typename sep_t>
  TTSString& LeftPadding(size_t count, const sep_t& sep,
    bool ensure_sep=false)
  {
    return Padding(count, sep, false, ensure_sep);
  }
  //...........................................................................
  template <typename sep_t>
  TTSString& RightPadding(size_t count, const sep_t& sep,
    bool ensure_sep=false)
  {
    return Padding(count, sep, true, ensure_sep);
  }
  //...........................................................................
  static TTSString FormatFloat(int NumberOfDigits, double v,
    bool Exponent = false)
  {
    TTSString<T, TC> fmt, rv;
    if (NumberOfDigits < 0) {
      NumberOfDigits = -NumberOfDigits;
      if (v >= 0) {
        fmt = ' ';
      }
    }
    fmt << "%." << NumberOfDigits;
    if (!Exponent) {
      if ((v < 0 ? -v : v) > pow(10.0, 80-NumberOfDigits-2)) {
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo,
          "format overrun");
      }
      fmt << 'f';
    }
    else {
      fmt << 'e';
    }
    fmt << '\0';
    rv.setTypeValue(fmt.Data(), v);
    return rv;
  }
  //...........................................................................
  /* checks if provided string represent an integer or float point number (inc
  exponental form) */
  static bool o_isnumber(const TC* data, size_t len) {
    if (len == 0)  return false;
    size_t sts = 0, ste = len; // string start
    while (o_iswhitechar(data[sts]) && ++sts < len) {
    }
    while (--ste > sts && o_iswhitechar(data[ste])) {
    }
    if (++ste <= sts) {
      return false;
    }
    if (data[sts] == '0') {
      len = ste - sts;
      if (len == 1) {
        return true;
      }
      // hex notation
      if (data[sts + 1] == 'x' || data[sts + 1] == 'X') {
        if (len == 2) {
          return false;
        }
        for (size_t i = sts + 2; i < ste; i++) {
          if (!o_ishexdigit(data[i])) {
            return false;
          }
        }
        return true;
      }
      // octal or float (0e-5 0.nnn) then...
      if (data[sts + 1] != '.' && !(data[sts + 1] == 'e' ||
        data[sts + 1] == 'E'))
      {
        for (size_t i = sts + 1; i < ste; i++) {
          if (data[i] < '0' || data[i] > '7') {
            return false;
          }
        }
        return true;
      }
    }
    bool expfound = false, fpfound = false;
    short digit_cnt = 0;
    if (data[sts] == '+' || data[sts] == '-') {
      sts++;
    }
    for (size_t i = sts; i < ste; i++) {
      TC ch = data[i];
      if (o_isdigit(ch)) {
        digit_cnt++;
        continue;
      }
      else if (ch == '.') {
        if (fpfound) {
          return false;
        }
        fpfound = true;
      }
      else if (ch == 'e' || ch == 'E') {
        if (expfound) {
          return false;
        }
        expfound = true;
        if (++i == ste) { // exponent cannot be the last char
          return false;
        }
        ch = data[i];
        if (ch == '-') {
        }
        else if (ch == '+') {
          ;
        }
        else if (o_isdigit(ch)) { // anonymously positive exp
          digit_cnt++;
          i--;
        }
        else { // invalid dddd.ddddE-/+/ddd format
          return false;
        }
      }
      else {
        return false;
      }
    }
    return (digit_cnt != 0);
  }
  /* checks if the string represent and integer (ocatal, decimal or hex) or
  float point number  (inc exponental form) */
  bool IsNumber() const {  return o_isnumber(T::Data(), T::_Length);  }
  //...........................................................................
  void SetIncrement(size_t ni)  {  T::_Increment = ni;  }
  void SetCapacity(size_t newc)  {
    if (T::SData == 0) {
      T::SData = new struct T::Buffer(newc + T::_Increment);
    }
    else if (newc > T::GetCapacity()) {
      T::checkBufferForModification(newc);
    }
  }
  //...........................................................................
  TTSString& AppendFromStream(IInputStream& ios, size_t len)  {
    T::checkBufferForModification(T::_Length + len+1);
    ios.Read((void*)&T::SData->Data[T::_Start+T::_Length], len*sizeof(TC));
    T::_Length += len;
    return *this;
  }
  //...........................................................................
  TTSString& FromBinaryStream(IInputStream& ios) {
    uint32_t code, len, charsize;
    ios.Read(&code, sizeof(uint32_t));
    charsize = ExtractCharSize(code);
    if (sizeof(TC) != charsize) {
      if (charsize != 0 || sizeof(TC) == 2) {
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo,
          "incompatible Char size");
      }
    }
    len = ExtractLength(code);
    if (len > (uint32_t)(ios.GetSize() - ios.GetPosition())) {
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo,
        "invalid stream content");
    }
    T::_Start = 0;
    T::_Increment = 8;
    T::_Length = len;
    if (T::SData != 0) {
      if (T::SData->RefCnt == 1) { // owed by this object
        T::SData->SetCapacity(T::_Length);
      }
      else {
        T::SData->RefCnt--;
        T::SData = 0;
      }
    }
    if (T::SData == 0) {
      T::SData = new struct T::Buffer(T::_Length);
    }
    ios.Read((void*)T::SData->Data, T::_Length*sizeof(TC));
    return *this;
  }
  //...........................................................................
  void ToBinaryStream(IOutputStream& os) const {
    uint32_t len = (uint32_t)(CodeLength(sizeof(TC), T::_Length));
    os.Write(&len, sizeof(uint32_t));
    os.Write((void*)T::Data(), T::_Length*sizeof(TC));
  }
  //...........................................................................
  static TTSString CharStr(TC ch, size_t count)  {
    TTSString<T,TC> rv(EmptyString(), count);
    rv.Insert(ch, 0, count);
    return rv;
  }
  //...........................................................................
  olxch GetLast() const {
    if (T::_Length == 0) {
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "empty string");
    }
    return T::SData->Data[T::_Start+T::_Length-1];
  }
  //...........................................................................
  // just checks if string contains any chars > 127
  static bool o_needs_converting(const wchar_t* data, size_t len) {
    for (size_t i = 0; i < len; i++) {
      if (data[i] > 127 || data[i] < 0) {
        return true;
      }
    }
    return false;
  }
  static bool o_needs_converting(const char* data, size_t len) {
    for (size_t i = 0; i < len; i++) {
      if ((uint8_t)data[i] > 127) {
        return true;
      }
    }
    return false;
      }
  //...........................................................................
#ifdef __BORLANDC__
  typedef TTSString<TCString, char> olxcstr;
  typedef TTSString<TWString, wchar_t> olxwstr;
#endif
  //...........................................................................
  bool NeedsConverting() const {
    return o_needs_converting(T::Data(), T::_Length);
  }
  /* converts a wide char string into multibyte string properly
   (using current locale)
   */
  static TTSString FromCStr(const wchar_t* wstr, size_t len=~0);
  static TTSString FromCStr(const char* mbs, size_t len=~0);
  static olxwstr FromUTF8(const char* mbs, size_t len = ~0);

  olxcstr ToMBStr() const;
  olxwstr ToWCStr() const;
  olxcstr ToUTF8() const;
  //...........................................................................
  virtual IOlxObject* Replicate() const {  return new TTSString<T,TC>(*this);  }
  //...........................................................................
  // streaming and enclosing (quotation in particular) helpers
  template <typename sep_t> struct SeperatedStream {
    TTSString& dest;
    sep_t separator;
    SeperatedStream(TTSString& str, const sep_t& sep)
      : dest(str), separator(sep) {}
    template <typename val_t>
    SeperatedStream& operator << (const val_t& v)  {
      if( !dest.IsEmpty() ) dest << separator;
      dest << v;
      return *this;
    }
  };

  template <typename sep_t> struct Encloser {
    TTSString& dest;
    sep_t s, e;
    Encloser(TTSString& str, const sep_t& _s, const sep_t& _e)
      : dest(str), s(_s), e(_e) {}
    template <typename val_t>
    TTSString& operator << (const val_t& v)  {
      return dest << s << v << e;
    }
  };

  template <typename sep_t>
  SeperatedStream<sep_t> stream(const sep_t& separator) {
    return SeperatedStream<sep_t>(*this, separator);
  }

  SeperatedStream<olxstr> stream(const char *separator) {
    return SeperatedStream<olxstr>(*this, TTSString(separator));
  }

  template <typename sep_t>
  Encloser<sep_t> enclose(const sep_t& front, const sep_t& rear) {
    return Encloser<sep_t>(*this, front, rear);
  }

  Encloser<TC> quote(TC quote_char='\'') {
    return Encloser<TC>(*this, quote_char, quote_char);
  }

  template <typename sep_t>
  TTSString(const SeperatedStream<sep_t>& str)  {  InitFromString(str.dest);  }

  TTSString(const TTStrBuffer<TC,TTSString<T,TC> > &buf)  {
    T::_Start = 0;
    T::_Increment = 8;
    T::_Length = buf.Length();
    TC *arr = olx_malloc<TC>(T::_Length+1);
    buf.Read(arr);
    T::SData = new struct T::Buffer(arr, T::_Length);
  }
  //...........................................................................
  template <typename seq_t>
  static bool o_isoneof(TC ch, const seq_t &seq, size_t l) {
    for (size_t i = 0; i < l; i++)
    if (ch == seq[i])
      return true;
    return false;
  }
  template <typename seq_t>
  static bool o_isoneof(TC ch, const seq_t &seq) {
    return o_isoneof(ch, seq, o_strlen(seq));
  }
  // convenience method
  static bool o_isoneof(TC ch, char a, char b) {
    return ch == a || ch == b;
  }
  static bool o_isoneof(TC ch, wchar_t a, wchar_t b) {
    return ch == a || ch == b;
  }

  template <typename AC, bool lc>
  static int32_t o_hashcode(const AC* data, size_t len) {
    int32_t h = 0;
    for (size_t i = 0; i < len; i++) {
      h = 31 * h + (lc ? o_ltolower(data[i]) : data[i]);
    }
    return h;
  }


  template <bool lc>
  int32_t HashCode() const { return o_hashcode<TC, lc>(T::Data(), T::_Length); }
  // Java-compatible string hash code
  int32_t HashCode() const { return o_hashcode<TC, false>(T::Data(), T::_Length); }

  template <class list_t, typename accessor_t>
  static TTSString Join(const list_t &l, const accessor_t &accessor,
    const TTSString &sep, size_t start=0, size_t end=InvalidIndex)
  {
    if (end == InvalidIndex) {
      end = l.Count();
    }
    if (start == end) {
      return EmptyString();
    }
#ifdef OLX_DEBUG
    if (start >= l.Count()) {
      TExceptionBase::ThrowIndexOutOfRange(__POlxSourceInfo, start, 0, l.Count());
    }
    if (end > l.Count()) {
      TExceptionBase::ThrowIndexOutOfRange(__POlxSourceInfo, end, 0, l.Count());
    }
#endif
    TTStrBuffer<TC, TTSString<T, TC> > rv;
    for (size_t i = start; i < end; i++) {
      rv << sep << TTSString(accessor(olx_ref::get(l[i])));
    }
    return TTSString(rv).SubStringFrom(sep.Length());
  }

  template <class list_t>
  static TTSString Join(const list_t &l, const TTSString &sep,
    size_t start=0, size_t end = InvalidIndex)
  {
    return Join(l, DummyAccessor(), sep, start, end);
  }

  template <class list_t>
  TTSString Join(const list_t &l) const {
    // return Join(l, *this);
    return Join(l, DummyAccessor(), *this);
  }

  template <class list_t, typename accessor_t>
  TTSString Join(const list_t &l, const accessor_t &acc) const {
    return Join(l, acc, *this);
  }

  template <class list_t>
  TTSString JoinRange(const list_t& l,
    size_t start, size_t end = InvalidIndex) const
  {
    return Join(l, *this, start, end);
  }

  template <class list_t, typename accessor_t>
  TTSString JoinRange(const list_t& l, const accessor_t& acc,
    size_t start, size_t end = InvalidIndex) const
  {
    return Join(l, acc, *this, start, end);
  }
};

template <class T>
olxstr olx_quote(const T& v, olxch q = '\'') {
  olxstr rv(olx_reserve(olxstr::o_strlen(v) + 2));
  return rv << q << v << q;
}

#include "olx_strcvt.h"
EndEsdlNamespace()
#endif
