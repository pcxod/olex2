/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "olx_cstring.h"
#include "olx_wstring.h"
#include <locale.h>
#include "../egc.h"

TWString::TWString()  {
  SData = NULL;
  _Start = _Length = 0;
  _Increment = 8;
}
//..............................................................................
TWString::TWString(const bool& v) : TTIString<wchar_t>(v ? WTrueString(): WFalseString())  {}
//..............................................................................
TWString::TWString(const char *str)  {
  _Start = 0;
  _Increment = 8;
  _Length = ((str==NULL) ? 0 : strlen(str));
  SData = new Buffer(_Length+_Increment);
  for( size_t i=0; i < _Length; i++ )
    SData->Data[i] = str[i];
}
TWString::TWString(char * const & str)  {
  _Start = 0;
  _Increment = 8;
  _Length = ((str==NULL) ? 0 : strlen(str));
  SData = new Buffer(_Length+_Increment);
  for( size_t i=0; i < _Length; i++ )
    SData->Data[i] = str[i];
}
TWString::TWString( const TCString& str )  {
  _Start = 0;
  _Increment = 8;
  _Length = str.Length();
  SData = new Buffer(_Length+_Increment);
  for( size_t i=0; i < _Length; i++ )
    SData->Data[i] = str[i];
}
TWString::TWString( const TTIString<char>& str )  {
  _Start = 0;
  _Increment = 8;
  _Length = str.Length();
  SData = new Buffer(_Length+_Increment);
  for( size_t i=0; i < _Length; i++ )
    SData->Data[i] = str[i];
}
// primitive Type constructor
TWString::TWString(const char& v)                  {
  _Start = 0;
  _Increment = 8;
  _Length = 1;  // one entity!!
  SData = new Buffer(_Increment);
  SData->Data[0] = v;
}

//TWString::operator TTIString<wchar_t> () const  {
//  return WStrRV(*this);
//}

const char * TWString::c_str() const  {  return TEGC::New<TCString>(*this).c_str();  }

TWString& TWString::operator << (const CharW &v)  {
  checkBufferForModification(_Length + 1);
  SData->Data[_Length] = v.GetValue();
  _Length ++;
  return *this;
}
TWString& TWString::operator << (const char &v)  {
  checkBufferForModification(_Length + 1);
  SData->Data[_Length] = v;
  _Length++;
  return *this;
}

TWString& TWString::AssignCharStr(const char *str, size_t len)  {
  _Start = 0;
  _Increment = 5;
  _Length = ((len == InvalidSize) ? strlen(str) : len);
  if( SData != NULL )  {
    if( SData->RefCnt == 1 )  // owed by this object
      SData->SetCapacity(_Length);
    else  {
      SData->RefCnt--;
      SData = NULL;
    }
  }
  if( SData == NULL )
    SData = new Buffer(_Length +_Increment);
  for( size_t i=0; i < _Length; i++ )
    SData->Data[i] = str[i];
  return *this;
}

TWString& TWString::operator = (const char &ch)          {
  _Start = 0;
  _Increment = 8;
  _Length = 1;
  if( SData != NULL )  {
    if( SData->RefCnt == 1 )  { // owed by this object
      SData->SetCapacity(_Length);
    }
    else  {
      SData->RefCnt--;
      SData = NULL;
    }
  }
  if( SData == NULL )  SData = new Buffer(_Increment);
  SData->Data[0] = ch;
  return *this;
}
//..............................................................................
TWString& TWString::operator = (const TCString& astr)  {
  return AssignCharStr(astr.raw_str(), astr.Length());
}
//..............................................................................
#ifdef _UNICODE
  TIString TWString::ToString() const {  return *this;  }
#else
  TIString TWString::ToString() const {  return TCString(*this);  }
#endif
