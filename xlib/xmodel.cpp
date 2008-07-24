#include "xmodel.h"

olxstr XScattererRef::GetLabel() const {
  olxstr name(scatterer->Label);
  if( scatterer->Owner->Number == -1 )  {
    if( symm != 0 )
      name << "_$" << (scatterer->Owner->Parent.UsedSymmIndex(*symm) + 1);
  }
  else  {
    name << '_' << scatterer->Owner->Number;
    if( symm != 0 )
      name << '$' << (scatterer->Owner->Parent.UsedSymmIndex(*symm) + 1);
  }
  return name;
}

