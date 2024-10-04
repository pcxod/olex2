/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

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
    coh(_coh_re, _coh_im), xs(_xs)
  {}
};
struct cm_Isotope {
  double Mr, W;
  const cm_Neutron_Scattering* neutron_scattering;
};

struct cm_Gaussians {
  double a1, a2, a3, a4, b1, b2, b3, b4, c;
  cm_Gaussians() : a1(0), a2(0), a3(0), a4(0), b1(0), b2(0), b3(0), b4(0), c(0)
  {}
  // constructor, note that b values are inverted!
  cm_Gaussians(double _a1, double _a2, double _a3, double _a4,
    double _b1, double _b2, double _b3, double _b4, double _c) :
    a1(_a1), a2(_a2), a3(_a3), a4(_a4),
      b1(-_b1), b2(-_b2), b3(-_b3), b4(-_b4), c(_c)
  {}
  // copy constructor
  cm_Gaussians(const cm_Gaussians& g) :
    a1(g.a1), a2(g.a2), a3(g.a3), a4(g.a4),
      b1(g.b1), b2(g.b2), b3(g.b3), b4(g.b4), c(g.c)
  {}

  double calc_sq(double sqv) const {
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
  short index;
public:
  const olxstr symbol, name;
  uint32_t def_color;
  const short z, isotope_count, henke_count;
  double r_pers, r_cov, r_vdw, r_bonding, r_sfil;
  // custom radii are to be used by procedure which require custom radii...
  mutable double r_custom;
  const cm_Gaussians* gaussians;  // 9 elements = 4 gaussians + const
  const cm_Isotope* isotopes;
  const cm_Anomalous_Henke* henke_data;
  const cm_Neutron_Scattering* neutron_scattering;
  cm_Element(const char* _symbol, const char* _name,
    uint32_t _def_color, short _z, double _r_vdw, double _r_cov, double _r_pers,
    double _r_bonding, double _r_sfil, const cm_Gaussians* _gaussians,
    const cm_Isotope* _isotopes, short _isotope_count,
    const cm_Anomalous_Henke* _henke_data, short _henke_count,
    const cm_Neutron_Scattering* _neutron_scattering) :
    Mr(0), index(-1),
    symbol(_symbol), name(_name), def_color(_def_color), z(_z),
    isotope_count(_isotope_count), henke_count(_henke_count), r_pers(_r_pers),
    r_cov(_r_cov), r_vdw(_r_vdw), r_bonding(_r_bonding), r_sfil(_r_sfil),
    r_custom(0), gaussians(_gaussians), isotopes(_isotopes),
    henke_data(_henke_data), neutron_scattering(_neutron_scattering)
  {
    if (z == 1 && symbol.CharAt(0) == 'D') {
      Mr = isotopes[1].Mr;
    }
    else {
      for (int i = 0; i < isotope_count; i++) {
        Mr += isotopes[i].Mr * isotopes[i].W;
      }
    }
  }

  compd CalcFpFdp(double eV) const {
    if (henke_data == NULL) {
      throw TFunctionFailedException(__OlxSourceInfo,
        "undefined f\' f\" data");
    }
    if (eV < henke_data[0].energy || eV > henke_data[henke_count - 1].energy) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        "energy is out of range");
    }
    for (int i = 0; i < henke_count; i++) {
      if (henke_data[i].energy == eV)
        return compd(henke_data[i].fp, henke_data[i].fdp);
      if (henke_data[i].energy > eV) {
        double k = (eV - henke_data[i - 1].energy) /
          (henke_data[i].energy - henke_data[i - 1].energy);
        double fp = cm_Anomalous_Henke::Undefined;
        double fdp = cm_Anomalous_Henke::Undefined;
        if (henke_data[i - 1].fp != cm_Anomalous_Henke::Undefined &&
          henke_data[i].fp != cm_Anomalous_Henke::Undefined)
        {
          fp = henke_data[i - 1].fp + k*(henke_data[i].fp - henke_data[i - 1].fp);
        }
        if (henke_data[i - 1].fdp != cm_Anomalous_Henke::Undefined &&
          henke_data[i].fdp != cm_Anomalous_Henke::Undefined)
        {

          fdp = henke_data[i - 1].fdp + k*(henke_data[i].fdp - henke_data[i - 1].fdp);
        }
        return compd(fp, fdp);
      }
    }
    return compd();
  }
  double GetMr() const {  return Mr;  }
  bool operator >  (const cm_Element& ce) const {  return z >  ce.z;  }
  bool operator >= (const cm_Element& ce) const {  return z >= ce.z;  }
  bool operator <  (const cm_Element& ce) const {  return z <  ce.z;  }
  bool operator <= (const cm_Element& ce) const {  return z <= ce.z;  }
  bool operator == (const cm_Element& ce) const {  return z == ce.z;  }
  bool operator != (const cm_Element& ce) const {  return z != ce.z;  }
  bool operator >  (short _z) const {  return z >  _z;  }
  bool operator >= (short _z) const {  return z >= _z;  }
  bool operator <  (short _z) const {  return z <  _z;  }
  bool operator <= (short _z) const {  return z <= _z;  }
  bool operator == (short _z) const {  return z == _z;  }
  bool operator != (short _z) const {  return z != _z;  }
  int Compare(const cm_Element &e) const { return z - e.z; }
  const olxstr &GetSymbol() const { return symbol; }
  
  olxstr GetChargedLabel(int charge) const {
    if (charge == 0) {
      return symbol;
    }
    olxstr rv = symbol;
    rv << olx_sign_char(charge);
    if (olx_abs(charge) > 1) {
      rv << olx_abs(charge);
    }
    return rv;
  }
  
  short GetIndex() const { return index; }
  friend class XElementLib;
};

