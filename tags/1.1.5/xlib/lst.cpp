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

int SortTrefTries(const TTrefTry* I1, const TTrefTry* I2) {
  if( I1->CFOM < I2->CFOM )  return -1;
  if( I1->CFOM > I2->CFOM )  return 1;
  int res = I1->Semivariants.Compare(I2->Semivariants);
  if( !res )  {
    if( I1->NQual > I2->NQual )  return -1;
    if( I1->NQual < I2->NQual )  return 1;
    return 0;
  }
  return res;
}
//..............................................................................
TLst::TLst()  {
  FR1 = FwR2 = FS = FRS = 0;
  FParams = FTotalRefs = FUniqRefs = 0;
  FPeak = FHole = 0;
  _HasFlack = FLoaded = false;
}
//..............................................................................
TLst::~TLst()  {  return;  }
//..............................................................................
void TLst::Clear()  {
  FDRefs.Clear();
  FSplitAtoms.Clear();
  TrefTries.Clear();
  PattSolutions.Clear();
  FR1 = FR1a = FwR2 = FS = FRS = FRint = FRsig = 0;
  FParams = FTotalRefs = FUniqRefs = FRefs4sig = 0;
  FRho = FF000 = FMu = 0;
  FPeak = FHole = 0;

  ErrorMsgs.Clear();
  _HasFlack = FLoaded = false;
}
//..............................................................................
bool TLst::LoadFromFile(const olxstr &FN)  {
  TEFile::CheckFileExists(__OlxSourceInfo, FN);
  TStrList SL;
  size_t ind;
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
  TStrList Toks;
  Clear();
  SL.LoadFromFile(FN);
  TLstRef *LstRef;
  TLstSplitAtom *SplitA;
  for( size_t i=0; i < SL.Count(); i++ )  {
    Toks.Clear();
    if( !TRefC )  {
      ind = SL[i].FirstIndexOf("Reflections read,");
      if( ind != InvalidIndex )  {
        Toks.Strtok(SL[i], ' ');
        if( Toks.Count() < 3 )  continue;
        FTotalRefs = Toks[0].ToInt();
        try  {  //bug: 662044  Reflections read, of which447594  rejected
          FUniqRefs = FTotalRefs - Toks[5].ToInt(); // uniq = total - rejected
        }
        catch(...)  {
          FUniqRefs = FUniqRefs = -1;
        }
        TRefC = true;
        continue;
      }
    }
    if( !URefC )  {
      ind = SL[i].FirstIndexOf("Unique reflections,");
      if( ind != InvalidIndex )  {
        Toks.Strtok(SL[i], ' ');
        if( Toks.Count() < 3 )  continue;
        FUniqRefs = Toks[0].ToInt();
        URefC = true;
        continue;
      }
    }
    if( !DRef )  {
      ind = SL[i].FirstIndexOf("Disagreeable Reflections");
      if( ind != InvalidIndex )  {
        i += 4;
        while( i < SL.Count() && (SL[i].FirstIndexOf("Bond") == InvalidIndex) )  {
          Toks.Strtok(SL[i], ' ');
          if( Toks.Count() < 8 )  break;
          size_t inc = 0, requiredCount = 8 ;
          if( Toks[0] == '*' )  {  inc ++;  requiredCount++;  }
          if( Toks.Count() >= requiredCount )  {
            LstRef = new TLstRef;
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
            i++;
          }
        }
        DRef = true;
        continue;
      }
    }
    if( !TrefT )  {
      ind = SL[i].FirstIndexOf("Try    Ralpha Nqual Sigma-1 M(abs) CFOM   Seminvariants");
      if( ind != InvalidIndex )  {
        i += 2;
        while( i < SL.Count() && (SL[i].FirstIndexOf("CFOM") == InvalidIndex) )  {
          Toks.Strtok(SL[i], ' ');
          if( Toks.Count() < 7 )  { i++;  continue;  }
          size_t inc = 0, requiredCount = 7;
          if( Toks[0] == '*' )  {  inc ++;  requiredCount++;  }
          if( Toks.Count() >= requiredCount )  {
            TTrefTry trtry;
            trtry.Try = Toks[0+inc].SubString(0, Toks[0+inc].Length()-1).ToInt();
            trtry.RAlpha = Toks[1+inc].ToDouble();
            trtry.NQual = Toks[2+inc].ToDouble();
            trtry.SigmaM1 = Toks[3+inc].ToDouble();
            trtry.Mabs = Toks[4+inc].ToDouble();
            trtry.CFOM = Toks[5+inc].Trim('*').ToDouble();
            trtry.Semivariants.SetSize((uint32_t)((Toks.Count() - 6 - inc) * Toks[inc+6].Length()));
            size_t bitIndex = 0;
            for( size_t j= 6 + inc; j < Toks.Count(); j++ )  {
              if( bitIndex >= trtry.Semivariants.Count() )  break;
              for( size_t k=0; k < Toks[j].Length(); k++ )  {
                if( Toks[j][k] == '+' )
                  trtry.Semivariants.Set(bitIndex, true);
                bitIndex++;
                if( bitIndex >= trtry.Semivariants.Count() )  break;
              }
            }
            TrefTries.AddCCopy(trtry);
            Toks.Clear();
            i++;
          }
        }
        TrefTries.QuickSorter.SortSF(TrefTries, SortTrefTries);
        TrefT = true;
        continue;
      }
    }
    if( !PattS )  {
      ind = SL[i].FirstIndexOf("Solution   1    CFOM =  ");
      if( ind != InvalidIndex )  {
        i += 7;
        TTypeList<TPattAtom>* sol = new TTypeList<TPattAtom>;
        PattSolutions.Add( sol );
        while( i < SL.Count() && (SL[i].FirstIndexOf("Patterson") == InvalidIndex) )  {
          if( SL[i].FirstIndexOf("Solution") != InvalidIndex )  {
            i += 7;
            if( i >= SL.Count() )  break;
            sol = new TTypeList<TPattAtom>();
            PattSolutions.Add( sol );
          }
          Toks.Strtok(SL[i], ' ');
          if( Toks.Count() < 6 )  { i++;  continue;  }
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
        continue;
      }
    }
    if( !RIS )  {
      ind = SL[i].FirstIndexOf("R(int) =");
      if( ind != InvalidIndex )  {
        Toks.Strtok(SL[i], ' ');
        if( Toks.Count() < 6 )  continue;
        FRint = Toks[2].ToDouble();
        FRsig = Toks[5].ToDouble();
        RIS = true;
        continue;
      }
    }
    if( !HP )  {
      ind = SL[i].FirstIndexOf("Highest peak");
      if( ind != InvalidIndex )  {
        Toks.Strtok(SL[i], ' ');
        if( Toks.Count() < 4 )  continue;
        FPeak = Toks[2].ToDouble();
        HP = true;
        continue;
      }
    }
    if( !DH )  {
      ind = SL[i].FirstIndexOf("Deepest hole");
      if( ind != InvalidIndex )  {
        Toks.Strtok(SL[i], ' ');
        if( Toks.Count() < 4 )  continue;
        FHole = Toks[2].ToDouble();
        DH = true;
        continue;
      }
    }
    if( !RF )  {
      ind = SL[i].FirstIndexOf("Final Structure Factor");
      if( ind != InvalidIndex )  {
        // extract total number of LS parameters
        Toks.Clear();
        i += 2;  if( i >= SL.Count() )  break;
        Toks.Strtok(SL[i], ' ');
        if( Toks.Count() > 5 )  FParams = Toks[6].ToInt();

        // extract R1 or 4sigma, R1a for all data and number of refs with Fo > 4sig(Fo)
        Toks.Clear();
        while( i < SL.Count() && SL[i].IndexOf("R1 = ") == InvalidIndex ) i++;
        if( i >= SL.Count() )  break;
        Toks.Strtok(SL[i], ' ');
        if( Toks.Count() > 8 )  {
          FR1 = Toks[2].ToDouble();
          FRefs4sig = Toks[4].ToInt();
          FR1a = Toks[9].ToDouble();
        }

        // extract wR2 && Goof && restrained GooF
        Toks.Clear();
        i ++;  if( i >= SL.Count() )  break;
        Toks.Strtok(SL[i].Replace(',', EmptyString()), ' ');
        if( Toks.Count() > 11 )  {
          FwR2 = Toks[2].ToDouble();
          FS = Toks[7].ToDouble();
          FRS = Toks[11].ToDouble();
        }

        RF = true;
        continue;
      }
    }
    if( !SA )  {
      ind = SL[i].FirstIndexOf("square atomic displacements");
      if( ind != InvalidIndex )  {
        i++;  // skip the line breaks
        while( i < SL.Count() && SL[i].IsEmpty() )  i++;
        /* do the search, the line break is the end of the section */
        while( i < SL.Count() && !SL[i].IsEmpty() )  {
          if( SL[i].FirstIndexOf("may be split into") != InvalidIndex )  {
            Toks.Clear();
            Toks.Strtok(SL[i], ' ');
            if( Toks.Count() < 15 )  {  i++;  continue;  }
            SplitA = new TLstSplitAtom();
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
        continue;
      }
    }
    if( !FlackF )  {
      ind = SL[i].FirstIndexOf("Flack x parameter");
      if( ind != InvalidIndex )  {
        Toks.Clear();
        Toks.Strtok(SL[i], ' ');
        if( Toks.Count() == 8 )  {
          FlackParam.V() = Toks[4].ToDouble();
          FlackParam.E() = Toks[7].ToDouble();
          _HasFlack = true;
        }
        FlackF = true;
        continue;
      }
    }
    if( !CellInfo &&
      SL[i].IndexOf("F(000) = ") != InvalidIndex &&
      SL[i].IndexOf("Mu = ") != InvalidIndex &&
      SL[i].IndexOf("Rho = ") != InvalidIndex )
    {
      Toks.Clear();
      Toks.Strtok(SL[i], ' ');
      if( Toks.Count() == 17 )  {
        FF000 = Toks[5].ToDouble();
        FMu = Toks[8].ToDouble();
        FRho = Toks[16].ToDouble();
        CellInfo = true;
        continue;
      }
    }
    if( !HasTwin && SL[i].StartsFromi(" TWIN ") )  {
      Toks.Clear();
      Toks.Strtok(SL[i], ' ');
      if( Toks.Count() == 11 )  {
        if( Toks.GetLastString().ToInt() != 2 )  continue;
        InvTwin = true;
        for( size_t j=1; j < 10; j++ )  {
          const double v = Toks[j].ToDouble();
          if( j == 1 || j == 5 || j == 9 )  {
            if( v != -1 )  {
              InvTwin = false;
              break;
            }
          }
          else if( v != 0 )  {
            InvTwin = false;
            break;
          }
        }
        HasTwin = true;
        continue;
      }
    }
    // errors
    ind = SL[i].FirstIndexOf("**");
    if( ind != InvalidIndex )  {
      AnAssociation2<olxstr,olxstr>& msg = ErrorMsgs.AddNew(SL[i], EmptyString());
      if( SL[i].IndexOf(':') != InvalidIndex )
        continue;
      if( i >= 2 )  {
        if( i > 2 && SL[i-3].EndsWith('=') )  {
          msg.B() << SL[i-3].SubStringTo( SL[i-3].Length()-1 );
          msg.B() << SL[i-2];
        }
        else  {
          msg.B() << SL[i-2];
        }
      }
    }
    if( DH && HP && DRef && TRefC &&URefC && RF && RIS)  break;
  }
  /* we do not consider SA, as it depends if there are any anisotropic atoms
   in the structure
  */
  if( DH || HP || DRef || TRefC || URefC || RF || RIS )
    FLoaded = true;
  if( HasTwin && InvTwin )  {
    for( size_t i=0; i < SL.Count(); i++ )  {
      olxstr& line = SL[SL.Count()-i-1];
      if( line.IndexOf("BASF  ") != InvalidIndex )  {
        Toks.Clear();
        Toks.Strtok(line, ' ');
        if( Toks.Count() == 6 )  {
          FlackParam.V() = Toks[1].ToDouble();
          FlackParam.E() = Toks[2].ToDouble();
          break;
        }
      }
    }
  }
  return FLoaded;
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
    Table[0][0] = "R1 (Fo > 4sig(Fo))"; Table[0][1] = R1();
    Table[1][0] = "R1 all data";        Table[1][1] = R1a();
    Table[2][0] = "wR2";                Table[2][1] = wR2();
    Table[3][0] = "GooF";               Table[3][1] = S();
    Table[4][0] = "Restrained GooF";    Table[4][1] = RS();
    Table[5][0] = "Parameters";         Table[5][1] = Params();
    Table[6][0] = "Highest peak";       Table[6][1] = Peak();
    Table[7][0] = "Deepest hole";       Table[7][1] = Hole();
    Table[8][0] = "Total reflections";  Table[8][1] = TotalRefs();
    Table[9][0] = "Unique reflections"; Table[9][1] = UniqRefs();
    Table[10][0] = "Reflections with Fo > 4sig(Fo)";   Table[10][1] = UniqRefs();
    Table[11][0] = "Rint";              Table[11][1] = Rint();
    Table[12][0] = "Rsigma";            Table[12][1] = Rsigma();
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
