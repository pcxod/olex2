#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "paramlist.h"
#include "exception.h"
UseEsdlNamespace()
using namespace exparse;

TParamList::TParamList(const TParamList &v)  {
  for( size_t i=0; i < v.Count(); i++ )
    AddParam(v.GetName(i), v.GetValue(i), false);
}
//..............................................................................
void TParamList::FromString(const olxstr &S, char Sep) {  // -t=op
  TStrList SL(S, Sep);
  if( SL.Count() == 1 )
    AddParam(SL[0], EmptyString());
  else if( SL.Count() > 1 )
    AddParam(SL[0], parser_util::unquote(SL[1]));
}
//..............................................................................
void TParamList::AddParam(const olxstr &Name, const olxstr &Val, bool Check)  {
  if( Check )  {
    size_t index = IndexOf(Name);
    if( index != InvalidIndex )  {
      GetObject(index) = Val;
      return;
    }
  }
  TStrStrList::Add(Name, Val);
}
//..............................................................................
