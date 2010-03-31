#ifndef olx_sdl_i_string_H
#define olx_sdl_i_string_H
#include <string.h>
#include <wctype.h>
#include <math.h>

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
  #define olx_strcmpn  strncmp
  #define olx_strcmpni strncasecmp
  #define olx_wcscmpn  wcsncmp
  #if defined(__MAC__)
    static int olx_wcscmpni(const wchar_t* s1, const wchar_t* s2, size_t len)  {
	  for( size_t i=0; i < len; i++ )  {
	    int diff = towupper(s1[i]) - towupper(s2[i]); 
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

// returns 10^val, cannot put it to emath due to dependencies...
template <typename FT> FT olx_pow10(size_t val)  {
  if( val == 0 )  return 1;
  FT rv = 10;
  while( --val > 0 ) rv *=10;
  return rv;
}

#ifndef __BORLANDC__
template <class,typename> class TTSString;

typedef TTSString<TCString, char> olxcstr;
typedef TTSString<TWString, wchar_t> olxwstr;

#ifdef _UNICODE
  typedef TTSString<TWString, wchar_t> olxstr;
#else
  typedef TTSString<TCString, char > olxstr;
#endif

extern const olxstr &EmptyString;
extern const olxstr &FalseString;
extern const olxstr &TrueString;
extern const olxstr &NullString;

extern const olxcstr &CEmptyString;
extern const olxcstr &CFalseString;
extern const olxcstr &CTrueString;
extern const olxcstr &CNullString;

extern const olxwstr &WEmptyString;
extern const olxwstr &WFalseString;
extern const olxwstr &WTrueString;
extern const olxwstr &WNullString;
#endif

template <class T, typename TC> class TTSString : public T  {
  void InitFromCharStr(const TC* str, size_t len)  {
    T::_Start = 0;
    T::_Increment = 8;
    T::_Length = len;
    T::SData = new struct T::Buffer(T::_Length+T::_Increment, str, T::_Length);
  }
template <class SC>
  void InitFromString(const SC& str)  {
#ifdef __GNUC__
    T::SData = str.GetBuffer();
    T::_Length = str.Length();
    T::_Start = str.Start();
#else
    T::SData = str.SData;
    T::_Length = str._Length;
    T::_Start = str._Start;
#endif
    if( T::SData != NULL )  
      T::SData->RefCnt++;
    T::_Increment = 8;
  }
  TTSString& AssignCharStr(const TC* str, size_t len=~0)  {
    T::_Start = 0;
    T::_Increment = 8;
    T::_Length = ((len == ~0) ? o_strlen(str) : len);
    if( T::SData != NULL )  {
      if( T::SData->RefCnt == 1 )  { // owed by this object
        T::SData->SetCapacity(T::_Length);
        memcpy(T::SData->Data, str, T::_Length * T::CharSize);
      }
      else  {
        T::SData->RefCnt--;
        T::SData = NULL;
      }
    }
    if( T::SData == NULL )  
      T::SData = new struct T::Buffer(T::_Length + T::_Increment, str, T::_Length);
    return *this;
  }

public:
  TTSString() : T()  {  }
  //............................................................................
  TTSString(const TTSString& str, size_t start, size_t length) {
    T::SData = str.SData;
    if( T::SData != NULL )  T::SData->RefCnt++;
    T::_Start = str._Start + start;
    T::_Length = length;
    T::_Increment = 8;
  }
  //............................................................................
  TTSString(const TTSString& str)  {  InitFromString(str);  }
  //............................................................................
  TTSString(const TTSString& str, size_t capacity)  {
    InitFromString(str);
    if( capacity != 0 )
      T::checkBufferForModification(T::_Length + capacity);
  }
  //............................................................................
  TTSString(const TC* str)  { InitFromCharStr(str, o_strlen(str));  }
  TTSString(TC* const& str) { InitFromCharStr(str, o_strlen(str));  }
  //............................................................................
  TTSString(const TC* str, size_t len)  { InitFromCharStr(str, len);  }
  TTSString(TC* const& str, size_t len) { InitFromCharStr(str, len);  }
  //............................................................................
  TTSString(const TC& v) {
    T::_Start = 0;
    T::_Increment = 8;
    T::_Length = 1;
    T::SData = new struct T::Buffer(T::_Length+T::_Increment);
    T::SData->Data[0] = v;
  }
  //............................................................................
  TTSString(const TTIString<TC>& str)  { InitFromString(str);  }
  //............................................................................
  // allocates requested size
  void Allocate(size_t sz)  {
    T::checkBufferForModification(sz);
    T::_Length = sz;
  } 

  template <class T1, typename TC1> TTSString(const TTSString<T1,TC1>& v) : T((const T1&)v)  {  }
  //............................................................................
  template <typename AC> TTSString(const AC& v) : T(v)  {  }
  //............................................................................
  // creates a string from external array allocated with new
  static TTSString FromExternal(TC* data, size_t len)  {
    TTSString rv;
    rv._Length = len;
    rv.SData = new struct T::Buffer(data, len);
    return rv;
  }
  //............................................................................
  TTSString& operator << (const TIString& v)  { T::Append(v.raw_str(), v.Length());  return *this;  }
  //............................................................................
  template <class AC> TTSString& operator << (const AC& v)  { T::operator << (v);  return *this;  }
  //............................................................................
  TTSString& operator << (const TC& v)  {
    T::checkBufferForModification(T::_Length + 1);
    T::SData->Data[T::_Length] = v;
    T::_Length++;
    return *this;
  }
  template <typename AC> TTSString& Append(const AC* data, size_t len)  {
    T::Append(data, len);
    return *this;
  }
  TTSString& operator << (TC* const& v)      { return Append(v, o_strlen(v));  }
  TTSString& operator << (const TC* v)        { return Append(v, o_strlen(v));  }
  TTSString& operator << (const TTSString& v) { return Append(v.Data(), v.Length());  }
  template <class T1, typename TC1>
  TTSString& operator << (const TTSString<T1,TC1>& v) { return Append(v.raw_str(), v.Length());  }
  TTSString& operator << (const bool& v)      { return operator << (v ? TrueString : FalseString);  }
  //............................................................................
  //............................................................................
  TTSString& operator = (const TTIString<TC>& v)   {
    if( T::SData != NULL && --T::SData->RefCnt == 0 )  delete T::SData;
#ifdef __GNUC__
    T::_Start = v.Start();
    T::_Length = v.Length();
    T::SData = v.GetBuffer();
#else
    T::_Start = v._Start;
    T::_Length = v._Length;
    T::SData = v.SData;
#endif
    if( T::SData != NULL )  T::SData->RefCnt++;
    return *this;
  }
  template <class AC> TTSString& operator = (const AC& v)   { T::operator =(v);  return *this; }
  //............................................................................
  //............................................................................
  TTSString& operator = (TC* const& str)  {  return AssignCharStr(str);  }
  TTSString& operator = (const TC* str)   {  return AssignCharStr(str);  }
  //............................................................................
  TTSString& operator = (bool v) {
    (*this) = (v ? TrueString : FalseString);
    return *this;
  }
  //............................................................................
  TTSString& operator = (const TC& ch)  {
    T::_Start = 0;
    T::_Increment = 8;
    T::_Length = 1;
    if( T::SData != NULL )  {
      if( T::SData->RefCnt == 1 )  { // owed by this object
        T::SData->SetCapacity(T::_Length);
        T::SData->Data[0] = ch;
      }
      else  {
        T::SData->RefCnt--;
        T::SData = NULL;
      }
    }
    if( T::SData == NULL )  {
      T::SData = new struct T::Buffer(T::_Length + T::_Increment);
      T::SData->Data[0] = ch;
    }
    return *this;
  }
  //............................................................................
  template <class T1, typename TC1> TTSString& operator = (const TTSString<T1,TC1>& v)  {
    T::operator = ((const T1&)v);
    return *this;
  }
  TTSString& operator = (const TTSString& v)  {
    if( &v == this )  
      return *this;
    if( T::SData != NULL && --T::SData->RefCnt == 0 )  delete T::SData;
    T::_Start = v._Start;
    T::_Length = v._Length;
    T::SData = v.SData;
    if( T::SData != NULL )  T::SData->RefCnt++;
    return *this;
  }
  //............................................................................
  TTSString SubString(size_t from, size_t count) const {
    return TTSString<T,TC>(*this, from, count);
  }
  TTSString SubStringFrom(size_t from, size_t indexFromEnd=0) const {
    return TTSString<T,TC>(*this, from, T::_Length-from-indexFromEnd);
  }
  TTSString SubStringTo(size_t to, size_t indexFromStart=0) const {
    return TTSString<T,TC>(*this, indexFromStart, to-indexFromStart);
  }
  //............................................................................
  static char o_toupper(char ch)     {  return (ch >='a' && ch <= 'z') ? (ch + 'A'-'a') : ch;  }
  static char o_tolower(char ch)     {  return (ch >='A' && ch <= 'Z') ? (ch + 'a'-'A') : ch;  }
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
  static bool o_isalphabetic(char ch)  {  
    return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');  
  }
  static bool o_isalphabetic(wchar_t ch)  {  
    return (ch >= L'a' && ch <= L'z') || (ch >= L'A' && ch <= L'Z');  
  }
  static bool o_isalphanumeric(char ch)  {  return o_isdigit(ch) || o_isalphabetic(ch);  }
  static bool o_isalphanumeric(wchar_t ch)  {  return o_isdigit(ch) || o_isalphabetic(ch);  }
  static bool o_iswhitechar(char ch)     {  return (ch == ' ' || ch == '\t');  }
  static bool o_iswhitechar(wchar_t ch)  {  return (ch == L' ' || ch == L'\t');  }
  static size_t o_strlen(const char* cstr)    {  return (cstr==NULL) ? 0 : strlen(cstr);  }
  static size_t o_strlen(const wchar_t* wstr) {  return (wstr==NULL) ? 0 : wcslen(wstr);  }
  static void o_strtup(TC* wht, size_t wht_len)  {
    for( size_t i=0; i < wht_len; i++ )  wht[i] = o_toupper(wht[i]);
  }
  static void o_strtlw(TC* wht, size_t wht_len)  {
    for( size_t i=0; i < wht_len; i++ )  wht[i] = o_tolower(wht[i]);
  }
  // returns length of common string
  template <typename OC, typename AC>
  static size_t o_cmnstr(OC* str1, size_t str1_len, AC* str2, size_t str2_len)  {
    size_t mlen = olx_min(str1_len, str2_len);
    for( size_t i=0; i < mlen; i++ )
      if( str1[i] != str2[i] )  return i;
    return mlen;
  }
  //............................................................................
  TTSString& UpperCase()  {
    T::checkBufferForModification(T::_Length);
    o_strtup( T::Data(), T::_Length );
    return *this;
  }
  //............................................................................
  TTSString& LowerCase()  {
    T::checkBufferForModification(T::_Length);
    o_strtlw( T::Data(), T::_Length );
    return *this;
  }
  //............................................................................
  TTSString ToUpperCase() const {
    TTSString<T, TC> rv(*this);
    return rv.UpperCase();
  }
  //............................................................................
  TTSString ToLowerCase() const {
    TTSString<T, TC> rv(*this);
    return rv.LowerCase();
  }
  //............................................................................
  static TTSString UpperCase(const TTSString& str)  {
    TTSString<T, TC> rv(str);
    return rv.UpperCase();
  }
  //............................................................................
  static TTSString LowerCase(const TTSString& str)  {
    TTSString<T, TC> rv(str);
    return rv.LowerCase();
  }
  //............................................................................
  TTSString CommonString(const TTSString& str)  const {
    return SubStringTo( o_cmnstr(T::Data(), T::_Length, str.Data(), str.Length()) );
  }
  //............................................................................
  static TTSString CommonString(const TTSString& str1, const TTSString& str2) {
    return str1.CommonString(str2);
  }
  //............................................................................
  template <typename OC, typename AC> static int o_memcmp(const OC* wht, const AC* with, size_t len) {
    for( size_t i=0; i < len; i++ )
      if( wht[i] != with[i])  return wht[i]-with[i];
    return 0;
  }
  static int o_memcmp(const char* wht, const char* with, size_t len) {
    return olx_strcmpn(wht, with, len);
  }
  static int o_memcmp(const wchar_t* wht, const wchar_t* with, size_t len) {
    return olx_wcscmpn(wht, with, len);
  }
  template <typename OC, typename AC> static int o_memcmpi(const OC* wht, const AC* with, size_t len) {
    for( size_t i=0; i < len; i++ )
      if( o_toupper(wht[i]) != o_toupper(with[i]))  return (o_toupper(wht[i])-o_toupper(with[i]));
    return 0;
  }
  static int o_memcmpi(const char* wht, const char* with, size_t len) {
    return olx_strcmpni(wht, with, len);

  }
  static int o_memcmpi(const wchar_t* wht, const wchar_t* with, size_t len) {
    return olx_wcscmpni(wht, with, len);
  }
  template <typename OC, typename AC>
  static int o_strcmp(const OC* wht, size_t len_a, const AC* with, size_t len_b, bool CI) {
    if( len_a == len_b )  {
      if( len_a == 0 )  return 0;
      return CI ? o_memcmpi(wht, with, len_a) : o_memcmp(wht, with, len_a);
    }
    if( len_a == 0 )  return -1;
    if( len_b == 0 )  return 1;
    int res = CI ? o_memcmpi(wht, with, olx_min(len_a, len_b)) :
                   o_memcmp(wht, with, olx_min(len_a, len_b));
    if( res != 0 )  return res;
    if( len_a < len_b )  return -1;
    return 1;
  }
  int Compare(const TTSString& v)       const { return o_strcmp(T::Data(), T::_Length, v.Data(), v._Length, false );  }
  int Compare(const char& v)            const { 
    if( T::_Length == 0 )  return -1;
    const int df = T::Data()[0] - v;
    return df != 0 ? df : (T::_Length == 1 ? 0 : 1);
  }
  int Compare(const wchar_t& v)         const { 
    if( T::_Length == 0 )  return -1;
    const int df = T::Data()[0] - v;
    return df != 0 ? df : (T::_Length == 1 ? 0 : 1);
  }
  template <typename AC>
    int Compare(const AC* v)            const { return o_strcmp(T::Data(), T::_Length, v, o_strlen(v), false );  }
  int Comparei(const TTSString& v)      const { return o_strcmp(T::Data(), T::_Length, v.Data(), v._Length, true );  }
  int Comparei(const char& v)           const { 
    if( T::_Length == 0 )  return -1;
    const int df = o_toupper(T::Data()[0]) - o_toupper(v);
    return df != 0 ? df : (T::_Length == 1 ? 0 : 1);
  }
  int Comparei(const wchar_t& v)        const { 
    if( T::_Length == 0 )  return -1;
    const int df = o_toupper(T::Data()[0]) - o_toupper(v);
    return df != 0 ? df : (T::_Length == 1 ? 0 : 1);
  }
  template <typename AC>
    int Comparei(const AC* v)           const { return o_strcmp(T::Data(), T::_Length, v, o_strlen(v), true );  }
  template <typename AC>
    bool Equals(const AC& v)            const {  return Compare(v) == 0;  }
  template <typename AC>
    bool Equalsi(const AC& v)           const {  return Comparei(v) == 0;  }

  template <typename AC>
    bool operator == (const AC& v)      const { return Equals(v); }
  template <typename AC>
    bool operator != (const AC& v)      const { return !Equals(v); }
  template <typename AC>
    bool operator >  (const AC& v)      const { return Compare(v) > 0; }
  template <typename AC>
    bool operator >= (const AC& v)      const { return Compare(v) >= 0; }
  template <typename AC>
    bool operator <  (const AC& v)      const { return Compare(v) < 0; }
  template <typename AC>
    bool operator <= (const AC& v)      const { return Compare(v) <= 0; }
  //............................................................................
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
    return (v._Length > T::_Length) ? false : (o_memcmp(T::Data(), v.Data(), v._Length) == 0);
  }
  bool StartsFromi(const TTSString& v) const {
    return (v._Length > T::_Length) ? false : (o_memcmpi(T::Data(), v.Data(), v._Length) == 0);
  }
  //
  template <typename AC> bool StartsFrom(AC v) const {
    return (T::_Length == 0) ? false : T::Data()[0] == v;
  }
  template <typename AC> bool StartsFromi(AC v) const {
    return (T::_Length == 0) ? false : o_tolower(T::Data()[0]) == o_tolower(v);
  }
  //............................................................................
  template <typename AC> size_t LeadingCharCount(AC v) const {
    size_t cc = 0;
    while( cc < T::_Length && T::CharAt(cc) == v )  cc++;
    return cc;
  }
  //............................................................................
  template <typename AC> bool EndsWith(const AC* v) const {
    size_t len = o_strlen(v);
    return (len > T::_Length) ? false : (o_memcmp(&T::Data()[T::_Length-len], v, len) == 0);
  }
  template <typename AC> bool EndsWithi(const AC* v) const {
    size_t len = o_strlen(v);
    return (len > T::_Length) ? false : (o_memcmpi(&T::Data()[T::_Length-len], v, len) == 0);
  }
  //
  bool EndsWith(const TTSString& v) const {
    return (v._Length > T::_Length) ? false : (o_memcmp(&T::Data()[T::_Length-v._Length], v.Data(), v._Length) == 0);
  }
  bool EndsWithi(const TTSString& v) const {
    return (v._Length > T::_Length) ? false : (o_memcmpi(&T::Data()[T::_Length-v._Length], v.Data(), v._Length) == 0);
  }
  //
  template <typename AC> bool EndsWith(AC ch) const {
    return T::_Length == 0 ? false : T::Data()[T::_Length-1] == ch;
  }
  template <typename AC> bool EndsWithi(AC ch) const {
    return T::_Length == 0 ? false : o_tolower(T::Data()[T::_Length-1]) == o_tolower(ch);
  }
  //............................................................................
  //............................................................................
  TTSString operator + (const TC* v) const { return (TTSString<T,TC>&)TTSString<T,TC>(*this).Append(v, o_strlen(v)); }
  //............................................................................
  TTSString operator + (TC v) const { return (TTSString<T,TC>(*this) << v); }
  //............................................................................
  TTSString operator + (const TTSString& v) const { return (TTSString<T,TC>(*this) << v); }
  //............................................................................
  template <typename OC, typename AC>
  static size_t o_strpos(const OC* whr, size_t whr_len, const AC* wht, size_t wht_len) {
    for( size_t i=0; i < whr_len; i++ )  {
      if( i+wht_len > whr_len )  return InvalidIndex;
      bool found = true;
      for( size_t j=0;  j < wht_len; j++ )
        if( whr[i+j] != wht[j] )  {
          found = false;
          break;
        }
      if( found ) return i;
    }
    return InvalidIndex;
  }
  //............................................................................
  template <typename OC, typename AC>
  static bool o_issubstr(const OC* whr, size_t whr_len, size_t pos, const AC* wht, size_t wht_len) {
    if( pos + wht_len >= whr_len )  return false;
    for( size_t i=0; i < wht_len; i++ )
      if( whr[pos+i] != wht[i] )
        return false;
    return true;
  }
  //............................................................................
  template <typename OC, typename AC>
  static size_t o_chrpos(const OC* whr, size_t whr_len, AC wht) {
    for( size_t i=0; i < whr_len; i++ )
      if( whr[i] == wht )
        return i;
    return InvalidIndex;
  }
  //............................................................................
  template <typename OC, typename AC>
  static size_t o_strposi(const OC* whr, size_t whr_len, const AC* wht, size_t wht_len) {
    for( size_t i=0; i < whr_len; i++ )  {
      if( i+wht_len > whr_len )  return InvalidIndex;
      bool found = true;
      for( size_t j=0;  j < wht_len; j++ )
        if( o_toupper(whr[i+j]) != o_toupper(wht[j]) )  {
          found = false;
          break;
        }
      if( found ) return i;
    }
    return InvalidIndex;
  }
  //............................................................................
  template <typename OC, typename AC>
  static bool o_issubstri(const OC* whr, size_t whr_len, size_t pos, const AC* wht, size_t wht_len) {
    if( pos + wht_len >= whr_len )  return false;
    for( size_t i=0; i < wht_len; i++ )
      if( o_toupper(whr[pos+i]) != o_toupper(wht[i]) )
        return false;
    return true;
  }
  //............................................................................
  template <typename OC, typename AC>
  static size_t o_chrposi(const OC* whr, size_t whr_len, AC wht) {
    wht = o_toupper(wht);
    for( size_t i=0; i < whr_len; i++ )
      if( o_toupper(whr[i]) == wht )
        return i;
    return InvalidIndex;
  }
  //............................................................................
  template <typename OC, typename AC>
  static size_t o_strposr(const OC* whr, size_t whr_len, const AC* wht, size_t wht_len) {
    if( wht_len > whr_len || whr_len == 0 || wht_len == 0)  return InvalidIndex;
    for( size_t i=whr_len-1; i != ~0; i-- )  {
      if( i < wht_len )  return InvalidIndex;
      bool found = true;
      for( size_t j=wht_len-1;  j != ~0; j-- )
        if( whr[i-j] != wht[j] )  {
          found = false;
          break;
        }
      if( found ) return i-1;
    }
    return InvalidIndex;
  }
  //............................................................................
  template <typename OC, typename AC>
  static size_t o_chrposr(const OC* whr, size_t whr_len, AC wht) {
    if( whr_len == 0 )  return InvalidIndex;
    for( size_t i=whr_len-1; i != ~0; i-- )
      if( whr[i] == wht )
        return i;
    return InvalidIndex;
  }
  //............................................................................
  template <typename OC, typename AC>
  static size_t o_strposri(const OC* whr, size_t whr_len, const AC* wht, size_t wht_len) {
    if( wht_len > whr_len || whr_len == 0 || wht_len == 0 )  return InvalidIndex;
    for( size_t i=whr_len-1; i != ~0; i-- )  {
      if( i < wht_len )  return InvalidIndex;
      bool found = true;
      for( size_t j=wht_len-1;  j != ~0 ; j-- )
        if( o_toupper(whr[i-j]) != o_toupper(wht[j]) )  {
          found = false;
          break;
        }
      if( found ) return i;
    }
    return InvalidIndex;
  }
  //............................................................................
  template <typename OC, typename AC>
  static size_t o_chrposri(const OC* whr, size_t whr_len, AC wht) {
    if( whr_len == 0 )  return InvalidIndex;
    wht = o_toupper(wht);
    for( size_t i=whr_len-1; i != ~0; i-- )
      if( o_toupper(whr[i]) == wht )
        return i;
    return InvalidIndex;
  }
  //............................................................................
  size_t IndexOf(const TTSString& wht)  const { return o_strpos(T::Data(), T::_Length, wht.Data(), wht._Length);  }
  size_t IndexOfi(const TTSString& wht) const { return o_strposi(T::Data(), T::_Length, wht.Data(), wht._Length);  }
  size_t FirstIndexOf(const TTSString& wht, size_t from = 0) const {
    size_t i = o_strpos(&T::Data()[from], T::_Length-from, wht.Data(), wht._Length);
    return ((i==InvalidIndex) ? InvalidIndex : (i + from));
  }
  size_t FirstIndexOfi(const TTSString& wht, size_t from = 0) const {
    size_t i = o_strposi(&T::Data()[from], T::_Length-from, wht.Data(), wht._Length);
    return ((i==InvalidIndex) ? InvalidIndex : (i + from));
  }
  bool IsSubStringAt(const TTSString& wht, size_t pos)   const {
    return o_issubstr(T::Data(), T::_Length, pos, wht.Data(), wht._Length);
  }
  bool IsSubStringAti(const TTSString& wht, size_t pos)   const {
    return o_issubstri(T::Data(), T::_Length, pos, wht.Data(), wht._Length);
  }
  size_t LastIndexOf(const TTSString& wht, size_t from = InvalidIndex)  const {
    return o_strposr(T::Data(), olx_min(from, T::_Length), wht.Data(), wht._Length);
  }
  size_t LastIndexOfi(const TTSString& wht, size_t from = InvalidIndex) const {
    return o_strposri(T::Data(), olx_min(from, T::_Length), wht.Data(), wht._Length);
  }
#ifdef __BORLANDC__
  size_t IndexOf(const TC* wht) const {  return o_strpos( T::Data(), T::_Length, wht, o_strlen(wht));  }
  size_t IndexOfi(const TC* wht) const {  return o_strposi( T::Data(), T::_Length, wht, o_strlen(wht));  }
  size_t IndexOf(TC wht) const { return o_chrpos( T::Data(), T::_Length, wht);  }
  size_t IndexOfi(TC wht) const { return o_chrposi( T::Data(), T::_Length, wht);  }
  size_t FirstIndexOf(const TC* wht, size_t from = 0) const {
    size_t i = o_strpos(&T::Data()[from], T::_Length-from, wht, o_strlen(wht));
    return ((i==InvalidIndex) ? InvalidIndex : (i + from));
  }
  size_t FirstIndexOfi(const TC* wht, size_t from = 0) const {
    size_t i = o_strposi(&T::Data()[from], T::_Length-from, wht, o_strlen(wht));
    return ((i==InvalidIndex) ? InvalidIndex : (i + from));
  }
  size_t FirstIndexOf(TC wht, size_t from = 0) const {
    size_t i = o_chrpos(&T::Data()[from], T::_Length-from, wht);
    return ((i==InvalidIndex) ? InvalidIndex : (i + from));
  }
  size_t FirstIndexOfi(TC wht, size_t from = 0) const {
    size_t i = o_chrposi(&T::Data()[from], T::_Length-from, wht);
    return ((i==InvalidIndex) ? InvalidIndex : (i + from));
  }
  size_t IsSubStringAt(const TC* wht, size_t pos) const {
    return o_issubstr(T::Data(), T::_Length, pos, wht, o_strlen(wht) );
  }
  size_t IsSubStringAti(const TC* wht, size_t pos) const {
    return o_issubstri(T::Data(), T::_Length, pos, wht, o_strlen(wht) );
  }
  size_t LastIndexOf(const TC* wht, size_t from = InvalidIndex) const {
    return o_strposr(T::Data(), olx_min(from, T::_Length), wht, o_strlen(wht));
  }
  size_t LastIndexOfi(const TC* wht, size_t from = InvalidIndex) const {
    return o_strposri(T::Data(), olx_min(from, T::_Length), wht, o_strlen(wht));
  }
  size_t LastIndexOf(TC wht, size_t from = InvalidIndex) const {
    return o_chrposr(T::Data(), olx_min(from, T::_Length), wht);
  }
  size_t LastIndexOfi(TC wht, size_t from = InvalidIndex) const {
    return o_chrposri(T::Data(), olx_min(from, T::_Length), wht);
  }
#else
  template <typename AC> size_t IndexOf(const AC* wht) const {
    return o_strpos( T::Data(), T::_Length, wht, o_strlen(wht));
  }
  template <typename AC> size_t IndexOfi(const AC* wht) const {
    return o_strposi(T::Data(), T::_Length, wht, o_strlen(wht));
  }
  template <typename AC> size_t IndexOf(AC wht) const { return o_chrpos( T::Data(), T::_Length, wht);  }
  template <typename AC> size_t IndexOfi(AC wht) const { return o_chrposi( T::Data(), T::_Length, wht);  }

  template <typename AC> size_t FirstIndexOf(const AC* wht, size_t from = 0) const {
    size_t i = o_strpos(&T::Data()[from], T::_Length-from, wht, o_strlen(wht));
    return ((i==InvalidIndex) ? InvalidIndex : (i + from));
  }
  template <typename AC> size_t FirstIndexOfi(const AC* wht, size_t from = 0) const {
    size_t i = o_strposi(&T::Data()[from], T::_Length-from, wht, o_strlen(wht));
    return ((i==InvalidIndex) ? InvalidIndex : (i + from));
  }
  template <typename AC> size_t FirstIndexOf(AC wht, size_t from = 0) const {
    size_t i = o_chrpos(&T::Data()[from], T::_Length-from, wht);
    return ((i==InvalidIndex) ? InvalidIndex : (i + from));
  }
  template <typename AC> size_t FirstIndexOfi(AC wht, size_t from = 0) const {
    size_t i = o_chrposi(&T::Data()[from], T::_Length-from, wht);
    return ((i==InvalidIndex) ? InvalidIndex : (i + from));
  }

  template <typename AC> bool IsSubStringAt(const AC* wht, size_t pos) const {
    return o_issubstr(T::Data(), T::_Length, pos, wht, o_strlen(wht) );
  }
  template <typename AC> bool IsSubStringAti(const AC* wht, size_t pos) const {
    return o_issubstri(T::Data(), T::_Length, pos, wht, o_strlen(wht) );
  }

  template <typename AC> size_t LastIndexOf(const AC* wht, size_t from = InvalidIndex) const {
    return o_strposr(T::Data(), olx_min(from, T::_Length), wht, o_strlen(wht));
  }
  template <typename AC> size_t LastIndexOfi(const AC* wht, size_t from = InvalidIndex) const {
    return o_strposri(T::Data(), olx_min(from, T::_Length), wht, o_strlen(wht));
  }
  template <typename AC> size_t LastIndexOf(AC wht, size_t from = InvalidIndex) const {
    return o_chrposr(T::Data(), olx_min(from, T::_Length), wht);
  }
  template <typename AC> size_t LastIndexOfi(AC wht, size_t from = InvalidIndex) const {
    return o_chrposri(T::Data(), olx_min(from, T::_Length), wht);
  }
#endif // __BORLANDC__
  //............................................................................
  // function checks for preceding radix encoding
  template <typename IT> static IT o_atois(const TC* data, size_t len, bool& negative, unsigned short Rad=10) {
    if( len == 0 )    
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "invalid integer format");
    size_t sts = 0; // string start, end
    while( o_iswhitechar(data[sts]) && ++sts < len )
    if( sts >= len )  
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "invalid integer format");
    // test for any particluar format specifier, here just '0x', for hexadecimal
    if( len > sts+1 && data[sts] == '0' && (data[sts+1] == 'x' || data[sts+1] == 'X') )  {
      Rad = 16;
      sts += 2;
    }
    else if( data[sts] == 'o' || data[sts] == 'O' )  {
      Rad = 8;
      sts++;
    }
    return o_atoi_s<IT>(&data[sts], len-sts, negative, Rad);
  }
  //............................................................................
  // function checks for preceding radix encoding
  static bool o_isints(const TC* data, size_t len, bool& negative) {
    if( len == 0 )  return false;
    size_t sts = 0; // string start, end
    while( o_iswhitechar(data[sts]) && ++sts < len )
    if( sts >= len )
      return false;
    // test for any particluar format specifier, here just '0x', for hexadecimal
    unsigned short Rad=10;
    if( data[sts] == '0' )  {
      negative = false;
      if( len == 1 )  return true;
      if( data[sts+1] == 'x' || data[sts+1] == 'X' )  {
        if( len == 2 )  return false;
        Rad = 16;
        sts += 2;
      }
      else if( data[sts] == '0' )  {
        Rad = 8;
        sts++;
      }
    }
    return o_isint_s(&data[sts], len-sts, negative, Rad);
  }
  //............................................................................
  template <typename IT> static IT o_atoi_safe(const TC* data, size_t len, unsigned short Rad=10) {
    bool negative;
    IT val = o_atois<IT>(data, len, negative, Rad);
    return negative ? -val : val;
  }
  //............................................................................
  template <typename IT> static IT o_atoui_safe(const TC* data, size_t len, unsigned short Rad=10) {
    bool negative;
    IT val = o_atois<IT>(data, len, negative, Rad);
    if( negative )
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "invalid usigned integer format");
    return val;
  }
  //............................................................................
  // preceding encoding like 0x ir 0 are treated correctly
  template <typename IT> IT SafeInt(unsigned short Rad=10) const {
    return o_atoi_safe<IT>(T::Data(), T::_Length, Rad);
  }
  //............................................................................
  // preceding encoding like 0x ir 0 are treated correctly
  template <typename IT> IT SafeUInt(unsigned short Rad=10) const {
    return o_atoui_safe<IT>(T::Data(), T::_Length, Rad);
  }
  //............................................................................
  // no '\0' at the end, got to do it ourselves, returns the value without applying chsig
  template <typename IT> static IT o_atoi_s(const TC* data, size_t len, bool& negative, unsigned short Rad=10) {
    if( len == 0 )    
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "invalid integer format");
    size_t sts = 0, ste = len; // string start, end
    while( o_iswhitechar(data[sts]) && ++sts < len ) ;
    while( --ste > sts && o_iswhitechar(data[ste]) ) ;
    if( ++ste <= sts )
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "invalid integer format");
    IT val=0;
    negative = false;
    if( data[sts] == '-' )  {  
      negative = true;  
      sts++;  
    }
    else if( data[sts] == '+' )
      sts++;
    if( sts == ste )  
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "invalid integer format");
    if( Rad > 10 )  {
      for( size_t i=sts; i < ste; i++ )  {
        short pv = 0;
        if( data[i] <= '9' && data[i] >= '0' )  // the order is important, chars are rearer
          pv = data[i] - '0';
        else if( data[i] <= 'Z' && data[i] >= 'A' )
          pv = data[i] - 'A' + 10;
        else if( data[i] <= 'z' && data[i] >= 'a' )
          pv = data[i] - 'a' + 10;
        else  // throw
          TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "invalid digit");
        if( pv >= Rad )
          TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "digit out of range");
        val = val*Rad + pv;
       }
     }
     else  {
       for( size_t i=sts; i < len; i++ )  {
         if( data[i] <= '9' && data[i] >= '0' )  {
           const short pv = data[i] - '0';
            if( pv >= Rad )
              TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "digit out of range");
           val = val*Rad + pv;
         }
         else  // throw
           TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "invalid digit");
       }
     }
     return val;
  }
  //............................................................................
  // no '\0' at the end, got to do it ourselves, returns the value without applying chsig
  static bool o_isint_s(const TC* data, size_t len, bool& negative, unsigned short Rad=10) {
    if( len == 0 )  return false;
    size_t sts = 0, ste = len; // string start, end
    while( o_iswhitechar(data[sts]) && ++sts < len ) ;
    while( --ste > sts && o_iswhitechar(data[ste]) ) ;
    if( ++ste <= sts )
      return false;
    negative = false;
    if( data[sts] == '-' )  {  
      negative = true;  
      sts++;  
    }
    else if( data[sts] == '+' )
      sts++;
    if( sts == ste )  return false;
    for( size_t i=sts; i < ste; i++ )  {
      short pv = 0;
      if( data[i] <= '9' && data[i] >= '0' )  pv = data[i] - '0';
      else if( data[i] <= 'Z' && data[i] >= 'A' )  pv = data[i] - 'A' + 10;
      else if( data[i] <= 'z' && data[i] >= 'a' )  pv = data[i] - 'a' + 10;
      else  return false;
      if( pv >= Rad )
        return false;
    }
    return true;
  }
  //............................................................................
  template <typename IT> static IT o_atoi(const TC* data, size_t len, unsigned short Rad=10) {
    bool negative;
    IT val = o_atoi_s<IT>(data, len, negative, Rad);
    return negative ? -val : val;
  }
  //............................................................................
  static bool o_isint(const TC* data, size_t len) {
    bool negative;
    return o_isints(data, len, negative);
  }
  //............................................................................
  template <typename IT> static IT o_atoui(const TC* data, size_t len, unsigned short Rad=10) {
    bool negative;
    IT val = o_atoi_s<IT>(data, len, negative, Rad);
    if( negative )
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "invalid unsigned integer format");
    return val;
  }
  //............................................................................
  static bool o_isuint(const TC* data, size_t len) {
    bool negative;
    bool v = o_isints(data, len, negative);
    return v && !negative;
  }
  //............................................................................
  template <typename IT> IT RadInt(unsigned short Rad=10) const  {
     return o_atoi<IT>(T::Data(), T::_Length, Rad);
  }
  //............................................................................
  template <typename IT> IT RadUInt(unsigned short Rad=10) const  {
     return o_atoui<IT>(T::Data(), T::_Length, Rad);
  }
  //............................................................................
  int ToInt() const {  return o_atoi<int>(T::Data(), T::_Length, 10);  }
  bool IsInt() const {  return o_isint(T::Data(), T::_Length);  }
  //............................................................................
  unsigned int ToUInt() const {  return o_atoui<unsigned int>( T::Data(), T::_Length, 10);  }
  bool IsUInt() const {  return o_isuint(T::Data(), T::_Length);  }
  //............................................................................
  size_t ToSizeT() const {  return o_atoui<size_t>( T::Data(), T::_Length, 10);  }
  //............................................................................
  bool ToBool() const {  return (Comparei(TrueString) == 0);  }
  //............................................................................
  // no '\0' at the end, got to do it ourselves
  template <class FT> static FT o_atof(const TC* data, size_t len) {
    if( len == 0 )  
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "invalid float number format");
    size_t sts = 0, ste = len; // string start, end
    while( o_iswhitechar(data[sts]) && ++sts < len ) ;
    while( --ste > sts && o_iswhitechar(data[ste]) ) ;
    if( ++ste <= sts )
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "invalid float number format");
    bool negative = false;
    if( data[sts] == '-' )  {  negative = true;  sts++;  }
    else if( data[sts] == '+' )  sts++;
    if( sts == ste )
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "invalid float number format");
    FT bp=0, ap=0, apexp=1;
    size_t exp = 0;
    bool fpfound = false, expfound = false, expneg = false;
    for( size_t i=sts; i < ste; i++ )  {
      if( data[i] <= '9' && data[i] >= '0' )
        if( expfound )
          exp = exp*10 + (data[i] - '0');
        else if( fpfound  )  {
          ap = ap*10 + (data[i] - '0');
          apexp *= 10;
        }
        else
          bp = bp*10 + (data[i] - '0');
      else if( data[i] == '.' )
        fpfound = true;
      else if( data[i] == 'e' || data[i] == 'E' )  {
        expfound = true;
        if( ++i == len )  break;
        if( data[i] == '-' )
          expneg = true;
        else if( data[i] == '+' )
          ;
        else if( data[i] >= '0' && data[i] <= '9' )  // anonymous positive exp
         i--;
        else  // invalid dddd.ddddE-/+/ddd format
          TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "invalid digit in exponent");
      }
      else  // invalid char for a number
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "invalid float number digit");
    }
    bp = (expneg) ? (bp + ap/apexp)/olx_pow10<FT>(exp) : (bp + ap/apexp)*olx_pow10<FT>(exp);
    return negative ? -bp : bp;
  }
  //............................................................................
  double ToDouble() const {  return o_atof<double>(T::Data(), T::_Length);  }
  //............................................................................
  template <typename FT> FT ToFloat() const {  return o_atof<FT>(T::Data(), T::_Length);  }
  //............................................................................
  void SetLength(size_t newLen)  {
    if( newLen < T::_Length )
      DecLength(T::_Length - newLen);
    else  {
      T::checkBufferForModification(newLen);
      T::_Length = newLen;
    }
  }
  //............................................................................
  TTSString& Delete(size_t from, size_t count)  {
    register size_t dv = from+count;
    if( dv > T::_Length )  
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "invalid size to delete");
    else  {
      if( dv == T::_Length )  { 
        if( from != 0 )  T::_Length -= count;  // substring to
        else            T::_Length = 0;  // empty string ...
        return *this;
      }
    }
    // delete from start - just substring from
    if( from == 0 )  {  T::_Start += count;  T::_Length -= count;  return *this;  }
    
    T::checkBufferForModification(T::_Length);
    memmove( &T::Data()[from], &T::Data()[from+count], (T::_Length-from-count)*T::CharSize);
    T::DecLength(count);
    return *this;
  }
  //............................................................................
  //replaces sequences of chars with a single char
  template <typename AC> static size_t o_strdcs(TC* whr, size_t whr_len, AC wht)  {
    size_t ni = 0;
    for( size_t i=0; i < whr_len; i++, ni++ )  {
      if( whr[i] == wht && ((i+1) < whr_len && whr[i+1] == wht) )  {
        ni--;
        continue;
      }
      else if( ni != i )  whr[ni] = whr[i];
    }
    return whr_len - ni;
  }
  //............................................................................
  template <typename OC> TTSString& DeleteSequencesOf(OC wht)  {
    T::checkBufferForModification(T::_Length);
    DecLength( o_strdcs(T::Data(), T::_Length, wht) );
    return *this;
  }
  //............................................................................
  template <typename AC> static TTSString DeleteSequencesOf(const TTSString& str, AC wht)  {
    return TTSString<T,TC>(str).DeleteSequencesOf(wht);
  }
  //............................................................................
  //removes chars from string and return the number of removed chars
  template <typename AC> static size_t o_strdch(TC* whr, size_t whr_len, AC wht)  {
    size_t rn = 0;
    for( size_t i=0; i < whr_len; i++ )  {
      if( whr[i] == wht )  {
        rn++;
        continue;
      }
      else
        whr[i-rn] = whr[i];
    }
    return rn;
  }
  //............................................................................
  //removes a set of chars from string and return the number of removed chars
  template <typename AC> static size_t o_strdchs(TC* whr, size_t whr_len, const AC* wht, size_t wht_len)  {
    size_t rn = 0;
    for( size_t i=0; i < whr_len; i++ )  {
      bool found = false;
      for( size_t j=0; j < wht_len; j++ )  {
        if( whr[i] == wht[j] )  {
          rn++;
          found = true;
          break;
        }
      }
      if( !found )
        whr[i-rn] = whr[i];
    }
    return rn;
  }
  //............................................................................
  template <typename AC> TTSString& DeleteChars(AC wht)  {
    T::checkBufferForModification(T::_Length);
    T::DecLength( o_strdch(T::Data(), T::_Length, wht) );
    return *this;
  }
  //............................................................................
  // deletes a set of chars
  TTSString& DeleteCharSet(const TTSString& wht)  {
    T::checkBufferForModification(T::_Length);
    T::DecLength( o_strdchs(T::Data(), T::_Length, wht.raw_str(), wht.Length()) );
    return *this;
  }
  //............................................................................
  template <typename AC> static TTSString DeleteChars(const TTSString& str, AC wht)  {
    return TTSString<T,TC>(str).DeleteChars(wht);
  }
  //............................................................................
  template <typename AC> static TTSString DeleteCharSet(const TTSString& str, const TTSString& wht)  {
    return TTSString<T,TC>(str).DeleteCharSet(wht);
  }
  //............................................................................
  static size_t o_strins(const TC* wht, size_t wht_len, TC* to, size_t to_len, size_t at, size_t amount=1)  {
    size_t toshift = wht_len*amount;
    if( at < to_len )
      memmove(&to[at+toshift], &to[at], (to_len - at)*T::CharSize);
    for( size_t i=0; i < amount; i++ )
      memcpy(&to[at+wht_len*i], wht, wht_len*T::CharSize);
    return toshift;
  }
  //............................................................................
  static size_t o_chrins(TC wht, TC* to, size_t to_len, size_t at, size_t amount=1)  {
    if( at < to_len )
      memmove(&to[at+amount], &to[at], (to_len - at)*T::CharSize);
    //memset(to, wht, CharSize*amount);
    for( size_t i=0; i < amount; i++ )
      to[at+i] = wht;
    return amount;
  }
  //............................................................................
  TTSString& Insert(const TTSString& wht, size_t whr, size_t amount=1)  {
    if( whr > T::_Length )  
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "index out of range");
    T::checkBufferForModification(T::_Length + wht._Length*amount);
    T::IncLength( o_strins(wht.Data(), wht._Length, T::Data(), T::_Length, whr, amount) );
    return *this;
  }
  //............................................................................
  TTSString& Insert(const TC* wht, size_t whr, size_t amount=1)  {
    if( whr > T::_Length )  
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "index out of range");
    size_t wht_len = o_strlen(wht);
    T::checkBufferForModification(T::_Length + wht_len*amount);
    T::IncLength( o_strins(wht, wht_len, T::Data(), T::_Length, whr, amount) );
    return *this;
  }
  //............................................................................
  TTSString& Insert(TC wht, size_t whr, size_t amount=1)  {
    if( whr > T::_Length )  
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "index out of range");
    T::checkBufferForModification(T::_Length + amount);
    T::IncLength( o_chrins(wht, T::Data(), T::_Length, whr, amount) );
    return *this;
  }
  //............................................................................
  // counts number of occurences of chars
  template <typename OC>
  static size_t o_chrcnt(OC wht, const TC* whr, size_t whr_len)  {
    size_t cnt = 0;
    for( size_t i=0; i < whr_len; i++ )  {
      if( whr[i] == wht )
        cnt++;
    }
    return cnt;
  }
  //............................................................................
  template <typename AC> size_t CharCount(AC wht) const {  return o_chrcnt(wht, T::Data(), T::_Length);  }
  //............................................................................
  template <typename OC>
  static size_t o_strcnt(const OC* wht, size_t wht_len, const TC* whr, size_t whr_len)  {
    if( wht_len > whr_len )  return 0;
    size_t cnt = 0;
    for( size_t i=0; i < whr_len; i++ )  {
      if( i+wht_len > whr_len )  return cnt;
      bool found = true;
      for( size_t j=0;  j < wht_len; j++ )
        if( whr[i+j] != wht[j] )  {
          found = false;
          break;
        }
      if( found ) cnt++;
    }
    return cnt;
  }
  // replaces a string with another, returns number of replacements
  template <typename OC, typename AC>
  static size_t o_strrplstr(const OC* wht, const size_t wht_len,
                         const AC* with, const size_t with_len,
                         TC* whr, size_t whr_len)  {
    if( wht_len > whr_len )  return 0;
    size_t cnt = 0;
    for( size_t i=0; i < whr_len; i++ )  {                              
      if( i+wht_len > whr_len )  return cnt;
      bool found = true;
      for( size_t j=0;  j < wht_len; j++ )  {
        if( whr[i+j] != wht[j] )  {
          found = false;
          break;
        }
      }
      if( found )  {
        if( with_len != wht_len )  {
          memmove(&whr[i+with_len], &whr[i+wht_len], (whr_len-i-wht_len)*T::CharSize);
          whr_len -= (wht_len-with_len);
        }
        for( size_t j=0;  j < with_len; j++ )
          whr[i+j] = with[j];
        cnt++;
        i += (with_len-1);
      }
    }
    return cnt;
  }
  //............................................................................
  template <typename OC, typename AC>
  static size_t o_strrplch(const OC* wht, const size_t wht_len,
                         AC with,
                         TC* whr, size_t whr_len)  {
    if( wht_len > whr_len )  return 0;
    size_t cnt = 0;
    for( size_t i=0; i < whr_len; i++ )  {
      if( i+wht_len > whr_len )  return cnt;
      bool found = true;
      for( size_t j=0;  j < wht_len; j++ )  {
        if( whr[i+j] != wht[j] )  {
          found = false;
          break;
        }
      }
      if( found )  {
        if( wht_len != 1 )  {
          memmove(&whr[i+1], &whr[i+wht_len], (whr_len-i-wht_len)*T::CharSize);
          whr_len -= (wht_len-1);
        }
        whr[i] = with;
        cnt++;
      }
    }
    return cnt;
  }
  //............................................................................
  template <typename OC, typename AC>
  static size_t o_chrplstr(OC wht,
                         const AC* with, const size_t with_len,
                         TC* whr, size_t whr_len)  {
    size_t cnt = 0;
    for( size_t i=0; i < whr_len; i++ )  {
      if( whr[i] == wht )  {
        if( with_len != 1 )  {
          memmove(&whr[i+with_len], &whr[i+1], (whr_len-i-1)*T::CharSize);
          whr_len -= (1-with_len);
        }
        for( size_t j=0;  j < with_len; j++ )
          whr[i+j] = with[j];
        cnt++;
        i += (with_len-1);
      }
    }
    return cnt;
  }
  //............................................................................
  template <typename OC, typename AC> // replace chars
  static size_t o_chrplch(OC wht,  AC with,
                         TC* whr, size_t whr_len)  {
    size_t cnt = 0;
    for( size_t i=0; i < whr_len; i++ )  {
      if( whr[i] == wht )  {
          whr[i] = with;
        cnt++;
      }
    }
    return cnt;
  }
  //............................................................................
  TTSString& Replace(const TTSString& wht, const TTSString& with)  {
    size_t extra_len = 0;
    if( wht._Length < with._Length )  {
      extra_len = (with._Length - wht._Length) * o_strcnt(wht.Data(), wht._Length, T::Data(), T::_Length);
      if( extra_len == 0 )  return *this;
    }
    T::checkBufferForModification(T::_Length+extra_len);
    size_t rv = o_strrplstr(wht.Data(), wht._Length, with.raw_str(), with._Length, T::Data(), T::_Length);
    T::_Length -= (wht._Length - with._Length)*rv;
    return *this;
  }
