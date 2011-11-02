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
namespace olx_analysis {

struct peaks {
  static int peak_sort(const TCAtom *a1, const TCAtom *a2) {
    return olx_cmp(a2->GetQPeak(), a1->GetQPeak());
  }
  static double mean(const TCAtomPList &peaks);
  static ConstPtrList<TCAtom> extract(TAsymmUnit &au,
    bool *all_peaks=NULL);
  struct range {
  protected:
    mutable double mean;
  public:
    TCAtomPList peaks;
    range() : mean(0) {}
    double get_mean() const {
      return (mean != 0) ? mean : (mean=peaks::mean(peaks));
    }
    void delete_all();
  };
  struct result {
    bool only_peaks;
    size_t peak_count;
    double mean_peak;
    TTypeList<range> peak_ranges;
  };
  static ConstTypeList<range> analyse(const TCAtomPList &peaks);
  static result analyse_full(TAsymmUnit &au);
  static TCAtomPList &proximity_clean(TCAtomPList &peaks);
};

class Analysis {
  static int hr_sort(
    const AnAssociation2<double, olxstr> *a1,
    const AnAssociation2<double, olxstr> *a2)
  {
    return olx_cmp(a1->GetA(), a2->GetA());
  }
public:
  static bool trim_18(TAsymmUnit &au);

  static double find_scale(TLattice &latt);

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
}; // end namespace olx_analysis
EndXlibNamespace()
#endif
