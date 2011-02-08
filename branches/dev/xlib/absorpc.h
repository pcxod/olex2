#ifndef __olx_xl_absorpc_H
#define __olx_xl_absorpc_H
#include "xbase.h"
#include "edict.h"
#include "math/spline.h"
/*
http://www.nist.gov/pml/data/xraycoef/index.cfm

Tables of the photon mass attenuation coefficient mu/rho and the
mass energy-absorption coefficient muen/rho are presented for all of the elements
Z = 1 to 92. The tables cover energies of the photon (x-ray, gamma ray, bremsstrahlung)
from 1 keV to 20 MeV. The mu/rho values are taken from the current photon interaction
database at the National Institute of Standards and Technology, and the muen/rho values
are based on the new calculations by Seltzer described in Radiation Research 136, 147
(1993).
*/

BeginXlibNamespace()

struct cm_Absorption_Coefficient {  
  double energy, mu_over_rho, muen_over_rho;
  double GetMuOverRho() const {  return mu_over_rho;  }
  double GetMuEnOverRho() const {  return muen_over_rho;  }
};

struct cm_Absorption_Coefficient_Reg  {
  olxdict<olxstr, const cm_Absorption_Coefficient*, olxstrComparator<true> > entries;
  cm_Absorption_Coefficient_Reg();
  const cm_Absorption_Coefficient* locate(const olxstr& symbol)  {
    return entries.Find(symbol, NULL);
  }
  // cm^2/g
  double CalcMuOverRhoForE(double eV, const cm_Absorption_Coefficient* ac) const {
    return _CalcForE(eV, ac, &cm_Absorption_Coefficient::GetMuOverRho);
  }
  // cm^2/g
  double CalcMuenOverRhoForE(double eV, const cm_Absorption_Coefficient* ac)  {
    return _CalcForE(eV, ac, &cm_Absorption_Coefficient::GetMuEnOverRho);
  }
  double _CalcForE(double eV, const cm_Absorption_Coefficient* ac,
    double (cm_Absorption_Coefficient::*f)() const) const
  {
    eV /= 1e6;
    if( ac == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "undefined absorption data");
    if( eV < ac[0].energy )
      throw TInvalidArgumentException(__OlxSourceInfo, "energy is out of range");
    if( eV == ac[0].energy )
      return ac[0].muen_over_rho;
    size_t cnt = 0;
    const cm_Absorption_Coefficient* _ac = ac;
    while( ac->energy < eV && ac->energy != 0 )  {
      cnt++;
      ac++;
    }
    if( ac->energy == 0 )
      throw TInvalidArgumentException(__OlxSourceInfo, "energy is out of range");
    // go left
    size_t l_cnt = cnt-1;
    while( l_cnt > 0 && (cnt-l_cnt) < 4 && _ac[l_cnt].energy != 0 )  {
      if( _ac[l_cnt-1].energy == _ac[l_cnt].energy )  // absorption edge
        break;
      l_cnt--;
    }
    // go right
    size_t r_cnt = cnt+1;
    while( (r_cnt-cnt) < 4 && _ac[r_cnt].energy != 0 )  {
      if( _ac[r_cnt+1].energy == _ac[r_cnt].energy )  // absorption edge
        break;
      r_cnt++;
    }
    if( (r_cnt-l_cnt) >= 5 )  {  // use spline interpolation
      math::spline::Spline3<double> s;
      s.x.Resize(r_cnt-l_cnt);
      s.y.Resize(r_cnt-l_cnt);
      for( size_t i=l_cnt; i < r_cnt; i++ )  {
        s.x(i-l_cnt) = _ac[i].energy;
        s.y(i-l_cnt) = (_ac[i].*f)();
      }
      return math::spline::Builder<double>::akima(s).interpolate(eV);
    }
    if( ac->energy > eV )  {    
      if( (ac->*f)() == ((ac-1)->*f)() )
        return ac->muen_over_rho;
      const double k = (eV-(ac-1)->energy)/(ac->energy - (ac-1)->energy);
      return ((ac-1)->*f)() + k*((ac->*f)()-((ac-1)->*f)());
    }
    else if( ac->energy == eV )
      return ac->muen_over_rho;
    throw TFunctionFailedException(__OlxSourceInfo, "cannot happen");
  }

};

