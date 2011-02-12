#include "ostring.h"

const olxstr &esdl::EmptyString = olxstr("");
const olxstr &esdl::NullString = (const olxstr&)(*(olxstr*)NULL);
const olxstr &esdl::TrueString = olxstr("true");
const olxstr &esdl::FalseString = olxstr("false");

const olxcstr &esdl::CEmptyString = olxcstr("");
const olxcstr &esdl::CNullString = (const olxcstr&)(*(olxcstr*)NULL);
const olxcstr &esdl::CTrueString = olxcstr("true");
const olxcstr &esdl::CFalseString = olxcstr("false");

const olxwstr &esdl::WEmptyString = olxwstr("");
const olxwstr &esdl::WNullString = (const olxwstr&)(*(olxwstr*)NULL);
const olxwstr &esdl::WTrueString = olxwstr("true");
const olxwstr &esdl::WFalseString = olxwstr("false");
