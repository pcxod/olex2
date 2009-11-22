#include "egc.h"

#include "xmacro.h"
#include "macroerror.h"
#include "paramlist.h"

#include "hkl.h"
#include "cif.h"
#include "ecomplex.h"
#include "log.h"

//..............................................................................
struct TGraphRSRef  {
  double ds, Fo, Fc;
  static int SortByDs(const TGraphRSRef& r1, const TGraphRSRef& r2)  {
    double v = r1.ds - r2.ds;
    if( v < 0 )  return -1;
    if( v > 0 ) return 1;
    return 0;
  }
};
struct TGraphRSBin {
  double SFo, SFc, Sds, Minds, Maxds;
  int Count;
  TGraphRSBin(double minds, double maxds)  {
    SFo = SFc = Sds = 0;
    Count = 0;
    Minds = minds;
    Maxds = maxds;
  }
};
void XLibMacros::macGraphSR(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXApp &XApp = TXApp::GetInstance();
  TCif* C;
  if( XApp.CheckFileType<TCif>() )  C = &XApp.XFile().GetLastLoader<TCif>();
  else  {
    olxstr fcffn = TEFile::ChangeFileExt(XApp.XFile().GetFileName(), "fcf");
    if( !TEFile::Exists(fcffn) )  {
      fcffn = TEFile::ChangeFileExt(XApp.XFile().GetFileName(), "fco");
      if( !TEFile::Exists(fcffn) )  {
        E.ProcessingError(__OlxSrcInfo, "please load fcf file or make sure the one exists in current folder");
        return;
      }
    }
    C = &TEGC::New<TCif>();
    C->LoadFromFile(fcffn);
  }
  TCifLoop* hklLoop = C->FindLoop("_refln");
  if( !hklLoop )  {
    E.ProcessingError(__OlxSrcInfo, "no hkl loop found");
    return;
  }
  XApp.GetLog() << (olxstr("Processing file ") << TEFile::ExtractFileName(C->GetFileName()) << '\n' );
  short list = -1;
  size_t hInd = hklLoop->GetTable().ColIndex("_refln_index_h");
  size_t kInd = hklLoop->GetTable().ColIndex("_refln_index_k");
  size_t lInd = hklLoop->GetTable().ColIndex("_refln_index_l");
  // list 3, F
  size_t mfInd = hklLoop->GetTable().ColIndex("_refln_F_meas");
  size_t sfInd = hklLoop->GetTable().ColIndex("_refln_F_sigma");
  size_t aInd = hklLoop->GetTable().ColIndex("_refln_A_calc");
  size_t bInd = hklLoop->GetTable().ColIndex("_refln_B_calc");
  // list 4, F^2
  size_t mf2Ind = hklLoop->GetTable().ColIndex("_refln_F_squared_meas");
  size_t sf2Ind = hklLoop->GetTable().ColIndex("_refln_F_squared_sigma");
  size_t cf2Ind = hklLoop->GetTable().ColIndex("_refln_F_squared_calc");

  if( mf2Ind != InvalidIndex && sf2Ind != InvalidIndex && cf2Ind != InvalidIndex )
    list = 4;
  else if( mfInd != InvalidIndex && sfInd != InvalidIndex && aInd != InvalidIndex && bInd != InvalidIndex )
    list = 3;

  if( hInd == InvalidIndex || kInd == InvalidIndex || lInd == InvalidIndex || list == InvalidIndex ) {
    E.ProcessingError(__OlxSrcInfo, "list 3/4 data is expected");
    return;
  }

  olxstr outputFileName;
  if( Cmds.Count() != 0 )
    outputFileName = Cmds[0];
  else
    outputFileName << "graphsr";
  outputFileName = TEFile::ChangeFileExt(outputFileName, "csv");

  TAsymmUnit& au = C->GetAsymmUnit();
  const mat3d& hkl2c = au.GetHklToCartesian();
  vec3d hkl;
  TPtrList<TGraphRSBin> bins;

  TTypeList<TGraphRSRef> refs;
  refs.SetCapacity(hklLoop->GetTable().RowCount());

  for( size_t i=0; i < hklLoop->GetTable().RowCount(); i++ )  {
    TStrPObjList<olxstr,TCifLoopData*>& row = hklLoop->GetTable()[i];
    hkl[0] = row[hInd].ToInt();
    hkl[1] = row[kInd].ToInt();
    hkl[2] = row[lInd].ToInt();
    hkl *= hkl2c;
    TGraphRSRef& ref = refs.AddNew();
    ref.ds = hkl.Length()*0.5;

    if( list == 3 )  {
      ref.Fo = olx_abs(row[mfInd].ToDouble());
      ref.Fc = TEComplex<double>(row[aInd].ToDouble(), row[bInd].ToDouble()).mod();
    }
    else if( list == 4 )  {
      ref.Fo = row[mf2Ind].ToDouble();
      ref.Fc = row[cf2Ind].ToDouble();
      if( ref.Fo < 0 )  ref.Fo = 0;
      else              ref.Fo = sqrt(ref.Fo);
      if( ref.Fc < 0 )  ref.Fc = 0;
      else              ref.Fc = sqrt(ref.Fc);
    }
  }
  olxstr strBinsCnt( Options.FindValue("b") );
  size_t binsCnt = strBinsCnt.IsEmpty() ? 11 : strBinsCnt.ToSizeT()/2;
  refs.QuickSorter.SortSF(refs, TGraphRSRef::SortByDs);

  double minds=refs[0].ds, maxds=refs.Last().ds;
  double step = (maxds-minds)/binsCnt,
         hstep = step/2;
  for( size_t i=0; i < binsCnt; i++ )  {
    bins.Add( new TGraphRSBin( minds + i*step, minds+(i+1)*step ) );
    if( (i+1) < binsCnt )
      bins.Add( new TGraphRSBin( minds + (i+1)*step - hstep, minds+(i+1)*step + hstep ) );
  }

  for( size_t i=0; i < refs.Count(); i++ )  {
    TGraphRSRef& ref = refs[i];
    for( size_t j=0; j < bins.Count(); j++ )  {
      if( ref.ds < bins[j]->Maxds && ref.ds >= bins[j]->Minds )  {
        bins[j]->Count ++;
        bins[j]->SFo += ref.Fo;
        bins[j]->SFc += ref.Fc;
        bins[j]->Sds += ref.ds;
      }
    }
  }

  TStrList output, header;
  TTypeList< AnAssociation2<double,double> > binData;
  for( size_t i=0; i < bins.Count(); i++ )  {
    if( bins[i]->Count != 0 )  {
      double rt = bins[i]->SFo / bins[i]->SFc;
      double d_s = bins[i]->Sds/bins[i]->Count;
      binData.AddNew(d_s, rt);
    }
    delete bins[i];
  }
  // find a gradient (tilt)
  if( binData.Count() != 0 )  {
    TTTable<TStrList> tab(binData.Count(), 3);
    tab.ColName(0) = "sin(theta)/lambda";
    tab.ColName(1) = "Sum(Fo)/Sum(Fc)";
    ematd points(2, binData.Count() );
    evecd line(5);
    for( size_t i=0; i < binData.Count(); i++ )  {
      points[0][i] = binData[i].GetA();
      points[1][i] = binData[i].GetB();
    }
    double rms = ematd::PLSQ(points, line, 3);
    
    for( size_t i=0; i < binData.Count(); i++ )  {
      tab[i][0] = olxstr::FormatFloat(3, binData[i].GetA());
      tab[i][1] = olxstr::FormatFloat(3, binData[i].GetB());
      double pv = evecd::PolynomValue(line, binData[i].GetA());
      tab[i][2] = olxstr::FormatFloat(3, pv);
      output.Add( olxstr(binData[i].GetA(), 50) << ',' << binData[i].GetB() << ',' << pv );
    }
    olxstr eq("y=");
    eq << olxstr::FormatFloat(3, line[0]);
    for( size_t i=1; i < line.Count(); i++ )  {
      if( line[i] < 0 )
        eq << olxstr::FormatFloat(3, line[i]);
       else
         eq << '+' << olxstr::FormatFloat(3, line[i]);
      eq << 'x';
      if( i > 1 )
        eq << '^' << i;
    }
    header.Add("Polynom ") << eq;
    header.Add("RMS = ") << olxstr::FormatFloat(3, rms);
    tab.CreateTXTList(header, "Sum(|Fo|)/Sum(|Fc|) vs sin(theta)/lambda.", false, false, EmptyString);
    XApp.GetLog() << header ;
    TCStrList(output).SaveToFile( outputFileName ) ;
    XApp.GetLog() << (outputFileName << " file was created\n" );
  }
}
