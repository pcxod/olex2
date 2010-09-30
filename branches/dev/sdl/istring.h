#ifndef olx_i_string
#define olx_i_string
#include <string.h>
// immutable string
class TIString {
 const char *SData;
 size_t _Length;
public:
  TIString(const char *data) : SData(data)  {
    _Length = strlen(data);
  }
  inline char operator [] (size_t i) const {  return SData[i];  }
  inline size_t Length() const {  return _Length;  }
  inline const char* c_str() const {  return SData; }
  inline const char* Data()  const {  return SData; }
  inline char Data(size_t i) const {  return SData[i]; }
  inline TIString SubStringFrom(size_t from)  {
    return TIString(&SData[from]);
  }
};


template <class T, class TC> class TTSString : public T  {
public:
  TTSString() : T()  {  }
  TTSString(const TTSString& str, size_t start, size_t length) : T(str, start, length)  {  }

  template <class AC> TTSString(AC v) : T(v)  {  }
  //............................................................................
  template <class AC> inline TTSString& operator = (AC v)   { T::operator =(v);  return *this; }
  //............................................................................
  template <class AC> inline TTSString& operator << (AC v)  { T::operator << (v);  return *this;  }
  //............................................................................
  //............................................................................
  inline TTSString SubString(size_t from, size_t count) const {
    return TTSString<T,TC>(*this, from, count);
  }
  inline TTSString SubStringFrom(size_t from, size_t indexFromEnd=0) const {
    return TTSString<T,TC>(*this, from, Length()-from-indexFromEnd);
  }
  inline TTSString SubStringTo(size_t to, size_t indexFromStart=0) const {
    return TTSString<T,TC>(*this, indexFromStart, to-indexFromStart);
  }
  //............................................................................
  static inline char toupper(char ch)  {  return (ch >='a' && ch <= 'z') ? (ch + 'A'-'a') : ch;  }
  static inline char tolower(char ch)  {  return (ch >='A' && ch <= 'Z') ? (ch + 'a'-'A') : ch;  }
  static inline char toupper(wchar_t ch)  {  return towupper(ch);  }
  static inline char tolower(wchar_t ch)  {  return towlower(ch);  }
  //............................................................................
  static int Compare(const TC* what, size_t len_a, const TC *with, size_t len_b, bool CI) {
    if( len_a == len_b )  {
      if( len_a == 0 )  return 0;
      return CI ? strncasecmp(what, with, len_a*sizeof(TC)) : memcmp(what, with, len_a*sizeof(TC));
    }
    if( len_a == 0 )  return -1;
    if( len_b == 0 )  return 1;
    int res = CI ? strncasecmp(what, with, olx_min(len_a, len_b)*sizeof(TC)) :
                   memcmp(what, with, olx_min(len_a, len_b)*sizeof(TC));
    if( res != 0 )  return res;
    if( len_a < len_b )  return -1;
    return 1;
  }
  inline int Compare(const TTSString& v)       const { return Compare(Data(), Length(), v.Data(), v.Length(), false );  }
  inline int CompareCI(const TTSString& v)     const { return Compare(Data(), Length(), v.Data(), v.Length(), true );  }
  inline int Compare(const TC *v)              const { return Compare(Data(), Length(), v, strlen(v), false );  }
  inline int CompareCI(const TC *v)            const { return Compare(Data(), Length(), v, strlen(v), true );  }
  inline bool operator == (const TTSString& v) const { return Compare(Data(), Length(), v.Data(), v.Length(), false ) == 0; }
  inline bool operator == (const TC *v)        const { return Compare(Data(), Length(), v, strlen(v), false ) == 0; }
  inline bool operator != (const TTSString& v) const { return Compare(Data(), Length(), v.Data(), v.Length(), false ) != 0; }
  inline bool operator != (const TC *v)        const { return Compare(Data(), Length(), v, strlen(v), false ) != 0; }
  inline bool operator > (const TC *v)         const { return Compare(Data(), Length(), v, strlen(v), false ) > 0; }
  inline bool operator < (const TC *v)         const { return Compare(Data(), Length(), v, strlen(v), false ) < 0; }
  inline bool operator >= (const TC *v)        const { return Compare(Data(), Length(), v, strlen(v), false ) >= 0; }
  inline bool operator <= (const TC *v)        const { return Compare(Data(), Length(), v, strlen(v), false ) <= 0; }
  inline bool operator > (const TTSString &v)  const { return Compare(Data(), Length(), v.Data(), v.Length(), false ) > 0; }
  inline bool operator < (const TTSString &v)  const { return Compare(Data(), Length(), v.Data(), v.Length(), false ) < 0; }
  inline bool operator >= (const TTSString &v) const { return Compare(Data(), Length(), v.Data(), v.Length(), false ) >= 0; }
  inline bool operator <= (const TTSString &v) const { return Compare(Data(), Length(), v.Data(), v.Length(), false ) <= 0; }
  //............................................................................
  TTSString operator + (const TC *v)           const { return (TTSString<T,TC>&)TTSString<T,TC>(*this).Append(v, strlen(v)); }
  //............................................................................
  static int IndexOf(const TC *where, size_t len_a, const TC *what, size_t len_b, bool CI) {
    if( len_a < len_b )  return -1;
    if( !CI )  {
      for( size_t i=0; i < len_a; i++ )  {
        if( i+len_b >= len_a )  return -1;
        bool found = true;
        for( size_t j=0;  j < len_b; j++ )
          if( where[i+j] != what[j] )  {
            found = false;
            break;
          }
        if( found ) return i;
      }
    }
    else  {
      for( size_t i=0; i < len_a; i++ )  {
        if( i+len_b >= len_a )  return -1;
        bool found = true;
        for( size_t j=0;  j < len_b; j++ )
          if( toupper(where[i+j]) != toupper(what[j]) )  {
            found = false;
            break;
          }
        if( found ) return i;
      }
    }
    return -1;
  }
  //............................................................................
  int IndexOf(const TTSString& what)   const { return IndexOf( Data(), Length(), what.Data(), what.Length(), false);  }
  int IndexOfCI(const TTSString& what) const { return IndexOf( Data(), Length(), what.Data(), what.Length(), true);  }
  int IndexOf(const TC *what)          const { return IndexOf( Data(), Length(), what, strlen(what), false);  }
  int IndexOfCI(const TC *what)        const { return IndexOf( Data(), Length(), what, strlen(what), true);  }
  //............................................................................
  // function checks for preceding radix encoding
  template <class IT> IT SafeInt(const TC *data, size_t len, unsigned short Rad=10) const  {
    if( len == 0 )    throw TInvalidArgumentException(__OlxSourceInfo, "invalid integer format");
    size_t sts = 0; // string start, end
    while( data[sts] == ' ' && ++sts < len )
    if( sts >= len )  throw TInvalidArgumentException(__OlxSourceInfo, "invalid integer format");
    // test for any particluar format specifier, here just '0x', for hexadecimal
    if( len > sts+1 && data[sts] == '0' && data[sts+1] == 'x' )  {
      Rad = 16;
      sts += 2;
    }
    return RadInt<IT>(&data[sts], len-sts, Rad);
  }
  template <class IT> inline IT SafeInt(unsigned short Rad=10) const  {
    return SafeInt<IT>(Data(), Length(), Rad);
  }
  //............................................................................
  // no leading '\0', got to do it ourselves
  template <class IT> IT RadInt(const TC *data, size_t len, unsigned short Rad=10) const  {
    if( len == 0 )    throw TInvalidArgumentException(__OlxSourceInfo, "invalid integer format");
    size_t sts = 0; // string start, end
    while( data[sts] == ' ' && ++sts < len )
    if( sts >= len )  throw TInvalidArgumentException(__OlxSourceInfo, "invalid integer format");
    IT val=0;
    bool Negative = false;
    if( data[sts] == '-' )  {  Negative = true;  sts++;  }
    else if( data[sts] == '+' )  sts++;
    if( sts == len )  throw TInvalidArgumentException(__OlxSourceInfo, "invalid ineteger format");
    if( Rad > 10 )  {
      for( size_t i=sts; i < len; i++ )  {
        IT pv = -1;
        if( data[i] <= '9' && data[i] >= '0' )  // the order is important, chars are rearer
          pv = data[i] - '0';
        else if( data[i] <= 'Z' && data[i] >= 'A' )
          pv = data[i] - 'A' + 10;
        else if( data[i] <= 'z' && data[i] >= 'a' )
          pv = data[i] - 'a' + 10;
        else  // throw
          break;
        val = val*Rad + pv;
       }
     }
     else  {
       for( size_t i=sts; i < len; i++ )  {
         if( data[i] <= '9' && data[i] >= '0' )
           val = val*Rad + (data[i] - '0');
         else  // throw
           break;
       }
     }
     return Negative ? -val : val;
   }
  template <class IT> inline IT RadInt(unsigned short Rad=10) const  {
     return RadInt<IT>(Data(), Length(), Rad);
   }
  //............................................................................
  // no leading '\0', got to do it ourselves
  static double ToDouble(const TC *data, size_t len) {
    if( len == 0 )  throw TInvalidArgumentException(__OlxSourceInfo, "invalid number format");
    size_t sts = 0; // string start
    while( data[sts] == ' ' && ++sts < len )
    if( sts >= len )
      throw TInvalidArgumentException(__OlxSourceInfo, "invalid number format");
    bool negative = false;
    if( data[sts] == '-' )  {  negative = true;  sts++;  }
    else if( data[sts] == '+' )  sts++;
    if( sts == len )
      throw TInvalidArgumentException(__OlxSourceInfo, "invalid number format");
    double bp=0, ap=0, apexp=1, exp=0;
    bool fpfound = false, expfound = false, expneg = false;
    for( size_t i=sts; i < len; i++ )  {
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
        else if( data[i] >= '0' && data[i] <= '9' )  // anonymous positiv exp
         i--; 
        else  // invalid dddd.ddddE-/+/ddd format
          break;
      }
      else  // invalid char for a number
        break;
    }
    bp = (expneg) ? (bp + ap/apexp)/pow10(exp) : (bp + ap/apexp)*pow10(exp);
    return negative ? -bp : bp;
  }
  inline double ToDouble() const {  return ToDouble(Data(), Length());  }
  //............................................................................
  int Strtok(TArrayList< TTSString<T, TC> > &toks, const TTSString<T, TC> &sep) const {
    int cnt = toks.Count();
    if( sep.IsEmpty() )
      toks.Add( *this );
    else if( sep.Length() == 1 )  {
      size_t start = 0;
      char ch = sep[0];
      for( size_t i=0; i < Length(); i++ )  {
        if( Data(i) == ch )  {
          toks.Add( TTSString<T,TC>(*this, start, i-start) );
          start = i+1;
        }
      }
      if( start < Length() )
        toks.Add( TTSString<T,TC>(*this, start, Length()-start) );
    }
    else  {
      size_t start = 0;
      for( size_t i=0; i < Length(); i++ )  {
        if( i+sep.Length() >= Length() )  break;
        bool found = true;
        for( size_t j=0;  j < sep.Length(); j++ )
        if( Data(i+j) != sep[j] )  {
          found = false;
          break;
        }
        if( found )  {
          toks.Add( TTSString<T,TC>(*this, start, i-start) );
          start = i+sep.Length();
        }
      }
      if( start < Length() )
        toks.Add( TTSString<T,TC>(*this, start, Length()-start) );
    }
    return toks.Count() - cnt;
  }
};

typedef TTSString<TIString, char> MString;

#endif
