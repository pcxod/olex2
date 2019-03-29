/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

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
  static int SortByDs(const TGraphRSRef &r1, const TGraphRSRef &r2)  {
    return olx_cmp(r1.ds, r2.ds);
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
void XLibMacros::macGraphSR(TStrObjList &Cmds, const TParamList &Options, TMacroData &E)  {
  TXApp &XApp = TXApp::GetInstance();
  olx_object_ptr<TCif> C;
  if (XApp.CheckFileType<TCif>()) {
    C = &XApp.XFile().GetLastLoader<TCif>();
    C.p->inc_ref();
  }
  else {
    olxstr fcffn = TEFile::ChangeFileExt(XApp.XFile().GetFileName(), "fcf");
    if (!TEFile::Exists(fcffn)) {
      fcffn = TEFile::ChangeFileExt(XApp.XFile().GetFileName(), "fco");
      if (!TEFile::Exists(fcffn)) {
        E.ProcessingError(__OlxSrcInfo,
          "please load fcf file or make sure the one exists in current folder");
        return;
      }
    }
    C = new TCif();
    C().LoadFromFile(fcffn);
  }
  cif_dp::cetTable* hklLoop = C().FindLoop("_refln");
  if (hklLoop == NULL) {
    E.ProcessingError(__OlxSrcInfo, "no hkl loop found");
    return;
  }
  XApp.NewLogEntry() << "Processing file " <<
    TEFile::ExtractFileName(C().GetFileName());
  short list = -1;
  const size_t hInd = hklLoop->ColIndex("_refln_index_h");
  const size_t kInd = hklLoop->ColIndex("_refln_index_k");
  const size_t lInd = hklLoop->ColIndex("_refln_index_l");
  // list 3, F
  const size_t mfInd = hklLoop->ColIndex("_refln_F_meas");
  const size_t sfInd = hklLoop->ColIndex("_refln_F_sigma");
  const size_t aInd = hklLoop->ColIndex("_refln_A_calc");
  const size_t bInd = hklLoop->ColIndex("_refln_B_calc");
  // list 4, F^2
  const size_t mf2Ind = hklLoop->ColIndex("_refln_F_squared_meas");
  const size_t sf2Ind = hklLoop->ColIndex("_refln_F_squared_sigma");
  const size_t cf2Ind = hklLoop->ColIndex("_refln_F_squared_calc");

  if( (mf2Ind|sf2Ind|cf2Ind) != InvalidIndex )
    list = 4;
  else if( (mfInd|sfInd|aInd|bInd) != InvalidIndex )
    list = 3;

  if( (hInd|kInd|lInd) == InvalidIndex || list == -1 ) {
    E.ProcessingError(__OlxSrcInfo, "list 3/4 data is expected");
    return;
  }

  olxstr outputFileName;
  if( Cmds.Count() != 0 )
    outputFileName = Cmds[0];
  else
    outputFileName << "graphsr";
  outputFileName = TEFile::ChangeFileExt(outputFileName, "csv");

  TAsymmUnit& au = C().GetAsymmUnit();
  const mat3d& hkl2c = au.GetHklToCartesian();
  vec3d hkl;
  TPtrList<TGraphRSBin> bins;

  TTypeList<TGraphRSRef> refs;
  refs.SetCapacity(hklLoop->RowCount());

  for( size_t i=0; i < hklLoop->RowCount(); i++ )  {
    const cif_dp::CifRow& row = (*hklLoop)[i];
    hkl[0] = row[hInd]->GetStringValue().ToInt();
    hkl[1] = row[kInd]->GetStringValue().ToInt();
    hkl[2] = row[lInd]->GetStringValue().ToInt();
    hkl *= hkl2c;
    TGraphRSRef& ref = refs.AddNew();
    ref.ds = hkl.Length()*0.5;

    if( list == 3 )  {
      ref.Fo = olx_abs(row[mfInd]->GetStringValue().ToDouble());
      ref.Fc = TEComplex<double>(
        row[aInd]->GetStringValue().ToDouble(), row[bInd]->GetStringValue().ToDouble()).mod();
    }
    else if( list == 4 )  {
      ref.Fo = row[mf2Ind]->GetStringValue().ToDouble();
      ref.Fc = row[cf2Ind]->GetStringValue().ToDouble();
      if( ref.Fo < 0 )  ref.Fo = 0;
      else              ref.Fo = sqrt(ref.Fo);
      if( ref.Fc < 0 )  ref.Fc = 0;
      else              ref.Fc = sqrt(ref.Fc);
    }
  }
  olxstr strBinsCnt( Options.FindValue("b") );
  size_t binsCnt = strBinsCnt.IsEmpty() ? 11 : strBinsCnt.ToSizeT()/2;
  QuickSorter::SortSF(refs, TGraphRSRef::SortByDs);

  double minds=refs[0].ds, maxds=refs.GetLast().ds;
  double step = (maxds-minds)/binsCnt,
         hstep = step/2;
  for (size_t i=0; i < binsCnt; i++) {
    bins.Add( new TGraphRSBin( minds + i*step, minds+(i+1)*step ) );
    if ((i + 1) < binsCnt) {
      bins.Add(new TGraphRSBin(minds + (i + 1)*step - hstep,
        minds + (i + 1)*step + hstep));
    }
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
  TTypeList< olx_pair_t<double,double> > binData;
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
      output.Add(olxstr(binData[i].GetA(), 50) << ',' << binData[i].GetB() <<
        ',' << pv);
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
    tab.CreateTXTList(header, "Sum(|Fo|)/Sum(|Fc|) vs sin(theta)/lambda.",
      false, false, EmptyString());
    XApp.NewLogEntry() << header;
    TEFile::WriteLines(outputFileName, TCStrList(output));
    XApp.NewLogEntry() << outputFileName << " file was created";
  }
}
