//#define __OLX_USE_FASTSYMM

#include "sfutil.h"
#include "cif.h"
#include "hkl.h"
#include "unitcell.h"
#include "estopwatch.h"

using namespace SFUtil;

#ifdef __OLX_USE_FASTSYMM
  DefineFSFactory(ISF_Util, SF_Util)
#endif
//...........................................................................................
ISF_Util* SFUtil::GetSF_Util_Instance(const TSpaceGroup& sg)  {
#ifdef __OLX_USE_FASTSYMM
  ISF_Util* sf_util = fs_factory_ISF_Util(sg.GetName());
  if( sf_util == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid space group");
  return sf_util;
#else
  smatd_list ml;
  sg.GetMatrices(ml, mattAll);
  return new SF_Util<SG_Impl>(ml);
#endif
}
//...........................................................................................
void SFUtil::ExpandToP1(const TArrayList<vec3i>& hkl, const TArrayList<compd>& F, const TSpaceGroup& sg, TArrayList<StructureFactor>& out)  {
  if( hkl.Count() != F.Count() )
    throw TInvalidArgumentException(__OlxSourceInfo, "hkl array and structure factors dimentions must be equal");
  ISF_Util* sf_util = GetSF_Util_Instance(sg);
  out.SetCount( sf_util->GetSGOrder()* hkl.Count() );
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
  //      SinCos(-T_PI*out[ind].ps, &sa, &ca);
  //      out[ind].val = F[i]*compd(ca,sa);
  //    }
  //    else
  //      out[ind].val = F[i];
  //  }  
  //}
  //end test
}
//...........................................................................................
void SFUtil::FindMinMax(const TArrayList<StructureFactor>& F, vec3i& min, vec3i& max)  {
  min = vec3i(100, 100, 100);
  max = vec3i(-100, -100, -100);
  for( size_t i=0; i < F.Count(); i++ )  {
    if( F[i].hkl[0] > max[0] )  max[0] = F[i].hkl[0];
    if( F[i].hkl[0] < min[0] )  min[0] = F[i].hkl[0];

    if( F[i].hkl[1] > max[1] )  max[1] = F[i].hkl[1];
    if( F[i].hkl[1] < min[1] )  min[1] = F[i].hkl[1];

    if( F[i].hkl[2] > max[2] )  max[2] = F[i].hkl[2];
    if( F[i].hkl[2] < min[2] )  min[2] = F[i].hkl[2];
  }
}
//...........................................................................................
olxstr SFUtil::GetSF(TRefList& refs, TArrayList<compd>& F, 
                     short mapType, short sfOrigin, short scaleType)  {
  TXApp& xapp = TXApp::GetInstance();
  TStopWatch sw(__FUNC__);
  if( sfOrigin == sfOriginFcf )  {
    olxstr fcffn( TEFile::ChangeFileExt(xapp.XFile().GetFileName(), "fcf") );
    if( !TEFile::Exists(fcffn) )  {
      fcffn = TEFile::ChangeFileExt(xapp.XFile().GetFileName(), "fco");
      if( !TEFile::Exists(fcffn) )
        return "please load fcf file or make sure the one exists in current folder";
    }
    sw.start("Loading CIF");
    TCif cif;
    cif.LoadFromFile( fcffn );
    sw.stop();
    TCifLoop* hklLoop = cif.FindLoop("_refln");
    if( hklLoop == NULL )  {
      return "no hkl loop found";
    }
    sw.start("Extracting CIF data");
    size_t hInd = hklLoop->GetTable().ColIndex("_refln_index_h");
    size_t kInd = hklLoop->GetTable().ColIndex("_refln_index_k");
    size_t lInd = hklLoop->GetTable().ColIndex("_refln_index_l");
    // list 3, F
    size_t mfInd = hklLoop->GetTable().ColIndex("_refln_F_meas");
    size_t sfInd = hklLoop->GetTable().ColIndex("_refln_F_sigma");
    size_t aInd = hklLoop->GetTable().ColIndex("_refln_A_calc");
    size_t bInd = hklLoop->GetTable().ColIndex("_refln_B_calc");

    if( (hInd|kInd|lInd|mfInd|sfInd|aInd|bInd) == InvalidIndex  ) {
      return "list 3 fcf file is expected";
    }
    refs.SetCapacity( hklLoop->GetTable().RowCount() );
    F.SetCount( hklLoop->GetTable().RowCount() );
    for( size_t i=0; i < hklLoop->GetTable().RowCount(); i++ )  {
      TCifRow& row = hklLoop->GetTable()[i];
      TReflection& ref = refs.AddNew(row[hInd].ToInt(), row[kInd].ToInt(), 
        row[lInd].ToInt(), row[mfInd].ToDouble(), row[sfInd].ToDouble());
      if( mapType == mapTypeDiff )  {
        const compd rv(row[aInd].ToDouble(), row[bInd].ToDouble());
        double dI = (ref.GetI() - rv.mod());
        F[i] = compd::polar(dI, rv.arg());
      }
      else if( mapType == mapType2OmC )  {
        const compd rv(row[aInd].ToDouble(), row[bInd].ToDouble());
        double dI = 2*ref.GetI() - rv.mod();
        F[i] = compd::polar(dI, rv.arg());
      }
      else if( mapType == mapTypeObs ) {
        const compd rv(row[aInd].ToDouble(), row[bInd].ToDouble());
        F[i] = compd::polar(ref.GetI(), rv.arg());
      }
      else  {
        F[i].SetRe(row[aInd].ToDouble());
        F[i].SetIm(row[bInd].ToDouble());
      }
    }
    sw.stop();
  }
  else  {  // olex2 calculated SF
    olxstr hklFileName(xapp.LocateHklFile());
    if( !TEFile::Exists(hklFileName) )
      return "could not locate hkl file";
    double av = 0;
    sw.start("Loading/Filtering/Merging HKL");
    TUnitCell::SymSpace sp = xapp.XFile().GetUnitCell().GetSymSpace();
    RefinementModel::HklStat ms =
      xapp.XFile().GetRM().GetFourierRefList<TUnitCell::SymSpace,RefMerger::ShelxMerger>(sp, refs);
    F.SetCount(refs.Count());
    //THklFile::SaveToFile("e:/1.tmp", refs);
    sw.start("Calculating structure factors");
    //xapp.CalcSF(refs, F);
    //sw.start("Calculation structure factors A");
    //fastsymm version is just about 10% faster...
    CalcSF(xapp.XFile(), refs, F, !sp.IsCentrosymmetric());
    sw.start("Scaling structure factors");
    if( mapType != mapTypeCalc )  {
      // find a linear scale between F
      double a = 0, k = 1;
      if( scaleType == scaleRegression )  {
        CalcFScale(F, refs, k, a);
        if( TBasicApp::GetInstance().IsProfiling() )
          TBasicApp::GetLog().Info(olxstr("Fc^2 = ") << k << "*Fo^2" << (a >= 0 ? " +" : " ") << a );
      }
      else  {  // simple scale on I/sigma > 3
        k = CalcFScale(F, refs);
        if( TBasicApp::GetInstance().IsProfiling() )
          TBasicApp::GetLog().Info(olxstr("Fc^2 = ") << k << "*Fo^2");
      }
      const size_t f_cnt = F.Count();
      for( size_t i=0; i < f_cnt; i++ )  {
        double dI = refs[i].GetI() < 0 ? 0 : sqrt(refs[i].GetI());
        dI *= k;
        if( scaleType == scaleRegression )
          dI += a;
        if( mapType == mapTypeDiff )  {
          dI -= F[i].mod();
          F[i] = compd::polar(dI, F[i].arg());
        }
        else if( mapType == mapType2OmC )  {
          dI *= 2;
          dI -= F[i].mod();
          F[i] = compd::polar(dI, F[i].arg());
        }
        else if( mapType == mapTypeObs )  {
          F[i] = compd::polar(dI, F[i].arg());
        }
      }
    }
  }
  sw.print(xapp.GetLog(), &TLog::Info);
  return EmptyString;
}
//...........................................................................................
void SFUtil::PrepareCalcSF(const TAsymmUnit& au, double* U, ElementPList& scatterers, TCAtomPList& alist)  {
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
  
  for( size_t i=0; i < au.AtomCount(); i++ )  {
    TCAtom& ca = au.GetAtom(i);
    if( ca.IsDeleted() || ca.GetType() == iQPeakZ )  continue;
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
      elp->GetQuad(quad);  // default is Ucart
      au.UcartToUcif(quad);
      for( int k=0; k < 6; k++ )
        U[ind+k] = -TQ_PI*quad[k]*BM[k];
    }
    else
      U[ind] = -EQ_PI*ca.GetUiso();
  }
}
//...........................................................................................
void SFUtil::CalcSF(const TXFile& xfile, const TRefList& refs, TArrayList<TEComplex<double> >& F, bool useFpFdp)  {
  TSpaceGroup* sg = NULL;
  try  { sg = &xfile.GetLastLoaderSG();  }
  catch(...)  {
    throw TFunctionFailedException(__OlxSourceInfo, "unknown space group");
  }
  ISF_Util* sf_util = GetSF_Util_Instance(*sg);
  if( sf_util == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid space group");
  TAsymmUnit& au = xfile.GetAsymmUnit();
  double *U = new double[6*au.AtomCount() + 1];
  TPtrList<TCAtom> alist;
  ElementPList scatterers;
  PrepareCalcSF(au, U, scatterers, alist);

  sf_util->Calculate(
    xfile.GetRM().expl.GetRadiationEnergy(), 
    refs, 
    au.GetHklToCartesian(), 
    F, scatterers, 
    alist, 
    U, 
    useFpFdp
  );
  delete sf_util;
  delete [] U;
}
//...........................................................................................
void SFUtil::CalcSF(const TXFile& xfile, const TRefPList& refs, TArrayList<TEComplex<double> >& F, bool useFpFdp)  {
  TSpaceGroup* sg = NULL;
  try  { sg = &xfile.GetLastLoaderSG();  }
  catch(...)  {
    throw TFunctionFailedException(__OlxSourceInfo, "unknown space group");
  }
  ISF_Util* sf_util = GetSF_Util_Instance(*sg);
  if( sf_util == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid space group");
  TAsymmUnit& au = xfile.GetAsymmUnit();
  double *U = new double[6*au.AtomCount() + 1];
  TPtrList<TCAtom> alist;
  ElementPList scatterers;
  PrepareCalcSF(au, U, scatterers, alist);

  sf_util->Calculate(
    xfile.GetRM().expl.GetRadiationEnergy(), 
    refs, au.GetHklToCartesian(), 
    F, scatterers, 
    alist, 
    U, 
    useFpFdp
  );
  delete sf_util;
  delete [] U;
}

