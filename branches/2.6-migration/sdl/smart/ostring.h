#ifndef __SMART_OLX_STR
#define __SMART_OLX_STR

#include "olx_istring.h"

BeginEsdlNamespace()

#ifdef __BORLANDC__

typedef TTSString<TCString, char> CString;
typedef TTSString<TWString, wchar_t> WString;

#ifdef _UNICODE
  typedef TTSString<TWString, wchar_t> olxstr;
#else
  typedef TTSString<TCString, char > olxstr;
#endif

extern const olxstr &EmptyString;
extern const olxstr &FalseString;
extern const olxstr &TrueString;
extern const olxstr &NullString;

extern const CString &CEmptyString;
extern const CString &CFalseString;
extern const CString &CTrueString;
extern const CString &CNullString;

extern const WString &WEmptyString;
extern const WString &WFalseString;
extern const WString &WTrueString;
extern const WString &WNullString;
#endif

template <bool CaseInsensetive>  class olxstrComparator  {
public:
  template <class S1, class S2>  static inline int Compare(const S1& A, const S2& B )  {
    return (CaseInsensetive) ? A.Comparei( B ): A.Compare( B );
  }
};
EndEsdlNamespace()
#endif
