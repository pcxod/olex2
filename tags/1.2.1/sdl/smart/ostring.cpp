/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "ostring.h"

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
