/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "auto.h"
#include "ins.h"
#include "integration.h"
#include "xmacro.h"
#include "sortedlist.h"
#include "beevers-lipson.h"
#include "arrays.h"
#include "maputil.h"
#include "estopwatch.h"
#include "analysis.h"

struct _auto_BI {
  int type;
  uint32_t min_bonds, max_bonds;
};
static _auto_BI _autoMaxBond[] = {
  {iOxygenZ, 1, 2},
  {iFluorineZ, 0, 1},
  {iChlorineZ, 0, 1},
  {iPhosphorusZ, 3, 6},
};

// helper function
size_t imp_auto_AtomCount(const TAsymmUnit& au) {
  size_t ac = 0;
  for (size_t i = 0; i < au.AtomCount(); i++) {
    const TCAtom& a = au.GetAtom(i);
    if (a.IsDeleted() || a.GetType() < 3) {
      continue;
    }
    ac++;
  }
  return ac;
}

void XLibMacros::funATA(const TStrObjList& Cmds, TMacroData& Error) {
  TXApp& xapp = TXApp::GetInstance();
  olxstr folder(Cmds.IsEmpty() ? EmptyString() : Cmds[0]);
  int arg = 0;
  if (folder.IsNumber()) {
    arg = folder.ToInt();
    folder.SetLength(0);
  }
  bool dry_run = arg == -1;
  if (!dry_run) {
    olex2::IOlex2Processor::GetInstance()->processMacro("clean -npd -d");
  }
  static olxstr FileName(xapp.XFile().GetFileName());
  if (!folder.IsEmpty()) {
    TAutoDB::GetInstance().ProcessFolder(folder);
  }
  TLattice& latt = xapp.XFile().GetLattice();
  TAsymmUnit& au = latt.GetAsymmUnit();
  ElementPList elm_l;
  if (arg == 1) {
    elm_l = olx_analysis::helper::get_user_elements();
  }
  TAutoDB::AnalysisStat stat;
  uint64_t st = TETime::msNow();
  TAutoDB::GetInstance().AnalyseStructure(xapp.XFile().GetFileName(), latt,
    0, stat, dry_run, elm_l.IsEmpty() ? 0 : &elm_l);
  st = TETime::msNow() - st;
  TBasicApp::NewLogEntry(logInfo) << "Elapsed time " << st << " ms";
  olex2::IOlex2Processor::GetInstance()->processMacro("fuse");
  size_t ac = imp_auto_AtomCount(au);
  if (ac == 0) { // clearly something is wrong when it happens...
    ac = 1;
  }
  // sometimes things get stuck while there are some NPD atoms
  if ((double)stat.ConfidentAtomTypes / ac < 0.2 && !dry_run) {
    olex2::IOlex2Processor::GetInstance()->processMacro("clean -d");
  }
  Error.SetRetVal(olxstr(stat.AtomTypeChanges != 0) << ';' <<
    (double)stat.ConfidentAtomTypes * 100 / ac);
}
//..............................................................................
void XLibMacros::macAtomInfo(TStrObjList& Cmds, const TParamList& Options,
  TMacroData& Error)
{
  TXApp& xapp = TXApp::GetInstance();
  TSAtomPList satoms;
  xapp.FindSAtoms(Cmds.Text(' '), satoms);
  TStrList report;
  for (size_t i = 0; i < satoms.Count(); i++) {
    TAutoDB::GetInstance().AnalyseNode(*satoms[i], report);
  }
  xapp.NewLogEntry() << report;
}
//..............................................................................
void XLibMacros::macVATA(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TXApp& xapp = TXApp::GetInstance();
  TEFile log(Cmds.Text(' '), "a+b");
  TStrList report;
  TAutoDB::GetInstance().ValidateResult(
    xapp.XFile().GetFileName(), xapp.XFile().GetLattice(), report);
  report.SaveToTextStream(log);
}
//..............................................................................
struct Main_BaiComparator {
  template <class item_a_t, class item_b_t>
  static int Compare(const item_a_t &a, const item_b_t &b) {
      return olx_ref::get(a).Object->z - olx_ref::get(b).Object->z;
  }
};
void helper_CleanBaiList(TStringToList<olxstr, const cm_Element*>& list,
  SortedElementPList& au_bais)
{
  TXApp& xapp = TXApp::GetInstance();
  if( xapp.CheckFileType<TIns>() )  {
    TIns& ins = xapp.XFile().GetLastLoader<TIns>();
    list.Clear();
    const ContentList& cl = ins.GetRM().GetUserContent();
    for( size_t i=0; i < cl.Count(); i++ )  {
      au_bais.Add(cl[i].element);
      list.Add(cl[i].element->symbol, cl[i].element);
    }
    QuickSorter::Sort(list, Main_BaiComparator());
  }
}

