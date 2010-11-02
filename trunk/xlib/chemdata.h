#ifndef __chem_lib_data
#define __chem_lib_data
#include "xbase.h"
#include "ecomplex.h"
#include "edict.h"
#include "henke.h"

// atomic number of some atoms
static const short 
  iHydrogenZ   = 1,
  iBoronZ      = 5,  
  iCarbonZ     = 6,
  iNitrogenZ   = 7,
  iOxygenZ     = 8,
  iFluorineZ   = 9,
  iSodumZ      = 11,
  iMagnesiumZ  = 12,
  iSiliconZ    = 14,
  iPhosphorusZ = 15,
  iSulphurZ    = 16,
  iChlorineZ   = 17,
  iPotassiumZ  = 19,
  iCalciumZ    = 20,
  iSeleniumZ   = 34,
  iBromineZ    = 35,
  iQPeakZ      = -1;

const  short
  iHydrogenIndex    = 0,
  iBoronIndex       = 4,
  iCarbonIndex      = 5,
  iNitrogenIndex    = 6,
  iOxygenIndex      = 7,
  iFluorineIndex    = 8,
  iSodiumIndex      = 10,
  iMagnesiumIndex   = 11,
  iSiliconIndex     = 13,
  iPhosphorusIndex  = 14,
  iSulphurIndex     = 15,
  iChlorineIndex    = 16,
  iPotassiumIndex   = 18,
  iCalciumIndex     = 19,
  iQPeakIndex       = 104,
  iDeuteriumIndex   = 105,
  iMaxElementIndex = 105;  // for iterations

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

struct cm_Gaussians {  
  double a1, a2, a3, a4, b1, b2, b3, b4, c;  
  cm_Gaussians() : a1(0), a2(0), a3(0), a4(0), b1(0), b2(0), b3(0), b4(0), c(0)  {}
  // constructor, note that b values are inverted!
  cm_Gaussians(double _a1, double _a2, double _a3, double _a4, 
    double _b1, double _b2, double _b3, double _b4, double _c) :
    a1(_a1), a2(_a2), a3(_a3), a4(_a4), 
      b1(-_b1), b2(-_b2), b3(-_b3), b4(-_b4), c(_c)  {  }
  // copy constructor
  cm_Gaussians(const cm_Gaussians& g) :
    a1(g.a1), a2(g.a2), a3(g.a3), a4(g.a4), 
      b1(g.b1), b2(g.b2), b3(g.b3), b4(g.b4), c(g.c)  {  }

  inline double calc_sq(double sqv) const {
    return a1*exp(b1*sqv) + a2*exp(b2*sqv) + a3*exp(b3*sqv) + a4*exp(b4*sqv) + c;
  }
  cm_Gaussians& operator = (const cm_Gaussians& g)  {
    a1 = g.a1;  a2 = g.a2;  a3 = g.a3;  a4 = g.a4;
    b1 = g.b1;  b2 = g.b2;  b3 = g.b3;  b4 = g.b4;
    c = g.c;
    return *this;
  }
  bool operator == (const cm_Gaussians& g) const {
    return (a1 == g.a1 && a2 == g.a2 && a3 == g.a3 && a4 == g.a4 &&
            b1 == g.b1 && b2 == g.b2 && b3 == g.b3 && b4 == g.b4 &&
            c == g.c);
  }
  bool operator != (const cm_Gaussians& g) const {  return !(*this == g);  }
};

struct cm_Element {
protected:
  double Mr;
public:
  const olxstr symbol, name;
  const cm_Gaussians* gaussians;  // 9 elements = 4 gaussians + const
  const cm_Isotope* isotopes;
  const cm_Anomalous_Henke* henke_data; 
  const cm_Neutron_Scattering* neutron_scattering;
  uint32_t def_color;
  const short z, isotope_count, henke_count;
  const short index;
  double r_pers, r_bonding, r_sfil, r_cov, r_vdw;
  cm_Element(short _index, const char* _symbol, const char* _name, uint32_t _def_color, short _z, 
    double _r_vdw, double _r_cov, double _r_pers, double _r_bonding, double _r_sfil, const cm_Gaussians* _gaussians, 
    const cm_Isotope* _isotopes, short _isotope_count, 
    const cm_Anomalous_Henke* _henke_data, short _henke_count, 
    const cm_Neutron_Scattering* _neutron_scattering) :
    index(_index),
    symbol(_symbol), name(_name), def_color(_def_color), z(_z), r_pers(_r_pers), 
    r_cov(_r_cov), r_vdw(_r_vdw), r_bonding(_r_bonding), r_sfil(_r_sfil), gaussians(_gaussians), 
    isotopes(_isotopes), isotope_count(_isotope_count),
    henke_data(_henke_data), henke_count(_henke_count),
    neutron_scattering(_neutron_scattering),
    Mr(0.0)
  {
    for( int i=0; i < isotope_count; i++ )
      Mr += isotopes[i].Mr*isotopes[i].W;
  }

