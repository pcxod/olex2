//---------------------------------------------------------------------------
#ifndef erangeH
#define erangeH

#include "ebase.h"
//#include "estring.h"
#include "typelist.h"

BeginEsdlNamespace()


class TERange
{
public:
  static void ListFromString(const olxstr &Range, TIntList &List);
  static void StringFromList(olxstr &Range, const TIntList &List);
};

EndEsdlNamespace()

#endif
