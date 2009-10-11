#include "ostring.h"

const olxstr &esdl::EmptyString = olxstr("");
const olxstr &esdl::NullString = (const olxstr&)(*(olxstr*)NULL);
const olxstr &esdl::TrueString = olxstr("true");
const olxstr &esdl::FalseString = olxstr("false");

const CString &esdl::CEmptyString = CString("");
const CString &esdl::CNullString = (const CString&)(*(CString*)NULL);
const CString &esdl::CTrueString = CString("true");
const CString &esdl::CFalseString = CString("false");

const WString &esdl::WEmptyString = WString("");
const WString &esdl::WNullString = (const WString&)(*(WString*)NULL);
const WString &esdl::WTrueString = WString("true");
const WString &esdl::WFalseString = WString("false");
