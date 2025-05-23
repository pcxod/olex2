/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

// use to switch fast symm on #define __OLX_USE_FASTSYMM
#include "sfutil.h"
#include "cif.h"
#include "hkl.h"
#include "unitcell.h"
#include "estopwatch.h"
#include "twinning.h"
#include "refutil.h"

using namespace SFUtil;

#ifdef __OLX_USE_FASTSYMM
DefineFSFactory(ISF_Util, SF_Util)
#endif
//..............................................................................
ISF_Util* SFUtil::GetSF_Util_Instance(const TSpaceGroup& sg) {
#ifdef __OLX_USE_FASTSYMM
  ISF_Util* sf_util = fs_factory_ISF_Util(sg.GetName());
  if (sf_util == NULL)
    throw TFunctionFailedException(__OlxSourceInfo, "invalid space group");
  return sf_util;
#else
  smatd_list all_m, unq_m;
  sg.GetMatrices(all_m, mattAll);
  sg.GetMatrices(unq_m, mattAll ^ (mattInversion | mattCentering));
  return new SF_Util<SG_Impl>(all_m, unq_m,
    sg.IsCentrosymmetric() && sg.GetInversionCenter().IsNull());
  //return new SF_Util<SG_Impl>(all_m, all_m, false);
#endif
}
//..............................................................................
void SFUtil::ExpandToP1(const TArrayList<vec3i>& hkl, const TArrayList<compd>& F,
  const TSpaceGroup& sg, TArrayList<StructureFactor>& out)
{
  if (hkl.Count() != F.Count()) {
    throw TInvalidArgumentException(__OlxSourceInfo,
      "hkl array and structure factors dimensions missmatch");
  }
  ISF_Util* sf_util = GetSF_Util_Instance(sg);
  out.SetCount(sf_util->GetSGOrder()* hkl.Count());
  sf_util->Expand(hkl, F, out);
  delete sf_util;
  // test
  //smatd_list ml;
  //sg.GetMatrices(ml, mattAll);
  //const size_t ml_cnt = ml.Count();
  //out.SetCount( ml_cnt* hkl.Count());
  //for (size_t i = 0; i < hkl.Count(); i++) {
  //  const size_t off = i * ml_cnt;
  //  for (size_t j = 0; j < ml_cnt; j++) {
  //    const size_t ind = off + j;
  //    out[ind].hkl = hkl[i] * ml[j].r;
  //    out[ind].ps = ml[j].t.DotProd(hkl[i]);
  //    if (out[ind].ps != 0) {
  //      double ca = 1, sa = 0;
  //      olx_sincos(-T_PI * out[ind].ps, &sa, &ca);
  //      out[ind].val = F[i] * compd(ca, sa);
  //    }
  //    else {
  //      out[ind].val = F[i];
  //    }
  //  }
  //}
  //QuickSorter::Sort(out, TComparableComparator());
  //for (size_t i = 0; i < out.Count(); i++) {
  //  size_t j = i;
  //  while (++j < out.Count() && out[i].hkl == out[j].hkl) {
  //    out.Delete(j--);
  //  }
  //}
  //end test
}
//..............................................................................
void SFUtil::FindMinMax(const TArrayList<StructureFactor>& F,
  vec3i& min, vec3i& max)
{
  min = vec3i(100, 100, 100);
  max = vec3i(-100, -100, -100);
  for (size_t i = 0; i < F.Count(); i++) {
    vec3i::UpdateMinMax(F[i].hkl, min, max);
  }
}
//..............................................................................
olxstr SFUtil::GetSF(TRefList& refs, TArrayList<compd>& F,
  short mapType, short sfOrigin, short scaleType, double scale,
  short friedelPairs, bool anom_only)
{
  TXApp& xapp = TXApp::GetInstance();
  TStopWatch sw(__FUNC__);
  TUnitCell::SymmSpace sp = xapp.XFile().GetUnitCell().GetSymmSpace();
  SymmSpace::InfoEx info_ex = SymmSpace::Compact(sp);
  if (sfOrigin == sfOriginFcf) {
    olxstr fcffn = TEFile::ChangeFileExt(xapp.XFile().GetFileName(), "fcf");
    cif_dp::cetTable* hklLoop = 0;
    olx_object_ptr<TCif> cif;
    if (xapp.CheckFileType<TCif>()) {
      hklLoop = xapp.XFile().GetLastLoader<TCif>()
        .FindLoopGlobal("_refln", false);
    }
    if (hklLoop == 0 && !TEFile::Exists(fcffn)) {
      fcffn = TEFile::ChangeFileExt(xapp.XFile().GetFileName(), "fco");
      if (!TEFile::Exists(fcffn)) {
        return "please load fcf file or make sure the one exists in current folder";
      }
    }
    if (hklLoop == 0) {
      sw.start("Loading CIF");
      cif = new TCif();
      cif->LoadFromFile(fcffn);
      hklLoop = cif->FindLoopGlobal("_refln", false);
      sw.stop();
    }
    if (hklLoop == 0) {
      return "no hkl loop found";
    }
    sw.start("Extracting CIF data");
    const size_t hInd = hklLoop->ColIndex("_refln_index_h");
    const size_t kInd = hklLoop->ColIndex("_refln_index_k");
    const size_t lInd = hklLoop->ColIndex("_refln_index_l");
    // list 3, F
    size_t mfInd = hklLoop->ColIndex("_refln_F_meas");
    size_t sfInd = hklLoop->ColIndex("_refln_F_sigma");
    bool fo_squared = false, fc_squared = false;;
    if (mfInd == InvalidIndex) {
      mfInd = hklLoop->ColIndex("_refln_F_squared_meas");
      sfInd = hklLoop->ColIndex("_refln_F_squared_sigma");
      fo_squared = true;
    }
    const size_t aInd = hklLoop->ColIndex("_refln_A_calc");
    const size_t bInd = hklLoop->ColIndex("_refln_B_calc");
    const size_t fcInd = hklLoop->ColIndex("_refln_F_calc");
    const size_t fcqInd = hklLoop->ColIndex("_refln_F_squared_calc");
    const size_t fcpInd = hklLoop->ColIndex("_refln_phase_calc");
    if ((hInd | kInd | lInd | mfInd | sfInd) == InvalidIndex) {
      return "undefined FCF data";
    }
    if ((aInd | bInd) == InvalidIndex && (fcInd | fcpInd) == InvalidIndex &&
        (fcqInd | fcpInd) == InvalidIndex)
    {
      return "undefined FCF data - list 3 or 6 is expected";
    }
    int list = (fcInd | fcpInd) != InvalidIndex ? 6
      : ((aInd | bInd) != InvalidIndex ? 3 : -6);
    fc_squared = list == -6;
    refs.SetCapacity(hklLoop->RowCount());
    F.SetCount(hklLoop->RowCount());
    olx_pset<uint32_t> read_in;
    if (list == 3 && !info_ex.centrosymmetric) {
      read_in.SetCapacity(refs.Count());
    }
    for (size_t i = 0; i < hklLoop->RowCount(); i++) {
      const cif_dp::CifRow& row = (*hklLoop)[i];
      TReflection& ref = refs.AddNew(
        row[hInd]->GetStringValue().ToInt(),
        row[kInd]->GetStringValue().ToInt(),
        row[lInd]->GetStringValue().ToInt(),
        row[mfInd]->GetStringValue().ToDouble(),
        row[sfInd]->GetStringValue().ToDouble());
      if (!fo_squared) {
        ref.SetS(olx_abs(ref.GetI())*ref.GetS() * sqrt(2.0));
        ref.SetI(olx_sqr(ref.GetI()));
      }
      compd rv;
      if (list == 3) {
        rv.Re() = row[aInd]->GetStringValue().ToDouble();
        rv.Im() = row[bInd]->GetStringValue().ToDouble();
        if (!info_ex.centrosymmetric) {
          read_in.Add(TReflection::CalcHKLHash(
            TReflection::Standardise(ref.GetHkl(), info_ex)));
        }
      }
      else if (list == 6) {
        rv = compd::polar(row[fcInd]->GetStringValue().ToDouble(),
          row[fcpInd]->GetStringValue().ToDouble()*M_PI/180);
      }
      else if (list == -6) {
        rv = compd::polar(sqrt(row[fcqInd]->GetStringValue().ToDouble()),
          row[fcpInd]->GetStringValue().ToDouble()*M_PI / 180);
      }
      if (mapType == mapTypeDiff) {
        double dI = (sqrt(olx_max(0, ref.GetI())) - rv.mod());
        F[i] = compd::polar(dI, rv.arg());
      }
      else if (mapType == mapType2OmC) {
        double dI = 2* sqrt(olx_max(0, ref.GetI())) - rv.mod();
        F[i] = compd::polar(dI, rv.arg());
      }
      else if (mapType == mapTypeObs) {
        F[i] = compd::polar(sqrt(olx_max(0, ref.GetI())), rv.arg());
      }
      else {
        F[i] = rv;
      }
    }
    // expand the FPs
    if (list == 3 && !info_ex.centrosymmetric) {
      F.SetCapacity(F.Count() * 2);
      refs.SetCapacity(refs.Count() * 2);
      size_t sz = refs.Count();
      for (size_t i = 0; i < sz; i++) {
        vec3i nr = -refs[i].GetHkl();
        uint32_t h = TReflection::CalcHKLHash(
          TReflection::Standardise(nr, info_ex));
        if (read_in.Contains(h)) {
          continue;
        }
        refs.Add(new TReflection(nr, refs[i].GetI(), refs[i].GetS()));
        F.Add(F[i].conj());
      }
    }
    //sw.start("Expanding the set");
    //TUnitCell::SymmSpace sp = xapp.XFile().GetUnitCell().GetSymmSpace();
    
    sw.stop();
  }
  else {  // olex2 calculated SF
    RefinementModel& rm = xapp.XFile().GetRM();
    if (rm.GetReflections().IsEmpty()) {
      return "no reflections";
    }
    sw.start("Loading/Filtering/Merging HKL");
    if (rm.GetHKLF() < 5) {
      RefinementModel::HklStat ms = rm.GetRefinementRefList<
        TUnitCell::SymmSpace, RefMerger::ShelxMerger>(sp, refs);
      sw.start("Calculating structure factors");
      if (xapp.XFile().GetRM().Vars.HasBASF()) {
        twinning::handler dt(info_ex, refs,
          xapp.XFile().GetRM().GetBASFAsDoubleList(),
          xapp.XFile().GetRM().GetTWIN_mat(),
          xapp.XFile().GetRM().GetTWIN_n());
        F.SetCount(dt.unique_indices.Count());
        //TArrayList<compd> Fc(dt.unique_indices.Count());
        CalcSF(xapp.XFile(), dt.unique_indices, F, true, anom_only);
        dt.detwin(twinning::detwinner_shelx(), refs, F);
        //dt.detwin_and_merge(twinning::detwinner_shelx(),
        //  RefMerger::ShelxMerger(), refs, Fc, &F);

        F.SetCount(refs.Count());
        //CalcSF(xapp.XFile(), refs, F, true);
        //xapp.XFile().GetRM().DetwinAlgebraic(refs, ms, info_ex);
      }
      else {
        F.SetCount(refs.Count());
        // this is a reference implementation for tests
        //xapp.CalcSF(refs, F);
        //sw.start("Calculation structure factors A");
        //fastsymm version is just about 10% faster...
        CalcSF(xapp.XFile(), refs, F, true, anom_only);
      }
    }
    else {
      twinning::handler twin(info_ex, rm.GetReflections(),
        RefUtil::ResolutionAndSigmaFilter(rm), rm.GetBASFAsDoubleList());
      TArrayList<compd> Fc(twin.unique_indices.Count());
      SFUtil::CalcSF(xapp.XFile(), twin.unique_indices, Fc, true, anom_only);
      twin.detwin_and_merge(twinning::detwinner_shelx(),
        RefMerger::ShelxMerger(), refs, Fc, &F);
    }

    //xapp.XFile().GetRM().DetwinRatio(refs, F, ms, info_ex);
    //xapp.XFile().GetRM().DetwinAlgebraic(refs, ms, info_ex);
    RefinementModel::EXTI::Shelxl exti_cr = rm.GetShelxEXTICorrector();
    RefinementModel::SWAT::Shelxl swat_cr = rm.GetShelxSWATCorrector();
    if (exti_cr.IsValid() || swat_cr.IsValid()) {
      sw.start("EXTI/SWAT");
      for (size_t i = 0; i < F.Count(); i++) {
        if (exti_cr.IsValid()) {
          F[i] *= exti_cr.CalcForFc(refs[i].GetHkl(), F[i].qmod());
        }
        else {
          F[i] *= swat_cr.CalcForFc(refs[i].GetHkl());
        }
      }
    }
    sw.start("Scaling structure factors");
    if (mapType != mapTypeCalc) {
      // find a linear scale between F
      double scale_k = 1;
      if (scaleType == scaleExternal) {
        double sc = RefUtil::CalcFScaleShelx(rm, F, refs);
        if (olx_abs((sc - scale) / (sc + scale)) > 0.01) {
          TBasicApp::NewLogEntry(logWarning) << "External scale deviates too much."
            " Using updated value of " << olxstr::FormatFloat(2, sc) <<
            " vs the given value of " << olxstr::FormatFloat(2, scale);
          scale_k = sc;
        }
        else {
          scale_k = scale;
        }
      }
      else if (scaleType == scaleExternalForced) {
        scale_k = scale;
      }
      else {
        if (scaleType == scaleSigma) {
          scale_k= RefUtil::CalcFScaleSigma(rm, F, refs);
        }
        else {
          scale_k = RefUtil::CalcFScaleShelx(rm, F, refs);
        }
        if (TBasicApp::GetInstance().IsProfiling()) {
          TBasicApp::NewLogEntry(logInfo) << "Fc = " << scale_k << "*Fo";
        }
      }
      for (size_t i=0; i < F.Count(); i++) {
        double Fo = scale_k * TReflection::GetF(refs[i]);
        if (mapType == mapTypeDiff) {
          F[i] = compd::polar(Fo, F[i].arg()) - F[i];
        }
        else if (mapType == mapType2OmC) {
          F[i] = compd::polar(2*Fo, F[i].arg()) - F[i];
        }
        else if (mapType == mapTypeObs) {
          F[i] = compd::polar(Fo, F[i].arg());
        }
      }
    }
  }
  return EmptyString();
}
//..............................................................................
void SFUtil::PrepareCalcSF(const TAsymmUnit& au, double* U,
  TTypeList<XScatterer>& scatterers, ElementPList &types,
  TCAtomPList& alist)
{
  const mat3d& hkl2c = au.GetHklToCartesian();
  double quad[6];
  // the thermal ellipsoid scaling factors
  double BM[6] = { hkl2c[0].Length(), hkl2c[1].Length(), hkl2c[2].Length(), 0, 0, 0 };
  BM[3] = 2 * BM[1] * BM[2];
  BM[4] = 2 * BM[0] * BM[2];
  BM[5] = 2 * BM[0] * BM[1];
  BM[0] *= BM[0];
  BM[1] *= BM[1];
  BM[2] *= BM[2];
  au.GetAtoms().ForEach(ACollectionItem::TagSetter(0));
  if (au.GetRefMod() && !au.GetRefMod()->OmittedAtoms().IsEmpty()) {
    TTypeList<ExplicitCAtomRef> l =
      au.GetRefMod()->OmittedAtoms().ExpandList(*au.GetRefMod());
    for (size_t i = 0; i < l.Count(); i++) {
      l[i].GetAtom().SetTag(1);
    }
  }
  for (size_t i = 0; i < au.AtomCount(); i++) {
    TCAtom& ca = au.GetAtom(i);
    if (ca.IsDeleted() || ca.GetType() == iQPeakZ || ca.GetTag() != 0) {
      continue;
    }
    size_t ind = types.IndexOf(ca.GetType());
    if (ind == InvalidIndex) {
      types.Add(ca.GetType());
      scatterers.AddNew(ca.GetType().symbol);
      ind = types.Count() - 1;
    }
    ca.SetTag(ind);
    ind = alist.Count() * 6;
    alist.Add(&ca);
    TEllipsoid* elp = ca.GetEllipsoid();
    if (elp != 0) {
      elp->GetShelxQuad(quad);  // default is Ucart
      au.UcartToUcif(quad);
      for (int k = 0; k < 6; k++) {
        U[ind + k] = -TQ_PI * quad[k] * BM[k];
      }
    }
    else {
      U[ind] = -EQ_PI * ca.GetUiso();
    }
  }
}
//..............................................................................
void SFUtil::_CalcSF(const TXFile& xfile, const IMillerIndexList& refs,
  TArrayList<TEComplex<double> >& F, bool UseFdp, const bool anom_only)
{
  TSpaceGroup &sg = xfile.GetLastLoaderSG();
  olx_object_ptr<ISF_Util> sf_util = GetSF_Util_Instance(sg);
  if (!sf_util.ok()) {
    throw TFunctionFailedException(__OlxSourceInfo, "invalid space group");
  }
  TAsymmUnit& au = xfile.GetAsymmUnit();
  olx_array_ptr<double> U = new double[6*au.AtomCount() + 1];
  TPtrList<TCAtom> alist;
  TTypeList<XScatterer> scatterers;
  ElementPList types;
  PrepareCalcSF(au, U, scatterers, types, alist);
  tensor::tensor_rank_2::initialise();
  tensor::tensor_rank_3::initialise();
  tensor::tensor_rank_4::initialise();
  const double ev = xfile.GetRM().expl.GetRadiationEnergy();
  olxstr_dict<XScatterer *, true> scs;
  for (size_t i=0; i < xfile.GetRM().SfacCount(); i++) {
    XScatterer &sc = xfile.GetRM().GetSfacData(i);
    scs(sc.GetLabel(), &sc);
  }
  for (size_t i=0; i < scatterers.Count(); i++) {
    XScatterer *xs = scs.Find(scatterers[i].GetLabel(), 0);
    if (xs == 0) {
      scatterers[i].SetSource(*types[i], ev);
    }
    else {
      if (xs->IsSet(XScatterer::setGaussian)) {
        scatterers[i].SetGaussians(xs->GetGaussians());
      }
      else {
        scatterers[i].SetGaussians(*types[i]->gaussians);
      }
      if (!UseFdp) {
        scatterers[i].SetFpFdp(xs->GetFpFdp().GetRe());
      }
      else {
        scatterers[i].SetFpFdp(xs->GetFpFdp());
      }
    }
  }
  sf_util->Calculate(
    refs,
    au.GetHklToCartesian(),
    F, scatterers,
    alist,
    U,
    anom_only
  );
  // apply modifications
  try {
    TXApp &xapp = TXApp::GetInstance();
    THklFile hkl;
    if (xapp.CheckFileType<TIns>()) {
      TIns& ins = xapp.XFile().GetLastLoader<TIns>();
      if (ins.InsExists("ABIN")) {
        olxstr fab_name = TEFile::ChangeFileExt(xapp.XFile().LocateHklFile(),
          "fab");
        if (!TEFile::Exists(fab_name)) {
          TBasicApp::NewLogEntry(logError) << "FAB file is missing";
        }
        else {
          hkl.LoadFromFile(fab_name, false, "free");
        }
      }
    }
    else if (xapp.CheckFileType<TCif>()) {
      using namespace cif_dp;
      TCif& cif = xapp.XFile().GetLastLoader<TCif>();
      cetStringList* s_fab = dynamic_cast<cetStringList *>(cif.FindEntry("_shelx_fab_file"));
      if (s_fab != 0) {
        hkl.LoadFromStrings(s_fab->lines, false, "free");
      }
    }
    if (hkl.RefCount() != 0) {
      TUnitCell::SymmSpace sp = xapp.XFile().GetUnitCell().GetSymmSpace();
      SymmSpace::InfoEx info_ex = SymmSpace::Compact(sp);
      olxdict<uint32_t, compd, TPrimitiveComparator> fab;
      TStrList missing;
      for (size_t i = 0; i < hkl.RefCount(); i++) {
        TReflection& r = hkl[i];
        int idx;
        vec3i si = TReflection::Standardise(r.GetHkl(), info_ex, &idx);
        compd v(r.GetI(), r.GetS());
        fab.Add(TReflection::CalcHKLHash(si),
          v * TReflection::PhaseShift(r.GetHkl(), info_ex, idx));
        if (!info_ex.centrosymmetric) {
          vec3i ni = -r.GetHkl();
          si = TReflection::Standardise(ni, info_ex, &idx);
          fab.Add(TReflection::CalcHKLHash(si),
            v.conj() * TReflection::PhaseShift(ni, info_ex, idx));
        }
      }
      for (size_t i = 0; i < F.Count(); i++) {
        int mi;
        vec3i stf = TReflection::Standardise(refs[i], info_ex, &mi);
        size_t idx = fab.IndexOf(TReflection::CalcHKLHash(stf));
        if (idx == InvalidIndex) {
          if (!TReflection::IsAbsent(stf, info_ex)) {
            missing << refs[i].ToString();
          }
          continue;
        }
        compd ps = TReflection::PhaseShift(refs[i], info_ex, mi);
        F[i] += fab.GetValue(idx) / ps;
      }
      if (!missing.IsEmpty()) {
        TBasicApp::NewLogEntry(logError) << "Missing modifications for the"
          " following reflections";
        TBasicApp::NewLogEntry(logError) << missing;
      }
    }
  }
  catch (const TExceptionBase &e) {
    TBasicApp::NewLogEntry(logError) << "Failed to apply Fc modifications";
    TBasicApp::NewLogEntry(logExceptionTrace) << e;
  }
}
//..............................................................................
