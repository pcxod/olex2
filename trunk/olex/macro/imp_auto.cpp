// if this is not inluded: internal compiler error at 0xc19d9e with base 0xc10000
#include "egc.h"

#include "mainform.h"

#ifdef __OD_BUILD__
#include "auto.h"
#include "xatom.h"
#include "ins.h"

void TMainForm::funTestAuto(const TStrObjList &Cmds, TMacroError &Error)  {
  if( !Cmds.Count() )
    ProcessXPMacro("clean -npd", Error);
  // Qpeak rings analysis ...
  try  {
    TTypeList< TSAtomPList > rings;
    FXApp->FindRings("QQQQQQ", rings);
    for(int i=0; i < rings.Count(); i++ )  {
      double rms = TSPlane::CalcRMS( rings[i] );
      if( rms < 0.1 && TNetwork::IsRingRegular( rings[i]) )  {
        for( int j=0; j < rings[i].Count(); j++ )  {
          rings[i][j]->CAtom().SetLabel("Cph");
        }
      }
    }
  }
  catch( ... )  {  ;  }
  static TAtomTypePermutator AtomPermutator;
  static olxstr FileName = FXApp->XFile().GetFileName();
  AtomPermutator.SetActive( false );
//  AtomPermutator.SetActive( !Options.Contains("p") );

  if( !AtomPermutator.IsActive() )
    AtomPermutator.Init();

  if( FileName != FXApp->XFile().GetFileName() )  {
    AtomPermutator.Init();
    FileName = FXApp->XFile().GetFileName();
  }
  if( AtomPermutator.IsActive() )
    AtomPermutator.ReInit( FXApp->XFile().GetAsymmUnit() );
  olxstr autodbf( FXApp->BaseDir() + "auto.db");
  if( TAutoDB::GetInstance() == NULL )  {
    TEGC::AddP( new TAutoDB(*((TXFile*)FXApp->XFile().Replicate()) ) );
    if( TEFile::FileExists( autodbf ) )  {
      TEFile dbf(autodbf, "rb");
      TAutoDB::GetInstance()->LoadFromStream( dbf );
    }
  }
  if( Cmds.Count() != 0 )  {
    TAutoDB::GetInstance()->ProcessFolder( Cmds[0] );
    TEFile dbf(autodbf, "w+b");
    TAutoDB::GetInstance()->SaveToStream( dbf );
  }

  TLattice& latt = FXApp->XFile().GetLattice();

  int64_t st = TETime::msNow();
  bool res = TAutoDB::GetInstance()->AnalyseStructure( FXApp->XFile().GetFileName(), latt, AtomPermutator);
  st = TETime::msNow() - st;
  TBasicApp::GetLog().Info( olxstr("Elapsed time ") << st << " ms");

  if( AtomPermutator.IsActive() )
    AtomPermutator.Permutate();
  ProcessXPMacro("fuse", Error);
  Error.SetRetVal( res );
}
//..............................................................................
void TMainForm::macAtomInfo(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXAtomPList xatoms;
  FindXAtoms(Cmds, xatoms, true, false);
  if( TAutoDB::GetInstance() == NULL )  {
    TEGC::AddP( new TAutoDB(*((TXFile*)FXApp->XFile().Replicate()) ) );
    olxstr autodbf( FXApp->BaseDir() + "auto.db");
    if( TEFile::FileExists( autodbf ) )  {
      TEFile dbf(autodbf, "rb");
      TAutoDB::GetInstance()->LoadFromStream( dbf );
    }
  }
  TStrList report;
  for( int i=0; i < xatoms.Count(); i++ )  {
    TAutoDB::GetInstance()->AnalyseNode( xatoms[i]->Atom(), report );
  }
  TBasicApp::GetLog() << ( report );

}
//..............................................................................
void TMainForm::macValidateAuto(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TEFile log(Cmds.Text(' '), "a+b");
  if( TAutoDB::GetInstance() == NULL )  {
    TEGC::AddP( new TAutoDB(*((TXFile*)FXApp->XFile().Replicate()) ) );
    olxstr autodbf( FXApp->BaseDir() + "auto.db");
    if( TEFile::FileExists( autodbf ) )  {
      TEFile dbf(autodbf, "rb");
      TAutoDB::GetInstance()->LoadFromStream( dbf );
    }
  }
  TStrList report;
  TAutoDB::GetInstance()->ValidateResult( FXApp->XFile().GetFileName(), FXApp->XFile().GetLattice(), report);
  for( int i=0; i < report.Count(); i++ )
    log.Writenl( report[i] );
}
//..............................................................................
struct Main_BaiComparator {
  static int Compare(const TPrimitiveStrListData<olxstr,TBasicAtomInfo*>* a, 
                     const TPrimitiveStrListData<olxstr,TBasicAtomInfo*>* b)  {
      return a->GetObject()->GetIndex() - b->GetObject()->GetIndex();
  }
};

