#include "xmodel.h"
//................................................................................
olxstr XScattererRef::GetLabel() const {
  olxstr name(scatterer->Label);
  if( scatterer->Residue->Number == -1 )  {
    if( symm != 0 )
      name << "_$" << (scatterer->Parent.UsedSymmIndex(*symm) + 1);
  }
  else  {
    name << '_' << scatterer->Residue->Number;
    if( symm != 0 )
      name << '$' << (scatterer->Parent.UsedSymmIndex(*symm) + 1);
  }
  return name;
}
//................................................................................
