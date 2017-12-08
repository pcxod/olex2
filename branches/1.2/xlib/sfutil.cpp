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
  return new SF_Util<SG_Impl>(all_m, unq_m, sg.IsCentrosymmetric());
  //return new SF_Util<SG_Impl>(all_m, all_m, false);
#endif
}
//..............................................................................
void SFUtil::ExpandToP1(const TArrayList<vec3i>& hkl, const TArrayList<compd>& F,
  const TSpaceGroup& sg, TArrayList<StructureFactor>& out)
{
  if (hkl.Count() != F.Count())
    throw TInvalidArgumentException(__OlxSourceInfo,
      "hkl array and structure factors dimensions missmatch");
  ISF_Util* sf_util = GetSF_Util_Instance(sg);
  out.SetCount(sf_util->GetSGOrder()* hkl.Count());
  sf_util->Expand(hkl, F, out);
  delete sf_util;
  // test
  //smatd_list ml;
  //sg.GetMatrices(ml, mattAll);
  //const int ml_cnt = ml.Count();
  //out.SetCount( ml_cnt* hkl.Count() );
  //for( size_t i=0; i < hkl.Count(); i++ )  {
  //  const int off = i*ml_cnt;
  //  for( size_t j=0; j < ml_cnt; j++ )  {
  //    const int ind = off+j;
  //    out[ind].hkl = hkl[i]*ml[j].r;
  //    out[ind].ps = ml[j].t.DotProd(hkl[i]);
  //    if( out[ind].ps != 0 )  {
  //      double ca=1, sa=0;
  //      olx_sincos(-T_PI*out[ind].ps, &sa, &ca);
  //      out[ind].val = F[i]*compd(ca,sa);
  //    }
  //    else
  //      out[ind].val = F[i];
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
  short mapType, short sfOrigin, short scaleType, double scale)
{
  TXApp& xapp = TXApp::GetInstance();
  TStopWatch sw(__FUNC__);
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
      cif().LoadFromFile(fcffn);
      hklLoop = cif().FindLoopGlobal("_refln", false);
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
    bool squared = false;
    if (mfInd == InvalidIndex) {
      mfInd = hklLoop->ColIndex("_refln_F_squared_meas");
      sfInd = hklLoop->ColIndex("_refln_F_squared_sigma");
      squared = true;
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
    refs.SetCapacity(hklLoop->RowCount());
    F.SetCount(hklLoop->RowCount());
    for (size_t i=0; i < hklLoop->RowCount(); i++) {
      const cif_dp::CifRow& row = (*hklLoop)[i];
      TReflection& ref = refs.AddNew(
        row[hInd]->GetStringValue().ToInt(),
        row[kInd]->GetStringValue().ToInt(),
        row[lInd]->GetStringValue().ToInt(),
        row[mfInd]->GetStringValue().ToDouble(),
        row[sfInd]->GetStringValue().ToDouble());
      if (squared) {
        ref.SetI(sqrt(olx_max(0.0, ref.GetI())));
      }
      compd rv;
      if (list == 3) {
        rv.Re() = row[aInd]->GetStringValue().ToDouble();
        rv.Im() = row[bInd]->GetStringValue().ToDouble();
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
        double dI = (ref.GetI() - rv.mod());
        F[i] = compd::polar(dI, rv.arg());
      }
      else if (mapType == mapType2OmC) {
        double dI = 2*ref.GetI() - rv.mod();
        F[i] = compd::polar(dI, rv.arg());
      }
      else if (mapType == mapTypeObs) {
        F[i] = compd::polar(ref.GetI(), rv.arg());
      }
      else {
        F[i] = rv;
      }
    }
    sw.stop();
  }
  else {  // olex2 calculated SF
    olxstr hklFileName = xapp.XFile().LocateHklFile();
    if (!TEFile::Exists(hklFileName)) {
      return "could not locate hkl file";
    }
    sw.start("Loading/Filtering/Merging HKL");
    TUnitCell::SymmSpace sp = xapp.XFile().GetUnitCell().GetSymmSpace();
    SymmSpace::InfoEx info_ex = SymmSpace::Compact(sp);
    RefinementModel& rm = xapp.XFile().GetRM();
    if (rm.GetHKLF() < 5) {
      RefinementModel::HklStat ms =
        rm.GetRefinementRefList<
          TUnitCell::SymmSpace,RefMerger::ShelxMerger>(sp, refs);
      F.SetCount(refs.Count());
      sw.start("Calculating structure factors");
      //xapp.CalcSF(refs, F);
      //sw.start("Calculation structure factors A");
      //fastsymm version is just about 10% faster...
      CalcSF(xapp.XFile(), refs, F, !info_ex.centrosymmetric);
      xapp.XFile().GetRM().DetwinShelx(refs, F, ms, info_ex);
      //xapp.XFile().GetRM().DetwinMixed(refs, F, ms, info_ex);
      //xapp.XFile().GetRM().DetwinAlgebraic(refs, ms, info_ex);
    }
    else {
      twinning::general twin(info_ex, rm.GetReflections(),
        RefUtil::ResolutionAndSigmaFilter(rm), rm.GetBASFAsDoubleList());
      TArrayList<compd> Fc(twin.unique_indices.Count());
      SFUtil::CalcSF(xapp.XFile(), twin.unique_indices, Fc);
      twin.detwin_and_merge(twinning::detwinner_shelx(),
        RefMerger::ShelxMerger(), refs, Fc, &F);
    }

    //xapp.XFile().GetRM().DetwinRatio(refs, F, ms, info_ex);
    //xapp.XFile().GetRM().DetwinAlgebraic(refs, ms, info_ex);

    xapp.XFile().GetRM().CorrectExtiForF(refs, F, sp);
    sw.start("Scaling structure factors");
    if (mapType != mapTypeCalc) {
      // find a linear scale between F
      double a = 0, k = 1;
      if (scaleType == scaleExternal) {
        k = scale;
      }
      else if (scaleType == scaleRegression) {
        CalcFScale(F, refs, k, a);
        if (TBasicApp::GetInstance().IsProfiling()) {
          TBasicApp::NewLogEntry(logInfo) << "Fc^2 = " << k << "*Fo^2" <<
            (a >= 0 ? " +" : " ") << a;
        }
      }
      else {  // simple scale on I/sigma > 3
        k = CalcFScale(F, refs);
        if (TBasicApp::GetInstance().IsProfiling())
          TBasicApp::NewLogEntry(logInfo) << "Fc^2 = " << k << "*Fo^2";
      }
      const size_t f_cnt = F.Count();
      for (size_t i=0; i < f_cnt; i++) {
        const TReflection &r = refs[i];
        double dI = r.GetI() < 0 ? 0 : sqrt(r.GetI());
        dI *= k;
        if (scaleType == scaleRegression) {
          dI += a;
        }
        if (mapType == mapTypeDiff) {
          F[i] = compd::polar(dI, F[i].arg()) - F[i];
        }
        else if (mapType == mapType2OmC) {
          dI *= 2;
          dI -= F[i].mod();
          F[i] = compd::polar(dI, F[i].arg());
        }
        else if (mapType == mapTypeObs) {
          F[i] = compd::polar(dI, F[i].arg());
        }
      }
    }
  }
  return EmptyString();
}
//..............................................................................
void SFUtil::PrepareCalcSF(const TAsymmUnit& au, double* U, ElementPList& scatterers,
  TCAtomPList& alist)
{
  const mat3d& hkl2c = au.GetHklToCartesian();
  double quad[6];
  // the thermal ellipsoid scaling factors
  double BM[6] = {hkl2c[0].Length(), hkl2c[1].Length(), hkl2c[2].Length(), 0, 0, 0};
  BM[3] = 2*BM[1]*BM[2];
  BM[4] = 2*BM[0]*BM[2];
  BM[5] = 2*BM[0]*BM[1];
  BM[0] *= BM[0];
  BM[1] *= BM[1];
  BM[2] *= BM[2];

  au.GetAtoms().ForEach(ACollectionItem::TagSetter(0));
  if (au.GetRefMod() && !au.GetRefMod()->OmittedAtoms().IsEmpty()) {
    TTypeList<ExplicitCAtomRef> l =
      au.GetRefMod()->OmittedAtoms().ExpandList(*au.GetRefMod());
    for (size_t i=0; i < l.Count(); i++)
      l[i].GetAtom().SetTag(1);
  }
  for( size_t i=0; i < au.AtomCount(); i++ )  {
    TCAtom& ca = au.GetAtom(i);
    if (ca.IsDeleted() || ca.GetType() == iQPeakZ || ca.GetTag() != 0)
      continue;
    size_t ind = scatterers.IndexOf(ca.GetType());
    if( ind == InvalidIndex )  {
      scatterers.Add(ca.GetType());
      ind = scatterers.Count() - 1;
    }
    ca.SetTag(ind);
    ind = alist.Count()*6;
    alist.Add(&ca);
    TEllipsoid* elp = ca.GetEllipsoid();
    if( elp != NULL )  {
      elp->GetShelxQuad(quad);  // default is Ucart
      au.UcartToUcif(quad);
      for( int k=0; k < 6; k++ )
        U[ind+k] = -TQ_PI*quad[k]*BM[k];
    }
    else
      U[ind] = -EQ_PI*ca.GetUiso();
  }
}
//..............................................................................
void SFUtil::_CalcSF(const TXFile& xfile, const IMillerIndexList& refs,
  TArrayList<TEComplex<double> >& F, bool UseFdp)
{
  TSpaceGroup* sg = NULL;
  try  { sg = &xfile.GetLastLoaderSG();  }
  catch(...)  {
    throw TFunctionFailedException(__OlxSourceInfo, "unknown space group");
  }
  olx_object_ptr<ISF_Util> sf_util = GetSF_Util_Instance(*sg);
  if (!sf_util.is_valid()) {
    throw TFunctionFailedException(__OlxSourceInfo, "invalid space group");
  }
  TAsymmUnit& au = xfile.GetAsymmUnit();
  olx_array_ptr<double> U = new double[6*au.AtomCount() + 1];
  TPtrList<TCAtom> alist;
  ElementPList scatterers;
  PrepareCalcSF(au, U, scatterers, alist);
  TArrayList<compd> fpfdp(scatterers.Count(), olx_list_init::zero());
  const double ev = xfile.GetRM().expl.GetRadiationEnergy();
  olxstr_dict<XScatterer *, true> scs;
  for (size_t i=0; i < xfile.GetRM().SfacCount(); i++) {
    XScatterer &sc = xfile.GetRM().GetSfacData(i);
    scs(sc.GetLabel(), &sc);
  }
  for (size_t i=0; i < scatterers.Count(); i++) {
    XScatterer *xs = scs.Find(scatterers[i]->symbol, NULL);
    if (xs == NULL) {
      fpfdp[i] = scatterers[i]->CalcFpFdp(ev);
      fpfdp[i] -= scatterers[i]->z;
    }
    else {
      fpfdp[i] = xs->GetFpFdp();
    }
    if (!UseFdp)
      fpfdp[i].SetIm(0);
  }
  sf_util().Calculate(
    refs,
    au.GetHklToCartesian(),
    F, scatterers,
    alist,
    U,
    fpfdp
  );
  // apply modifications
  try {
    TXApp &xapp = TXApp::GetInstance();
    TUnitCell::SymmSpace sp = xapp.XFile().GetUnitCell().GetSymmSpace();
    SymmSpace::InfoEx info_ex = SymmSpace::Compact(sp);
    olxdict<uint32_t, compd, TPrimitiveComparator> fab;
    if (xapp.CheckFileType<TIns>()) {
      TIns &ins = xapp.XFile().GetLastLoader<TIns>();
      if (ins.InsExists("ABIN")) {
        olxstr fab_name = TEFile::ChangeFileExt(xapp.XFile().LocateHklFile(),
          "fab");
        if (!TEFile::Exists(fab_name)) {
          TBasicApp::NewLogEntry(logError) << "FAB file is missing";
        }
        else {
          THklFile hkl;
          hkl.LoadFromFile(fab_name, false, "free");
          for (size_t i = 0; i < hkl.RefCount(); i++) {
            TReflection &r = hkl[i];
            r.Standardise(info_ex);
            uint32_t key = r.GetHKLHash();
#ifdef _DEBUG
            if (fab.HasKey(key)) {
              TBasicApp::NewLogEntry(logError) <<
                "Duplicate modifications in the FAB";
            }
#endif
            fab.Add(key, compd(r.GetI(), r.GetS()));
          }
          for (size_t i = 0; i < F.Count(); i++) {
            size_t idx = fab.IndexOf(TReflection::CalcHKLHash(refs[i]));
            if (idx == InvalidIndex) {
              TBasicApp::NewLogEntry(logError) << "Missing modification";
            }
            else {
              F[i] += fab.GetValue(idx);
            }
          }
        }
      }
    }
  }
  catch (const TExceptionBase &e) {
    TBasicApp::NewLogEntry(logError) << "Failed to apply Fc modifications";
    TBasicApp::NewLogEntry(logExceptionTrace) << e;
  }
}
//..............................................................................
