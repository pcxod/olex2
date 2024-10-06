/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_absorpc_H
#define __olx_xl_absorpc_H
#include "xbase.h"
#include "edict.h"
/*
http://www.nist.gov/pml/data/xraycoef/index.cfm

Tables of the photon mass attenuation coefficient mu/rho and the
mass energy-absorption coefficient muen/rho are presented for all of the elements
Z = 1 to 92. The tables cover energies of the photon (x-ray, gamma ray, bremsstrahlung)
from 1 keV to 20 MeV. The mu/rho values are taken from the current photon interaction
database at the National Institute of Standards and Technology, and the muen/rho values
are based on the new calculations by Seltzer described in Radiation Research 136, 147
(1993).

Further enhanced by IT tables volume C, table 4.2.4.3

4.2.4. X-ray absorption (or attenuation) coefficients
*/

BeginXlibNamespace()

struct cm_Absorption_Coefficient {
  double energy, mu_over_rho;
  double GetMuOverRho() const {  return mu_over_rho;  }
};

struct cm_Absorption_Entry  {
  const cm_Absorption_Coefficient* data;
  size_t size;
  cm_Absorption_Entry(const cm_Absorption_Coefficient* _data) :
    data(_data), size(0)  {}
  cm_Absorption_Entry() : data(NULL), size(0)  {}
};

struct cm_Absorption_Coefficient_Reg  {
  olxdict<olxstr, cm_Absorption_Entry, olxstrComparator<true> > entries;
  cm_Absorption_Coefficient_Reg();
  cm_Absorption_Entry* find(const olxstr& symbol)  {
    const size_t i = entries.IndexOf(symbol);
    return i == InvalidIndex ? NULL : &entries.GetValue(i);
  }
  cm_Absorption_Entry& get(const olxstr& symbol) {
    const size_t i = entries.IndexOf(symbol);
    if (i == InvalidIndex) {
      throw TFunctionFailedException(__OlxSourceInfo,
        olxstr("could not locate entry for ") << symbol);
    }
    return entries.GetValue(i);
  }
  // cm^2/g
  double CalcMuOverRhoForE(double eV, cm_Absorption_Entry& ac) const {
    return _CalcForE(eV, ac, &cm_Absorption_Coefficient::GetMuOverRho);
  }
  double _CalcForE(double eV, cm_Absorption_Entry& ac,
    double (cm_Absorption_Coefficient::*f)() const) const;
};

