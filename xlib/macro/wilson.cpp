#include "egc.h"
#include "xmacro.h"

#include "hkl.h"
#include "unitcell.h"
#include "symmlib.h"

#include "etable.h"


//..............................................................................
struct TWilsonRef  {
  double ds, Fo2, Fe2;
  static int SortByDs(const TWilsonRef& r1, const TWilsonRef& r2)  {
    double v = r1.ds - r2.ds;
    if( v < 0 )  return -1;
    if( v > 0 ) return 1;
    return 0;
  }
  static int SortByFo2(const TWilsonRef& r1, const TWilsonRef& r2)  {
    double v = r1.Fo2 - r2.Fo2;
    if( v < 0 )  return -1;
    if( v > 0 ) return 1;
    return 0;
  }
  TWilsonRef()  {
    ds = Fo2 = Fe2 = 0;
  }
};
struct TWilsonBin {
  double SFo2, SFe2, Sds, Minds, Maxds;
  int Count;
  TWilsonBin(double minds, double maxds)  {
    SFe2 = SFo2 = Sds = 0;
    Count = 0;
    Maxds = maxds;
    Minds = minds;
  }
};
struct TWilsonEBin {
  double MinE, MaxE;
  int Count;
  TWilsonEBin(double minE, double maxE)  {
    Count = 0;
    MaxE = maxE;
    MinE = minE;
  }
};
void XLibMacros::macWilson(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXApp &XApp = TXApp::GetInstance();

  olxstr HklFN( XApp.LocateHklFile() );
  if( HklFN.IsEmpty() )  HklFN = TEFile::ChangeFileExt(XApp.XFile().GetFileName(), "hkl");
  if( !TEFile::FileExists(HklFN) )  {
    E.ProcessingError(__OlxSrcInfo, "could not locate the HKL file" );
    return;
  }
  THklFile Hkl;
  TVPointD hkl;
  Hkl.LoadFromFile(HklFN);

  olxstr outputFileName;
  if( Cmds.Count() != 0 )
    outputFileName = Cmds[0];
  else
    outputFileName << "wilson";
  outputFileName = TEFile::ChangeFileExt(outputFileName, "csv");

  TAsymmUnit& au = XApp.XFile().GetAsymmUnit();
  const TMatrixD& hkl2c = au.GetHklToCartesian();

  TSpaceGroup* sg = TSymmLib::GetInstance()->FindSG(au);
  if( sg == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "Undefined space group" );
    return;
  }

  TMatrixDList ml;
  sg->GetMatrices(ml, mattAll^mattIdentity);
//  if( !sg->IsCentrosymmetric() )  // merge friedel pairs
//  {
//    TMatrixD I(3,4);
//    I.E();
//    I *= -1;
//    ml.InsertCCopy(0, I);  // merge wil searhc for it ...
//  }

  TRefList Refs;
  THklFile::MergeStats st = Hkl.SimpleMerge(ml, Refs);
  ml.AddNew(3,4).E();  // add the identity matrix
  TPtrList<TWilsonBin> bins;
  TTypeList<TWilsonRef> refs;
  refs.SetCapacity(Refs.Count());

  TScattererLib scat_lib(9);
  TTypeList< AnAssociation2<TLibScatterer*, int> > scatterers;
  TPtrList<TBasicAtomInfo> bais;
  for( int i=0; i < au.AtomCount(); i++ )  {
    const TCAtom& ca = au.GetAtom(i);
    if( ca.IsDeleted() || ca.GetAtomInfo() == iQPeakIndex )  continue;
    int ind = bais.IndexOf( &ca.GetAtomInfo() );
    if( ind == -1 )  {
      scatterers.AddNew<TLibScatterer*, int>(scat_lib.Find(ca.GetAtomInfo().GetSymbol()), 0);
      ind = scatterers.Count() -1;
      if( scatterers[ind].GetA() == NULL ) {
        throw TFunctionFailedException(__OlxSourceInfo, olxstr("could not locate scatterer: ") << ca.GetAtomInfo().GetSymbol() );
      }
      bais.Add( &ca.GetAtomInfo() );
    }
    scatterers[ind].B() ++;
  }
  if( scatterers.IsEmpty() )  {
    bais.Add( &au.GetAtomsInfo()->GetAtomInfo(iCarbonIndex) );
    int atomCount = (int)XApp.XFile().GetUnitCell().CalcVolume()/18;
    scatterers.AddNew<TLibScatterer*, int>(scat_lib.Find("C"), atomCount);
  }
  else  {
    for( int i=0; i < scatterers.Count(); i++ )
      scatterers[i].B() *= au.GetZ();
  }

  double AvI = 0;
  const double T_PI = 2*M_PI;
  for( int i=0; i < Refs.Count(); i++ )  {
    AvI += Refs[i].GetI();
    Refs[i].MulHkl(hkl, hkl2c);
    TWilsonRef& ref = refs.AddNew();
    ref.ds = hkl.QLength()*0.25;
    ref.Fo2 = Refs[i].GetI();

    double fe = 0;
    for( int j=0; j < scatterers.Count(); j++)  {
      double v = scatterers[j].GetA()->Calc_sq(ref.ds);
      fe += v*v*scatterers[j].GetB();
    }
    ref.Fe2 = fe;// * Refs[i].GetDegeneracy();
  }

  AvI /= Refs.Count();