void XLibMacros::macClean(TStrObjList &Cmds, const TParamList &Options,
  TMacroData &Error)
{
  TXApp& xapp = TXApp::GetInstance();
  TStringToList<olxstr, const cm_Element*> sfac;
  SortedElementPList AvailableTypes;
  static ElementPList StandAlone;
  if (StandAlone.IsEmpty()) {
    StandAlone.Add(XElementLib::GetByIndex(iOxygenIndex));
    StandAlone.Add(XElementLib::GetByIndex(iMagnesiumIndex));
    StandAlone.Add(XElementLib::GetByIndex(iChlorineIndex));
    StandAlone.Add(XElementLib::GetByIndex(iPotassiumIndex));
    StandAlone.Add(XElementLib::GetByIndex(iCalciumIndex));
    StandAlone.Add(XElementLib::FindByZ(iBromineZ));
    StandAlone.Add(XElementLib::FindByZ(53)); // iodine
  }
  helper_CleanBaiList(sfac, AvailableTypes);
  const bool runFuse = !Options.Contains("f");
  const bool check_demotion = Options.Contains('d');
  const bool enforce_formula = TAutoDB::GetInstance().IsEnforceFormula();
  size_t changeNPD = ~0;
  if (Options.Contains("npd")) {
    olxstr _v = Options.FindValue("npd");
    if (_v.IsEmpty()) {
      changeNPD = 0;
    }
    else {
      changeNPD = _v.ToSizeT();
    }
  }
  const bool analyseQ = !Options.Contains("aq");
  const bool assignTypes = !Options.Contains("at");
  const double aqV = Options.FindValue("aq", "0.2").ToDouble(); // R+aqV
  // qpeak analysis
  TAsymmUnit& au = xapp.XFile().GetAsymmUnit();
  if (analyseQ) {
    sorted::PrimitiveAssociation<double, TCAtom*> SortedQPeaks;
    TTypeList< olx_pair_t<double, TCAtomPList*> > vals;
    size_t cnt = 0;
    double avQPeak = 0;
    bool OnlyQPeakModel = true;
    for (size_t i = 0; i < au.AtomCount(); i++) {
      if (au.GetAtom(i).IsDeleted())  continue;
      if (au.GetAtom(i).GetType() != iQPeakZ)
        OnlyQPeakModel = false;
      else {
        SortedQPeaks.Add(au.GetAtom(i).GetQPeak(), &au.GetAtom(i));
        avQPeak += au.GetAtom(i).GetQPeak();
        cnt++;
      }
    }
    if (cnt != 0) {
      avQPeak /= cnt;
    }
    cnt = 0;
    if (!SortedQPeaks.IsEmpty()) {
      vals.AddNew<double, TCAtomPList*>(0, new TCAtomPList);
      for (size_t i = SortedQPeaks.Count() - 1; i >= 1; i--) {
        if ((SortedQPeaks.GetKey(i) -
          SortedQPeaks.GetKey(i - 1)) / SortedQPeaks.GetKey(i) > 0.1)
        {
          vals.GetLast().a += SortedQPeaks.GetKey(i);
          vals.GetLast().b->Add(SortedQPeaks.GetValue(i));
          cnt++;
          vals.GetLast().a /= cnt;
          cnt = 0;
          vals.AddNew<double, TCAtomPList*>(0, new TCAtomPList);
          continue;
        }
        vals.GetLast().a += SortedQPeaks.GetKey(i);
        vals.GetLast().b->Add(SortedQPeaks.GetValue(i));
        cnt++;
      }
      vals.GetLast().b->Add(SortedQPeaks.GetValue(0));
      cnt++;
      if (cnt > 1) {
        vals.GetLast().a /= cnt;
      }

      TBasicApp::NewLogEntry(logInfo) << "Average QPeak: " << avQPeak;
      TBasicApp::NewLogEntry(logInfo) << "QPeak steps:";
      for (size_t i = 0; i < vals.Count(); i++)
        TBasicApp::NewLogEntry(logInfo) << vals[i].GetA();

      //    double thVal = 2;
      double thVal = (avQPeak < 2) ? 2 : avQPeak*0.75;

      TBasicApp::NewLogEntry(logInfo) << "QPeak threshold:" << thVal;

      if (SortedQPeaks.Count() == 1) {  // only one peak present
        if (SortedQPeaks.GetKey(0) < thVal) {
          SortedQPeaks.GetValue(0)->SetDeleted(true);
        }
      }
      else {
        for (size_t i = vals.Count() - 1; i != InvalidIndex; i--) {
          if (vals[i].GetA() < thVal) {
            for (size_t j = 0; j < vals[i].GetB()->Count(); j++) {
              vals[i].GetB()->GetItem(j)->SetDeleted(true);
            }
          }
        }
      }
      for (size_t i = 0; i < vals.Count(); i++) {
        delete vals[i].b;
      }
    }
  }
  // end qpeak analysis

  // distance analysis
  TLattice& latt = xapp.XFile().GetLattice();
  // qpeaks first
  TCAtomPList QPeaks;
  for (size_t i = 0; i < au.AtomCount(); i++) {
    TCAtom& ca = au.GetAtom(i);
    if (!ca.IsDeleted() && ca.GetType() == iQPeakZ) {
      QPeaks.Add(ca);
    }
  }
  for (size_t i = 0; i < QPeaks.Count(); i++) {
    if (QPeaks[i]->IsDeleted()) {
      continue;
    }
    TTypeList<olx_pair_t<TCAtom*, vec3d> > neighbours;
    TAutoDBNode nd(*QPeaks[i], latt.GetUnitCell().GetMatrix(0),  &neighbours);
    bool remove = false;
    for (size_t j = 0; j < nd.DistanceCount(); j++) {
      // at least an H-bond
      if (nd.GetDistance(j) < (neighbours[j].GetA()->GetType().r_bonding + aqV))
      {
        if (neighbours[j].GetA()->GetType() == iQPeakZ) {
          if (nd.GetDistance(j) < 1) {
            if (neighbours[j].GetA()->GetQPeak() < QPeaks[i]->GetQPeak()) {
              neighbours[j].GetA()->SetDeleted(true);
            }
          }
        }
        else {
          remove = true;
          break;
        }
      }
      if (nd.GetDistance(j) > 1.8) {
        //        if( neighbours[j].GetA()->GetAtomInfo().GetIndex() < 20 )  {  // Ca
        //          QPeaks[i]->SetDeleted(true);
        //          QPeaks[i]->CAtom().SetDeleted(true);
        //        }
      }
      if (remove) {
        break;
      }
    }
    if (remove || (nd.NodeCount() == 2 && nd.GetAngle(0) < 90)) {
      if (remove || (!neighbours[0].GetA()->IsDeleted() &&
        !neighbours[1].GetA()->IsDeleted()))
      {
        QPeaks[i]->SetDeleted(true);
        continue;
      }
    }

    //    for( size_t j=0; j < nd.AngleCount(); j++ )  {
    //      if( nd.GetAngle(j) < 90
    //    }
  }
  // call whatever left carbons ...
  if (assignTypes) {
    for (size_t i = 0; i < QPeaks.Count(); i++) {
      if (QPeaks[i]->IsDeleted()) {
        continue;
      }
      TBasicApp::NewLogEntry(logInfo) << QPeaks[i]->GetLabel() << " -> C";
      TCAtom &a = *QPeaks[i];
      a.SetLabel("C", false);
      a.SetUiso(0.025);
      a.SetType(XElementLib::GetByIndex(iCarbonIndex));
      // clean up if H are already in place
      for (size_t j = 0; j < a.AttachedSiteCount(); j++) {
        TCAtom &aa = a.GetAttachedAtom(j);
        for (size_t k = 0; k < aa.AttachedSiteCount(); k++) {
          TCAtom &aaa = aa.GetAttachedAtom(k);
          if (aaa.GetType() == iHydrogenZ) {
            aaa.SetDeleted(true);
          }
        }
      }
    }
  }
  // mark ring atoms...
  {
    using namespace olx_analysis;
    TTypeList<fragments::fragment> frags = fragments::extract(au);
    for (size_t i = 0; i < frags.Count(); i++) {
      TCAtomPList r_atoms;
      frags[i].breadth_first_tags(~0, &r_atoms);
      TTypeList<fragments::ring> rings = frags[i].get_rings(r_atoms);
      for (size_t j = 0; j < rings.Count(); j++) {
        rings[j].atoms.ForEach(TCAtom::FlagSetter(catom_flag_RingAtom, true));
      }
    }
  }

  TDoubleList Uisos;
  if (xapp.XFile().GetFileName() == TAutoDB::GetInstance().GetLastFileName()) {
    Uisos.Assign(TAutoDB::GetInstance().GetUisos());
  }
  for (size_t i = 0; i < latt.FragmentCount(); i++) {
    TNetwork &frag = latt.GetFragment(i);
    size_t fac = 0;
    for (size_t j = 0; j < frag.NodeCount(); j++) {
      if (!frag.Node(j).IsDeleted() && frag.Node(j).GetType() != iQPeakZ) {
        fac++;
      }
    }
    if (fac > 7) { // skip up to PF6 or so for Uiso analysis
      while (Uisos.Count() <= i) {
        Uisos.Add(0.0);
      }
      bool changes = true;
      while (changes) {
        changes = false;
        if (olx_abs(Uisos[i]) < 1e-6) {
          size_t ac = 0;
          for (size_t j = 0; j < frag.NodeCount(); j++) {
            TSAtom& sa = frag.Node(j);
            if (sa.IsDeleted() || sa.GetType().z < 2) {
              continue;
            }
            if (sa.GetEllipsoid() != 0 && sa.GetEllipsoid()->IsNPD()) {
              continue;
            }
            Uisos[i] += sa.CAtom().GetUiso();
            ac++;
          }
          if (ac != 0) {
            Uisos[i] /= ac;
          }
        }
        if (Uisos[i] > 1e-6) {
          for (size_t j = 0; j < frag.NodeCount(); j++) {
            TSAtom& sa = frag.Node(j);
            if (sa.IsDeleted() || sa.GetType() == iHydrogenZ) {
              continue;
            }
            if (sa.GetType() != iQPeakZ && sa.CAtom().GetUiso() > Uisos[i] * 3) {
              size_t bc = 0;
              for (size_t bi = 0; bi < sa.CAtom().AttachedSiteCount(); bi++) {
                if (sa.CAtom().GetAttachedAtom(bi).GetType() > 1)
                  bc++;
              }
              if (bc > 1 || (sa.CAtom().GetUiso() > Uisos[i] * 3.5)) {
                if (check_demotion &&
                  olx_analysis::helper::can_demote(sa.GetType(), AvailableTypes))
                {
                  continue;
                }
                TBasicApp::NewLogEntry(logInfo) << sa.GetLabel() <<
                  " too large, deleting";
                sa.SetDeleted(true);
                sa.CAtom().SetDeleted(true);
                changes = true;
              }
            }
          }
        }
      }
    }
    else if (assignTypes && fac == 1) {  // treat O and Cl
      TSAtom *pas_ = 0;
      for (size_t j = 0; j < frag.NodeCount(); j++) {
        if (!frag.Node(j).IsDeleted() && frag.Node(j).GetType() != iQPeakZ) {
          pas_ = &frag.Node(j);
          break;
        }
      }
      if (pas_ == 0) {
        continue; // ?
      }
      TSAtom& sa = *pas_;
      bool alone = true;
      if (!sa.CAtom().IsFixedType()) {
        for (size_t j = 0; j < sa.CAtom().AttachedSiteCount(); j++) {
          if (sa.CAtom().GetAttachedAtom(j).GetType() != iQPeakZ) {
            alone = false;
            break;
          }
        }
      }
      else {
        alone = false;
      }
      if (alone) {
        bool assignHeaviest = false, assignLightest = false;
        const TAutoDB::AnalysisStat& stat = TAutoDB::GetInstance().GetStats();
        size_t ac = imp_auto_AtomCount(au);
        if (ac == 0) { // this would be really strange
          ac++;
        }
        // now we can make up types
        if (stat.SNAtomTypeAssignments == 0 &&
          ((double)stat.ConfidentAtomTypes / ac) > 0.1)
        {
          bool found = false;
          int min_dz = 100;
          size_t closest_idx = InvalidIndex;
          const cm_Element *to_asign = NULL;
          for (size_t j = 0; j < StandAlone.Count(); j++) {
            int dz = olx_abs(sa.GetType().z - StandAlone[j]->z);
            if (dz < min_dz) {
              closest_idx = j;
              min_dz = dz;
            }
          }
          if (sa.CAtom().GetUiso() < 0.01) {  // search heavier
            size_t start = (sa.GetType().z >= StandAlone[closest_idx]->z
              ? closest_idx + 1 : closest_idx);
            for (size_t k = start; k < StandAlone.Count(); k++) {
              if (sa.GetType().z < StandAlone[k]->z &&
                (!enforce_formula || AvailableTypes.Contains(StandAlone[k])))
              {
                sa.CAtom().SetLabel(StandAlone[k]->symbol, false);
                sa.CAtom().SetType(*StandAlone[k]);
                olx_analysis::helper::reset_u(sa.CAtom());
                break;
              }
            }
          }
          else if (sa.CAtom().GetUiso() > 0.2) {  // search lighter
            size_t start = (sa.GetType().z > StandAlone[closest_idx]->z
              ? closest_idx : closest_idx - 1);
            if (start == InvalidIndex) {
              sa.SetDeleted(true);
              sa.CAtom().SetDeleted(true);
            }
            for (size_t k = start; k != InvalidIndex; k--) {
              if (!enforce_formula || AvailableTypes.Contains(StandAlone[k])) {
                sa.CAtom().SetLabel(StandAlone[k]->symbol, false);
                sa.CAtom().SetType(*StandAlone[k]);
                olx_analysis::helper::reset_u(sa.CAtom());
                break;
              }
            }
          }
        }
      }
    }
    for (size_t j = 0; j < latt.GetFragment(i).NodeCount(); j++) {
      TSAtom& sa = latt.GetFragment(i).Node(j);
      if (sa.IsDeleted() || sa.CAtom().IsDeleted() || sa.GetType().GetMr() < 3) {
        continue;
      }
      TTypeList<olx_pair_t<TCAtom*, vec3d> > neighbours;
      TAutoDBNode nd(sa, &neighbours);
      for (size_t k = 0; k < nd.DistanceCount(); k++) {
        if (neighbours[k].GetA()->IsDeleted()) {
          continue;
        }
        if (nd.GetDistance(k) < (neighbours[k].GetA()->GetType().r_bonding + aqV) &&
          neighbours[k].GetA()->GetType() != iHydrogenZ) {
          if (neighbours[k].GetA()->GetType() == iQPeakZ ||
            neighbours[k].GetA()->GetType() <= iFluorineZ)
          {
            olx_analysis::helper::delete_atom(*neighbours[k].GetA());
          }
          else {
            if (sa.GetType() == iQPeakZ || sa.GetType() <= iFluorineZ) {
              olx_analysis::helper::delete_atom(sa.CAtom());
            }
          }
        }
      }
    }
  }
  // check blown up
  for (size_t i = 0; i < au.AtomCount(); i++) {
    TCAtom &ca = au.GetAtom(i);
    if (ca.IsDeleted() || ca.GetType() == iHydrogenZ) {
      continue;
    }
    if (ca.GetType() != iQPeakZ && ca.GetUiso() > 0.125) {
      if (check_demotion &&
        olx_analysis::helper::can_demote(ca, AvailableTypes))
      {
        continue;
      }
      TBasicApp::NewLogEntry(logInfo) << ca.GetLabel() << " blown up";
      olx_analysis::helper::delete_atom(ca);
    }
  }
  // treating NPD atoms... promoting to the next available type
  if (changeNPD > 0 && !sfac.IsEmpty()) {
    size_t atoms_transformed = 0;
    TCAtomPList to_isot;
    int delta_z = TAutoDB::GetInstance().GetBAIDelta();
    for (size_t i = 0; i < au.AtomCount(); i++) {
      TCAtom& ca = au.GetAtom(i);
      if (ca.IsFixedType()) {
        continue;
      }
      if ((ca.GetEllipsoid() != 0 && ca.GetEllipsoid()->IsNPD()) ||
        (ca.GetUiso() <= 0.005))
      {
        size_t ind = sfac.IndexOfObject(&ca.GetType());
        if (ind != InvalidIndex && ((ind + 1) < sfac.Count())) {
          if (delta_z > 0 &&  // obey the same rules
            olx_abs(ca.GetType().z - sfac.GetObject(ind + 1)->z) > delta_z)
          {
            if (ca.GetEllipsoid() != 0) {
              to_isot.Add(ca);
            }
            continue;
          }
          const cm_Element &e = olx_analysis::Analysis::check_proposed_element(
            ca, *sfac.GetObject(ind + 1));
          if (e != ca.GetType()) {
            TBasicApp::NewLogEntry(logInfo) << "NPD " << ca.GetLabel() <<
              " type change from " << ca.GetType().GetSymbol() << " to " <<
              e.symbol;
            ca.SetType(e);
            olx_analysis::helper::reset_u(ca);
            if (++atoms_transformed >= changeNPD) {
              break;
            }
          }
        }
      }
    }
    latt.SetAnis(to_isot, false);
  }
  //end treating NDP atoms
  if (runFuse && olex2::IOlex2Processor::GetInstance() != 0) {
    olex2::IOlex2Processor::GetInstance()->processMacro("fuse");
  }
}
//..............................................................................
struct Main_SfacComparator {
  template <class item_a_t, class item_b_t>
  static int Compare(const item_a_t &a, const item_b_t &b) {
      return olx_ref::get(b).GetB()->z - olx_ref::get(a).GetB()->z;
  }
};
void XLibMacros::funVSS(const TStrObjList &Cmds, TMacroData &Error) {
  using namespace olx_analysis;
  TXApp& xapp = TXApp::GetInstance();
  TLattice& latt = xapp.XFile().GetLattice();
  TUnitCell& uc = latt.GetUnitCell();
  TAsymmUnit& au = latt.GetAsymmUnit();
  int ValidatedAtomCount = 0, AtomCount = 0;
  const bool use_formula = (Cmds.IsEmpty() ? false : Cmds[0].ToBool());
  const bool enforce_formula = TAutoDB::GetInstance().IsEnforceFormula();
  if (use_formula) {
    TTypeList< olx_pair_t<double, const cm_Element*> > sl;
    double ac = 0;
    const ContentList& cl = xapp.XFile().GetRM().GetUserContent();
    ElementPList elm_l = olx_analysis::helper::get_user_elements();
    for (size_t i = 0; i < cl.Count(); i++) {
      if (cl[i].element->z == iHydrogenZ) {
        continue;
      }
      sl.AddNew(cl[i].count, cl[i].element);
      ac += cl[i].count;
    }
    QuickSorter::Sort(sl, Main_SfacComparator());  // sorts ascending
    double auv = latt.GetUnitCell().CalcVolume() / latt.GetUnitCell().MatrixCount();
    double ratio = auv / (18 * ac);
    for (size_t i = 0; i < sl.Count(); i++) {
      sl[i].a = ratio*sl[i].GetA();
    }

    sorted::PrimitiveAssociation<double, TCAtom*> SortedQPeaks;
    for (size_t i = 0; i < au.AtomCount(); i++) {
      if (au.GetAtom(i).IsDeleted()) {
        continue;
      }
      if (au.GetAtom(i).GetType() == iQPeakZ) {
        SortedQPeaks.Add(au.GetAtom(i).GetQPeak(), &au.GetAtom(i));
      }
      else {
        for (size_t j = 0; j < sl.Count(); j++) {
          if (*sl[j].GetB() == au.GetAtom(i).GetType()) {
            sl[j].a -= 1. / au.GetAtom(i).GetDegeneracy();
            break;
          }
        }
      }
    }
    for (size_t i = 0; i < sl.Count(); i++) {
      if (SortedQPeaks.IsEmpty()) {
        break;
      }
      while (sl[i].GetA() > 0) {
        TCAtom &p = *SortedQPeaks.GetLastValue();
        sl[i].a -= 1. / p.GetDegeneracy();
        p.SetLabel((olxstr(sl[i].GetB()->symbol) << i), false);
        const cm_Element &e = Analysis::check_proposed_element(p, *sl[i].b);
        if (elm_l.Contains(&e)) {
          p.SetType(e);
        }
        else {
          p.SetType(*sl[i].b);
        }
        p.SetQPeak(0);
        SortedQPeaks.Delete(SortedQPeaks.Count() - 1);
        if (SortedQPeaks.IsEmpty()) {
          break;
        }
      }
    }
    // get rid of the rest of Q-peaks and "validate" geometry of atoms
    for (size_t i = 0; i < au.AtomCount(); i++) {
      if (au.GetAtom(i).GetType() == iQPeakZ) {
        au.GetAtom(i).SetDeleted(true);
      }
    }
    xapp.XFile().EndUpdate();
    // validate max bonds
    ASObjectProvider& objects = latt.GetObjects();
    TTypeList<TAtomEnvi> bc_to_check;
    const size_t maxb_cnt = sizeof(_autoMaxBond) / sizeof(_autoMaxBond[0]);
    for (size_t i = 0; i < objects.atoms.Count(); i++) {
      TSAtom& sa = objects.atoms[i];
      for (size_t j = 0; j < maxb_cnt; j++) {
        if (sa.GetType() == _autoMaxBond[j].type) {
          bc_to_check.AddCopy(uc.GetAtomEnviList(sa));
          if (bc_to_check.GetLast().Count() <= _autoMaxBond[j].max_bonds &&
            bc_to_check.GetLast().Count() >= _autoMaxBond[j].min_bonds)
          {
            bc_to_check.NullItem(bc_to_check.Count() - 1);
          }
        }
      }
    }
    bc_to_check.Pack();
    bool changes = true;
    while (changes) {
      changes = false;
      for (size_t i = 0; i < bc_to_check.Count(); i++) {
        size_t sati = InvalidIndex;
        for (size_t j = 0; j < sl.Count(); j++) {
          if (bc_to_check[i].GetBase().GetType() == *sl[j].GetB()) {
            sati = j;
            break;
          }
        }
        if (sati != InvalidIndex && (sati + 1) < sl.Count()) {
          if (!enforce_formula || elm_l.Contains(sl[sati + 1].GetB())) {
            bc_to_check[i].GetBase().CAtom().SetType(*sl[sati + 1].GetB());
            changes = true;
          }
        }
      }
    }
    if (!bc_to_check.IsEmpty()) {
      xapp.XFile().EndUpdate();
    }
  }
  TArrayList<olx_pair_t<TCAtom const*, vec3d> > res;
  for (size_t i = 0; i < au.AtomCount(); i++) {
    if (au.GetAtom(i).IsDeleted() || au.GetAtom(i).GetType() < 2) {
      continue;
    }
    uc.FindInRangeAC(au.GetAtom(i).ccrd(), 1e-2,
      au.GetAtom(i).GetType().r_bonding + 1.3, res);
    vec3d center = au.Orthogonalise(au.GetAtom(i).ccrd());
    for (size_t j = 0; j < res.Count(); j++) {
      if (res[j].GetA()->GetType() < 2) {
        res.Delete(j--);
      }
    }
    AtomCount++;
    double wght = 1;
    if (res.Count() > 1) {
      double awght = 1. / (res.Count()*(res.Count() - 1));
      for (size_t j = 0; j < res.Count(); j++) {
        if (res[j].GetB().QLength() < 1)
          wght -= 0.5 / res.Count();
        for (size_t k = j + 1; k < res.Count(); k++) {
          double cang = (res[j].GetB() - center).CAngle(res[k].GetB() - center);
          if (cang > 0.588) { // 56 degrees
            wght -= awght;
          }
        }
      }
    }
    else if (res.Count() == 1) {  // just one bond
      if (res[0].GetB().QLength() < 1) {
        wght = 0;
      }
    }
    else { // no bonds, cannot say anything
      wght = 0;
    }

    if (wght >= 0.95) {
      ValidatedAtomCount++;
    }
    res.Clear();
  }
  TAsymmUnit::TLabelChecker lc(au);
  for (size_t i = 0; i < au.AtomCount(); i++) {
    TCAtom &a = au.GetAtom(i);
    au.GetAtom(i).SetLabel(lc.CheckLabel(a, a.GetLabel(), 0, false), false);
  }

  Error.SetRetVal(AtomCount == 0 ? 0
    : (double)ValidatedAtomCount * 100 / AtomCount);
}
//..............................................................................
double TryPoint(TArray3D<float>& map, const TUnitCell& uc, const vec3i& p,
  const TArray3D<bool> &mask)
{
  TRefList refs;
  TArrayList<compd> F;
  TArrayList<SFUtil::StructureFactor> P1SF;
  const TUnitCell::SymmSpace sym_space = uc.GetSymmSpace();
  SFUtil::GetSF(refs, F, SFUtil::mapTypeDiff, SFUtil::sfOriginOlex2, SFUtil::scaleRegression);
  SFUtil::ExpandToP1(refs, F, sym_space, P1SF);
  const vec3s dim = map.GetSize();
  const vec3d norm(1./dim[0], 1./dim[1], 1./dim[2]);
  BVFourier::CalcEDM(P1SF, map.Data, uc.CalcVolume());
  //TArrayList<MapUtil::peak> _Peaks;
  //TTypeList<MapUtil::peak> Peaks;
  //MapUtil::Integrate<float>(map.Data, dim, (float)((mi.maxVal - mi.minVal)/2.5), _Peaks);
  //MapUtil::MergePeaks(sym_space, norm, _Peaks, Peaks);

  //double sum=0;
  //size_t count=0;
  //for( size_t i=0; i < Peaks.Count(); i++ )  {
  //  const MapUtil::peak& peak = Peaks[i];
  //  const vec3d cnt = norm*peak.center;
  //  const double ed = peak.summ/peak.count;
  //  if( uc.FindClosestDistance(crd, cnt) < 1.0 )  {
  //    sum += ed;
  //    count++;
  //  }
  //}
  //return count == 0 ? 0 : sum/count;
  return MapUtil::IntegrateMask(map.Data, p, mask);
}

