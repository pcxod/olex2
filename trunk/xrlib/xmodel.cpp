#include "xmodel.h"
//................................................................................
olxstr XScattererRef::GetLabel() const {
  olxstr name(scatterer->label);
  if( scatterer->owner.Residue->Number == -1 )  {
    if( symm != 0 )
      name << "_$" << (scatterer->owner.Parent.UsedSymmIndex(*symm) + 1);
  }
  else  {
    name << '_' << scatterer->owner.Residue->Number;
    if( symm != 0 )
      name << '$' << (scatterer->owner.Parent.UsedSymmIndex(*symm) + 1);
  }
  return name;
}
//................................................................................