struct ElementCount {
  const cm_Element* element;
  double count;
  int charge;
  ElementCount()
    : element(0), count(0), charge(0)
  {
    throw TNotImplementedException(__OlxSourceInfo);
  }
  ElementCount(const cm_Element& _e, double _c, int ch)
    : element(&_e), count(_c), charge(ch)
  {}
  ElementCount(const cm_Element& _e, double _c)
    : element(&_e), count(_c), charge(0)
  {}
  ElementCount(const ElementCount& e)
    : element(e.element), count(e.count), charge(e.charge)
  {}
  ElementCount& operator += (double v) {
    count += v;
    return *this;
  }
  ElementCount& operator += (const ElementCount& v) {
#ifdef _DEBUG
    if (element != v.element || v.charge != charge) {
      throw TInvalidArgumentException(__OlxSourceInfo, "elements must match");
    }
#endif
    count += v.count;
    return *this;
  }
  bool operator == (const ElementCount& e) const {
    return element == e.element && charge == e.charge;
  }
  
  int hashCode() const {
    return element->GetIndex() + charge * 200;
  }

  olxstr ToString() const {
    return ToString(*element, count, charge);
  }

  static olxstr ToString(const cm_Element &element, double count, int charge) {
    olxstr rv;
    if (charge != 0) {
      rv << '(' << element.symbol << olx_sign_char(charge);
      int ch = olx_abs(charge);
      if (ch > 1) {
        rv << ch;
      }
      rv << ')';
    }
    else {
      rv = element.symbol;
    }
    return rv << olxstr::FormatFloat(3, count).TrimFloat();
  }
};

typedef TPtrList<const cm_Element> ElementPList;
typedef sorted::PointerComparable<const cm_Element>
  SortedElementPList;
typedef ConstSortedPointerList<const cm_Element, TComparableComparator>
  ConstSortedElementPList;
typedef TTypeList<ElementCount> ContentList;
typedef olxdict<const cm_Element*, double, TPointerComparator> ElementRadii;
typedef olxdict<const cm_Element*, double, TPointerComparator> ElementDict;

// sorts element by Z descending
struct ElementZSorter  {
  template <class item_t>
  static int Compare(const item_t &s1, const item_t &s2)  {
    return olx_ref::get(s2).z - olx_ref::get(s1).z;
  }
};
// sorts element by symbol ascending
struct ElementSymbolSorter {
  template <class item_t>
  static int Compare(const item_t &s1, const item_t &s2) {
    return olx_ref::get(s1).symbol.Compare(olx_ref::get(s2).symbol);
  }
};


class XElementLib {
  static void ParseSimpleElementStr(const olxstr& str, TStrList& toks);
  static void ExpandShortcut(const olxstr& sh, ContentList& res, double cnt=1.0);
  // checks if p is an element symbol, will correctly distinguis "C " and "Cd"
  static bool IsShortcut(const olxstr& c) {
    return c.Equalsi("Ph") || c.Equalsi("Cp") || c.Equalsi("Me") ||
      c.Equalsi("Et") || c.Equalsi("Bu") ||
      c.Equalsi("Py") || c.Equalsi("Tf");
  }
  static TPtrList<cm_Element> &GetElementList();
  static olxstr_dict<cm_Element *, true> &GetElementDict();
public:
  static double Wavelength2eV(double lambda) {
    static const double ev_angstrom  = 6626.0755 * 2.99792458 / 1.60217733;
    return ev_angstrom/lambda;
  }
  // and exact symbol as C or Cr is expected
  static cm_Element* FindBySymbol(const olxstr& symbol, int *charge=0);
  // for compatibility with old routines...
  static cm_Element& GetByIndex(short);
  // finds an element by Z
  static cm_Element* FindByZ(short);
  // a label might be passed as C1 or Cr2
  static cm_Element* FindBySymbolEx(const olxstr& symbol);
  // returns element with Z+1
  static cm_Element* NextZ(const cm_Element& elm);
  // returns element with Z-1
  static cm_Element* PrevZ(const cm_Element& elm);
  // extracts symbol from a label, like C for C1 or Cr for Cr2
  static const olxstr& ExtractSymbol(const olxstr& label)  {
    cm_Element* type = FindBySymbolEx(label);
    return type == 0 ? EmptyString() : type->symbol;
  }
  // returns true if labels is a symbol
  static bool IsElement(const olxstr& label, int *charge=0) {
    return FindBySymbol(label, charge) != 0;
  }
  // checks if p is a label starting from an element symbol
  static bool IsAtom(const olxstr& label)  {
    return (FindBySymbolEx(label) != 0);
  }

