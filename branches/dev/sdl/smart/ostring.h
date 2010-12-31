#ifndef __SMART_OLX_STR
#define __SMART_OLX_STR

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

template <bool CaseInsensetive>  class olxstrComparator  {
public:
  template <class S1, class S2>  static inline int Compare(const S1& A, const S2& B)  {
    return CaseInsensetive ? A.Comparei(B): A.Compare(B);
  }
};

EndEsdlNamespace()
#endif
