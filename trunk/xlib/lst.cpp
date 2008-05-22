//---------------------------------------------------------------------------//
// namespace TXFiles: TLst - lst file parser
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "lst.h"
#include "efile.h"
#include "etable.h"

int SortTrefTries(const void* I1, const void* I2) {
  if( ((TTrefTry*)I1)->CFOM < ((TTrefTry*)I2)->CFOM )  return -1;
  if( ((TTrefTry*)I1)->CFOM > ((TTrefTry*)I2)->CFOM )  return 1;
  int res = ((TTrefTry*)I1)->Semivariants.Compare( ((TTrefTry*)I2)->Semivariants );
  if( !res )  {
    if( ((TTrefTry*)I1)->NQual > ((TTrefTry*)I2)->NQual )  return -1;
    if( ((TTrefTry*)I1)->NQual < ((TTrefTry*)I2)->NQual )  return 1;
    return 0;
  }
  return res;
}

//----------------------------------------------------------------------------//
// TInsCFile function bodies
//----------------------------------------------------------------------------//
TLst::TLst()  {
  FDRefs = new TEList;
  FSplitAtoms = new TEList;
  PattSolutions = TrefTries = NULL;

  FR1 = FwR2 = FS = FRS = 0;
  FParams = FTotalRefs = FUniqRefs = 0;
  FPeak = FHole = 0;
  FLoaded = false;
}
//..............................................................................
TLst::~TLst()  {
  Clear();
  delete FDRefs;
  delete FSplitAtoms;
  if( TrefTries != NULL )  delete TrefTries;
  if( PattSolutions != NULL )  delete PattSolutions;
}
//..............................................................................
void TLst::Clear()  {
  for( int i=0; i < DRefCount(); i++ )        delete DRef(i);
  FDRefs->Clear();

  for( int i=0; i < SplitAtomCount(); i++ )   delete SplitAtom(i);
  FSplitAtoms->Clear();

  for( int i=0; i < TrefTryCount(); i++ )     delete &TrefTry(i);
  if( TrefTries )  TrefTries->Clear();

  for( int i=0; i < PattSolutionCount(); i++ ) delete &PattSolution(i);
  if( PattSolutions )  PattSolutions->Clear();

  FR1 = FR1a = FwR2 = FS = FRS = FRint = FRsig = 0;
  FParams = FTotalRefs = FUniqRefs = FRefs4sig = 0;
  FPeak = FHole = 0;

  ErrorMsgs.Clear();
  FLoaded = false;
}
//..............................................................................
bool TLst::LoadFromFile(const olxstr &FN)  {
  TEFile::CheckFileExists(__OlxSourceInfo, FN);
  TStrList SL;
  int ind;
  bool TRefC  = false,
       URefC  = false,
       DRef   = false,
       HP     = false,
       DH     = false,
       RF     = false,
       RIS    = false,
       SA     = false, // split atoms
       TrefT  = false,
       PattS  = false;
  TStrList Toks;
  Clear();
  SL.LoadFromFile( FN );
  TLstRef *LstRef;
  TLstSplitAtom *SplitA;

  for( int i=0; i < SL.Count(); i++ )  {
    Toks.Clear();
    if( !TRefC )  {
      ind = SL.String(i).FirstIndexOf("Reflections read,");
      if( ind >= 0 )  {
        Toks.Strtok(SL.String(i), ' ');
        if( Toks.Count() < 3 )  continue;
        FTotalRefs = Toks[0].ToInt();
        FUniqRefs = FTotalRefs - Toks[5].ToInt(); // uniq = total - rejected
        TRefC = true;
        continue;
      }
    }
    if( !URefC )  {
      ind = SL.String(i).FirstIndexOf("Unique reflections,");
      if( ind >= 0 )  {
        Toks.Strtok(SL.String(i), ' ');
        if( Toks.Count() < 3 )  continue;
        FUniqRefs = Toks[0].ToInt();
        URefC = true;
        continue;
      }
    }
    if( !DRef )  {
      ind = SL.String(i).FirstIndexOf("Disagreeable Reflections");
      if( ind >= 0 )  {
        i += 4;
        while( i < SL.Count() && (SL.String(i).FirstIndexOf("Bond") == -1) )  {
          Toks.Strtok(SL.String(i), ' ');
          if( Toks.Count() < 8 )  break;
          int inc = 0, requiredCount = 8 ;
          if( Toks.String(0) == '*' )  {  inc ++;  requiredCount++;  }
          if( Toks.Count() > requiredCount )  {
            LstRef = new TLstRef;
            LstRef->H = Toks[0+inc].ToInt();
            LstRef->K = Toks[1+inc].ToInt();
            LstRef->L = Toks[2+inc].ToInt();
            LstRef->DF = Toks[5+inc].ToDouble();
            LstRef->Res = Toks[7+inc].ToDouble();
            FDRefs->Add(LstRef);
            Toks.Clear();
            i++;
          }
        }
        DRef = true;
        continue;
      }
    }
    if( !TrefT )  {
      ind = SL.String(i).FirstIndexOf("Try    Ralpha Nqual Sigma-1 M(abs) CFOM   Seminvariants");
      if( ind >= 0 )  {
        i += 2;
        while( i < SL.Count() && (SL.String(i).FirstIndexOf("CFOM") == -1) )  {
          Toks.Strtok(SL.String(i), ' ');
          if( Toks.Count() < 7 )  { i++;  continue;  }
          if( !TrefTries )  TrefTries = new TEList();
          TTrefTry* trtry = new TTrefTry;
          int inc = 0, requiredCount = 7;
          if( Toks.String(0) == '*' )  {  inc ++;  requiredCount++;  }
          if( Toks.Count() > requiredCount )  {
            trtry->Try = Toks[0+inc].ToInt();
            trtry->RAlpha = Toks[1+inc].ToDouble();
            trtry->NQual = Toks[2+inc].ToDouble();
            trtry->SigmaM1 = Toks[3+inc].ToDouble();
            trtry->Mabs = Toks[4+inc].ToDouble();
            trtry->CFOM = Toks[5+inc].ToDouble();

            trtry->Semivariants.SetSize( (Toks.Count() - 6 - inc) * Toks[inc+6].Length() );
            int bitIndex = 0;
            for( int j= 6 + inc; j < Toks.Count(); j++ )  {
              if( bitIndex >= trtry->Semivariants.Count() )  break;
              for( int k=0; k < Toks[j].Length(); k++ )  {
                if( Toks[j][k] == '+' )
                  trtry->Semivariants.Set(bitIndex, true);
                bitIndex++;
                if( bitIndex >= trtry->Semivariants.Count() )  break;
              }
            }

            TrefTries->Add(trtry);
            Toks.Clear();
            i++;
          }
        }
        TrefTries->Sort( SortTrefTries );
        TrefT = true;
        continue;
      }
    }
    if( !PattS )
    {
      ind = SL.String(i).FirstIndexOf("Solution   1    CFOM =  ");
      if( ind >= 0 )  {
        i += 7;
        if( !PattSolutions )  PattSolutions = new TEList();
        TTypeList<TPattAtom>* sol = new TTypeList<TPattAtom>();
        PattSolutions->Add( sol );
        while( i < SL.Count() && (SL.String(i).FirstIndexOf("Patterson") == -1) )  {
          if( SL.String(i).FirstIndexOf("Solution") != -1 )  {
            i += 7;
            if( i >= SL.Count() )  break;
            sol = new TTypeList<TPattAtom>();
            PattSolutions->Add( sol );
          }
          Toks.Strtok(SL.String(i), ' ');
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
      ind = SL.String(i).FirstIndexOf("R(int) =");
      if( ind >= 0 )  {
        Toks.Strtok(SL.String(i), ' ');
        if( Toks.Count() < 6 )  continue;
        FRint = Toks[2].ToDouble();
        FRsig = Toks[5].ToDouble();
        RIS = true;
        continue;
      }
    }
    if( !HP )  {
      ind = SL.String(i).FirstIndexOf("Highest peak");
      if( ind >= 0 )  {
        Toks.Strtok(SL.String(i), ' ');
        if( Toks.Count() < 4 )  continue;
        FPeak = Toks[2].ToDouble();
        HP = true;
        continue;
      }
    }
    if( !DH )  {
      ind = SL.String(i).FirstIndexOf("Deepest hole");
      if( ind >= 0 )  {
        Toks.Strtok(SL.String(i), ' ');
        if( Toks.Count() < 4 )  continue;
        FHole = Toks[2].ToDouble();
        DH = true;
        continue;
      }
    }
    if( !RF )  {
      ind = SL.String(i).FirstIndexOf("Final Structure Factor");
      if( ind >= 0 )  {
        // extract total number of LS parameters
        Toks.Clear();
        i += 2;  if( i >= SL.Count() )  break;
        Toks.Strtok(SL.String(i), ' ');
        if( Toks.Count() > 5 )  FParams = Toks[6].ToInt();

        // extract R1 or 4sigma, R1a for all data and number of refs with Fo > 4sig(Fo)
        Toks.Clear();
        while( i < SL.Count() && SL.String(i).IndexOf("R1 = ") == -1  ) i++;
        if( i >= SL.Count() )  break;
        Toks.Strtok(SL.String(i), ' ');
        if( Toks.Count() > 8 )  {
          FR1 = Toks[2].ToDouble();
          FRefs4sig = Toks[4].ToInt();
          FR1a = Toks[9].ToDouble();
        }

        // extract wR2 && Goof && restrained GooF
        Toks.Clear();
        i ++;  if( i >= SL.Count() )  break;
        Toks.Strtok(SL.String(i), ' ');
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
      ind = SL.String(i).FirstIndexOf("square atomic displacements");
      if( ind >= 0 )  {
        i++;  // skip the line breaks
        while( i < SL.Count() && !SL.String(i).Length() )  i++;
        /* do the search, the line break is the end of the section */
        while( i < SL.Count() && SL.String(i).Length() )  {
          if( SL.String(i).FirstIndexOf("may be split into") >= 0 )  {
            Toks.Clear();
            Toks.Strtok(SL.String(i), ' ');
            if( Toks.Count() < 15 )  {  i++;  continue;  }
            SplitA = new TLstSplitAtom();
            SplitA->AtomName = Toks[3];
            SplitA->PositionA[0] = Toks[8].ToDouble();
            SplitA->PositionA[1] = Toks[9].ToDouble();
            SplitA->PositionA[2] = Toks[10].ToDouble();
            SplitA->PositionB[0] = Toks[12].ToDouble();
            SplitA->PositionB[1] = Toks[13].ToDouble();
            SplitA->PositionB[2] = Toks[14].ToDouble();
            FSplitAtoms->Add(SplitA);
          }
          i++;
        }
        SA = true;
        continue;
      }
    }
    // errors
    ind = SL.String(i).FirstIndexOf("**");
    if( ind >= 0 )  {
      AnAssociation2<olxstr,olxstr>& msg = ErrorMsgs.AddNew(SL.String(i), "");
      if( SL.String(i).IndexOf(':') != -1 )
        continue;
      if( i >= 2 )  {
        if( i > 2 && SL.String(i-3).EndsWith('=') )  {
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
  if( DH || HP || DRef || TRefC || URefC || RF || RIS )  FLoaded = true;
  return FLoaded;
}
//..............................................................................
bool TLst::ExportHTML( const short Param, TStrList &Html, bool TableDef)  {
  TTTable<TStrList> Table;
  if( Param == slstReflections )  {
    if( !DRefCount() )  return false;
    TLstRef *Ref;
    Table.Resize(DRefCount(), 4);
    Table.ColName(0) = "H";
    Table.ColName(1) = "K";
    Table.ColName(2) = "L";
    Table.ColName(3) = "&Delta;(F<sup>2</sup>)/esd";
    for( int i=0; i < DRefCount(); i++ )  {
      Ref = DRef(i);
      Table.Row(i)->String(0) = Ref->H;
      Table.Row(i)->String(1) = Ref->K;
      Table.Row(i)->String(2) = Ref->L;
      Table.Row(i)->String(3) = Ref->DF;
    }
    Table.CreateHTMLList(Html, EmptyString, true, false, TableDef);
    return true;
  }
  if( Param == slslRefineData )  {
    Table.Resize(13, 2);
    Table.Row(0)->String(0) = "R1 (Fo > 4sig(Fo))"; Table.Row(0)->String(1) = R1();
    Table.Row(1)->String(0) = "R1 all data";        Table.Row(1)->String(1) = R1a();
    Table.Row(2)->String(0) = "wR2";                Table.Row(2)->String(1) = wR2();
    Table.Row(3)->String(0) = "GooF";               Table.Row(3)->String(1) = S();
    Table.Row(4)->String(0) = "Restrained GooF";    Table.Row(4)->String(1) = RS();
    Table.Row(5)->String(0) = "Parameters";         Table.Row(5)->String(1) = Params();
    Table.Row(6)->String(0) = "Highest peak";       Table.Row(6)->String(1) = Peak();
    Table.Row(7)->String(0) = "Deepest hole";       Table.Row(7)->String(1) = Hole();
    Table.Row(8)->String(0) = "Total reflections";  Table.Row(8)->String(1) = TotalRefs();
    Table.Row(9)->String(0) = "Unique reflections"; Table.Row(9)->String(1) = UniqRefs();
    Table.Row(10)->String(0) = "Reflections with Fo > 4sig(Fo)";   Table.Row(10)->String(1) = UniqRefs();
    Table.Row(11)->String(0) = "Rint";               Table.Row(11)->String(1) = Rint();
    Table.Row(12)->String(0) = "Rsigma";               Table.Row(12)->String(1) = Rsigma();
    Table.CreateHTMLList(Html, EmptyString, false, false, TableDef);
    return true;
  }
  return false;
}
//..............................................................................

 
