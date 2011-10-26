/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xlib_analysis_H
#define __olx_xlib_analysis_H
#include "lattice.h"
#include "symmlib.h"
#include "xapp.h"
#include "auto.h"
#include "filetree.h"

BeginXlibNamespace()
class Analysis {
  static int peak_sort(const TCAtom *a1, const TCAtom *a2) {
    return olx_cmp(a2->GetQPeak(), a1->GetQPeak());
  }
  struct AC_Sort {
    vec3d center;
    AC_Sort(const vec3d &_center) : center(_center) {}
    int Compare(
      const AnAssociation2<const TCAtom *, vec3d> *a1,
      const AnAssociation2<const TCAtom *, vec3d> *a2) const
    {
      return olx_cmp(
        center.QDistanceTo(a1->GetB()),
        center.QDistanceTo(a2->GetB()));
    }
  };
  static int hr_sort(
    const AnAssociation2<double, olxstr> *a1,
    const AnAssociation2<double, olxstr> *a2)
  {
    return olx_cmp(a1->GetA(), a2->GetA());
  }
public:
  static bool trim_18(TAsymmUnit &au) {
    size_t mult = (au.MatrixCount()+1)*
      TCLattice::GetLattMultiplier(au.GetLatt());
    size_t mn = olx_round(au.CalcCellVolume()/(mult*18));
    if (au.AtomCount() <= mn)
      return false;
    TCAtomPList atoms, peaks;
    for (size_t i=0; i < au.AtomCount(); i++) {
      TCAtom &a = au.GetAtom(i);
      if (a.GetType() == iQPeakZ)
        peaks.Add(a);
      else
        atoms.Add(a);
    }
    peaks.QuickSorter.SortSF(peaks, peak_sort);
    TTypeList<TCAtomPList> peak_ranges;
    if (!peaks.IsEmpty())
      peak_ranges.AddNew().Add(peaks.GetLast());
    for (size_t i=peaks.Count()-2; i != InvalidIndex; i--) {
      TCAtomPList &range = peak_ranges.GetLast();
      TCAtom *cmpr = range.GetLast();
      if (peaks[i]->GetQPeak()/cmpr->GetQPeak() < 1.25 &&
          peaks[i]->GetQPeak()/range[0]->GetQPeak() < 2)
        range.Add(peaks[i]);
      else
        peak_ranges.AddNew().Add(peaks[i]);
    }
    if (mn > atoms.Count()) {
      size_t i = peak_ranges.Count();
      while (--i !=InvalidIndex && mn > atoms.Count())
        atoms.AddList(peak_ranges[i]);
      for (; i != InvalidIndex; i--) {
        TCAtomPList &range = peak_ranges[i];
        for (size_t j=0; j < range.Count(); j++)
          range[j]->SetDeleted(true);
      }
    }
    else {
      for (size_t i=mn; i < atoms.Count(); i++)
        atoms[i]->SetDeleted(true);
    }
    if (atoms.Count() > mn*1.25) {
      for (size_t i=mn; i < atoms.Count(); i++)
        atoms[i]->SetDeleted(true);
    }
    return true;
  }

  static double find_scale(TLattice &latt) {
    ConstTypeList<TAutoDB::TAnalysisResult> ares =
      TAutoDB::GetInstance().AnalyseStructure(latt);
    double res = 0, wght = 0;;
    for (size_t i=0; i < ares.Count(); i++)  {
      if (ares[i].atom->GetType() == iQPeakZ) {
        TTypeList<TAutoDB::Type> hits;
        if(!ares[i].list3.IsEmpty())
          hits = ares[i].list3;
        else if(!ares[i].list2.IsEmpty())
          hits = ares[i].list2;
        else  {
          hits = ares[i].list1;
          hits.AddList(ares[i].enforced);
        }
        size_t m = olx_min(2, hits.Count());
        for (size_t j=0; j < m; j++) {
          res += hits[j].type.z*hits[j].fom/ares[i].atom->GetQPeak();
          wght += hits[j].fom;
        }
      }
    }
    if (wght!=0)
      res /= wght;
    return res;
  }

  static void funTrim(const TStrObjList& Params, TMacroError& E)  {
    E.SetRetVal(trim_18(TXApp::GetInstance().XFile().GetAsymmUnit()));
  }

  static void funFindScale(const TStrObjList& Params, TMacroError& E)  {
    bool apply = Params.IsEmpty() ? false : Params[0].ToBool();
    TLattice &latt = TXApp::GetInstance().XFile().GetLattice();
    double scale = find_scale(latt);
    if (scale > 0) {
      for (size_t i=0; i < latt.GetObjects().atoms.Count(); i++) {
        TSAtom &a = latt.GetObjects().atoms[i];
        if (a.GetType() == iQPeakZ && apply) {
          int z = olx_round(a.CAtom().GetQPeak()*scale);
          cm_Element *tp = XElementLib::FindByZ(z),
             *tp1 = NULL;
          // find previous halogen vs noble gas or alkaline metal
          if (tp != NULL) {
            if (XElementLib::IsGroup8(*tp) ||
                XElementLib::IsGroup1(*tp) ||
                XElementLib::IsGroup2(*tp))
            {
              tp1 = XElementLib::PrevGroup(7, tp);
            }
            a.CAtom().SetType(tp1 == NULL ? *tp : *tp1);
          }
        }
      }
    }
    E.SetRetVal(scale);
  }

  static TLibrary *ExportLibrary(const olxstr& name="analysis")  {
    TLibrary* lib = new TLibrary(name);
    lib->RegisterStaticFunction(
      new TStaticFunction(&Analysis::funTrim, "Trim", fpNone|fpOne,
      "Trims the size of the assymetric unit according to the 18 A^3 rule."
      "Returns true if any atoms were deleted")
    );
    lib->RegisterStaticFunction(
      new TStaticFunction(&Analysis::funFindScale, "Scale", fpNone|fpOne,
      "Scales the Q-peaks according to found fragments."
      "Returns the scale or 0")
    );
    return lib;
  }
};
EndXlibNamespace()
#endif
