#ifndef __chem_lib_data
#define __chem_lib_data

#include "xbase.h"
#include "ecomplex.h"

// atomic number of some atoms
static const short 
  iHydrogenZ = 1,
  iBoronZ    = 5,  
  iCarbonZ   = 6,
  iNitrogenZ = 7,
  iOxygenZ   = 8,
  iFluorineZ = 9,
  iSiliconZ  = 14,
  iPhosphorusZ = 15,
  iSulphurZ    = 16,
  iChlorineZ   = 17,
  iQPeakZ      = -1;

/*
  the source of data for Henke tables and scattering data is cctbx
  neutron scattering data was downloaded from this website:
  http://www.ncnr.nist.gov/resources/n-lengths/index.html
  Note from cctbx:
  The original data can be found at:
    ftp://grace.lbl.gov/pub/sf/

  From ftp://grace.lbl.gov/pub/sf/read.me:

                Low-Energy X-ray Interaction Coefficients:
                Photoabsorption, Scattering, and Reflection
                        E = 30-30,000 eV, Z = 1-92

                B. L. Henke, E. M. Gullikson, and J. C. Davis
                        Center for X-Ray Optics, 2-400
                        Lawrence Berkeley Laboratory
                        Berkeley, California 94720

  Reference: B. L. Henke, E. M. Gullikson, and J. C. Davis,
  Atomic Data and Nuclear Data Tables Vol. 54 No. 2 (July 1993).
*/

BeginXlibNamespace()

struct cm_Neutron_Scattering {
  compd coh;
  double xs; 
  cm_Neutron_Scattering(double _coh_re, double _coh_im, double _xs) :
  coh(_coh_re, _coh_im), xs(_xs)  {  }
};
struct cm_Isotope {  
  double Mr, W;  
  const cm_Neutron_Scattering* neutron_scattering;
};
struct cm_Anomalous_Henke {  
  double energy, fp, fdp;  
  static const double Undefined;
};
struct cm_Gaussians {  
  double a1, a2, a3, a4, b1, b2, b3, b4, c;  
  cm_Gaussians(double _a1, double _a2, double _a3, double _a4, 
    double _b1, double _b2, double _b3, double _b4, double _c) :
    a1(_a1), a2(_a2), a3(_a3), a4(_a4), 
      b1(-_b1), b2(-_b2), b3(-_b3), b4(-_b4), c(_c)  {  }

  inline double calc_sq(double sqv) const {
    return a1*exp(b1*sqv) + a2*exp(b2*sqv) + a3*exp(b3*sqv) + a4*exp(b4*sqv) + c;
  }
};

struct cm_Element {
  const olxstr symbol, name;
  const cm_Gaussians* gaussians;  // 9 elements = 4 gaussians + const
  const cm_Isotope* isotopes;
  const cm_Anomalous_Henke* henke_data; 
  const cm_Neutron_Scattering* neutron_scattering;
  unsigned int def_color;
  const short z, isotope_count, henke_count;
  double r_pers, r_bonding, r_sfil;
  cm_Element(const char* _symbol, const char* _name, unsigned int _def_color, short _z, 
    double _r_pers, double _r_bonding, double _r_sfil, const cm_Gaussians* _gaussians, 
    const cm_Isotope* _isotopes, short _isotope_count, 
    const cm_Anomalous_Henke* _henke_data, short _henke_count, 
    const cm_Neutron_Scattering* _neutron_scattering) :
    symbol(_symbol), name(_name), def_color(_def_color), z(_z), r_pers(_r_pers), 
    r_bonding(_r_bonding), r_sfil(_r_sfil), gaussians(_gaussians), 
    isotopes(_isotopes), isotope_count(_isotope_count),
    henke_data(_henke_data), henke_count(_henke_count), neutron_scattering(_neutron_scattering) {  }

  compd CalcFpFdp(double eV) const  {
    if( henke_data == NULL )
      throw TFunctionFailedException(__OlxSourceInfo, "undefined f\' f\" data");
    if( eV < henke_data[0].energy || eV > henke_data[henke_count-1].energy )
      throw TInvalidArgumentException(__OlxSourceInfo, "energy is out of range");
    if( eV == henke_data[0].energy )  return compd(henke_data[0].fp, henke_data[0].fdp);
    if( eV == henke_data[henke_count-1].energy )  return compd(henke_data[henke_count-1].fp, henke_data[henke_count-1].fdp);
    for( int i=0; i < henke_count; i++ )  {
      if( henke_data[i].energy > eV )  {
        double k = (eV-henke_data[i-1].energy)/(henke_data[i].energy - henke_data[i-1].energy);
        double fp  = cm_Anomalous_Henke::Undefined;
        double fdp = cm_Anomalous_Henke::Undefined;
        if( henke_data[i-1].fp != cm_Anomalous_Henke::Undefined && 
          henke_data[i].fp != cm_Anomalous_Henke::Undefined )
          fp = henke_data[i-1].fp + k*(henke_data[i].fp-henke_data[i-1].fp);
        if( henke_data[i-1].fdp != cm_Anomalous_Henke::Undefined && 
          henke_data[i].fdp != cm_Anomalous_Henke::Undefined )
          fdp = henke_data[i-1].fdp + k*(henke_data[i].fdp-henke_data[i-1].fdp);
        return compd(fp, fdp);
      }
      else if( henke_data[i].energy == eV )
        return compd(henke_data[i].fp, henke_data[i].fdp);
    }
    throw TFunctionFailedException(__OlxSourceInfo, "cannot happen");
  }
  inline bool operator >  (const cm_Element& ce) const {  return z >  ce.z;  }
  inline bool operator >= (const cm_Element& ce) const {  return z >= ce.z;  }
  inline bool operator <  (const cm_Element& ce) const {  return z <  ce.z;  }
  inline bool operator <= (const cm_Element& ce) const {  return z <= ce.z;  }
  inline bool operator == (const cm_Element& ce) const {  return z == ce.z;  }
  inline bool operator >  (short _z) const {  return z >  _z;  }
  inline bool operator >= (short _z) const {  return z >= _z;  }
  inline bool operator <  (short _z) const {  return z <  _z;  }
  inline bool operator <= (short _z) const {  return z <= _z;  }
  inline bool operator == (short _z) const {  return z == _z;  }
};

class XElementLib {
public:
  // and exact symbol as C or Cr is expected
  static cm_Element* FindBySymbol(const olxstr& symbol);
  // a label might be passed as C1 or Cr2
  static cm_Element* FindBySymbolEx(const olxstr& symbol);
  // extracts symbol from a label, like C for C1 or Cr for Cr2
  static inline const olxstr& ExtractSymbol(const olxstr& label)  {
    cm_Element* elm = FindBySymbolEx(label);
    return elm == NULL ? EmptyString : elm->symbol;
  }
  // returns true if labels starts from a symbol
  static inline bool IsElement(const olxstr& label) {  return !ExtractSymbol(label).IsEmpty();  }
};

EndXlibNamespace()
#endif
