/******************************************************************************
* Copyright (c) 2004-2014 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xlib_solid_angles_H
#define __olx_xlib_solid_angles_H
#include "lattice.h"
#include "xatom.h"

BeginGxlNamespace()
/* 'generic' point analyser - */
class APointAnalyser : public IEObject {
protected:
  virtual void ToDataItem_(TDataItem &di) const = 0;
public:
  virtual ~APointAnalyser()  {}
  virtual uint32_t Analyse(vec3f &p) = 0;
  virtual void SetDryRun(bool v) = 0;
  virtual void GetReady() = 0;
  virtual olxstr GetIdString() const = 0;
  void ToDataItem(TDataItem &di) const;
  static APointAnalyser *FromDataItem(const TDataItem &di);
  // object factory
  static olxstr_dict<APointAnalyser *(*)(const TDataItem &di), false> &
    Registry()
  {
    static olxstr_dict<APointAnalyser *(*)(const TDataItem &di), false> r;
    return r;
  }
};

/* Used for the solid angle analysis as described here:
Guzei, I.A., Wendt, M.Dalton Trans., 2006, 3991–3999.
*/
struct PointAnalyser : public APointAnalyser {
  const TSAtom &center;
  TArrayList<uint32_t> colors;
  bool emboss, dry_run, clone;
  uint8_t alpha;
  olx_critical_section &cs;
  ElementRadii radii;
protected:
  PointAnalyser(const PointAnalyser &pa)
    : center(pa.center), colors(pa.colors),
    emboss(pa.emboss), dry_run(pa.dry_run), clone(true), alpha(pa.alpha),
    cs(pa.cs), areas(pa.areas)
  {}
public:
  PointAnalyser(TXAtom &c);
  ~PointAnalyser() {
    if (!clone) {
      delete &areas;
      delete &cs;
    }
  }
  void SetDryRun(bool v) { dry_run = v; }
  uint32_t Analyse(vec3f &p_);
  olxstr GetIdString() const {
    return IdString();
  }
  void GetReady();
  static olxstr IdString() {
    static olxstr name = "SolidAngles";
    return name;
  }
  static void Register() {
    APointAnalyser::Registry().Add(IdString(),
      &PointAnalyser::Load);
  }
  void ToDataItem_(TDataItem &di) const;
  IEObject *Replicate() const {
    return new PointAnalyser(*this);
  }
  static APointAnalyser *Load(const TDataItem &di);
  olxstr_dict<size_t, false> &areas;
};

EndGxlNamespace()
#endif
