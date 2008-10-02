#include "sfutil.h"
#include "cif.h"
#include "hkl.h"

DefineFSFactory(ISF_expansion,SF_expansion)

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
  if( sfOrigin == sfOriginFcf )  {
    olxstr fcffn( TEFile::ChangeFileExt(xapp.XFile().GetFileName(), "fcf") );
    if( !TEFile::FileExists(fcffn) )  {
      fcffn = TEFile::ChangeFileExt(xapp.XFile().GetFileName(), "fco");
      if( !TEFile::FileExists(fcffn) )
        return "please load fcf file or make sure the one exists in current folder";
    }
    TCif cif( xapp.AtomsInfo() );
    cif.LoadFromFile( fcffn );
    TCifLoop* hklLoop = cif.FindLoop("_refln");
    if( hklLoop == NULL )  {
      return "no hkl loop found";
    }
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
  }
  else  {  // olex2 calculated SF
    olxstr hklFileName( xapp.LocateHklFile() );
    if( !TEFile::FileExists(hklFileName) )
      return "could not locate hkl file";

    THklFile Hkl;
    Hkl.LoadFromFile(hklFileName);
    double av = 0;
    for( int i=0; i < Hkl.RefCount(); i++ )
      av += Hkl[i].GetI() < 0 ? 0 : Hkl[i].GetI();
    av /= Hkl.RefCount();

    THklFile::MergeStats ms = Hkl.Merge( xapp.XFile().GetLastLoaderSG(), true, refs);
    F.SetCount(refs.Count());
    xapp.CalcSF(refs, F);
    if( mapType != mapTypeCalc )  {
      // find a linear scale between F
      evecd line(2);
      double simple_scale = 0;
      if( scaleType == scaleRegression )  {
        ematd points(2, F.Count() );
        for( int i=0; i < F.Count(); i++ )  {
          points[0][i] = sqrt(refs[i].GetI());
          points[1][i] = F[i].mod();
        }
        double rms = ematd::PLSQ(points, line, 1);
        TBasicApp::GetLog() << olxstr("Trendline scale: ") << line.ToString() << '\n';
      }
      else  {  // simple scale on I/sigma > 3
        double sF2o = 0, sF2c = 0;
        for( int i=0; i < F.Count(); i++ )  {
          if( refs[i].GetI()/refs[i].GetS() < 3 )  continue;
          sF2o += refs[i].GetI();
          sF2c += F[i].qmod();
        }
        double simple_scale = sqrt(sF2o/sF2c);
        TBasicApp::GetLog() << olxstr("Simple scale: ") << olxstr::FormatFloat(3,simple_scale) << '\n';
      }
      for( int i=0; i < F.Count(); i++ )  {
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
  return EmptyString;
}

