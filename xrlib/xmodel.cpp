#include "xmodel.h"
//................................................................................
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
//................................................................................
//................................................................................
XSite& XScatterer::SetSite(XSite& site)  {
  if( Site != NULL )
    Site->Scatterers.Remove(this);
  Site = &site;
  Site->Scatterers.Add(this);
  return site;
}
//................................................................................
XTDP&  XScatterer::SetTDP(XTDP& tdp)  {
  if( TDP != NULL )
    TDP->Scatterers.Remove(this);
  TDP = &tdp;
  Site->Scatterers.Add(this);
  return tdp;
}
