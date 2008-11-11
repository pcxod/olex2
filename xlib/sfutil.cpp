#include "sfutil.h"
#include "cif.h"
#include "hkl.h"
#include "estopwatch.h"

DefineFSFactory(ISF_expansion,SF_expansion)
DefineFSFactory(ISF_calculation,SF_calculation)

//...........................................................................................
void SFUtil::ExpandToP1(const TArrayList<vec3i>& hkl, const TArrayList<compd>& F, const TSpaceGroup& sg, TArrayList<StructureFactor>& out)  {
  if( hkl.Count() != F.Count() )
    throw TInvalidArgumentException(__OlxSourceInfo, "hkl array and structure factors dimentions must be equal");
  ISF_expansion* sf_expansion = fs_factory_ISF_expansion(sg.GetName());
  if( sf_expansion == NULL )
    throw TFunctionFailedException(__OlxSourceInfo, "invalid space group");
  out.SetCount( sf_expansion->GetSGOrder()* hkl.Count() );
  sf_expansion->Expand(hkl, F, out);
  delete sf_expansion;
}
//...........................................................................................
void SFUtil::FindMinMax(const TArrayList<StructureFactor>& F, vec3i& min, vec3i& max)  {
  min = vec3i(100, 100, 100);
  max = vec3i(-100, -100, -100);
  for( int i=0; i < F.Count(); i++ )  {
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
    if( !TEFile::FileExists(fcffn) )  {
      fcffn = TEFile::ChangeFileExt(xapp.XFile().GetFileName(), "fco");
      if( !TEFile::FileExists(fcffn) )
        return "please load fcf file or make sure the one exists in current folder";
    }
    sw.start("Loading CIF");
    TCif cif( xapp.AtomsInfo() );
    cif.LoadFromFile( fcffn );
    sw.stop();
    TCifLoop* hklLoop = cif.FindLoop("_refln");
    if( hklLoop == NULL )  {
      return "no hkl loop found";
    }
    sw.start("Extracting CIF data");
    int hInd = hklLoop->Table().ColIndex("_refln_index_h");
    int kInd = hklLoop->Table().ColIndex("_refln_index_k");
    int lInd = hklLoop->Table().ColIndex("_refln_index_l");
    // list 3, F
    int mfInd = hklLoop->Table().ColIndex("_refln_F_meas");
    int sfInd = hklLoop->Table().ColIndex("_refln_F_sigma");
    int aInd = hklLoop->Table().ColIndex("_refln_A_calc");
    int bInd = hklLoop->Table().ColIndex("_refln_B_calc");

    if( hInd == -1 || kInd == -1 || lInd == -1 || 
      mfInd == -1 || sfInd == -1 || aInd == -1 || bInd == -1  ) {
        return "list 3 fcf file is expected";
    }
    refs.SetCapacity( hklLoop->Table().RowCount() );
    F.SetCount( hklLoop->Table().RowCount() );
    for( int i=0; i < hklLoop->Table().RowCount(); i++ )  {
      TStrPObjList<olxstr,TCifLoopData*>& row = hklLoop->Table()[i];
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
    olxstr hklFileName( xapp.LocateHklFile() );
    if( !TEFile::FileExists(hklFileName) )
      return "could not locate hkl file";
    THklFile Hkl;
    sw.start("Loading HKL file");
    Hkl.LoadFromFile(hklFileName);
    sw.stop();
    double av = 0;
    for( int i=0; i < Hkl.RefCount(); i++ )
      av += Hkl[i].GetI() < 0 ? 0 : Hkl[i].GetI();
    av /= Hkl.RefCount();
    sw.start("Merging HKL");
    THklFile::MergeStats ms = Hkl.Merge( xapp.XFile().GetLastLoaderSG(), true, refs);
    F.SetCount(refs.Count());
    sw.start("Calculation structure factors");
    xapp.CalcSF(refs, F);
    sw.start("Scaling structure factors");
    if( mapType != mapTypeCalc )  {
      // find a linear scale between F
      evecd line(2);
      double simple_scale = 1.0;
      const int f_cnt = F.Count();
      if( scaleType == scaleRegression )  {
        ematd points(2, f_cnt );
        for( int i=0; i < f_cnt; i++ )  {
          points[0][i] = sqrt(refs[i].GetI());
          points[1][i] = F[i].mod();
        }
        double rms = ematd::PLSQ(points, line, 1);
        TBasicApp::GetLog().Info(olxstr("Trendline scale: ") << line.ToString());
      }
      else  {  // simple scale on I/sigma > 3
        double sF2o = 0, sF2c = 0;
        for( int i=0; i < f_cnt; i++ )  {
          if( refs[i].GetI() < 3*refs[i].GetS() )  continue;
          sF2o += refs[i].GetI();
          sF2c += F[i].qmod();
        }
        double simple_scale = sqrt(sF2o/sF2c);
        TBasicApp::GetLog().Info(olxstr("Simple scale: ") << olxstr::FormatFloat(3,simple_scale));
      }
      for( int i=0; i < f_cnt; i++ )  {
        double dI = sqrt(refs[i].GetI());
        if( scaleType == scaleSimple )
          dI /= simple_scale;
        else if( scaleType == scaleRegression )  {
          dI *= line[1];
          dI += line[0];
        }
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
  sw.print( xapp.GetLog(), &TLog::Info );
  return EmptyString;
}
//...........................................................................................
void SFUtil::CalcSF(TXFile& xfile, const TRefList& refs, TArrayList<TEComplex<double> >& F, bool useFpDfp)  {
  TSpaceGroup* sg = NULL;
  try  { sg = &xfile.GetLastLoaderSG();  }
  catch(...)  {
    throw TFunctionFailedException(__OlxSourceInfo, "unknown spacegroup");
  }
  TAsymmUnit& au = xfile.GetAsymmUnit();
  const mat3d& hkl2c = au.GetHklToCartesian();
  double quad[6];
  const static double EQ_PI = 8*QRT(M_PI);
  const static double TQ_PI = 2*QRT(M_PI);
  double WaveLength = 0.71073;

  // the thermal ellipsoid scaling factors
  double BM[6] = {hkl2c[0].Length(), hkl2c[1].Length(), hkl2c[2].Length(), 0, 0, 0};
  BM[3] = 2*BM[1]*BM[2];
  BM[4] = 2*BM[0]*BM[2];
  BM[5] = 2*BM[0]*BM[1];
  BM[0] *= BM[0];
  BM[1] *= BM[1];
  BM[2] *= BM[2];
  
  TPtrList<TBasicAtomInfo> bais;
  TPtrList<TCAtom> alist;
  double *Ucifs = new double[6*au.AtomCount() + 1];
  TPtrList<cm_Element> scatterers;
  for( int i=0; i < au.AtomCount(); i++ )  {
    TCAtom& ca = au.GetAtom(i);
    if( ca.IsDeleted() || ca.GetAtomInfo() == iQPeakIndex )  continue;
    int ind = bais.IndexOf( &ca.GetAtomInfo() );
    if( ind == -1 )  {
      if( ca.GetAtomInfo() == iDeuteriumIndex ) // treat D as H
        scatterers.Add(XElementLib::FindBySymbol("H"));
      else 
        scatterers.Add(XElementLib::FindBySymbol(ca.GetAtomInfo().GetSymbol()));
     
      if( scatterers.Last() == NULL ) {
        delete [] Ucifs;
        throw TFunctionFailedException(__OlxSourceInfo, olxstr("could not locate scatterer: ") << ca.GetAtomInfo().GetSymbol() );
      }
      bais.Add( &ca.GetAtomInfo() );
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
        Ucifs[ind+k] = -TQ_PI*quad[k]*BM[k];
    }
    else  {
      Ucifs[ind] = ca.GetUiso();
      Ucifs[ind] *= -EQ_PI;
    }
  }

  ISF_calculation* sf_calculation = fs_factory_ISF_calculation(sg->GetName());
  if( sf_calculation == NULL )  {
    delete [] Ucifs;
    throw TFunctionFailedException(__OlxSourceInfo, "invalid space group");
  }
  sf_calculation->Calculate(WaveLength, refs, F, scatterers, alist, Ucifs);
  delete sf_calculation;

  double sF2o = 0, sF2c = 0;
  const int f_cnt = F.Count();
  for( int i=0; i < f_cnt; i++ )  {
    if( refs[i].GetI() < 3*refs[i].GetS() )  continue;
    sF2o += refs[i].GetI();
    sF2c += F[i].qmod();
  }
  double simple_scale = sF2o/sF2c;
  for( int i=0; i < f_cnt; i++ ) 
    refs[i].SetI( refs[i].GetI()/simple_scale );
  delete [] Ucifs;
}
