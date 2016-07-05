/******************************************************************************
* Copyright (c) 2004-2014 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "xscatterer.h"

void XScatterer::SetSource(const cm_Element& src, double energy) {
  if (src.gaussians == NULL) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      "given scatterer is only partially initialised");
  }
  gaussians = *src.gaussians;
  Label = src.symbol;
  wt = src.GetMr();
  r = src.r_bonding;
  fpfdp = src.CalcFpFdp(energy) - src.z;
  source = &src;
  set_items = setAll^setMu;
}
//.............................................................................
void XScatterer::Merge(const XScatterer& sc) {
  if ((sc.set_items & setGaussian))
    SetGaussians(sc.gaussians);
  if ((sc.set_items & setDispersion))
    SetFpFdp(sc.fpfdp);
  if ((sc.set_items & setMu))
    SetMu(sc.mu);
  if ((sc.set_items & setR))
    SetR(sc.r);
  if ((sc.set_items & setWt))
    SetWeight(sc.wt);
}
//.............................................................................
XScatterer& XScatterer::operator = (const XScatterer& sc) {
  gaussians = sc.gaussians;
  fpfdp = sc.fpfdp;
  mu = sc.mu;
  r = sc.r;
  wt = sc.wt;
  set_items = sc.set_items;
  return *this;
}
//.............................................................................
olxstr XScatterer::ToInsString() const {
  if (set_items == setDispersion || set_items == (setDispersion | setMu)) {
    olxstr rv("DISP", 80);
    rv.stream(' ') << Label <<
      olxstr::FormatFloat(3, fpfdp.GetRe()).TrimFloat() <<
      olxstr::FormatFloat(3, fpfdp.GetIm()).TrimFloat();
    if ((set_items & setMu) != 0)
      rv << ' ' << olxstr::FormatFloat(3, mu).TrimFloat();
    return rv;
  }
  else if (set_items == setAll) {
    olxstr rv("SFAC ", 100);
    rv.stream(' ') << Label << olxstr::FormatFloat(3, gaussians.a1) <<
      olxstr::FormatFloat(3, -gaussians.b1).TrimFloat() <<
      olxstr::FormatFloat(3, gaussians.a2).TrimFloat() <<
      olxstr::FormatFloat(3, -gaussians.b2).TrimFloat() <<
      olxstr::FormatFloat(3, gaussians.a3).TrimFloat() <<
      olxstr::FormatFloat(3, -gaussians.b3).TrimFloat() <<
      olxstr::FormatFloat(3, gaussians.a4).TrimFloat() <<
      olxstr::FormatFloat(3, -gaussians.b4).TrimFloat() <<
      olxstr::FormatFloat(3, gaussians.c).TrimFloat() <<
      olxstr::FormatFloat(3, fpfdp.GetRe()).TrimFloat() <<
      olxstr::FormatFloat(3, fpfdp.GetIm()).TrimFloat() <<
      olxstr::FormatFloat(3, mu).TrimFloat() <<
      olxstr::FormatFloat(3, r).TrimFloat() <<
      olxstr::FormatFloat(3, wt).TrimFloat();
    return rv;
  }
  throw TInvalidArgumentException(__OlxSourceInfo,
    "failed to produce INS string");
}
//.............................................................................
void XScatterer::ToDataItem(TDataItem& _di) const {
  TDataItem& di = _di.AddItem(Label, set_items);
  olxstr data;
  if ((set_items & setGaussian) != 0)  {
    data.stream(' ') << gaussians.a1 << gaussians.a2 << gaussians.a3
      << gaussians.a4 << gaussians.b1 << gaussians.b2 << gaussians.b3
      << gaussians.b4 << gaussians.c;
  }
  if ((set_items & setDispersion) != 0)  {
    if (!data.IsEmpty())  data << ' ';
    data << fpfdp.GetRe() << ' ' << fpfdp.GetIm();
  }
  if ((set_items & setMu) != 0)  {
    if (!data.IsEmpty())  data << ' ';
    data << mu;
  }
  if ((set_items & setR) != 0)  {
    if (!data.IsEmpty())  data << ' ';
    data << r;
  }
  if ((set_items & setWt) != 0)  {
    if (!data.IsEmpty())  data << ' ';
    data << wt;
  }
  di.AddField("data", data);
}
//.............................................................................
//.............................................................................
void XScatterer::FromDataItem(const TDataItem& di) {
  Label = di.GetName();
  set_items = di.GetValue().ToInt();
  const TStrList toks(di.GetFieldByName("data"), ' ');
  size_t ind = 0;
  if ((set_items & setGaussian) != 0) {
    gaussians.a1 = toks[ind++].ToDouble();
    gaussians.a2 = toks[ind++].ToDouble();
    gaussians.a4 = toks[ind++].ToDouble();
    gaussians.a4 = toks[ind++].ToDouble();
    gaussians.b1 = toks[ind++].ToDouble();
    gaussians.b2 = toks[ind++].ToDouble();
    gaussians.b3 = toks[ind++].ToDouble();
    gaussians.b4 = toks[ind++].ToDouble();
    gaussians.c = toks[ind++].ToDouble();
  }
  if ((set_items & setDispersion) != 0) {
    fpfdp.Re() = toks[ind++].ToDouble();
    fpfdp.Im() = toks[ind++].ToDouble();
  }
  if ((set_items & setMu) != 0)
    mu = toks[ind++].ToDouble();
  if ((set_items & setR) != 0)
    r = toks[ind++].ToDouble();
  if ((set_items & setWt) != 0)
    wt = toks[ind++].ToDouble();
  source = XElementLib::FindBySymbol(Label);
}
//.............................................................................
#ifdef _PYTHON
PyObject* XScatterer::PyExport() {
  PyObject* main = PyDict_New();
  if ((set_items & setGaussian) != 0) {
    PythonExt::SetDictItem(main, "gaussian",
      Py_BuildValue("(dddd)(dddd)d", gaussians.a1, gaussians.a2,
      gaussians.a3, gaussians.a4, gaussians.b1, gaussians.b2, gaussians.b3,
      gaussians.b4, gaussians.c));
  }
  if ((set_items & setDispersion) != 0) {
    PythonExt::SetDictItem(main, "fpfdp",
      Py_BuildValue("(dd)", fpfdp.GetRe(), fpfdp.GetIm()));
  }
  if ((set_items & setMu) != 0)
    PythonExt::SetDictItem(main, "mu", Py_BuildValue("d", mu));
  if ((set_items & setR) != 0)
    PythonExt::SetDictItem(main, "r", Py_BuildValue("d", r));
  if ((set_items & setWt) != 0)
    PythonExt::SetDictItem(main, "wt", Py_BuildValue("d", wt));
  return main;
}
#endif
//.............................................................................
int XScatterer::ChargeFromLabel(const olxstr &l, const cm_Element *e_) {
  const cm_Element *e = (e_ == 0 ? XElementLib::FindBySymbolEx(l) : e_);
  if (e == 0) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      olxstr("element symbol: ").quote() << l);
  }
  olxstr cs = l.SubStringFrom(e->symbol.Length());
  if (cs.IsEmpty()) {
    return 0;
  }
  bool st = olxstr::o_isoneof(cs.CharAt(0), '+', '-'),
    ed = olxstr::o_isoneof(cs.GetLast(), '+', '-');
  if (!st && !ed) {
    return 0;
  }
  if (st) {
    int s = (cs.CharAt(0) == '-' ? -1 : 1);
    return (cs.Length() == 1 ? s : s*cs.SubStringFrom(1).ToInt());
  }
  else {
    int s = (cs.GetLast() == '-' ? -1 : 1);
    return (cs.Length() == 1 ? s : s*cs.SubStringFrom(0,1).ToInt());
  }
}
//.............................................................................
olxstr XScatterer::NormaliseCharge(const olxstr &l, const cm_Element *e_) {
  const cm_Element *e = (e_ == 0 ? XElementLib::FindBySymbolEx(l) : e_);
  if (e == 0) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      olxstr("element symbol: ").quote() << l);
  }
  olxstr cs = l.SubStringFrom(e->symbol.Length());
  if (cs.IsEmpty()) {
    return l;
  }
  bool st = olxstr::o_isoneof(cs.CharAt(0), '+', '-');
  if (st) {
    return l;
  }
  bool ed = olxstr::o_isoneof(cs.GetLast(), '+', '-');
  if (st) {
    int s = (cs.CharAt(0) == '-' ? -1 : 1);
    if (cs.Length() == 1) {
      return l;
    }
    if (cs.SubStringFrom(1) == '1') {
      return olxstr(e->symbol) << (s < 0 ? '-' : '+');
    }
    return l;
  }
  else if (ed) {
    int s = (cs.GetLast() == '-' ? -1 : 1);
    int ch = cs.SubStringFrom(0, 1).ToInt();
    if (ch == 1) {
      return olxstr(e->symbol) << (s < 0 ? '-' : '+');
    }
    return olxstr(e->symbol) << (s < 0 ? '-' : '+') << ch;
  }
  return l;
}
//.............................................................................