extern const cm_Absorption_Coefficient _cm_absorpc_H[];
extern const cm_Absorption_Coefficient _cm_absorpc_He[];
extern const cm_Absorption_Coefficient _cm_absorpc_Li[];
extern const cm_Absorption_Coefficient _cm_absorpc_Be[];
extern const cm_Absorption_Coefficient _cm_absorpc_B[];
extern const cm_Absorption_Coefficient _cm_absorpc_C[];
extern const cm_Absorption_Coefficient _cm_absorpc_N[];
extern const cm_Absorption_Coefficient _cm_absorpc_O[];
extern const cm_Absorption_Coefficient _cm_absorpc_F[];
extern const cm_Absorption_Coefficient _cm_absorpc_Ne[];
extern const cm_Absorption_Coefficient _cm_absorpc_Na[];
extern const cm_Absorption_Coefficient _cm_absorpc_Mg[];
extern const cm_Absorption_Coefficient _cm_absorpc_Al[];
extern const cm_Absorption_Coefficient _cm_absorpc_Si[];
extern const cm_Absorption_Coefficient _cm_absorpc_P[];
extern const cm_Absorption_Coefficient _cm_absorpc_S[];
extern const cm_Absorption_Coefficient _cm_absorpc_Cl[];
extern const cm_Absorption_Coefficient _cm_absorpc_Ar[];
extern const cm_Absorption_Coefficient _cm_absorpc_K[];
extern const cm_Absorption_Coefficient _cm_absorpc_Ca[];
extern const cm_Absorption_Coefficient _cm_absorpc_Sc[];
extern const cm_Absorption_Coefficient _cm_absorpc_Ti[];
extern const cm_Absorption_Coefficient _cm_absorpc_V[];
extern const cm_Absorption_Coefficient _cm_absorpc_Cr[];
extern const cm_Absorption_Coefficient _cm_absorpc_Mn[];
extern const cm_Absorption_Coefficient _cm_absorpc_Fe[];
extern const cm_Absorption_Coefficient _cm_absorpc_Co[];
extern const cm_Absorption_Coefficient _cm_absorpc_Ni[];
extern const cm_Absorption_Coefficient _cm_absorpc_Cu[];
extern const cm_Absorption_Coefficient _cm_absorpc_Zn[];
extern const cm_Absorption_Coefficient _cm_absorpc_Ga[];
extern const cm_Absorption_Coefficient _cm_absorpc_Ge[];
extern const cm_Absorption_Coefficient _cm_absorpc_As[];
extern const cm_Absorption_Coefficient _cm_absorpc_Se[];
extern const cm_Absorption_Coefficient _cm_absorpc_Br[];
extern const cm_Absorption_Coefficient _cm_absorpc_Kr[];
extern const cm_Absorption_Coefficient _cm_absorpc_Rb[];
extern const cm_Absorption_Coefficient _cm_absorpc_Sr[];
extern const cm_Absorption_Coefficient _cm_absorpc_Y[];
extern const cm_Absorption_Coefficient _cm_absorpc_Zr[];
extern const cm_Absorption_Coefficient _cm_absorpc_Nb[];
extern const cm_Absorption_Coefficient _cm_absorpc_Mo[];
extern const cm_Absorption_Coefficient _cm_absorpc_Tc[];
extern const cm_Absorption_Coefficient _cm_absorpc_Ru[];
extern const cm_Absorption_Coefficient _cm_absorpc_Rh[];
extern const cm_Absorption_Coefficient _cm_absorpc_Pd[];
extern const cm_Absorption_Coefficient _cm_absorpc_Ag[];
extern const cm_Absorption_Coefficient _cm_absorpc_Cd[];
extern const cm_Absorption_Coefficient _cm_absorpc_In[];
extern const cm_Absorption_Coefficient _cm_absorpc_Sn[];
extern const cm_Absorption_Coefficient _cm_absorpc_Sb[];
extern const cm_Absorption_Coefficient _cm_absorpc_Te[];
extern const cm_Absorption_Coefficient _cm_absorpc_I[];
extern const cm_Absorption_Coefficient _cm_absorpc_Xe[];
extern const cm_Absorption_Coefficient _cm_absorpc_Cs[];
extern const cm_Absorption_Coefficient _cm_absorpc_Ba[];
extern const cm_Absorption_Coefficient _cm_absorpc_La[];
extern const cm_Absorption_Coefficient _cm_absorpc_Ce[];
extern const cm_Absorption_Coefficient _cm_absorpc_Pr[];
extern const cm_Absorption_Coefficient _cm_absorpc_Nd[];
extern const cm_Absorption_Coefficient _cm_absorpc_Pm[];
extern const cm_Absorption_Coefficient _cm_absorpc_Sm[];
extern const cm_Absorption_Coefficient _cm_absorpc_Eu[];
extern const cm_Absorption_Coefficient _cm_absorpc_Gd[];
extern const cm_Absorption_Coefficient _cm_absorpc_Tb[];
extern const cm_Absorption_Coefficient _cm_absorpc_Dy[];
extern const cm_Absorption_Coefficient _cm_absorpc_Ho[];
extern const cm_Absorption_Coefficient _cm_absorpc_Er[];
extern const cm_Absorption_Coefficient _cm_absorpc_Tm[];
extern const cm_Absorption_Coefficient _cm_absorpc_Yb[];
extern const cm_Absorption_Coefficient _cm_absorpc_Lu[];
extern const cm_Absorption_Coefficient _cm_absorpc_Hf[];
extern const cm_Absorption_Coefficient _cm_absorpc_Ta[];
extern const cm_Absorption_Coefficient _cm_absorpc_W[];
extern const cm_Absorption_Coefficient _cm_absorpc_Re[];
extern const cm_Absorption_Coefficient _cm_absorpc_Os[];
extern const cm_Absorption_Coefficient _cm_absorpc_Ir[];
extern const cm_Absorption_Coefficient _cm_absorpc_Pt[];
extern const cm_Absorption_Coefficient _cm_absorpc_Au[];
extern const cm_Absorption_Coefficient _cm_absorpc_Hg[];
extern const cm_Absorption_Coefficient _cm_absorpc_Tl[];
extern const cm_Absorption_Coefficient _cm_absorpc_Pb[];
extern const cm_Absorption_Coefficient _cm_absorpc_Bi[];
extern const cm_Absorption_Coefficient _cm_absorpc_Po[];
extern const cm_Absorption_Coefficient _cm_absorpc_At[];
extern const cm_Absorption_Coefficient _cm_absorpc_Rn[];
extern const cm_Absorption_Coefficient _cm_absorpc_Fr[];
extern const cm_Absorption_Coefficient _cm_absorpc_Ra[];
extern const cm_Absorption_Coefficient _cm_absorpc_Ac[];
extern const cm_Absorption_Coefficient _cm_absorpc_Th[];
extern const cm_Absorption_Coefficient _cm_absorpc_Pa[];
extern const cm_Absorption_Coefficient _cm_absorpc_U[];
extern const cm_Absorption_Coefficient _cm_absorpc_Np[];
extern const cm_Absorption_Coefficient _cm_absorpc_Pu[];
extern const cm_Absorption_Coefficient _cm_absorpc_Am[];
extern const cm_Absorption_Coefficient _cm_absorpc_Cm[];
extern const cm_Absorption_Coefficient _cm_absorpc_Bk[];
extern const cm_Absorption_Coefficient _cm_absorpc_Cf[]; 
EndXlibNamespace()
#endif
