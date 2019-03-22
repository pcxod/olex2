/******************************************************************************
* Copyright (c) 2004-2014 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_x_scatterer_H
#define __olx_x_scatterer_H
#include "chemdata.h"
#ifdef _PYTHON
#include "pyext.h"
#endif

BeginXlibNamespace()
/* scatterer wrapping to allow user defined values. */
class XScatterer {
  cm_Gaussians gaussians;
  double mu, wt, r;
  compd fpfdp;
  const cm_Element* source;
  olxstr Label;
  short set_items;
public:
  static const short
    setGaussian   = 0x0001,
    setDispersion = 0x0002,
    setMu         = 0x0004,
    setR          = 0x0008,
    setWt         = 0x0010,
    setAll = setGaussian|setDispersion|setMu|setR|setWt;
  // creates a dummy scatterer
  XScatterer(const olxstr& lbl)
    : mu(0), wt(0), r(0), source(NULL), Label(lbl), set_items(0)
  {}
  // creates scatterer from the library
  XScatterer(const cm_Element& src, double energy)
    : mu(0), set_items(0)
  {
    SetSource(src, energy);
  }
  /* searches for the scatterer in the library and initialises data. If
  scatterer no found, throws exception
  */
  XScatterer(const olxstr& lbl, double energy) : mu(0), set_items(0) {
    cm_Element* src = XElementLib::FindBySymbol(lbl);
    if (src == NULL) {
      throw TInvalidArgumentException(__OlxSourceInfo,
        olxstr("unknown scatterer symbol: ").quote() << lbl);
    }
    SetSource(*src, energy);
  }
  // copy constructor
  XScatterer(const XScatterer& sc)
    : gaussians(sc.gaussians),
    mu(sc.mu),
    wt(sc.wt),
    r(sc.r),
    fpfdp(sc.fpfdp),
    source(sc.source),
    Label(sc.Label),
    set_items(sc.set_items)
  {}
  // initialises data from the provided library element
  void SetSource(const cm_Element& src, double energy);
  // updates a scetterer info
  void Merge(const XScatterer& sc);
  XScatterer& operator = (const XScatterer& sc);
  // sets custom fp and fdp
  void SetFpFdp(const compd& v) {
    fpfdp = v;
    set_items |= setDispersion;
  }
  bool IsNeutron() const {
    if (IsSet(setGaussian)) {
      return gaussians.a1 == 0 && gaussians.b1 == 0 &&
        gaussians.a2 == 0 && gaussians.b2 == 0 &&
        gaussians.a3 == 0 && gaussians.b3 == 0 &&
        gaussians.a4 == 0 && gaussians.b4 == 0;
    }
    return false;
  }
  const compd& GetFpFdp() const {  return fpfdp;  }
  // sets custom bonding radius
  void SetR(double v)  {
    r = v;
    set_items |= setR;
  }
  double GetR() const {  return r;  }
  // sets custom molecular weight
  void SetWeight(double v)  {
    wt = v;
    set_items |= setWt;
  }
  double GetWeight() const {  return wt;  }
  // sets custom adsorption coefficient
  void SetMu(double v)  {
    mu = v;
    set_items |= setMu;
  }
  double GetMu() const {  return mu;  }
  // sets custom gaussians
  void SetGaussians(const cm_Gaussians& g)  {
    gaussians = g;
    set_items |= setGaussian;
  }
  bool IsSet(short what) const { return (set_items&what) != 0; }
  DefPropC(olxstr, Label)
    // return an INS file string representation
    olxstr ToInsString() const;
  double calc_sq(double sqv) const {
    return gaussians.calc_sq(sqv);
  }
  compd calc_sq_anomalous(double sqv) const {
    return compd(gaussians.calc_sq(sqv) + fpfdp.GetRe(), fpfdp.GetIm());
  }
  bool IsSFAC() const {  return (set_items == setAll);  }
  bool IsDISP() const {  return (set_items == setDispersion ||
    set_items == (setDispersion|setMu));  }
  void ToDataItem(TDataItem& _di) const;
  void FromDataItem(const TDataItem& di);
#ifdef _PYTHON
  PyObject* PyExport();
#endif
  static int ChargeFromLabel(const olxstr &l, const cm_Element *e_ = 0);
  /* Normalises a charged label from O2- to O-2 etc
  */
  static olxstr NormaliseCharge(const olxstr &l, const cm_Element *e_ = 0);
};

EndXlibNamespace()

#endif
