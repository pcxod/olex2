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

BeginXlibNamespace()
class Analysis {
  static int peak_sort(const TCAtom *a1, const TCAtom *a2) {
    return olx_cmp(a2->GetQPeak(), a1->GetQPeak());
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
    size_t j=0;
    while (atoms.Count() < mn && j < peaks.Count())
      atoms.Add(peaks[j++]); 
    for (size_t i=mn; i < atoms.Count(); i++)
      atoms[i]->SetDeleted(true);
    for (size_t i=j; i < peaks.Count(); i++)
      peaks[i]->SetDeleted(true);
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
          hits.AddListC(ares[i].enforced);
        }
        for (size_t j=0; j < hits.Count(); j++) {
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
    TLattice &latt = TXApp::GetInstance().XFile().GetLattice();
    double scale = find_scale(latt);
    if (scale > 0) {
      for (size_t i=0; i < latt.GetObjects().atoms.Count(); i++) {
        TSAtom &a = latt.GetObjects().atoms[i];
        if (a.GetType() == iQPeakZ) {
          int z = olx_round(a.CAtom().GetQPeak()*scale);
          cm_Element *tp = XElementLib::FindByZ(z);
          if (tp != NULL)
            a.CAtom().SetType(*tp);
        }
      }
    }
    E.SetRetVal(scale);
  }

  static TLibrary *ExportLibrary(const olxstr& name="analysis")  {
    TLibrary* lib = new TLibrary(name);
    lib->RegisterStaticFunction(
      new TStaticFunction(&Analysis::funTrim, "Trim", fpNone|fpOne,
      "Trims the size of the assymetric unit according to thw 18 A^3 rule."
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