//  scatterers.Clear();

  olxstr strBinsCnt( Options.FindValue("b") );
  int binsCnt = strBinsCnt.IsEmpty() ? 10 : strBinsCnt.ToInt();
  if( binsCnt <= 0 ) binsCnt = 10;

  refs.QuickSorter.SortSF(refs, TWilsonRef::SortByDs);

  double minds=refs[0].ds, maxds=refs.Last().ds;
  double step = (maxds-minds)/binsCnt,
         hstep = step/2;
  for( int i=0; i < binsCnt; i++ )  {
    bins.Add( new TWilsonBin( minds + i*step, minds+(i+1)*step ) );
    if( (i+1) < binsCnt )
      bins.Add( new TWilsonBin( minds + (i+1)*step - hstep, minds+(i+1)*step + hstep ) );
  }

  for( int i=0; i < refs.Count(); i++ )  {
    const TWilsonRef& ref = refs[i];
    for( int j=0; j < bins.Count(); j++ )  {
      if( ref.ds < bins[j]->Maxds && ref.ds >= bins[j]->Minds )  {
        bins[j]->Count ++;
        bins[j]->SFo2 += ref.Fo2;
        bins[j]->SFe2 += ref.Fe2;
        bins[j]->Sds += ref.ds;
      }
    }
  }

  TStrList output, header;
  TTypeList< AnAssociation2<double,double> > binData;

  for( int i=0; i < bins.Count(); i++ )  {
    if( bins[i]->Count == 0 )  continue;
    bins[i]->SFo2 /= bins[i]->Count;
    bins[i]->SFe2 /= bins[i]->Count;
    bins[i]->Sds /= bins[i]->Count;

    //double fo = 0;
    //for( int j=0; j < scatterers.Count(); j++)  {
    //  double v = scatterers[j].GetA()->Calc_sq(bins[i]->Sds);
    //  fo += v*v*scatterers[j].GetB();
    //}

    //bins[i]->SFe2 = fo;
    binData.AddNew( log(bins[i]->SFo2/bins[i]->SFe2), bins[i]->Sds );
    delete bins[i];
  }
  if( binData.Count() != 0 )  {
    TTTable<TStrList> tab(binData.Count(), 2);
    tab.ColName(0) = "sin(theta)/lambda";
    tab.ColName(1) = "ln(<Fo2>)/(Fexp2)";
    TMatrixD points(2, binData.Count() );
    TVectorD line(2);
    for(int i=0; i < binData.Count(); i++ )  {
      points[0][i] = binData[i].GetB();
      points[1][i] = binData[i].GetA();
      tab[i][0] = olxstr::FormatFloat(3, points[0][i]);
      tab[i][1] = olxstr::FormatFloat(3, points[1][i]);
      output.Add( olxstr(points[1][i]) << ',' << points[0][i]);
    }
    double rms = TMatrixD::PLSQ(points, line, 1);
    olxstr &l = header.Add("Trendline y = ") << line[1] << "*x ";
    if( line[0] > 0 )
      l << '+';
    l << line[0];
    header.Add("RMS = ") << olxstr::FormatFloat(3, rms);
    olxstr scat;
    for( int i=0; i < scatterers.Count(); i++ )
      scat << bais[i]->GetSymbol() << scatterers[i].GetB() << ' ';
    tab.CreateTXTList(header, olxstr("Graph data for ") << scat, false, false, EmptyString);
    XApp.GetLog() << header;
    TCStrList(output).SaveToFile( outputFileName ) ;
    XApp.GetLog() << ( outputFileName << " file was created\n" );
    double K = exp(line[0]), B = -line[1]/2;
    double E2 = 0, SE2 = 0;
    int iE2GT2 = 0;
    for( int i=0; i < refs.Count(); i++ )  {
      refs[i].Fo2 /= (K*exp(-2*B*refs[i].ds)*Refs[i].GetDegeneracy()*refs[i].Fe2);
      if( refs[i].Fo2 < 0 )  refs[i].Fo2 = 0;
      SE2 += refs[i].Fo2;
      E2 += fabs(refs[i].Fo2-1);
      if( refs[i].Fo2 > 4 )  iE2GT2 ++;
    }
    E2 /= Refs.Count();
    XApp.GetLog() << ( olxstr("From Wilson plot: B = ") << olxstr::FormatFloat(3,B)
                                                 << " K = " << olxstr::FormatFloat(3,K) << '\n' );
    XApp.GetLog() << ( olxstr("<|E*E-1|> = ") << olxstr::FormatFloat(3,E2)
                                                   << "  [0.736 <- centro  +> 0.968]" << '\n' );
    XApp.GetLog() << ( olxstr("%|E| > 2 = ") << olxstr::FormatFloat(3,(double)iE2GT2*100/refs.Count() )
                                                   << "  [1.800 <- centro  +> 4.600]" << '\n' );
    if( E2 -(0.968+0.736)/2 < 0 )
      E.SetRetVal<bool>( false );
    else
      E.SetRetVal<bool>( true );
    /*
    refs.QuickSorter.SortSF(refs, TWilsonRef::SortByFo2);

    TPtrList<TWilsonEBin> ebins;
    int ebinsCnt = 400;
    double minE=refs[0].Fo2, maxE=refs.Last().Fo2;
    double estep = (maxE-minE)/ebinsCnt,
           ehstep = estep/2;

    for( int i=0; i < ebinsCnt; i++ )  {
      ebins.Add( new TWilsonEBin( minE + i*estep, minE+(i+1)*estep ) );
      if( (i+1) < ebinsCnt )
        ebins.Add( new TWilsonEBin( minE + (i+1)*estep - ehstep, minE+(i+1)*estep + ehstep ) );
    }

    for( int i=0; i < refs.Count(); i++ )  {
      TWilsonRef& ref = refs[i];
      for( int j=0; j < ebins.Count(); j++ )  {
        if( ref.Fo2 < ebins[j]->MaxE && ref.Fo2 >= ebins[j]->MinE )
          ebins[j]->Count ++;
      }
    }
    TTypeList< AnAssociation2<double,double> > ebinData;

    for( int i=0; i < ebins.Count(); i++ )  {
      if( ebins[i]->Count == 0 )  continue;
      ebinData.AddNew( (ebins[i]->MaxE+ebins[i]->MinE)/2, ebins[i]->Count );
      delete ebins[i];
    }
    if( ebinData.Count() != 0 )  {
      output.Clear();
      for(int i=0; i < ebinData.Count(); i++ )  {
        output.Add( olxstr(ebinData[i].GetA()) << ',' << ebinData[i].GetB());
      }
      output.SaveToFile( "Evalues.csv" ) ;
    }
  */
  }
}





