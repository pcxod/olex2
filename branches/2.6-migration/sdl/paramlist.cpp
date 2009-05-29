//---------------------------------------------------------------------------//
// namespace TEObjects
// TParamList - list of associated strings : named parameters list
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//

#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include <stddef.h>
#include <stdio.h>
//#include <io.h>
#include <string.h>
#include "paramlist.h"
#include "exception.h"

UseEsdlNamespace()
//..............................................................................
TParamList::TParamList(){  ;  }
//..............................................................................
TParamList::TParamList(const TParamList &v)  {
  for( int i=0; i < v.Count(); i++ )
    AddParam(v.GetName(i), v.GetValue(i), false);
}
//..............................................................................
TParamList::~TParamList(){  Clear(); }
//..............................................................................
void TParamList::FromString(const olxstr &S, char Sep) {  // -t=op
  TStrList SL(S, Sep);
  if( SL.Count() == 1 )
    AddParam(SL[0], EmptyString);
  else if( SL.Count() > 1 )  {
    ProcessStringParam(SL[1]);
    AddParam(SL[0], SL[1]);
  }
}
//..............................................................................
void TParamList::AddParam(const olxstr &Name, const olxstr &Val, bool Check)  {
  if( Check )  {
    int index = IndexOf(Name);
    if( index != -1 )  {
      GetObject(index) = Val;
      return;
    }
  }
  TStrStrList::Add(Name, Val);
}
//..............................................................................
bool TParamList::ProcessStringParam(olxstr &Param)  {
  if( !Param.Length() )  return false;
  if( Param.FirstIndexOf('\'') == -1 && Param.FirstIndexOf('"') == -1)  return false;
  int i=0;
  olxch Sep;
  olxstr NP;
  while( i < Param.Length() && ( (Param[i] != '\'')||(Param[i] != '"')) && Param[i]==' ') // skip spaces
    i++;

  if( i < Param.Length() )  {
    if( (Param[i] == '\'') || (Param[i] == '"') )
      Sep = Param[i];
    else
      return false; 
  }
  i++;  // skip first '
  while( i < Param.Length() && Param[i] != Sep ) // skip spaces
  { NP << Param[i]; i++; }
  Param = NP;
  return true;
}
//..............................................................................
bool TParamList::GetQuotationChar( const olxstr &Param, olxch& Char )  {
  if( Param.Length() < 2 )  return false;
  int stringStart = 0, stringEnd = Param.Length()-1;
  // skip spaces at the beginning
  while( Param[stringStart] == ' ' && (stringStart < stringEnd) )  stringStart++;
  if( stringEnd - stringStart < 2 )  return false;
  // skip trailing spaces
  while( Param[stringEnd] == ' ' )  stringEnd--;
  if( stringEnd - stringStart < 2 )  return false;

  olxch sChar = Param[stringStart], eChar = Param[stringEnd];
  if( (sChar == eChar) && (sChar == '\'' || sChar == '\"') )  {
    // check if there are any other same chars inside the string
    for( int i=stringStart+1; i < stringEnd-1; i++ )
      if( Param[i] == sChar )  return false;
    Char = sChar;
    return true;
  }
  
  return false;
}
//..............................................................................