extern const cm_Absorption_Coefficient _cm_absorpc_Ac[];
extern const cm_Absorption_Coefficient _cm_absorpc_Ag[];
extern const cm_Absorption_Coefficient _cm_absorpc_Al[];
extern const cm_Absorption_Coefficient _cm_absorpc_Ar[];
extern const cm_Absorption_Coefficient _cm_absorpc_As[];
extern const cm_Absorption_Coefficient _cm_absorpc_At[];
extern const cm_Absorption_Coefficient _cm_absorpc_Au[];
extern const cm_Absorption_Coefficient _cm_absorpc_B[];
extern const cm_Absorption_Coefficient _cm_absorpc_Ba[];
extern const cm_Absorption_Coefficient _cm_absorpc_Be[];
extern const cm_Absorption_Coefficient _cm_absorpc_Bi[];
extern const cm_Absorption_Coefficient _cm_absorpc_Br[];
extern const cm_Absorption_Coefficient _cm_absorpc_C[];
extern const cm_Absorption_Coefficient _cm_absorpc_Ca[];
extern const cm_Absorption_Coefficient _cm_absorpc_Cd[];
extern const cm_Absorption_Coefficient _cm_absorpc_Ce[];
extern const cm_Absorption_Coefficient _cm_absorpc_Cl[];
extern const cm_Absorption_Coefficient _cm_absorpc_Co[];
extern const cm_Absorption_Coefficient _cm_absorpc_Cr[];
extern const cm_Absorption_Coefficient _cm_absorpc_Cs[];
extern const cm_Absorption_Coefficient _cm_absorpc_Cu[];
extern const cm_Absorption_Coefficient _cm_absorpc_Dy[];
extern const cm_Absorption_Coefficient _cm_absorpc_Er[];
extern const cm_Absorption_Coefficient _cm_absorpc_Eu[];
extern const cm_Absorption_Coefficient _cm_absorpc_F[];
extern const cm_Absorption_Coefficient _cm_absorpc_Fe[];
extern const cm_Absorption_Coefficient _cm_absorpc_Fr[];
extern const cm_Absorption_Coefficient _cm_absorpc_Ga[];
extern const cm_Absorption_Coefficient _cm_absorpc_Gd[];
extern const cm_Absorption_Coefficient _cm_absorpc_Ge[];
extern const cm_Absorption_Coefficient _cm_absorpc_H[];
extern const cm_Absorption_Coefficient _cm_absorpc_He[];
extern const cm_Absorption_Coefficient _cm_absorpc_Hf[];
extern const cm_Absorption_Coefficient _cm_absorpc_Hg[];
extern const cm_Absorption_Coefficient _cm_absorpc_Ho[];
extern const cm_Absorption_Coefficient _cm_absorpc_I[];
extern const cm_Absorption_Coefficient _cm_absorpc_In[];
extern const cm_Absorption_Coefficient _cm_absorpc_Ir[];
extern const cm_Absorption_Coefficient _cm_absorpc_K[];
extern const cm_Absorption_Coefficient _cm_absorpc_Kr[];
extern const cm_Absorption_Coefficient _cm_absorpc_La[];
extern const cm_Absorption_Coefficient _cm_absorpc_Li[];
extern const cm_Absorption_Coefficient _cm_absorpc_Lu[];
extern const cm_Absorption_Coefficient _cm_absorpc_Mg[];
extern const cm_Absorption_Coefficient _cm_absorpc_Mn[];
extern const cm_Absorption_Coefficient _cm_absorpc_Mo[];
extern const cm_Absorption_Coefficient _cm_absorpc_N[];
extern const cm_Absorption_Coefficient _cm_absorpc_Na[];
extern const cm_Absorption_Coefficient _cm_absorpc_Nb[];
extern const cm_Absorption_Coefficient _cm_absorpc_Nd[];
extern const cm_Absorption_Coefficient _cm_absorpc_Ne[];
extern const cm_Absorption_Coefficient _cm_absorpc_Ni[];
extern const cm_Absorption_Coefficient _cm_absorpc_O[];
extern const cm_Absorption_Coefficient _cm_absorpc_Os[];
extern const cm_Absorption_Coefficient _cm_absorpc_P[];
extern const cm_Absorption_Coefficient _cm_absorpc_Pa[];
extern const cm_Absorption_Coefficient _cm_absorpc_Pb[];
extern const cm_Absorption_Coefficient _cm_absorpc_Pd[];
extern const cm_Absorption_Coefficient _cm_absorpc_Pm[];
extern const cm_Absorption_Coefficient _cm_absorpc_Po[];
extern const cm_Absorption_Coefficient _cm_absorpc_Pr[];
extern const cm_Absorption_Coefficient _cm_absorpc_Pt[];
extern const cm_Absorption_Coefficient _cm_absorpc_Ra[];
extern const cm_Absorption_Coefficient _cm_absorpc_Rb[];
extern const cm_Absorption_Coefficient _cm_absorpc_Re[];
extern const cm_Absorption_Coefficient _cm_absorpc_Rh[];
extern const cm_Absorption_Coefficient _cm_absorpc_Rn[];
extern const cm_Absorption_Coefficient _cm_absorpc_Ru[];
extern const cm_Absorption_Coefficient _cm_absorpc_S[];
extern const cm_Absorption_Coefficient _cm_absorpc_Sb[];
extern const cm_Absorption_Coefficient _cm_absorpc_Sc[];
extern const cm_Absorption_Coefficient _cm_absorpc_Se[];
extern const cm_Absorption_Coefficient _cm_absorpc_Si[];
extern const cm_Absorption_Coefficient _cm_absorpc_Sm[];
extern const cm_Absorption_Coefficient _cm_absorpc_Sn[];
extern const cm_Absorption_Coefficient _cm_absorpc_Sr[];
extern const cm_Absorption_Coefficient _cm_absorpc_Ta[];
extern const cm_Absorption_Coefficient _cm_absorpc_Tb[];
extern const cm_Absorption_Coefficient _cm_absorpc_Tc[];
extern const cm_Absorption_Coefficient _cm_absorpc_Te[];
extern const cm_Absorption_Coefficient _cm_absorpc_Th[];
extern const cm_Absorption_Coefficient _cm_absorpc_Ti[];
extern const cm_Absorption_Coefficient _cm_absorpc_Tl[];
extern const cm_Absorption_Coefficient _cm_absorpc_Tm[];
extern const cm_Absorption_Coefficient _cm_absorpc_U[];
extern const cm_Absorption_Coefficient _cm_absorpc_V[];
extern const cm_Absorption_Coefficient _cm_absorpc_W[];
extern const cm_Absorption_Coefficient _cm_absorpc_Xe[];
extern const cm_Absorption_Coefficient _cm_absorpc_Y[];
extern const cm_Absorption_Coefficient _cm_absorpc_Yb[];
extern const cm_Absorption_Coefficient _cm_absorpc_Zn[];
extern const cm_Absorption_Coefficient _cm_absorpc_Zr[];

EndXlibNamespace()
#endif
