#include "egc.h"
#include "xmacro.h"

#include "hkl.h"
#include "unitcell.h"
#include "symmlib.h"

#include "etable.h"
#include "emath.h"


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
  double SFo2, Fe2, Sds, Minds, Maxds;
  int Count;
  TWilsonBin(double minds, double maxds)  {
    Fe2 = SFo2 = Sds = 0;
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
  if( !TEFile::FileExists(HklFN) )  {
    E.ProcessingError(__OlxSrcInfo, "could not locate the HKL file" );
    return;
  }
  THklFile Hkl;
  vec3d hkl;
  Hkl.LoadFromFile(HklFN);

  olxstr outputFileName;
  if( Cmds.Count() != 0 )
    outputFileName = Cmds[0];
  else
    outputFileName << XApp.XFile().GetFileName();
  outputFileName = TEFile::ChangeFileExt(outputFileName, "wilson.csv");

  TAsymmUnit& au = XApp.XFile().GetAsymmUnit();
  const mat3d& hkl2c = au.GetHklToCartesian();

  TSpaceGroup* sg = TSymmLib::GetInstance()->FindSG(au);
  if( sg == NULL )  {
    E.ProcessingError(__OlxSrcInfo, "Undefined space group" );
    return;
  }

  TRefList Refs;
  THklFile::MergeStats st = Hkl.Merge(*sg, true, Refs);
  TTypeList<TWilsonBin> bins;
  TTypeList<TWilsonRef> refs;
  refs.SetCapacity(Refs.Count());

  TScattererLib scat_lib(9);
  TTypeList< AnAssociation2<TLibScatterer*, double> > scatterers;
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
    scatterers[ind].B() += ca.GetOccp();
  }
  if( scatterers.IsEmpty() )  {
    bais.Add( &au.GetAtomsInfo()->GetAtomInfo(iCarbonIndex) );
    int atomCount = (int)XApp.XFile().GetUnitCell().CalcVolume()/18;
    scatterers.AddNew<TLibScatterer*, int>(scat_lib.Find("C"), atomCount);
  }
  else  {
    for( int i=0; i < scatterers.Count(); i++ )
      scatterers[i].B() *= XApp.XFile().GetUnitCell().MatrixCount();
  }

  double minds=100, maxds=0;
  for( int i=0; i < Refs.Count(); i++ )  {
    Refs[i].MulHkl(hkl, hkl2c);
    TWilsonRef& ref = refs.AddNew();
    ref.ds = hkl.QLength()*0.25;
    if( ref.ds < minds )  minds = ref.ds;
    if( ref.ds > maxds )  maxds = ref.ds;
    ref.Fo2 = Refs[i].GetI() * Refs[i].GetDegeneracy();
    if( Refs[i].IsCentric() )
      ref.Fo2 /= 2;
    for( int j=0; j < scatterers.Count(); j++ )  {
      double v = scatterers[j].GetA()->Calc_sq( ref.ds );
      ref.Fe2 += v*v*scatterers[j].GetB();
    }
  }

  // get parameters
  int binsCnt = Options.FindValue("b", "10").ToInt();
  bool picture = Options.Contains("p");
  if( binsCnt <= 0 ) binsCnt = 10;

  double dsR = maxds-minds;
  if( ! picture )  {  /// use spherical bins
    double Vtot = SphereVol(sqrt(maxds)), Vstep = Vtot/binsCnt, 
      Vstart = SphereVol(sqrt(minds)),
      Vhstep = Vstep/2;
    for( int i=0; i < binsCnt; i++ )  {
      double sds = SphereRad(Vstart);
      sds *= sds;
      double eds = SphereRad(Vstart+Vstep);
      eds *= eds;
      bins.AddNew(sds, eds);
      if( (i+1) < binsCnt )  {  // add intermediate, overlapping bin for smoothing
        sds = SphereRad(Vstart+Vhstep);
        sds *= sds;
        eds = SphereRad(Vstart+Vhstep+Vstep);
        eds *= eds;
        bins.AddNew(sds, eds);
      }
      Vstart += Vstep;
    }
  }
  else  {
    double step = (maxds-minds)/binsCnt,
      hstep = step/2;
    for( int i=0; i < binsCnt; i++ )  {
      bins.AddNew( minds + i*step, minds+(i+1)*step );
      if( (i+1) < binsCnt )
        bins.AddNew(minds + (i+1)*step - hstep, minds+(i+1)*step + hstep );
    }
  }
  // calculate Fexpected for the bins
  for( int i=0; i < bins.Count(); i++ )  {
    for( int j=0; j < scatterers.Count(); j++ )  {
      double v = scatterers[j].GetA()->Calc_sq( (bins[i].Maxds+bins[i].Minds)/2 );
      bins[i].Fe2 += v*v*scatterers[j].GetB();
    }
  }
  // fill the bins and assign Fexp to the refelections
  for( int i=0; i < refs.Count(); i++ )  {
    TWilsonRef& ref = refs[i];
    for( int j=0; j < bins.Count(); j++ )  {
      if( ref.ds < bins[j].Maxds && ref.ds > bins[j].Minds )  {
        bins[j].Count ++;
        bins[j].SFo2 += ref.Fo2;
        bins[j].Sds += ref.ds;
      }
    }
  }

  TTypeList< AnAssociation2<double,double> > binData;
  // normalise summs
  for( int i=0; i < bins.Count(); i++ )  {
    if( bins[i].Count == 0 )  continue;
    bins[i].SFo2 /= bins[i].Count;
    bins[i].Sds /= bins[i].Count;
    binData.AddNew( log(bins[i].SFo2/bins[i].Fe2), (bins[i].Maxds+bins[i].Minds)/2 );
  }

  TStrList output, header;
  if( binData.Count() != 0 )  {
    TTTable<TStrList> tab(binData.Count(), 2);
    tab.ColName(0) = "sin^2(theta)/lambda^2";
    tab.ColName(1) = "ln(<Fo2>)/(Fexp2)";
    ematd points(2, binData.Count() );
    evecd line(2);
    for(int i=0; i < binData.Count(); i++ )  {
      points[0][i] = binData[i].GetB();
      points[1][i] = binData[i].GetA();
      tab[i][0] = olxstr::FormatFloat(3, points[0][i]);
      tab[i][1] = olxstr::FormatFloat(3, points[1][i]);
      output.Add( olxstr(points[1][i]) << ',' << points[0][i]);
    }
    double rms = ematd::PLSQ(points, line, 1);
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
    double K = exp(line[0]), B = -line[1]/2;
    double E2 = 0, SE2 = 0;
    int iE2GT2 = 0;
    for( int i=0; i < refs.Count(); i++ )  {
      refs[i].Fo2 /= (K*exp(-2*B*refs[i].ds)*refs[i].Fe2);
      //refs[i].Fo2 /= (K*exp(-2*B*refs[i].ds)*Refs[i].GetDegeneracy()*refs[i].Fe2);
      if( refs[i].Fo2 < 0 )  refs[i].Fo2 = 0;
      SE2 += refs[i].Fo2;
      E2 += fabs(refs[i].Fo2-1);
      if( refs[i].Fo2 > 4 )  iE2GT2 ++;
    }
    E2 /= Refs.Count();
    XApp.GetLog() << ( olxstr("From Wilson plot: B = ") << olxstr::FormatFloat(3,B)
                                                 << " K = " << olxstr::FormatFloat(3,1./K) << '\n' );
    XApp.GetLog() << ( olxstr("<|E*E-1|> = ") << olxstr::FormatFloat(3,E2)
                                                   << "  [0.736 <- centro  +> 0.968]" << '\n' );
    XApp.GetLog() << ( olxstr("%|E| > 2 = ") << olxstr::FormatFloat(3,(double)iE2GT2*100/refs.Count())
                                                   << "  [1.800 <- centro  +> 4.600]" << '\n' );
    for(int i=0; i < binData.Count(); i++ )  {
      points[1][i] = binData[i].GetB();
      points[0][i] = binData[i].GetA();
    }
    rms = ematd::PLSQ(points, line, 1);

    output.Add("#Title = Wilson plot");
    output.Add("#y_label = sin^2(theta)/lambda^2");
    output.Add("#x_label = ln(<Fo^2>)/(Fexp^2)");
    output.Add(olxstr("#y = ") << olxstr::FormatFloat(3,line[1]) << "*x" 
      << ((line[1] < 0) ? " " : "+") << olxstr::FormatFloat(3,line[0]) );
    output.Add("#B = ") << olxstr::FormatFloat(3,B);
    output.Add("#K = ") << olxstr::FormatFloat(3,1./K);
    output.Add("#<|E^2-1|> = ") << olxstr::FormatFloat(3,E2);
    output.Add("#%|E| > 2 = ") << olxstr::FormatFloat(3,(double)iE2GT2*100/refs.Count());

    TCStrList(output).SaveToFile( outputFileName ) ;
    XApp.GetLog() << ( outputFileName << " file was created\n" );

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