  compd CalcFpFdp(double eV) const {
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
  double GetMr() const {  return Mr;  }
  inline bool operator >  (const cm_Element& ce) const {  return z >  ce.z;  }
  inline bool operator >= (const cm_Element& ce) const {  return z >= ce.z;  }
  inline bool operator <  (const cm_Element& ce) const {  return z <  ce.z;  }
  inline bool operator <= (const cm_Element& ce) const {  return z <= ce.z;  }
  inline bool operator == (const cm_Element& ce) const {  return z == ce.z;  }
  inline bool operator != (const cm_Element& ce) const {  return z != ce.z;  }
  inline bool operator >  (short _z) const {  return z >  _z;  }
  inline bool operator >= (short _z) const {  return z >= _z;  }
  inline bool operator <  (short _z) const {  return z <  _z;  }
  inline bool operator <= (short _z) const {  return z <= _z;  }
  inline bool operator == (short _z) const {  return z == _z;  }
  inline bool operator != (short _z) const {  return z != _z;  }
};

struct ElementCount {
  const cm_Element& element;
  double count;
  ElementCount(const cm_Element& _e, double _c) : element(_e), count(_c)  {}
  ElementCount(const ElementCount& e) : element(e.element), count(e.count)  {}
  ElementCount& operator += (double v) {
    count += v;
    return *this;
  }
  ElementCount& operator += (const ElementCount& v) {
#ifdef _DEBUG
    if( element != v.element )
      throw TInvalidArgumentException(__OlxSourceInfo, "elements must match");
#endif
    count += v.count;
    return *this;
  }
  bool operator == (const ElementCount& e) const {  return element == e.element;  }
};

typedef TPtrList<const cm_Element> ElementPList;
typedef TTypeList<ElementCount> ContentList;
typedef olxdict<const cm_Element*, double, TPrimitiveComparator> ElementRadii;
typedef olxdict<const cm_Element*, double, TPrimitiveComparator> ElementDict;

// sorts elemnt pointers by Z descending 
struct ElementPZSorter  {
  static int Compare(const cm_Element* s1, const cm_Element* s2)  {  return s2->z - s1->z;  }
};
// sorts elemnt pointers by symbol ascending
struct ElementPSymbolSorter  {
  static int Compare(const cm_Element* s1, const cm_Element* s2)  {  return s1->symbol.Compare(s2->symbol);  }
};


class XElementLib {
  static void ParseSimpleElementStr(const olxstr& str, TStrList& toks);
  static void ExpandShortcut(const olxstr& sh, ContentList& res, double cnt=1.0);
  // checks if p is an element symbol, will correctly distinguis "C " and "Cd"
  static bool IsShortcut(const olxstr& c)  {
    return c.Equalsi("Ph") || c.Equalsi("Cp") || c.Equalsi("Me") ||
      c.Equalsi("Et") || c.Equalsi("Bu") ||
      c.Equalsi("Py") || c.Equalsi("Tf");
  }
public:
  static double Wavelength2eV(double lambda) {
    static const double ev_angstrom  = 6626.0755 * 2.99792458 / 1.60217733;
    return ev_angstrom/lambda;
  }
  // and exact symbol as C or Cr is expected
  static cm_Element* FindBySymbol(const olxstr& symbol);
  // for compatibility with old routines...
  static cm_Element& GetByIndex(short);
  // a label might be passed as C1 or Cr2
  static cm_Element* FindBySymbolEx(const olxstr& symbol);
  // returns element with Z+1
  static cm_Element* NextZ(const cm_Element& elm);
  // returns element with Z-1
  static cm_Element* PrevZ(const cm_Element& elm);
  // extracts symbol from a label, like C for C1 or Cr for Cr2
  static const olxstr& ExtractSymbol(const olxstr& label)  {
    cm_Element* type = FindBySymbolEx(label);
    return type == NULL ? EmptyString : type->symbol;
  }
  // returns true if labels is a symbol
  static bool IsElement(const olxstr& label) {  return FindBySymbol(label) != NULL;  }
  // checks if p is a label starting from an element symbol
  static bool IsAtom(const olxstr& label)  {  return (FindBySymbolEx(label) != NULL);  }

  /* parses a string like C37H41P2BRhClO into a list of element names and their count,
  the provided list is being appended to and not cleared; returns a reference to provided
  ContentList*/
  static ContentList& ParseElementString(const olxstr& su, ContentList& cl);
  /* sorts the content list, so that C comes first, then H and then by Z descending;
  returns a reference to th provide ContentList */
  static ContentList& SortContentList(ContentList& cl);
};

EndXlibNamespace()
#endif
