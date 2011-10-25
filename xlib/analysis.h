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

  static double estimate_r(TLattice &latt, const TSAtom &a) {
    TTypeList<AnAssociation2<const TCAtom *, vec3d> > res;
    latt.GetUnitCell().FindInRangeAC(a.ccrd(), 3.0, res);
    res.QuickSorter.Sort(res, AC_Sort(a.crd()));
    double r = 0, ref_r = 0;
    size_t cnt = 0;
    if ( res.Count() > 1 ) {
      r = ref_r = (res[1].GetB()-a.crd()).Length();
      cnt++;
    }
    for (size_t i=2; i < res.Count(); i++) {
      double d = (res[i].GetB()-a.crd()).Length();
      if (d > 1.1*ref_r) break;
      r += d;
      cnt++;
      ref_r = d;
    }
    if (cnt > 0)
      r /= cnt;
    return r;
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
            a.CAtom().SetType( tp1 == NULL ? *tp : *tp1);
          }
        }
      }
    }
    E.SetRetVal(scale);
  }

  static void funEstomateR(const TStrObjList& Params, TMacroError& E)  {
    TXApp &app = TXApp::GetInstance();
    if (Params.Count()==1 && TEFile::IsDir(Params[0])) {
      TFileTree ft(Params[0]);
      ft.Expand();
      TStrList files;
      ft.GetRoot().ListFiles(files, "*.cif");
      TTypeList<TArrayList<int> > res(120, false);
      for (size_t i=0; i < files.Count(); i++) {
        try {
          app.Update();
          app.XFile().LoadFromFile(files[i]);
          TLattice &l = app.XFile().GetLattice();
          for (size_t j=0; j < l.GetObjects().atoms.Count(); j++) {
            TSAtom &a = l.GetObjects().atoms[j];
            if (a.GetType().z < 2)
              a.CAtom().SetDeleted(true);
          }
          for (size_t j=0; j < l.GetObjects().atoms.Count(); j++) {
            TSAtom &a = l.GetObjects().atoms[j];
            if (a.IsDeleted()) continue;
            if (res.IsNull(a.GetType().index)) {
              res.Set(a.GetType().index, new TArrayList<int>(326));
              for (size_t k=0; k < res[a.GetType().index].Count(); k++)
                res[a.GetType().index][k] = 0;
            }
            TTypeList<AnAssociation2<const TCAtom *, vec3d> > envi;
            l.GetUnitCell().FindInRangeAC(a.ccrd(), 4.0, envi);
            for (size_t ei=1; ei < envi.Count(); ei++) {
              double d = (envi[ei].GetB()-a.crd()).Length();
              if (d>0.75 && d<4) {
                res[a.GetType().index][olx_round(100*(d-0.75))] ++;
              }
            }
          }
        }
        catch(const TExceptionBase &e) {
          app.NewLogEntry() << (olxstr("Processing: ").quote() << files[i]);
          TStrList st;
          e.GetException()->GetStackTrace(st);
          app.NewLogEntry() << st;
        }
      }
      TCStrList out;
      bool header_saved = false;
      for (size_t i=0; i < res.Count(); i++) {
        if (res.IsNull(i)) continue;
        int mv = 0;
        for (size_t j=0; j < res[i].Count(); j++) {
          if (res[i][j] > mv)
            mv = res[i][j];
        }
        if (mv == 0) continue;
        if (!header_saved) {
          olxcstr &header = out.Add("Symbol");
          for (size_t j=0; j < res[i].Count(); j++) {
            header << ' ' << olxcstr::FormatFloat(2, (0.75 + (double)j/100));
          }
          header_saved = true;
        }
        olxcstr &line = out.Add(XElementLib::GetByIndex((short)i).symbol);
        for (size_t j=0; j < res[i].Count(); j++) {
          line << ' ' << olxcstr::FormatFloat(2, (100.0*res[i][j])/mv);
        }
      }
      out.SaveToFile(olxstr("f:/ad.xlt"));
    }
    TSAtomPList res;
    app.FindSAtoms(EmptyString(), res, false);
    if (res.Count() == 1)
      E.SetRetVal(estimate_r(app.XFile().GetLattice(), *res[0]));
  }

  static void funAnalyseR(const TStrObjList& Params, TMacroError& E)  {
    TXApp &app = TXApp::GetInstance();
    TStrList ad;
    ad.LoadFromFile(olxstr("f:/ad.xlt"));
    TSAtomPList res;
    app.FindSAtoms(EmptyString(), res, false);
    if (res.Count() == 1) {
      TTypeList<AnAssociation2<const TCAtom *, vec3d> > envi;
      app.XFile().GetLattice().GetUnitCell().FindInRangeAC(
        res[0]->ccrd(), 4.0, envi);
      TTypeList<AnAssociation2<double, olxstr> > hits;
      for (size_t i=1; i < ad.Count(); i++) {
        TStrList lt(ad[i], ' ');
        double sum = 1;
        for (size_t ei=1; ei < envi.Count(); ei++) {
          double d = (envi[ei].GetB()-res[0]->crd()).Length();
          if (d>0.75 && d<4) {
            int ci = olx_round(100*(d-0.75));
            double tv = lt[ci+1].ToDouble();
            /* need to sort the results by distance and put weight related to the
            distance
            */
            sum += pow(tv/100, 1./(envi.Count()-1));
          }
        }
        hits.AddNew(sqrt(sum)*100, lt[0]);
      }
      hits.QuickSorter.SortSF(hits, &hr_sort);
      for (size_t i=0; i < hits.Count(); i++)
        TBasicApp::NewLogEntry() << hits[i].GetB() << ": " << hits[i].GetA();
    }
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
    lib->RegisterStaticFunction(
      new TStaticFunction(&Analysis::funEstomateR, "EstimateR", fpNone|fpOne,
      "Estimates radius of the given atom by analying its environment."
      "Returns radius or 0")
    );
    lib->RegisterStaticFunction(
      new TStaticFunction(&Analysis::funAnalyseR, "AnalyseR", fpNone|fpOne,
      "Estimates radius of the given atom by analying its environment."
      "Returns radius or 0")
    );
    return lib;
  }
};
EndXlibNamespace()
#endif