  /* parses a string like C37H41P2BRhClO into a list of element names and their
  count, the provided list is being appended to and not cleared; returns a
  reference to provided ContentList
  */
  static ContentList& ParseElementString(const olxstr& su, ContentList& cl);
  static ContentList::const_list_type ParseElementString(const olxstr& su) {
    ContentList cl;
    return ParseElementString(su, cl);
  }
  /* sorts the content list, so that C comes first, then H and then by Z
  descending; returns a reference to th provide ContentList
  */
  static ContentList& SortContentList(ContentList& cl);
  /* merges charges charged entities, as the resuls the charge becomes
  meaningless
  */
  static ContentList& MergeCharges(ContentList& cl);

  // checks if a string is a shortcut for element group
  static bool IsElementShortcut(const olxstr& c)  {
    return c.Equalsi('M') || c.Equalsi('X');
  }
  // checks if given element is in main group 1-8
  static bool IsGroup(int group, const cm_Element &e)  {
    if( group < 3 )  {
      return e.z == (group+2) || e.z == (group+10) || e.z == (group+18) ||
        e.z == (group+36) || e.z == (group+54) || e.z == (group+86);
    }
    if( group == 8 && e.z == 2 )
      return true;
    return e.z == (group+2) || e.z == (group+10) || e.z == (group+28) ||
      e.z == (group+46) || e.z == (group+78) || e.z == (group+110);
  }
  /* checks if given element is in sub-group 1-10, Lanthanised are
  considered to be in group #3 */
  static bool IsTtransitionalGroup(int group, const cm_Element &e) {
    if( e.z >= 21 && e.z <= 30 )  { // Sc->Zn
      int z = e.z > 28 ? e.z-10 : e.z;
      return z == (group+18);
    }
    else if( e.z >= 39 && e.z <= 48 )  { //Y->Cd
      int z = e.z > 46 ? e.z-10 : e.z;
      return z == (group+36);
    }
    else if( e.z >= 57 && e.z <= 80 )  { //La->Hg
      if( e.z < 72 ) // Lanthanides
        return group == 3;
      int z = e.z > 78 ? e.z-10 : e.z;
      return z == (group+68);
    }
    return false;
  }
  // Sc->Zn, Y->Cd, Hf->Hg, lanthanides excluded
  static bool IsTransitionalMetal(const cm_Element &e)  {
    return (e.z >= 21 && e.z <= 30) ||
      (e.z >= 39 && e.z <= 48) ||
      (e.z >= 72 && e.z <= 80);
  }
  // Al, Ga, In, Sn, Tl, Pb, Bi
  static bool IsPostTransitionalMetal(const cm_Element &e)  {
    return e.z == 13 || e.z == 31 || e.z == 49 || e.z == 50 ||
      e.z == 81 || e.z == 82 || e.z == 83;
  }
  // B, Si, Ge, As, Sb, Te
  static bool IsMetalloid(const cm_Element &e)  {
    return e.z == 5 || e.z == 14 || e.z == 32 || e.z == 33 ||
      e.z == 51 || e.z == 52;
  }

  // finds next element which belongs to the given group
  static cm_Element *NextGroup(int group, const cm_Element *e);
  // finds previous element which belongs to the given group
  static cm_Element *PrevGroup(int group, const cm_Element *e);

  static bool IsMetal(const cm_Element &e) {
    return
      IsTransitionalMetal(e) ||
      IsPostTransitionalMetal(e) ||
      IsLanthanide(e) ||
      IsActinide(e) ||
      IsGroup1(e) || IsGroup2(e) ||
      (e.z > 5 && IsGroup3(e)); // after B
  }
  // Li->Fr
  static bool IsGroup1(const cm_Element &e) {  return IsGroup(1, e);  }
  // Be->Ra
  static bool IsGroup2(const cm_Element &e) {  return IsGroup(2, e);  }
  // B->Tl
  static bool IsGroup3(const cm_Element &e) {  return IsGroup(3, e);  }
  // C->Pb
  static bool IsGroup4(const cm_Element &e) {  return IsGroup(4, e);  }
  // N->Bi
  static bool IsGroup5(const cm_Element &e) {  return IsGroup(5, e);  }
  // O->Po
  static bool IsGroup6(const cm_Element &e) {  return IsGroup(6, e);  }
  static bool IsChalcogen(const cm_Element &e)  {  return IsGroup6(e);  }
  // F->At
  static bool IsGroup7(const cm_Element &e) {  return IsGroup(7, e);  }
  static bool IsHalogen(const cm_Element &e)  {  return IsGroup7(e);  }
  // La->Lu
  static bool IsLanthanide(const cm_Element &e)  {
    return e.z >= 57 && e.z <= 71;
  }
  // Ac->Lr
  static bool IsActinide(const cm_Element &e)  {
    return e.z >= 89 && e.z <= 103;
  }
  // He->Rn
  static bool IsGroup8(const cm_Element &e) {  return IsGroup(8, e);  }

};

EndXlibNamespace()
#endif