void TMainForm::macClean(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {

  if( TAutoDB::GetInstance() == NULL )  {
    olxstr autodbf( FXApp->BaseDir() + "auto.db");
    TEGC::AddP( new TAutoDB(*((TXFile*)FXApp->XFile().Replicate()) ) );
    if( TEFile::FileExists( autodbf ) )  {
      TEFile dbf(autodbf, "rb");
      TAutoDB::GetInstance()->LoadFromStream( dbf );
    }
  }

  bool runFuse = !Options.Contains("f");
  bool changeNPD = !Options.Contains("npd");
  // qpeak anlysis
  TPSTypeList<double, TCAtom*> SortedQPeaks;
  TTypeList< AnAssociation2<double, TCAtomPList*> > vals;
  int cnt = 0;
  double avQPeak = 0;
  TAsymmUnit& au = FXApp->XFile().GetAsymmUnit();
  bool OnlyQPeakModel = true;
  for( int i=0; i < au.AtomCount(); i++ )  {
    if( au.GetAtom(i).IsDeleted() )  continue;
    if( au.GetAtom(i).GetAtomInfo() != iQPeakIndex )
      OnlyQPeakModel  = false;
    else  {
      SortedQPeaks.Add( au.GetAtom(i).GetQPeak(), &au.GetAtom(i));
      avQPeak += au.GetAtom(i).GetQPeak();
      cnt++;
    }
  }
  if( cnt != 0 )
    avQPeak /= cnt;
  cnt = 0;
  if( SortedQPeaks.Count() != 0 )  {
    vals.AddNew<double, TCAtomPList*>(0, new TCAtomPList);
    for(int i=SortedQPeaks.Count()-1; i >=1; i-- )  {
      if( (SortedQPeaks.GetComparable(i) - SortedQPeaks.GetComparable(i-1))/SortedQPeaks.GetComparable(i) > 0.05 )  {
        //FGlConsole->PostText( olxstr("Threshold here: ") << SortedQPeaks.GetObject(i)->GetLabel() );
        vals.Last().A() += SortedQPeaks.GetComparable(i);
        vals.Last().B()->Add( SortedQPeaks.GetObject(i));
        cnt++;
        vals.Last().A() /= cnt;
        cnt = 0;
        vals.AddNew<double, TCAtomPList*>(0, new TCAtomPList);
        continue;
      }
      vals.Last().A() += SortedQPeaks.GetComparable(i);
      vals.Last().B()->Add( SortedQPeaks.GetObject(i));
      cnt ++;
    }
    vals.Last().B()->Add(SortedQPeaks.GetObject(0));
    cnt++;
    if( cnt > 1 )
      vals.Last().A() /= cnt;

    //for( int i=0; i < vals.Count(); i++ )
    //  FGlConsole->PostText( vals[i].GetA() );

//    double thVal = 2;
    double thVal = (avQPeak  < 2 ) ? 2 : avQPeak*0.75;

    if( SortedQPeaks.Count() == 1 )  {  // only one peak present
      if( SortedQPeaks.GetComparable(0) < thVal )
        SortedQPeaks.GetObject(0)->SetDeleted(true);
    }
    else  {
      double wght = (SortedQPeaks.Last().Comparable()-avQPeak)/
                    (avQPeak-SortedQPeaks.GetComparable(0));
      for( int i=vals.Count()-1; i >= 0; i-- )  {
        if( vals[i].GetA() < thVal )  {
          for( int j=0; j < vals[i].GetB()->Count(); j++ )
            vals[i].GetB()->Item(j)->SetDeleted(true);
        }
      }
    }
    for( int i=0; i < vals.Count(); i++ )
      delete vals[i].B();
  }
  // end qpeak analysis

  // distance analysis
  TTypeList<AnAssociation2<TCAtom*, TVPointD> > neighbours;
  TLattice& latt = FXApp->XFile().GetLattice();
  // qpeaks first
  TSAtomPList QPeaks;
  for( int i=0;  i < latt.AtomCount(); i++ )  {
    TSAtom& sa = latt.GetAtom(i);
    if( sa.IsDeleted() )  continue;
    if( sa.GetAtomInfo() == iQPeakIndex )
      QPeaks.Add( &sa );
  }
  for( int i=0; i < QPeaks.Count(); i++ )  {
    if( QPeaks[i]->IsDeleted() || QPeaks[i]->CAtom().IsDeleted() )  continue;
    neighbours.Clear();
    TAutoDBNode nd(*QPeaks[i], &neighbours);
    for( int j=0; j < nd.DistanceCount(); j++ )  {
      if( nd.GetDistance(j) < (neighbours[j].GetA()->GetAtomInfo().GetRad1()+0.3) )  {  // at leats H-bond
        if( neighbours[j].GetA()->GetAtomInfo() == iQPeakIndex )  {
          if( nd.GetDistance(j) < 1 )  {
            if( neighbours[j].GetA()->GetQPeak() < QPeaks[i]->CAtom().GetQPeak() )
              neighbours[j].GetA()->SetDeleted(true);
          }
        }
        else {
          QPeaks[i]->SetDeleted(true);
          QPeaks[i]->CAtom().SetDeleted(true);
        }
      }
      if( nd.GetDistance(j) > 1.8 )  {
//        if( neighbours[j].GetA()->GetAtomInfo().GetIndex() < 20 )  {  // Ca
//          QPeaks[i]->SetDeleted(true);
//          QPeaks[i]->CAtom().SetDeleted(true);
//        }
      }
    }
    if( nd.NodeCount() == 2 && nd.GetAngle(0) < 90 )  {
      if( !neighbours[0].GetA()->IsDeleted() && !neighbours[1].GetA()->IsDeleted() )  {
        QPeaks[i]->SetDeleted(true);
        QPeaks[i]->CAtom().SetDeleted(true);
        continue;
      }
    }

//    for( int j=0; j < nd.AngleCount(); j++ )  {
//      if( nd.GetAngle(j) < 90
//    }
  }
  // call whatever left carbons ...
  for( int i=0; i < QPeaks.Count(); i++ )  {
    if( QPeaks[i]->IsDeleted() || QPeaks[i]->CAtom().IsDeleted() )  continue;
    QPeaks[i]->CAtom().SetLabel( "C" );
  }

  TDoubleList Uisos;
  if( FXApp->XFile().GetFileName() == TAutoDB::GetInstance()->GetLastFileName() )
    Uisos.Assign( TAutoDB::GetInstance()->GetUisos() );
  for( int i=0; i < latt.FragmentCount(); i++ )  {
    if( latt.GetFragment(i).NodeCount() > 7 )   { // skip up to PF6 or so for Uiso analysis
      if( Uisos.Count() <= i )  Uisos.Add(0.0);
      if( Uisos[i] == 0 )  {
        int ac = 0;
        for( int j=0;  j < latt.GetFragment(i).NodeCount(); j++ )  {
          TSAtom& sa = latt.GetFragment(i).Node(j);
          if( sa.IsDeleted() || sa.GetAtomInfo() == iHydrogenIndex ||
                                sa.GetAtomInfo() == iQPeakIndex )  continue;
          Uisos[i] += sa.CAtom().GetUiso();
          ac++;
        }
        if( ac != 0 )  Uisos[i] /= ac;
      }
      if( Uisos[i] > 0 )  {
        for( int j=0;  j < latt.GetFragment(i).NodeCount(); j++ )  {
          TSAtom& sa = latt.GetFragment(i).Node(j);
          if( sa.IsDeleted() || sa.GetAtomInfo() == iHydrogenIndex )  continue;
          if( sa.GetAtomInfo() != iQPeakIndex && sa.CAtom().GetUiso() > Uisos[i]*3)  {
            sa.SetDeleted(true);
            sa.CAtom().SetDeleted(true);
            continue;
          }
        }
      }
    }
    else  {  // treat O an Cl
      if( latt.GetFragment(i).NodeCount() == 1 )  {
        TSAtom& sa = latt.GetFragment(i).Node(0);
        bool alone = true;
        for( int j=0; j < sa.CAtom().AttachedAtomCount(); j++ )
          if( sa.CAtom().GetAttachedAtom(j).GetAtomInfo() != iQPeakIndex )  {
            alone = false;
            break;
          }
        if( alone )  {
          if( sa.GetAtomInfo() == iQPeakIndex )  {
            sa.CAtom().SetLabel( "O" );
          }
          else if( sa.GetAtomInfo() == iOxygenIndex )  {
            if( sa.CAtom().GetUiso() < 0.01 )
              sa.CAtom().SetLabel( "Cl" );
          }
          else if( sa.GetAtomInfo() == iChlorineIndex )  {
            if( sa.CAtom().GetUiso() > 0.2 )
              sa.CAtom().SetLabel( "O" );
          }
          else  {
            sa.CAtom().SetLabel( "O" );
          }
        }
      }
      for( int j=0;  j < latt.GetFragment(i).NodeCount(); j++ )  {
        TSAtom& sa = latt.GetFragment(i).Node(j);
        if( sa.IsDeleted() || sa.GetAtomInfo() == iHydrogenIndex )  continue;
        if( sa.GetAtomInfo() != iQPeakIndex && sa.CAtom().GetUiso() > 0.25 )  {
          sa.SetDeleted(true);
          sa.CAtom().SetDeleted(true);
          continue;
        }
      }
    }
    for( int j=0;  j < latt.GetFragment(i).NodeCount(); j++ )  {
      TSAtom& sa = latt.GetFragment(i).Node(j);
      if( sa.IsDeleted() || sa.CAtom().IsDeleted() ||
          sa.GetAtomInfo() == iHydrogenIndex || sa.GetAtomInfo() == iQPeakIndex )  continue;

      neighbours.Clear();
      TAutoDBNode nd(sa, &neighbours);
      for( int k=0; k < nd.DistanceCount(); k++ )  {
        if( neighbours[k].GetA()->IsDeleted() )
          continue;
        if( nd.GetDistance(k) < (neighbours[k].GetA()->GetAtomInfo().GetRad1()+0.3) &&
            neighbours[k].GetA()->GetAtomInfo() != iHydrogenIndex )  {
          if( neighbours[k].GetA()->GetAtomInfo() == iQPeakIndex ||
                neighbours[k].GetA()->GetAtomInfo() <= iFluorineIndex )
          {
              neighbours[k].GetA()->SetDeleted(true);
          }
          else  {
            if( sa.GetAtomInfo() == iQPeakIndex || sa.GetAtomInfo() <= iFluorineIndex )  {
              sa.SetDeleted(true);
              sa.CAtom().SetDeleted(true);
            }
          }
        }
      }
    }
  }
  // treating NPD atoms... promoting to the next available type
  if( changeNPD )  {
    if( FXApp->CheckFileType<TIns>() )  {
      TIns* ins = (TIns*)FXApp->XFile().GetLastLoader();
      TStrPObjList<olxstr,TBasicAtomInfo*> sl(ins->GetSfac(), ' ');
      for( int i=0; i < sl.Count(); i++ ) 
        sl.Object(i) = au.GetAtomsInfo()->FindAtomInfoBySymbol(sl[i]);
      sl.QuickSort<Main_BaiComparator>();

      for( int i=0; i < latt.AtomCount(); i++ )  {
        TSAtom& sa = latt.GetAtom(i);
        if( (sa.GetEllipsoid() != NULL && sa.GetEllipsoid()->IsNPD()) ||
          (sa.CAtom().GetUiso() <= 0.005) )  {
            int ind = sl.IndexOfObject( &sa.GetAtomInfo() );
            if( ind >= 0 && ((ind+1) < sl.Count()) )  {
              sa.CAtom().AtomInfo( sl.Object(ind+1) );
            }
        }
      }
    }
  }
  //end treating NDP atoms
  if( runFuse )
    ProcessXPMacro("fuse", Error);
}
#endif //__OD_BUILD__