#ifndef __BORLANDC__ // what use of templates when this does not get them???
  //............................................................................
  template <typename AC> TTSString& Replace(const TTSString& wht, const AC* with)  {
    size_t extra_len = 0, with_len = o_length(with);
    if( wht._Length < with_len )  {
      extra_len = (with_len - wht._Length) * o_strcnt(wht.Data(), wht._Length, T::Data(), T::_Length);
      if( extra_len == 0 )  return *this;
    }
    T::checkBufferForModification(T::_Length+extra_len);
    size_t rv = o_strrplstr(wht.Data(), wht._Length, with, o_length(with), T::Data(), T::_Length);
    T::_Length -= (wht._Length - with_len)*rv;
    return *this;
  }
  //............................................................................
  template <typename AC> TTSString& Replace(const TTSString& wht, AC with)  {
    T::checkBufferForModification(T::_Length);
    size_t rv = o_strrplch(wht.Data(), wht._Length, with, T::Data(), T::_Length);
    T::_Length -= (wht._Length - 1)*rv;
    return *this;
  }
  //............................................................................
  template <typename OC, typename AC> TTSString& Replace(const OC* wht, const AC* with)  {
    size_t extra_len = 0, with_len = o_strlen(with), wht_len = o_strlen(wht);
    if( wht_len < with_len )  {
      extra_len = (with_len - wht_len) * o_strcnt(wht, wht_len, T::Data(), T::_Length);
      if( extra_len == 0 )  return *this;
    }
    T::checkBufferForModification(T::_Length+extra_len);
    size_t rv = o_strrplstr(wht, o_strlen(wht), with, o_strlen(with), T::Data(), T::_Length);
    T::_Length -= (wht_len - with_len)*rv;
    return *this;
  }
  //............................................................................
  template <typename OC, typename AC> TTSString& Replace(const OC* wht, AC with)  {
    size_t wht_len = o_strlen(wht);
    T::checkBufferForModification(T::_Length);
    size_t rv = o_strrplch(wht, wht_len, with, T::Data(), T::_Length);
    T::_Length -= (wht_len - 1)*rv;
    return *this;
  }
  //............................................................................
  template <typename OC> TTSString& Replace(const OC* wht, const TTSString& with)  {
    size_t extra_len = 0, wht_len = o_strlen(wht);
    if( wht_len < with._Length )  {
      extra_len = (with._Length - wht_len) * o_strcnt(wht, wht_len, T::Data(), T::_Length);
      if( extra_len == 0 )  return *this;
    }
    T::checkBufferForModification(T::_Length+extra_len);
    size_t rv = o_strrplstr(wht, o_strlen(wht), with.raw_str(), with._Length, T::Data(), T::_Length);
    T::_Length -= (wht_len - with._Length)*rv;
    return *this;
  }
  //............................................................................
  template <typename OC> TTSString& Replace(OC wht, const TTSString& with)  {
    size_t extra_len = 0;
    if( with._Length > 1 )  {
      extra_len = (with._Length - 1) * o_chrcnt(wht, T::Data(), T::_Length);
      if( extra_len == 0 )  return *this;
    }
    T::checkBufferForModification(T::_Length+extra_len);
    size_t rv = o_chrplstr(wht, with.Data(), with._Length, T::Data(), T::_Length);
    T::_Length -= (1 - with._Length)*rv;
    return *this;
  }
  //............................................................................
  template <typename OC, typename AC> TTSString& Replace(OC wht, const AC* with)  {
    size_t extra_len = 0, with_len = o_strlen(with);
    if( with_len > 1 )  {
      extra_len = (with_len - 1) * o_chrcnt(wht, T::Data(), T::_Length);
      if( extra_len == 0 )  return *this;
    }
    T::checkBufferForModification(T::_Length+extra_len);
    size_t rv = o_chrplstr(wht, with, o_strlen(with), T::Data(), T::_Length);
    T::_Length -= (1 - with_len)*rv;
    return *this;
  }
  //............................................................................
  template <typename OC, typename AC> TTSString& Replace(OC wht, AC with)  {
    T::checkBufferForModification(T::_Length);
    o_chrplch(wht, with, T::Data(), T::_Length);
    return *this;
  }
