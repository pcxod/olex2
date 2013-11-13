/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#include "lst.h"
#include "efile.h"
#include "etable.h"
#include "ins.h"
#include "xmacro.h"

int SortTrefTries(const TTrefTry &I1, const TTrefTry &I2) {
  if( I1.CFOM < I2.CFOM )  return -1;
  if( I1.CFOM > I2.CFOM )  return 1;
  int res = I1.Semivariants.Compare(I2.Semivariants);
  if( res == 0)
    return olx_cmp(I1.NQual, I2.NQual);
  return res;
}
//..............................................................................
//..............................................................................
void TLst::Clear()  {
  params.Clear();
  FDRefs.Clear();
  FSplitAtoms.Clear();
  TrefTries.Clear();
  PattSolutions.Clear();
  ErrorMsgs.Clear();
  Loaded = false;
}
//..............................................................................
bool TLst::LoadFromFile(const olxstr &FN)  {
  TEFile::CheckFileExists(__OlxSourceInfo, FN);
  TStrList SL;
  bool TRefC  = false,
       URefC  = false,
       DRef   = false,
       HP     = false,
       DH     = false,
       RF     = false,
       RIS    = false,
       SA     = false, // split atoms
       TrefT  = false,
       PattS  = false,
       FlackF = false,
       CellInfo = false,
       HasTwin = false,
       InvTwin = false;
  bool header_found = false;
  TStrList Toks;
  Clear();
  SL.LoadFromFile(FN);
  // skip INS file
  size_t start = 0;
  while (start < SL.Count() && !SL[start].Contains("Reflections read,")) {
    start++;
    continue;
  }
  for( size_t i=start; i < SL.Count(); i++ )  {
    Toks.Clear();
    if (!header_found && SL[i].Contains("++") &&
      (i+1) < SL.Count() && SL[i].Contains('+'))
    {
      i++;
      size_t hi = SL[i].FirstIndexOf('-');
      size_t spi = SL[i].LastIndexOf(' ', hi);
      if (spi == InvalidIndex || hi == InvalidIndex) continue;
      params("program", SL[i].SubString(spi+1, hi-spi-1));
      spi = SL[i].FirstIndexOf(' ', hi);
      if (spi == InvalidIndex) continue;
      params("version", SL[i].SubString(hi+1, spi-hi-1));
      i++;
      if (i >= SL.Count()) continue;
      Toks.Strtok(SL[i], ' ');
      if (Toks.Count() > 3 && Toks[Toks.Count()-3].Equals("Release")) {
        params.Add("version") = Toks[Toks.Count()-2];
      }
      header_found = true;
    }
    else if (!TRefC && SL[i].Contains("Reflections read,")) {
      Toks.Strtok(SL[i], ' ');
      if (Toks.Count() < 3)  continue;
      params("ref_total", Toks[0]);
      int rt = Toks[0].ToInt();
      try {  //bug: 662044  Reflections read, of which447594  rejected
        int ur = rt - Toks[5].ToInt(); // uniq = total - rejected
        params("ref_unique", ur);
      }
      catch(...)  {}
      TRefC = true;
    }
    else if (!URefC && SL[i].Contains("Unique reflections,")) {
      Toks.Strtok(SL[i], ' ');
      if (Toks.Count() < 3) continue;
      params("ref_unique", Toks[0]);
      URefC = true;
    }
    else if (!DRef  && SL[i].Contains("Disagreeable Reflections")) {
      while (++i < SL.Count() && !SL[i].IsEmpty())
        ;
      i += 2;
      while (++i < SL.Count()) {
        Toks.Strtok(SL[i], ' ');
        if (Toks.Count() < 8) break;
        size_t inc = 0, requiredCount = 8;
        if (Toks[0] == '*' ) { inc ++;  requiredCount++; }
        if (Toks.Count() >= requiredCount) {
          TLstRef *LstRef = new TLstRef;
          LstRef->H = Toks[0+inc].ToInt();
          LstRef->K = Toks[1+inc].ToInt();
          LstRef->L = Toks[2+inc].ToInt();
          LstRef->Fo = Toks[3+inc].ToDouble();
          LstRef->Fc = Toks[4+inc].ToDouble();
          LstRef->DF = Toks[5+inc].ToDouble();
          LstRef->Res = Toks[7+inc].ToDouble();
          LstRef->Deleted = false;
          FDRefs.Add(LstRef);
          Toks.Clear();
        }
      }
      DRef = true;
    }
    else if (!TrefT && SL[i].Contains(
        "Try    Ralpha Nqual Sigma-1 M(abs) CFOM   Seminvariants"))
    {
      i += 2;
      while (i < SL.Count() && !SL[i].Contains("CFOM")) {
        Toks.Strtok(SL[i], ' ');
        if (Toks.Count() < 7) { i++;  continue; }
        size_t inc = 0, requiredCount = 7;
        if (Toks[0] == '*') { inc ++;  requiredCount++; }
        if (Toks.Count() >= requiredCount) {
          TTrefTry trtry;
          trtry.Try = Toks[0+inc].SubString(0, Toks[0+inc].Length()-1).ToInt();
          trtry.RAlpha = Toks[1+inc].ToDouble();
          trtry.NQual = Toks[2+inc].ToDouble();
          trtry.SigmaM1 = Toks[3+inc].ToDouble();
          trtry.Mabs = Toks[4+inc].ToDouble();
          trtry.CFOM = Toks[5+inc].Trim('*').ToDouble();
          trtry.Semivariants.SetSize((uint32_t)((Toks.Count() - 6 - inc) *
            Toks[inc+6].Length()));
          size_t bitIndex = 0;
          for (size_t j= 6 + inc; j < Toks.Count(); j++) {
            if (bitIndex >= trtry.Semivariants.Count())  break;
            for (size_t k=0; k < Toks[j].Length(); k++) {
              if ( Toks[j][k] == '+')
                trtry.Semivariants.Set(bitIndex, true);
              bitIndex++;
              if (bitIndex >= trtry.Semivariants.Count()) break;
            }
          }
          TrefTries.AddCopy(trtry);
          Toks.Clear();
          i++;
        }
      }
      QuickSorter::SortSF(TrefTries, SortTrefTries);
      TrefT = true;
    }
    else if (!PattS && SL[i].Contains("Solution   1    CFOM =  ")) {
      i += 7;
      TTypeList<TPattAtom>* sol = new TTypeList<TPattAtom>;
      PattSolutions.Add( sol );
      while (i < SL.Count() && !SL[i].Contains("Patterson"))  {
        if (SL[i].Contains("Solution")) {
          i += 7;
          if (i >= SL.Count()) break;
          sol = new TTypeList<TPattAtom>();
          PattSolutions.Add( sol );
        }
        Toks.Strtok(SL[i], ' ');
        if (Toks.Count() < 6 ) { i++;  continue; }
        TPattAtom& atom = sol->AddNew();
        atom.SetName( Toks[0] );
        atom.GetCrd()[0] = Toks[2].ToDouble();
        atom.GetCrd()[1] = Toks[3].ToDouble();
        atom.GetCrd()[2] = Toks[4].ToDouble();
        atom.SetOccup( Toks[5].ToDouble() );
        Toks.Clear();
        i += 3;
      }
      PattS = true;
    }
    else if (!RIS && SL[i].Contains("R(int) =")) {
      Toks.Strtok(SL[i], ' ');
      if (Toks.Count() < 6) continue;
      params("Rint", Toks[2]);
      params("Rsig", Toks[5]);
      RIS = true;
    }
    else if (!HP && SL[i].Contains("Highest peak")) {
      Toks.Strtok(SL[i], ' ');
      if (Toks.Count() < 4)  continue;
      params("peak", Toks[1].Length() > 4 ? Toks[1].SubStringFrom(4) : Toks[2]);
      HP = true;
    }
    else if (!DH && SL[i].Contains("Deepest hole")) {
      Toks.Strtok(SL[i], ' ');
      if( Toks.Count() < 4 )  continue;
      params("hole", Toks[1].Length() > 4 ? Toks[1].SubStringFrom(4) : Toks[2]);
      DH = true;
    }
    else if (!RF && SL[i].Contains("Final Structure Factor")) {
      // extract total number of LS parameters
      Toks.Clear();
      if ((i+=2) >= SL.Count()) break;
      Toks.Strtok(SL[i], ' ');
      if (Toks.Count() > 5)
        params("param_n", Toks[6]);
      // extract R1 or 4sigma, R1a for all data and number of refs with Fo > 4sig(Fo)
      Toks.Clear();
      while (i < SL.Count() && !SL[i].Contains("R1 = "))
        i++;
      if (i >= SL.Count()) break;
      Toks.Strtok(SL[i], ' ');
      if (Toks.Count() > 8) {
        params("R1", Toks[2]);
        params("ref_4sig", Toks[4]);
        params("R1all", Toks[9]);
      }
      // extract wR2 && Goof && restrained GooF
      Toks.Clear();
      if (++i >= SL.Count()) break;
      Toks.Strtok(SL[i].Replace(',', EmptyString()), ' ');
      if (Toks.Count() > 11) {
        params("wR2", Toks[2]);
        params("S", Toks[7]);
        params("rS", Toks[11]);
      }
    }
    else if (!SA && SL[i].Contains("square atomic displacements")) {
      while( ++i < SL.Count() && SL[i].IsEmpty())
        ;
      /* do the search, the line break is the end of the section */
      while (i < SL.Count() && !SL[i].IsEmpty()) {
        if (SL[i].Contains("may be split into")) {
          Toks.Clear();
          Toks.Strtok(SL[i], ' ');
          if (Toks.Count() < 15) { i++;  continue; }
          TLstSplitAtom *SplitA = new TLstSplitAtom();
          SplitA->AtomName = Toks[3];
          SplitA->PositionA[0] = Toks[8].ToDouble();
          SplitA->PositionA[1] = Toks[9].ToDouble();
          SplitA->PositionA[2] = Toks[10].ToDouble();
          SplitA->PositionB[0] = Toks[12].ToDouble();
          SplitA->PositionB[1] = Toks[13].ToDouble();
          SplitA->PositionB[2] = Toks[14].ToDouble();
          FSplitAtoms.Add(SplitA);
        }
        i++;
      }
      SA = true;
    }
    else if (!FlackF && SL[i].Contains("Flack x parameter")) {
      Toks.Clear();
      Toks.Strtok(SL[i], ' ');
      if( Toks.Count() == 8 )  {
        TEValueD flack(Toks[4].ToDouble(), Toks[7].ToDouble());
        params("flack", flack.ToString());
      }
      FlackF = true;
    }
    else if (!FlackF && SL[i].Contains("Flack x") && // 2013 version
      SL[i].Contains("fit to all intensities"))
    {
      Toks.Clear();
      Toks.Strtok(SL[i], ' ');
      if (Toks.Count() > 3) {
        TEValueD flack = Toks[3];
        params("flack", flack.ToString());
      }
      FlackF = true;
    }
    else if (!CellInfo && SL[i].Contains("F(000) = ") &&
      SL[i].Contains("Mu = ") && SL[i].Contains("Rho = "))
    {
      Toks.Clear();
      Toks.Strtok(SL[i], ' ');
      if (Toks.Count() == 17) {
        params("F000", Toks[5]);
        params("Mu", Toks[8]);
        params("Rho", Toks[16]);
        CellInfo = true;
      }
    }
    else if (!HasTwin && SL[i].StartsFromi(" TWIN ")) {
      Toks.Clear();
      Toks.Strtok(SL[i], ' ');
      if (Toks.Count() == 11) {
        if (Toks.GetLastString().ToInt() != 2)
          continue;
        InvTwin = true;
        for (size_t j=1; j < 10; j++) {
          const double v = Toks[j].ToDouble();
          if (j == 1 || j == 5 || j == 9) {
            if (v != -1) {
              InvTwin = false;
              break;
            }
          }
          else if (v != 0) {
            InvTwin = false;
            break;
          }
        }
        HasTwin = true;
        continue;
      }
    }
    else {
      // errors
      if (SL[i].Contains("**")) {
        AnAssociation2<olxstr,olxstr>& msg = ErrorMsgs.AddNew(SL[i], EmptyString());
        if (SL[i].Contains(':'))
          continue;
        if (i >= 2) {
          if (i > 2 && SL[i-3].EndsWith('=')) {
            msg.B() << SL[i-3].SubStringTo(SL[i-3].Length()-1);
            msg.B() << SL[i-2];
          }
          else {
            msg.B() << SL[i-2];
          }
        }
      }
    }
    if (DH && HP && DRef && TRefC &&URefC && RF && RIS)
      break;
  }
  // backwards for shifts
  int params_found=0;
  for (size_t i=SL.Count()-1; i != InvalidIndex; i--) {
    if (SL[i].Contains("Max. shift")) {
      TStrList toks(SL[i], ' ');
      if (toks.Count() < 12 || !toks[3].IsNumber()) continue;
      if (!toks[3].IsNumber()) continue;
      params("max_shift", toks[3].ToDouble());
      params("max_shift_object", toks[6]);
      if (toks.Count() <= 12) {
        params("max_du", toks[9].SubStringFrom(1).ToDouble());
        params("max_du_object", toks[11]);
      }
      else {
        params("max_du", toks[10].ToDouble());
        params("max_du_object", toks[12]);
      }
      params_found++;
    }
    else if (SL[i].Contains("Mean shift/esd")) {
      TStrList toks(SL[i], ' ');
      if (toks.Count() < 4 || !toks[3].IsNumber()) continue;
      params("mean_shift", toks[3].ToDouble());
      params_found++;
    }
    else if (SL[i].Contains("N      value        esd    shift/esd  parameter")) {
      size_t oi = i;
      i++; // empty line
      while (++i < SL.Count()) {
        TStrList toks(SL[i], ' ');
        if (toks.Count() < 5) break;
        if (toks[4].Equalsi("OSF")) {
          params("osf",
            TEValueD(toks[1].ToDouble(), toks[2].ToDouble()).ToString());
        }
        else if (toks[4].Equalsi("EXTI")) {
          params("exti",
            TEValueD(toks[1].ToDouble(), toks[2].ToDouble()).ToString());
        }
        else if (toks[4].Equalsi("BASF")) {
          if (HasTwin && InvTwin) {
            params("flack",
              TEValueD(toks[1].ToDouble(), toks[2].ToDouble()).ToString());
          }
          else if (toks.Count() == 6) {
            params(olxstr("basf_") << toks[5],
              TEValueD(toks[1].ToDouble(), toks[2].ToDouble()).ToString());
          }
        }
      }
      params_found++;
      i = oi;
    }
    if (params_found == 3)
      break;
  }
  /* we do not consider SA, as it depends if there are any anisotropic atoms
   in the structure
  */
  if (DH || HP || DRef || TRefC || URefC || RF || RIS)
    Loaded = true;
  return Loaded;
}
//..............................................................................
bool TLst::ExportHTML( const short Param, TStrList &Html, bool TableDef)  {
  TTTable<TStrList> Table;
  if( Param == slstReflections )  {
    if( !DRefCount() )  return false;
    Table.Resize(DRefCount(), 4);
    Table.ColName(0) = "H";
    Table.ColName(1) = "K";
    Table.ColName(2) = "L";
    Table.ColName(3) = "&Delta;(F<sup>2</sup>)/esd";
    for( size_t i=0; i < DRefCount(); i++ )  {
      const TLstRef& Ref = DRef(i);
      if( Ref.Deleted )  continue;
      Table[i][0] = Ref.H;
      Table[i][1] = Ref.K;
      Table[1][2] = Ref.L;
      Table[i][3] = Ref.DF;
    }
    Table.CreateHTMLList(Html, EmptyString(), true, false, TableDef);
    return true;
  }
  if( Param == slslRefineData )  {
    Table.Resize(13, 2);
    Table[0][0] = "R1 (Fo > 4sig(Fo))";
      Table[0][1] = params.Find("R1", XLibMacros::NAString());
    Table[1][0] = "R1 all data";
      Table[1][1] = params.Find("R1all", XLibMacros::NAString());
    Table[2][0] = "wR2";
      Table[2][1] = params.Find("wR2", XLibMacros::NAString());
    Table[3][0] = "GooF";
      Table[3][1] = params.Find("S", XLibMacros::NAString());
    Table[4][0] = "Restrained GooF";
      Table[4][1] = params.Find("rS", XLibMacros::NAString());
    Table[5][0] = "Parameters";
      Table[5][1] = params.Find("param_n", XLibMacros::NAString());
    Table[6][0] = "Highest peak";
      Table[6][1] = params.Find("preak", XLibMacros::NAString());
    Table[7][0] = "Deepest hole";
      Table[7][1] = params.Find("hole", XLibMacros::NAString());
    Table[8][0] = "Total reflections";
      Table[8][1] = params.Find("ref_total", XLibMacros::NAString());
    Table[9][0] = "Unique reflections";
      Table[9][1] = params.Find("ref_uniq", XLibMacros::NAString());
    Table[10][0] = "Reflections with Fo > 4sig(Fo)";
      Table[10][1] = params.Find("ref_4sig", XLibMacros::NAString());
    Table[11][0] = "Rint";
      Table[11][1] = params.Find("Rint", XLibMacros::NAString());
    Table[12][0] = "Rsigma";
      Table[12][1] = params.Find("Rsig", XLibMacros::NAString());
    Table.CreateHTMLList(Html, EmptyString(), false, false, TableDef);
    return true;
  }
  return false;
}
//..............................................................................
void TLst::SynchroniseOmits(RefinementModel& rm)  {
  for( size_t i=0; i < FDRefs.Count(); i++ )  
    FDRefs[i].Deleted = false;
  for( size_t i=0; i < rm.OmittedCount(); i++ )  {
    const vec3i& r = rm.GetOmitted(i);
    for( size_t j=0; j < FDRefs.Count(); j++ )  {
      if( FDRefs[j].H == r[0] && FDRefs[j].K == r[1] && FDRefs[j].L == r[2] )  {
        FDRefs[j].Deleted = true;
        break;
      }
    }
  }
}
