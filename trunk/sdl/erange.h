#ifndef erangeH
#define erangeH

#include "ebase.h"
#include "typelist.h"

BeginEsdlNamespace()

class TERange {
public:
  static void ListFromString(const olxstr &Range, TSizeList &List);
  static void StringFromList(olxstr &Range, const TSizeList &List);
};

EndEsdlNamespace()

#endif