#else  // what else to do... dummy compiler, just 4 functions will do (str,str), (ch,str), (str,ch) and (ch,ch)
  TTSString& Replace(olxch wht, const TTSString& with)  {
    size_t extra_len = 0;
    if( with._Length > 1 )  {
      extra_len = (with._Length - 1) * o_chrcnt(wht, T::Data(), T::_Length);
      if( extra_len == 0 )  return *this;
    }
    T::checkBufferForModification(T::_Length+extra_len);
    size_t rv = o_chrplstr(wht, with.Data(), with._Length, T::Data(), T::_Length);
    T::_Length -= (1 - with._Length)*rv;
    return *this;
  }
  //............................................................................
  TTSString& Replace(const TTSString& wht, olxch with)  {
    T::checkBufferForModification(T::_Length);
    size_t rv = o_strrplch(wht.Data(), wht._Length, with, T::Data(), T::_Length);
    T::_Length -= (wht._Length - 1)*rv;
    return *this;
  }
  //............................................................................
  TTSString& Replace(olxch wht, olxch with)  {
    T::checkBufferForModification(T::_Length);
    o_chrplch(wht, with, T::Data(), T::_Length);
    return *this;
  }
#endif // __BORLANDC__
  //............................................................................
  template <typename AC> TTSString& Trim(AC wht)  {
    if( T::_Length == 0 )  return *this;
    size_t start = 0, end = T::_Length;
    while( TTIString<TC>::Data(start) == wht && ++start < end )  ;
    while( --end > start && TTIString<TC>::Data(end) == wht )  ;
    T::_Start += start;
    T::_Length = (end + 1 - start);
    return *this;
  }
  //............................................................................
  bool IsWhiteCharString() const {
    const TC* data = T::Data();
    for( size_t i=0; i < T::_Length; i++ )
      if( !o_iswhitechar(data[i]) )
        return false;
    return true;
  }
  //............................................................................
  TTSString& TrimWhiteChars(bool leading=true, bool trailing=true)  {
    if( T::_Length == 0 )  return *this;
    size_t start = 0, end = T::_Length-1;
    if( leading )
      while( o_iswhitechar(TTIString<TC>::Data(start)) && ++start < end )  continue;
    if( trailing )
      while( end > start && o_iswhitechar(TTIString<TC>::Data(end)) )  end--;
    T::_Start += start;
    T::_Length = (end + 1 - start);
    return *this;
  }
  //............................................................................
  TTSString& TrimFloat() {
    T::TrimFloat();
    return *this;
  }
  //............................................................................
  TTSString& Format(size_t count, bool Right, const TTSString& sep)  {
    size_t extra = count-((T::_Length > count) ? count-1 : T::_Length)*sep._Length;
    T::checkBufferForModification(T::_Length + extra);
    if( T::_Length > count )  return (*this << sep);
    Insert(sep, (Right) ? T::_Length : 0, count-T::_Length);
    return *this;
  }
  //............................................................................
  TTSString& Format(size_t count, bool Right, char sep)  {
    size_t extra = count-((T::_Length > count) ? count-1 : T::_Length);
    checkBufferForModification(T::_Length + extra);
    if( T::_Length > count )  return (*this << sep);
    Insert(sep, (Right) ? T::_Length : 0, count - T::_Length);
    return *this;
  }
  //............................................................................
  static TTSString FormatFloat(int NumberOfDigits, double v, bool Exponent = false)  {
    TTSString<T,TC> fmt, rv;
    if( NumberOfDigits < 0 )  {
      NumberOfDigits = -NumberOfDigits;
      if( v >= 0 )  fmt = ' ';
    }
    fmt << "%." << NumberOfDigits << ( (Exponent) ? 'e' : 'f') << '\0';
    rv.setTypeValue(fmt.Data(), v);
    return rv;
  }
  //............................................................................
  /* checks if provided string represent an integer or float point number (inc exponental form) */
  static bool o_isnumber(const TC* data, size_t len) {
    if( len == 0 )  return false;
    size_t sts = 0, ste = len; // string start
    while( o_iswhitechar(data[sts]) && ++sts < len ) ;
    while( --ste > sts && o_iswhitechar(data[ste]) ) ;
    if( ++ste <= sts )
      return false;
    if( data[sts] == '0' )  {
      len = ste-sts;
      if( len == 1 )  return true;
    // hex notation
      if( data[sts+1] == 'x' || data[sts+1] == 'X' )  {
        if( len == 2 )  return false;
        for( size_t i=sts+2; i < ste; i++ )  {
          if( !o_ishexdigit(data[i]) )
            return false;
        }
        return true;
      }
      // octal or float (0e-5 0.nnn) then...
      if( data[sts+1] != '.' && !(data[sts+1] == 'e' || data[sts+1] == 'E') )  {
        for( size_t i=sts+1; i < ste; i++ )  {
          if( data[i] < '0' || data[i] > '7' )
            return false;
        }
        return true;
      }
    }
    bool expfound = false, fpfound = false;
    short digit_cnt = 0;
    if( data[sts] == '+' || data[sts] == '-' )
      sts++;
    for( size_t i = sts; i < ste; i++ )  {
      TC ch = data[i];
      if( o_isdigit(ch) )  {
        digit_cnt++;
        continue;
      }
      else if( ch == '.' )  {
        if( fpfound )  
          return false;
        fpfound = true;
      }
      else if( ch == 'e' || ch == 'E' )  {
        if( expfound )  
          return false;
        expfound = true;
        if( ++i == ste )  // exponent cannot be the last char
          return false;
        ch = data[i];
        if( ch == '-' )
          ;
        else if( ch == '+' )
          ;
        else if( o_isdigit(ch) )  { // anonymously positive exp
          digit_cnt++;
          i--;
        }
        else  // invalid dddd.ddddE-/+/ddd format
          return false;
      }
      else
        return false;
    }
    return (digit_cnt != 0);
  }
  /* checks if the string represent and integer (ocatal, decimal or hex) or float point number  (inc exponental form) */
  bool IsNumber() const {  return o_isnumber(T::Data(), T::_Length);  }
  //............................................................................
  void SetIncrement(size_t ni)  {  T::_Increment = ni;  }
  void SetCapacity(size_t newc)  {
    if( T::SData == NULL )  T::SData = new struct T::Buffer(newc+T::_Increment);
    else if( newc > T::GetCapacity() )                
      T::checkBufferForModification(newc);
  }
  //............................................................................
  TTSString& AppendFromStream(IInputStream& ios, size_t len)  {
    T::checkBufferForModification(T::_Length + len+1);
    ios.Read((void*)&T::SData->Data[T::_Start+T::_Length], len*T::CharSize);
    T::_Length += len;
    return *this;
  }
  //............................................................................
  TTSString& FromBinaryStream(IInputStream& ios)  {
    uint32_t code, len, charsize;
    ios.Read(&code, sizeof(uint32_t));
    charsize = ExtractCharSize(code);
    if( T::CharSize != charsize )
      if( charsize != 0 || T::CharSize == 2 )
        TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "incompatible Char size");
    len = ExtractLength(code);
    if( len > (uint32_t)(ios.GetSize() - ios.GetPosition()) )
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "invalid stream content");
    T::_Start = 0;
    T::_Increment = 8;
    T::_Length = len;
    if( T::SData != NULL )  {
      if( T::SData->RefCnt == 1 )  // owed by this object
        T::SData->SetCapacity(T::_Length);
      else  {
        T::SData->RefCnt--;
        T::SData = NULL;
      }
    }
    if( T::SData == NULL )  T::SData = new struct T::Buffer(T::_Length);
    ios.Read((void*)T::SData->Data, T::_Length*T::CharSize);
    return *this;
  }
  //............................................................................
  void ToBinaryStream(IOutputStream& os) const {
    uint32_t len = (uint32_t)(CodeLength(T::CharSize, T::_Length));
    os.Write(&len, sizeof(uint32_t));
    os.Write( (void*)T::Data(), T::_Length*T::CharSize );
  }
  //............................................................................
  static TTSString CharStr(TC ch, size_t count)  {
    TTSString<T,TC> rv(EmptyString, count);
    rv.Insert(ch, 0, count);
    return rv;
  }
  //............................................................................
  olxch Last() const {  
    if( T::_Length == 0 )
      TExceptionBase::ThrowFunctionFailed(__POlxSourceInfo, "empty string");
    return T::SData->Data[T::_Start+T::_Length-1];
  }
  //............................................................................
  // just checks if string contains any chars > 127
  template <class AC> static bool o_needs_converting(const AC* data, size_t len) {
    for( size_t i=0; i < len; i++ )
      if( data[i] > 127 || data[i] < 0 )
        return true;
    return false;
  }
  //............................................................................
#ifdef __BORLANDC__
  typedef TTSString<TCString, char> olxcstr;
  typedef TTSString<TWString, wchar_t> olxwstr;
#endif
  //............................................................................

  bool NeedsConverting() const {  return o_needs_converting(T::Data(), T::_Length);  }
  // converts a wide char string into multibyte string properly (using curent locale)
  static olxcstr WStr2CStr(const wchar_t* wstr, size_t len=~0);
  static olxcstr WStr2CStr(const olxwstr& str);
  // converts a multibyte string into wide char string properly
  static olxwstr CStr2WStr(const char* mbs, size_t len=~0);
  static olxwstr CStr2WStr(const olxcstr& str);
  //............................................................................
  virtual IEObject* Replicate() const {  return new TTSString<T,TC>(*this);  }
  //............................................................................
};

EndEsdlNamespace()

#endif
