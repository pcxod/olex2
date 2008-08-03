#include "xmodel.h"
//................................................................................
olxstr XScattererRef::GetLabel() const {
  olxstr name(scatterer->label);
  if( scatterer->Owner.Residue->Number == -1 )  {
    if( symm != 0 )
      name << "_$" << (scatterer->Owner.Parent.UsedSymmIndex(*symm) + 1);
  }
  else  {
    name << '_' << scatterer->Owner.Residue->Number;
    if( symm != 0 )
      name << '$' << (scatterer->Owner.Parent.UsedSymmIndex(*symm) + 1);
  }
  return name;
}
//................................................................................