void XLibMacros::funFATA(const TStrObjList &Cmds, TMacroData &E) {
  TXApp& xapp = TXApp::GetInstance();
  TStopWatch sw(__FUNC__);
  double resolution = 0.2;
  resolution = 1. / resolution;
  TRefList refs;
  TArrayList<compd> F;
  olxstr err(SFUtil::GetSF(refs, F, SFUtil::mapTypeDiff,
    SFUtil::sfOriginOlex2, SFUtil::scaleRegression));
  if (!err.IsEmpty()) {
    E.ProcessingError(__OlxSrcInfo, err);
    return;
  }
  TAsymmUnit& au = xapp.XFile().GetAsymmUnit();
  TUnitCell& uc = xapp.XFile().GetUnitCell();
  TArrayList<SFUtil::StructureFactor> P1SF;
  const TUnitCell::SymmSpace sym_space = uc.GetSymmSpace();
  sw.start("Expanding structure factors to P1 (fast symm)");
  SFUtil::ExpandToP1(refs, F, sym_space, P1SF);
  sw.stop();
  const double vol = xapp.XFile().GetLattice().GetUnitCell().CalcVolume();
  // init map
  const vec3i dim(au.GetAxes()*resolution);
  TArray3D<float> map(0, dim[0] - 1, 0, dim[1] - 1, 0, dim[2] - 1);
  TArrayList<AnAssociation3<TCAtom*, double, size_t> > atoms(au.AtomCount());
  for (size_t i = 0; i < au.AtomCount(); i++) {
    atoms[i].a = &au.GetAtom(i);
    atoms[i].b = 0;
    atoms[i].c = 0;
    atoms[i].a->SetTag(i);
  }
  size_t found_cnt = 0;
  sw.start("Calculating electron density map in P1 (Beevers-Lipson)");
  BVFourier::MapInfo mi = BVFourier::CalcEDM(P1SF, map.Data, vol);
  sw.stop();
  sw.start("Integrating P1 map: ");
  ElementRadii radii;
  for (size_t i = 0; i < au.AtomCount(); i++) {
    if (radii.IndexOf(&au.GetAtom(i).GetType()) == InvalidIndex) {
      radii.Add(&au.GetAtom(i).GetType(), au.GetAtom(i).GetType().r_vdw*0.5);
    }
  }
  olx_pdict<short, TArray3D<bool>*> atom_masks =
    uc.BuildAtomMasks(map.GetSize(), &radii, 0);
  TSizeList mask_sizes(atom_masks.Count());
  for (size_t i = 0; i < atom_masks.Count(); i++) {
    TArray3D<bool> &mask = *atom_masks.GetValue(i);
    size_t cnt = 0;
    for (size_t ix = 0; ix < mask.Length1(); ix++) {
      for (size_t iy = 0; iy < mask.Length2(); iy++)
        for (size_t iz = 0; iz < mask.Length3(); iz++)
          if (mask.Data[ix][iy][iz])
            cnt++;
    }
    mask_sizes[i] = cnt;
  }
  for (size_t i = 0; i < atoms.Count(); i++) {
    if (atoms[i].GetA()->IsDeleted() || atoms[i].GetA()->GetType() == iQPeakZ)
      continue;
    vec3i p = (atoms[i].GetA()->ccrd()*map.GetSize()).Round<int>();
    size_t ti = atom_masks.IndexOf(atoms[i].GetA()->GetType().GetIndex());
    atoms[i].b = MapUtil::IntegrateMask(map.Data,
      p, *atom_masks.GetValue(ti));
    atoms[i].c = mask_sizes[ti];
    TBasicApp::NewLogEntry() << atoms[i].GetA()->GetLabel()
      << ": " << atoms[i].GetB() / mask_sizes[ti];
  }
  const double minEd = mi.sigma * 3;
  for (size_t i = 0; i < atoms.Count(); i++) {
    if (atoms[i].GetC() != 0) {
      const double ed = atoms[i].GetB() / atoms[i].GetC();
      if (olx_abs(ed) < minEd)  continue;
      if (atoms[i].GetA()->IsFixedType()) {
        TBasicApp::NewLogEntry() << "Skipping fixed type atoms '" <<
          atoms[i].GetA()->GetLabel() << '\'';
        continue;
      }
      double p_ed = 0, n_ed = 0;
      const cm_Element& original_type = atoms[i].GetA()->GetType();
      const size_t ti = atom_masks.IndexOf(original_type.GetIndex());
      TArray3D<bool> &mask = *atom_masks.GetValue(ti);
      cm_Element* n_e = XElementLib::NextZ(original_type);
      if (n_e != NULL) {
        atoms[i].GetA()->SetType(*n_e);
        sw.start("Trying next element");
        n_ed = TryPoint(map, xapp.XFile().GetUnitCell(),
          (atoms[i].GetA()->ccrd()*map.GetSize()).Round<int>(),
          mask) / mask_sizes[ti];
        sw.stop();
      }
      cm_Element* p_e = XElementLib::PrevZ(original_type);
      if (p_e != NULL) {
        sw.start("Trying previous element");
        atoms[i].GetA()->SetType(*p_e);
        p_ed = TryPoint(map, xapp.XFile().GetUnitCell(),
          (atoms[i].GetA()->ccrd()*map.GetSize()).Round<int>(),
          mask) / mask_sizes[ti];
        sw.stop();
      }
      atoms[i].GetA()->SetType(original_type);
      if (n_e != NULL && p_e != NULL) {
        if ((n_ed == 0 || olx_sign(n_ed) == olx_sign(p_ed)) && p_ed > 0) {
          found_cnt++;
          TBasicApp::NewLogEntry() << "Atom type changed from " << original_type.symbol <<
            " to " << n_e->symbol << " for " << atoms[i].GetA()->GetLabel();
          atoms[i].GetA()->SetType(*n_e);
        }
        else if (n_ed < 0 && (p_ed == 0 || olx_sign(p_ed) == olx_sign(n_ed))) {
          found_cnt++;
          TBasicApp::NewLogEntry() << "Atom type changed from " << original_type.symbol <<
            " to " << p_e->symbol << " for " << atoms[i].GetA()->GetLabel();
          atoms[i].GetA()->SetType(*p_e);
        }
        else if (n_ed != 0 && p_ed != 0) {
          if (olx_sign(n_ed) != olx_sign(p_ed)) {
            const double r = 2 * ed / (olx_abs(n_ed) + olx_abs(p_ed));
            if (olx_abs(r) > 0.5) {
              found_cnt++;
              if (r > 0) {
                TBasicApp::NewLogEntry() << "Atom type changed from " << original_type.symbol <<
                  " to " << n_e->symbol << " for " << atoms[i].GetA()->GetLabel();
                atoms[i].GetA()->SetType(*n_e);
              }
              else {
                TBasicApp::NewLogEntry() << (olxstr("Atom type changed from ") << original_type.symbol <<
                  " to " << p_e->symbol << " for " << atoms[i].GetA()->GetLabel() << '\n');
                atoms[i].GetA()->SetType(*p_e);
              }
            }
          }
          else { // same sign?
            found_cnt++;
            if (n_ed > 0) {
              TBasicApp::NewLogEntry() << "Atom type changed from " << original_type.symbol <<
                " to " << n_e->symbol << " for " << atoms[i].GetA()->GetLabel();
              atoms[i].GetA()->SetType(*n_e);
            }
            else {
              TBasicApp::NewLogEntry() << "Atom type changed from " << original_type.symbol <<
                " to " << p_e->symbol << " for " << atoms[i].GetA()->GetLabel();
              atoms[i].GetA()->SetType(*p_e);
            }
          }
        }
      }
    }
  }
  for (size_t i = 0; i < atom_masks.Count(); i++) {
    delete atom_masks.GetValue(i);
  }
  if (found_cnt == 0) {
    TBasicApp::NewLogEntry() << "No problems were found";
  }
  else {
    au.InitData();
    xapp.XFile().EndUpdate();
  }
  E.SetRetVal(found_cnt != 0);
}
