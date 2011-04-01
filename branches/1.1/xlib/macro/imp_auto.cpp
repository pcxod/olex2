// if this is not inluded: internal compiler error at 0xc19d9e with base 0xc10000
#include "egc.h"

#include "auto.h"
#include "ins.h"

#include "integration.h"
#include "xmacro.h"
#include "sortedlist.h"
#include "beevers-lipson.h"
#include "arrays.h"
#include "maputil.h"
#include "estopwatch.h"

struct _auto_BI {  
  int type;
  uint32_t max_bonds;  
};
static _auto_BI _autoMaxBond[] = { 
  {iOxygenIndex, 2},
  {iFluorineIndex, 1},
};

typedef SortedPtrList<const cm_Element, TPointerPtrComparator> SortedElementList;
// helper function
size_t imp_auto_AtomCount(const TAsymmUnit& au)  {
  size_t ac = 0;
  for( size_t i=0; i < au.AtomCount(); i++ )  {
    const TCAtom& a = au.GetAtom(i);
      if( a.IsDeleted() || a.GetType() < 3 )  continue;
    ac++;
  }
  return ac;
}

void XLibMacros::funATA(const TStrObjList &Cmds, TMacroError &Error)  {
  TXApp& xapp = TXApp::GetInstance();
  olxstr folder( Cmds.IsEmpty() ? EmptyString() : Cmds[0] );
  int arg = 0;
  if( folder.IsNumber() )  {
    arg = folder.ToInt();
    folder.SetLength(0);
  }
  if( folder.IsEmpty() && olex::IOlexProcessor::GetInstance() != NULL )
    olex::IOlexProcessor::GetInstance()->executeMacro("clean -npd");
  // Qpeak rings analysis ...
  try  {
    TTypeList< TSAtomPList > rings;
    xapp.FindRings("QQQQQQ", rings);
    for( size_t i=0; i < rings.Count(); i++ )  {
      double rms = TSPlane::CalcRMS( rings[i] );
      if( rms < 0.1 && TNetwork::IsRingRegular( rings[i]) )  {
        for( size_t j=0; j < rings[i].Count(); j++ )  {
          rings[i][j]->CAtom().SetLabel("Cph");
        }
      }
    }
  }
  catch( ... )  {  ;  }
  static olxstr FileName(xapp.XFile().GetFileName());
//  static TAtomTypePermutator AtomPermutator;
//  AtomPermutator.SetActive( false );
//  AtomPermutator.SetActive( !Options.Contains("p") );
//  if( !AtomPermutator.IsActive() )  AtomPermutator.Init();

//  if( FileName != xapp.XFile().GetFileName() )  {
//    AtomPermutator.Init();
//    FileName = xapp.XFile().GetFileName();
//  }
//  if( AtomPermutator.IsActive() )  AtomPermutator.ReInit( xapp.XFile().GetAsymmUnit() );
  olxstr autodbf( xapp.GetBaseDir() + "acidb.db");
  if( TAutoDB::GetInstance() == NULL )  {
    TEGC::AddP( new TAutoDB(*((TXFile*)xapp.XFile().Replicate()), xapp ) );
    if( TEFile::Exists( autodbf ) )  {
      TEFile dbf(autodbf, "rb");
      TAutoDB::GetInstance()->LoadFromStream(dbf);
    }
  }
  if( !folder.IsEmpty() )  {
    TAutoDB::GetInstance()->ProcessFolder(folder);
    TEFile dbf(autodbf, "w+b");
    TAutoDB::GetInstance()->SaveToStream(dbf);
  }

  TLattice& latt = xapp.XFile().GetLattice();
  TAsymmUnit& au = latt.GetAsymmUnit();
  ElementPList elm_l;
  if( arg == 1 )  {
    if( xapp.CheckFileType<TIns>() )  {
      TIns& ins = xapp.XFile().GetLastLoader<TIns>();
      const ContentList& cl = ins.GetRM().GetUserContent();
      for( size_t i=0; i < cl.Count(); i++ ) 
        elm_l.Add(cl[i].element);
    }    
  }
  TAutoDB::AnalysisStat stat;
  uint64_t st = TETime::msNow();
  TAutoDB::GetInstance()->AnalyseStructure( xapp.XFile().GetFileName(), latt, 
    NULL, stat, elm_l.IsEmpty() ? NULL : &elm_l);
  st = TETime::msNow() - st;
  TBasicApp::NewLogEntry(logInfo) << "Elapsed time " << st << " ms";

//  if( AtomPermutator.IsActive() )  AtomPermutator.Permutate();
  if( olex::IOlexProcessor::GetInstance() != NULL )
    olex::IOlexProcessor::GetInstance()->executeMacro("fuse");
  size_t ac = imp_auto_AtomCount(au);
  if( ac == 0 )  // clearly something is wron gwhen it happens...
    ac = 1;
  Error.SetRetVal( olxstr(stat.AtomTypeChanges!=0) << ';' << 
    (double)stat.ConfidentAtomTypes*100/ac );
}
//..............................................................................
void XLibMacros::macAtomInfo(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& xapp = TXApp::GetInstance();
  TSAtomPList satoms;
  xapp.FindSAtoms( Cmds.Text(' '), satoms );
  if( TAutoDB::GetInstance() == NULL )  {
    TEGC::AddP(new TAutoDB(*((TXFile*)xapp.XFile().Replicate()), xapp));
    olxstr autodbf( xapp.GetBaseDir() + "acidb.db");
    if( TEFile::Exists( autodbf ) )  {
      TEFile dbf(autodbf, "rb");
      TAutoDB::GetInstance()->LoadFromStream( dbf );
    }
  }
  TStrList report;
  for( size_t i=0; i < satoms.Count(); i++ ) 
    TAutoDB::GetInstance()->AnalyseNode(*satoms[i], report);
  xapp.NewLogEntry() << report;
}
//..............................................................................
void XLibMacros::macVATA(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& xapp = TXApp::GetInstance();
  TEFile log(Cmds.Text(' '), "a+b");
  if( TAutoDB::GetInstance() == NULL )  {
    TEGC::AddP( new TAutoDB(*((TXFile*)xapp.XFile().Replicate()), xapp ) );
    olxstr autodbf( xapp.GetBaseDir() + "acidb.db");
    if( TEFile::Exists( autodbf ) )  {
      TEFile dbf(autodbf, "rb");
      TAutoDB::GetInstance()->LoadFromStream( dbf );
    }
  }
  TStrList report;
  TAutoDB::GetInstance()->ValidateResult( xapp.XFile().GetFileName(), xapp.XFile().GetLattice(), report);
  report.SaveToTextStream(log);
}
//..............................................................................
struct Main_BaiComparator {
  static int Compare(const TPrimitiveStrListData<olxstr,const cm_Element*>* a, 
                     const TPrimitiveStrListData<olxstr,const cm_Element*>* b)  {
      return a->Object->z - b->Object->z;
  }
};
void helper_CleanBaiList(TStrPObjList<olxstr,const cm_Element*>& list, SortedElementList& au_bais)  {
  TXApp& xapp = TXApp::GetInstance();
  if( xapp.CheckFileType<TIns>() )  {
    TIns& ins = xapp.XFile().GetLastLoader<TIns>();
    list.Clear();   
    const ContentList& cl = ins.GetRM().GetUserContent();
    for( size_t i=0; i < cl.Count(); i++ )  {
      au_bais.Add(&cl[i].element);
      list.Add(cl[i].element.symbol, &cl[i].element);
    }
    list.QuickSort<Main_BaiComparator>();
  }
}

void XLibMacros::macClean(TStrObjList &Cmds, const TParamList &Options, TMacroError &Error)  {
  TXApp& xapp = TXApp::GetInstance();
  TStrPObjList<olxstr,const cm_Element*> sfac;
  SortedElementList AvailableTypes;
  static ElementPList StandAlone;
  if( StandAlone.IsEmpty() )  {
    StandAlone.Add(XElementLib::GetByIndex(iOxygenIndex));
    StandAlone.Add(XElementLib::GetByIndex(iMagnesiumIndex));
    StandAlone.Add(XElementLib::GetByIndex(iChlorineIndex));
    StandAlone.Add(XElementLib::GetByIndex(iPotassiumIndex));
    StandAlone.Add(XElementLib::GetByIndex(iCalciumIndex));
  }
  helper_CleanBaiList(sfac, AvailableTypes);
  if( TAutoDB::GetInstance() == NULL )  {
    olxstr autodbf( xapp.GetBaseDir() + "acidb.db");
    TEGC::AddP(new TAutoDB(*((TXFile*)xapp.XFile().Replicate()), xapp ));
    if( TEFile::Exists(autodbf) )  {
      TEFile dbf(autodbf, "rb");
      TAutoDB::GetInstance()->LoadFromStream(dbf);
    }
  }

  const bool runFuse = !Options.Contains("f");
  size_t changeNPD = ~0;
  if( Options.Contains("npd") )  {
    olxstr _v = Options.FindValue("npd");
    if( _v.IsEmpty() )
      changeNPD = 0;
    else
      changeNPD = _v.ToSizeT();
  }
  const bool analyseQ = !Options.Contains("aq");
  const bool assignTypes = !Options.Contains("at");
  const double aqV = Options.FindValue("aq", "0.3").ToDouble(); // R+aqV
  // qpeak anlysis
  TAsymmUnit& au = xapp.XFile().GetAsymmUnit();
  if( analyseQ )  {
    TPSTypeList<double, TCAtom*> SortedQPeaks;
    TTypeList< AnAssociation2<double, TCAtomPList*> > vals;
    size_t cnt = 0;
    double avQPeak = 0;
    bool OnlyQPeakModel = true;
    for( size_t i=0; i < au.AtomCount(); i++ )  {
      if( au.GetAtom(i).IsDeleted() )  continue;
      if( au.GetAtom(i).GetType() != iQPeakZ )
        OnlyQPeakModel  = false;
      else  {
        SortedQPeaks.Add(au.GetAtom(i).GetQPeak(), &au.GetAtom(i));
        avQPeak += au.GetAtom(i).GetQPeak();
        cnt++;
      }
    }
    if( cnt != 0 )
      avQPeak /= cnt;
    cnt = 0;
    if( !SortedQPeaks.IsEmpty() )  {
      vals.AddNew<double, TCAtomPList*>(0, new TCAtomPList);
      for( size_t i=SortedQPeaks.Count()-1; i >=1; i-- )  {
        if( (SortedQPeaks.GetKey(i) - SortedQPeaks.GetKey(i-1))/SortedQPeaks.GetKey(i) > 0.05 )  {
          //FGlConsole->PostText( olxstr("Threshold here: ") << SortedQPeaks.GetObject(i)->GetLabel() );
          vals.GetLast().A() += SortedQPeaks.GetKey(i);
          vals.GetLast().B()->Add( SortedQPeaks.GetObject(i));
          cnt++;
          vals.GetLast().A() /= cnt;
          cnt = 0;
          vals.AddNew<double, TCAtomPList*>(0, new TCAtomPList);
          continue;
        }
        vals.GetLast().A() += SortedQPeaks.GetKey(i);
        vals.GetLast().B()->Add( SortedQPeaks.GetObject(i));
        cnt ++;
      }
      vals.GetLast().B()->Add(SortedQPeaks.GetObject(0));
      cnt++;
      if( cnt > 1 )
        vals.GetLast().A() /= cnt;

      TBasicApp::NewLogEntry(logInfo) << "Average QPeak: " << avQPeak;
      TBasicApp::NewLogEntry(logInfo) << "QPeak steps:";
      for( size_t i=0; i < vals.Count(); i++ )
        TBasicApp::NewLogEntry(logInfo) << vals[i].GetA();

      //    double thVal = 2;
      double thVal = (avQPeak  < 2 ) ? 2 : avQPeak*0.75;

      TBasicApp::NewLogEntry(logInfo) << "QPeak threshold:" << thVal;

      if( SortedQPeaks.Count() == 1 )  {  // only one peak present
        if( SortedQPeaks.GetKey(0) < thVal )
          SortedQPeaks.GetObject(0)->SetDeleted(true);
      }
      else  {
        double wght = (SortedQPeaks.GetLastKey()-avQPeak)/
          (avQPeak-SortedQPeaks.GetKey(0));
        for( size_t i=vals.Count()-1; i != InvalidIndex; i-- )  {
          if( vals[i].GetA() < thVal )  {
            for( size_t j=0; j < vals[i].GetB()->Count(); j++ )
              vals[i].GetB()->GetItem(j)->SetDeleted(true);
          }
        }
      }
      for( size_t i=0; i < vals.Count(); i++ )
        delete vals[i].B();
    }
  }
  // end qpeak analysis

  // distance analysis
  TLattice& latt = xapp.XFile().GetLattice();
  ASObjectProvider& objects = latt.GetObjects();
  // qpeaks first
  TSAtomPList QPeaks;
  for( size_t i=0;  i < objects.atoms.Count(); i++ )  {
    TSAtom& sa = objects.atoms[i];
    if( sa.IsDeleted() || sa.CAtom().IsDeleted() )  continue;
    if( sa.GetType() == iQPeakZ )
      QPeaks.Add(sa);
  }
  for( size_t i=0; i < QPeaks.Count(); i++ )  {
    if( QPeaks[i]->IsDeleted() || QPeaks[i]->CAtom().IsDeleted() )  continue;
    TTypeList<AnAssociation2<TCAtom*, vec3d> > neighbours;
    TAutoDBNode nd(*QPeaks[i], &neighbours);
    for( size_t j=0; j < nd.DistanceCount(); j++ )  {
      if( nd.GetDistance(j) < (neighbours[j].GetA()->GetType().r_bonding+aqV) )  {  // at least an H-bond
        if( neighbours[j].GetA()->GetType() == iQPeakZ )  {
          if( nd.GetDistance(j) < 1 )  {
            if( neighbours[j].GetA()->GetQPeak() < QPeaks[i]->CAtom().GetQPeak() )  {
              neighbours[j].GetA()->SetDeleted(true);
            }
          }
        }
        else {
          QPeaks[i]->SetDeleted(true);
          QPeaks[i]->CAtom().SetDeleted(true);
          continue;
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

//    for( size_t j=0; j < nd.AngleCount(); j++ )  {
//      if( nd.GetAngle(j) < 90
//    }
  }
  // call whatever left carbons ...
  if( assignTypes )  {
    for( size_t i=0; i < QPeaks.Count(); i++ )  {
      if( QPeaks[i]->IsDeleted() || QPeaks[i]->CAtom().IsDeleted() )  continue;
      TBasicApp::NewLogEntry(logInfo) << QPeaks[i]->CAtom().GetLabel() << " -> C";
      QPeaks[i]->CAtom().SetLabel("C");
    }
  }

  TDoubleList Uisos;
  if( xapp.XFile().GetFileName() == TAutoDB::GetInstance()->GetLastFileName() )
    Uisos.Assign(TAutoDB::GetInstance()->GetUisos());
  for( size_t i=0; i < latt.FragmentCount(); i++ )  {
    if( latt.GetFragment(i).NodeCount() > 7 )   { // skip up to PF6 or so for Uiso analysis
      while( Uisos.Count() <= i ) Uisos.Add(0.0);
      if( olx_abs(Uisos[i]) < 1e-6 )  {
        size_t ac = 0;
        for( size_t j=0;  j < latt.GetFragment(i).NodeCount(); j++ )  {
          TSAtom& sa = latt.GetFragment(i).Node(j);
          if( sa.IsDeleted() || sa.GetType().GetMr() < 3 )  continue;
          Uisos[i] += sa.CAtom().GetUiso();
          ac++;
        }
        if( ac != 0 )  Uisos[i] /= ac;
      }
      if( Uisos[i] > 1e-6 )  {
        for( size_t j=0;  j < latt.GetFragment(i).NodeCount(); j++ )  {
          TSAtom& sa = latt.GetFragment(i).Node(j);
          if( sa.IsDeleted() || sa.GetType() == iHydrogenZ )  continue;
          if( sa.GetType() != iQPeakZ && sa.CAtom().GetUiso() > Uisos[i]*3)  {
            TBasicApp::NewLogEntry(logInfo) << sa.GetLabel() << " too large, deleting";
            sa.SetDeleted(true);
            sa.CAtom().SetDeleted(true);
            continue;
          }
        }
      }
    }
    else  if( assignTypes )  {  // treat O an Cl
      if( latt.GetFragment(i).NodeCount() == 1 && !latt.GetFragment(i).Node(0).IsDeleted() )  {
        TSAtom& sa = latt.GetFragment(i).Node(0);
        bool alone = true;
        for( size_t j=0; j < sa.CAtom().AttachedSiteCount(); j++ )
          if( sa.CAtom().GetAttachedAtom(j).GetType() != iQPeakZ )  {
            alone = false;
            break;
          }
        if( alone )  {
          bool assignHeaviest = false, assignLightest = false;
          const TAutoDB::AnalysisStat& stat = TAutoDB::GetInstance()->GetStats();
          size_t ac = imp_auto_AtomCount(au);
          if( ac == 0 ) // this would be really strange
            ac++;
          if( stat.SNAtomTypeAssignments == 0 && ((double)stat.ConfidentAtomTypes/ac) > 0.5 )  { // now we can make up types
            bool found = false;
            for( size_t j=0; j < StandAlone.Count(); j++ )  {
              if( sa.GetType() == *StandAlone[j] )  {
                found = true;
                if( sa.CAtom().GetUiso() < 0.01 )  {  // search heavier
                  bool assigned = false;
                  for( size_t k=j+1; k < StandAlone.Count(); k++ )  {
                    if( AvailableTypes.IndexOf(StandAlone[k]) != InvalidIndex )  {
                      sa.CAtom().SetLabel(StandAlone[k]->symbol, false);
                      sa.CAtom().SetType(*StandAlone[k]);
                      assigned = true;
                      break;
                    }
                  }
                  if( !assigned )  assignHeaviest = true;
                }
                else if( sa.CAtom().GetUiso() > 0.2 )  {  // search lighter
                  bool assigned = false;
                  for( size_t k=j-1; k != InvalidIndex; k-- )  {
                    if( AvailableTypes.IndexOf(StandAlone[k]) != InvalidIndex )  {
                      sa.CAtom().SetLabel(StandAlone[k]->symbol, false);
                      sa.CAtom().SetType(*StandAlone[k]);
                      assigned = true;
                      break;
                    }
                  }
                  if( !assigned )  assignLightest = true;
                }
              }
            }
            if( !found || assignLightest )  {  // make lightest then
              sa.CAtom().SetLabel(StandAlone[0]->symbol, false);
              sa.CAtom().SetType(*StandAlone[0]);
            }
            else if( assignHeaviest )  {
              sa.CAtom().SetLabel(StandAlone.GetLast()->symbol, false);
              sa.CAtom().SetType(*StandAlone.GetLast());
            }
          }
        }
      }
      for( size_t j=0;  j < latt.GetFragment(i).NodeCount(); j++ )  {
        TSAtom& sa = latt.GetFragment(i).Node(j);
        if( sa.IsDeleted() || sa.GetType() == iHydrogenZ )  continue;
        if( sa.GetType() != iQPeakZ && sa.CAtom().GetUiso() > 0.25 )  {
          TBasicApp::NewLogEntry(logInfo) << sa.GetLabel() << " blown up";
          sa.SetDeleted(true);
          sa.CAtom().SetDeleted(true);
          continue;
        }
      }
    }
    for( size_t j=0;  j < latt.GetFragment(i).NodeCount(); j++ )  {
      TSAtom& sa = latt.GetFragment(i).Node(j);
      if( sa.IsDeleted() || sa.CAtom().IsDeleted() || sa.GetType().GetMr() < 3  )  continue;
      TTypeList<AnAssociation2<TCAtom*, vec3d> > neighbours;
      TAutoDBNode nd(sa, &neighbours);
      for( size_t k=0; k < nd.DistanceCount(); k++ )  {
        if( neighbours[k].GetA()->IsDeleted() )
          continue;
        if( nd.GetDistance(k) < (neighbours[k].GetA()->GetType().r_bonding+aqV) &&
            neighbours[k].GetA()->GetType() != iHydrogenZ )  {
          if( neighbours[k].GetA()->GetType() == iQPeakZ ||
                neighbours[k].GetA()->GetType() <= iFluorineZ )
          {
            neighbours[k].GetA()->SetDeleted(true);
          }
          else  {
            if( sa.GetType() == iQPeakZ || sa.GetType() <= iFluorineZ )  {
              sa.SetDeleted(true);
              sa.CAtom().SetDeleted(true);
            }
          }
        }
      }
    }
  }
  // treating NPD atoms... promoting to the next available type
  if( changeNPD > 0 && !sfac.IsEmpty() )  {
    size_t atoms_transformed = 0;
    for( size_t i=0; i < objects.atoms.Count(); i++ )  {
      TSAtom& sa = objects.atoms[i];
      if( (sa.GetEllipsoid() != NULL && sa.GetEllipsoid()->IsNPD()) ||
        (sa.CAtom().GetUiso() <= 0.005) )
      {
        size_t ind = sfac.IndexOfObject(&sa.GetType());
        if( ind != InvalidIndex && ((ind+1) < sfac.Count()) )  {
          sa.CAtom().SetType(*sfac.GetObject(ind+1));
          if( ++atoms_transformed >= changeNPD )
            break;
        }
      }
    }
  }
  //end treating NDP atoms
  if( runFuse && olex::IOlexProcessor::GetInstance() != NULL )
    olex::IOlexProcessor::GetInstance()->executeMacro("fuse");
}
//..............................................................................
struct Main_SfacComparator {
  static int Compare(const AnAssociation2<int,const cm_Element*>* a, 
                     const AnAssociation2<int,const cm_Element*>* b)  {
      return b->GetB()->z - a->GetB()->z;
  }
};
void XLibMacros::funVSS(const TStrObjList &Cmds, TMacroError &Error)  {
  TXApp& xapp = TXApp::GetInstance();
  TLattice& latt = xapp.XFile().GetLattice();
  TUnitCell& uc = latt.GetUnitCell();
  TAsymmUnit& au = latt.GetAsymmUnit();
  int ValidatedAtomCount = 0, AtomCount=0;
  bool trim = Cmds[0].ToBool();
  bool use_formula = Cmds[0].ToBool();
  if( use_formula )  {
    TTypeList< AnAssociation2<int,const cm_Element*> > sl;
    size_t ac = 0;
    const ContentList& cl = xapp.XFile().GetRM().GetUserContent();
    for( size_t i=0; i < cl.Count(); i++ )  {
      if( cl[i].element == iHydrogenZ )  continue;
      sl.AddNew((int)cl[i].count, &cl[i].element);
      ac += (int)cl[i].count;
    }
    sl.QuickSorter.Sort<Main_SfacComparator>(sl);  // sorts ascending
    double auv = latt.GetUnitCell().CalcVolume()/latt.GetUnitCell().MatrixCount();
    double ratio = auv/(16*ac);
    for( size_t i=0; i < sl.Count(); i++ )
      sl[i].A() = olx_round(ratio*sl[i].GetA());

    TPSTypeList<double, TCAtom*> SortedQPeaks;
    for( size_t i=0; i < au.AtomCount(); i++ )  {
      if( au.GetAtom(i).IsDeleted() )  continue;
      if( au.GetAtom(i).GetType() == iQPeakZ )
        SortedQPeaks.Add(au.GetAtom(i).GetQPeak(), &au.GetAtom(i));
      else  {
        for( size_t j=0; j < sl.Count(); j++ )  {
          if( *sl[j].GetB() == au.GetAtom(i).GetType() )  {
            sl[j].A()--;
            break;
          }
        }
      }
    }
    for( size_t i=0; i < sl.Count(); i++ )  {
      while( sl[i].GetA() > 0 )  {
        if( SortedQPeaks.IsEmpty() )  break;
        sl[i].A() --;
        SortedQPeaks.GetLast().Object->SetLabel((olxstr(sl[i].GetB()->symbol) << i), false);
        SortedQPeaks.GetLast().Object->SetType(*sl[i].B());
        SortedQPeaks.GetLast().Object->SetQPeak(0);
        SortedQPeaks.Delete(SortedQPeaks.Count()-1);
      }
      if( SortedQPeaks.IsEmpty() ) break;
    }
    // get rid of the rest of Q-peaks and "validate" geometry of atoms
    for( size_t i=0; i < au.AtomCount(); i++ )  {
      if( au.GetAtom(i).GetType() == iQPeakZ ) 
        au.GetAtom(i).SetDeleted(true);
    }
    TArrayList<AnAssociation2<TCAtom const*, vec3d> > res;
    for( size_t i=0; i < au.AtomCount(); i++ )  {
      if( au.GetAtom(i).IsDeleted() )  continue;
      uc.FindInRangeAC(au.GetAtom(i).ccrd(), au.GetAtom(i).GetType().r_bonding+1.3, res);
      vec3d center = au.Orthogonalise(au.GetAtom(i).ccrd());
      for( size_t j=0; j < res.Count(); j++ )  {
        if( res[j].GetA()->GetId() == au.GetAtom(i).GetId() && center.QDistanceTo(res[j].GetB()) < 1e-4 )
          res.Delete(j--);
      }
      AtomCount++;
      double wght = 1;
      if( res.Count() > 1 )  {
        double awght = 1./(res.Count()*(res.Count()-1));
        for( size_t j=0; j < res.Count(); j++ )  {
          if( res[j].GetB().QLength() < 1 )
            wght -= 0.5/res.Count();
          for( size_t k=j+1; k < res.Count(); k++ )  {
            double cang = (res[j].GetB()-center).CAngle(res[k].GetB()-center);
            if( cang > 0.588 )  { // 56 degrees
              wght -= awght;
            }
          }
        }
      }
      else if( res.Count() == 1 ) {  // just one bond
        if( res[0].GetB().QLength() < 1 )
          wght = 0;
      }
      else  // no bonds, cannot say anything
        wght = 0;

      if( wght >= 0.95 )
        ValidatedAtomCount++;
      res.Clear();
    }
    xapp.XFile().EndUpdate();
    // validate max bonds
    TUnitCell& uc = xapp.XFile().GetUnitCell();
    TLattice& latt = xapp.XFile().GetLattice();
    ASObjectProvider& objects = latt.GetObjects();
    TTypeList<TAtomEnvi> bc_to_check;
    const size_t maxb_cnt = sizeof(_autoMaxBond)/sizeof(_autoMaxBond[0]);
    for( size_t i=0; i < objects.atoms.Count(); i++ )  {
      TSAtom& sa = objects.atoms[i];
      for( size_t j=0; j < maxb_cnt; j++ )  {
        if( sa.GetType() == _autoMaxBond[j].type )  {
          uc.GetAtomEnviList(sa, bc_to_check.AddNew()); 
          if( bc_to_check.GetLast().Count() <= _autoMaxBond[j].max_bonds )  {
            bc_to_check.NullItem(bc_to_check.Count()-1);
          }
        }
      }
    }
    bc_to_check.Pack();
    bool changes = true;
    while( changes )  {
      changes = false;
      for( size_t i=0; i < bc_to_check.Count(); i++ )  {
        size_t sati = InvalidIndex;
        for( size_t j=0; j < sl.Count(); j++ )  {
          if( bc_to_check[i].GetBase().GetType() == *sl[j].GetB() )  {
            sati = j;
            break;
          }
        }
        if( sati != InvalidIndex && (sati+1) < sl.Count() )  {
          bc_to_check[i].GetBase().CAtom().SetType(*sl[sati+1].GetB());
          changes = true;
        }
      }
    }
    if( !bc_to_check.IsEmpty() )
      xapp.XFile().EndUpdate();
  }
  else if( trim && false )  {
    double auv = latt.GetUnitCell().CalcVolume()/latt.GetUnitCell().MatrixCount();
    int ac = (int)olx_round(auv/18.6);
    index_t _to_delete = (int)au.AtomCount() - ac;
    if( _to_delete > 0 )  {
      size_t to_delete = _to_delete;
      TPSTypeList<double, TCAtom*> SortedQPeaks;
      TAsymmUnit& au = xapp.XFile().GetAsymmUnit();
      for( size_t i=0; i < au.AtomCount(); i++ )  {
        if( au.GetAtom(i).IsDeleted() )  continue;
        if( au.GetAtom(i).GetType() == iQPeakZ )
          SortedQPeaks.Add( au.GetAtom(i).GetQPeak(), &au.GetAtom(i));
      }
      for( size_t i=0; i < olx_min(to_delete,SortedQPeaks.Count()); i++ )
        SortedQPeaks.GetObject(i)->SetDeleted(true);
    }
    xapp.XFile().EndUpdate();
  }
  for( size_t i=0; i < au.AtomCount(); i++ )
    au.GetAtom(i).SetLabel( au.CheckLabel(NULL, au.GetAtom(i).GetLabel()), false);
//  TAutoDB::AnalysisStat stat;
//  TAutoDB::GetInstance()->AnalyseStructure( xapp.XFile().GetFileName(), latt, 
//    NULL, stat, NULL);
  Error.SetRetVal((double)ValidatedAtomCount*100/AtomCount);
}
//..............................................................................
double TryPoint(TArray3D<float>& map, const TUnitCell& uc, const vec3d& crd)  {
  TRefList refs;
  TArrayList<compd> F;
  TArrayList<SFUtil::StructureFactor> P1SF;
  const TUnitCell::SymSpace sym_space = uc.GetSymSpace();
  SFUtil::GetSF(refs, F, SFUtil::mapTypeDiff, SFUtil::sfOriginOlex2, SFUtil::scaleRegression);
  SFUtil::ExpandToP1(refs, F, sym_space, P1SF);
  const vec3s dim = map.GetSize();
  const vec3d norm(1./dim[0], 1./dim[1], 1./dim[2]);
  BVFourier::MapInfo mi = BVFourier::CalcEDM(P1SF, map.Data, dim, uc.CalcVolume());
  TArrayList<MapUtil::peak> _Peaks;
  TTypeList<MapUtil::peak> Peaks;
  MapUtil::Integrate<float>(map.Data, dim, (float)((mi.maxVal - mi.minVal)/2.5), _Peaks);
  MapUtil::MergePeaks(sym_space, norm, _Peaks, Peaks);

  double sum=0;
  size_t count=0;
  for( size_t i=0; i < Peaks.Count(); i++ )  {
    const MapUtil::peak& peak = Peaks[i];
    const vec3d cnt = norm*peak.center; 
    const double ed = peak.summ/peak.count;
    if( uc.FindClosestDistance(crd, cnt) < 1.0 )  {
      sum += ed;
      count++;
    }
  }
  return count == 0 ? 0 : sum/count;
}

void XLibMacros::funFATA(const TStrObjList &Cmds, TMacroError &E)  {
  TXApp& xapp = TXApp::GetInstance();
  TStopWatch sw(__FUNC__);
  double resolution = 0.2;
  resolution = 1./resolution;
  TRefList refs;
  TArrayList<compd> F;
  olxstr err(SFUtil::GetSF(refs, F, SFUtil::mapTypeDiff, SFUtil::sfOriginOlex2, SFUtil::scaleRegression));
  if( !err.IsEmpty() )  {
    E.ProcessingError(__OlxSrcInfo, err);
    return;
  }
  TAsymmUnit& au = xapp.XFile().GetAsymmUnit();
  TUnitCell& uc = xapp.XFile().GetUnitCell();
  TArrayList<SFUtil::StructureFactor> P1SF;
  const TUnitCell::SymSpace sym_space = uc.GetSymSpace();
  sw.start("Expanding structure factors to P1 (fast symm)");
  SFUtil::ExpandToP1(refs, F, sym_space, P1SF);
  sw.stop();
  const double vol = xapp.XFile().GetLattice().GetUnitCell().CalcVolume();
// init map
  const vec3i dim(au.GetAxes()*resolution);
  TArray3D<float> map(0, dim[0]-1, 0, dim[1]-1, 0, dim[2]-1);
  vec3d norm(1./dim[0], 1./dim[1], 1./dim[2]);
  const size_t PointCount = dim.Prod();
  TArrayList<AnAssociation3<TCAtom*,double, int> > atoms(au.AtomCount());
  for( size_t i=0; i < au.AtomCount(); i++ )  {
    atoms[i].A() = &au.GetAtom(i);
    atoms[i].B() = 0;
    atoms[i].C() = 0;
    atoms[i].A()->SetTag(i);
  }
  size_t found_cnt = 0;
  sw.start("Calculating electron density map in P1 (Beevers-Lipson)");
  BVFourier::MapInfo mi = BVFourier::CalcEDM(P1SF, map.Data, dim, vol);
  sw.stop();
  TArrayList<MapUtil::peak> _Peaks;
  TTypeList<MapUtil::peak> Peaks;
  sw.start("Integrating P1 map: ");
  MapUtil::Integrate<float>(map.Data, dim, (float)((mi.maxVal - mi.minVal)/2.5), _Peaks);
  MapUtil::MergePeaks(sym_space, norm, _Peaks, Peaks);
  sw.stop();
  for( size_t i=0; i < Peaks.Count(); i++ )  {
    const MapUtil::peak& peak = Peaks[i];
    const vec3d cnt = norm*peak.center; 
    const double ed = peak.summ/peak.count;
    TCAtom* oa = uc.FindOverlappingAtom(cnt, false, 0.5);
    if( oa != NULL && oa->GetType() != iQPeakZ )  {
      atoms[oa->GetTag()].B() += ed;
      atoms[oa->GetTag()].C()++;
    }
  }
  const double minEd = mi.sigma*3;
  for( size_t i=0; i < atoms.Count(); i++ )  {
    if( atoms[i].GetC() != 0 )  {
      const double ed = atoms[i].GetB()/atoms[i].GetC();  
      if( olx_abs(ed) < minEd )  continue;
      double p_ed = 0, n_ed  = 0; 
      const cm_Element& original_type = atoms[i].GetA()->GetType();
      cm_Element* n_e = XElementLib::NextZ(original_type);
      if( n_e != NULL )  {
        atoms[i].GetA()->SetType(*n_e);
        sw.start("Trying next element");
        n_ed = TryPoint(map, xapp.XFile().GetUnitCell(), atoms[i].GetA()->ccrd());
        sw.stop();
      }
      cm_Element* p_e = XElementLib::PrevZ(original_type);
      if( p_e != NULL )  {
        sw.start("Trying previous element");
        atoms[i].GetA()->SetType(*p_e);
        p_ed = TryPoint(map, xapp.XFile().GetUnitCell(), atoms[i].GetA()->ccrd());
        sw.stop();
      }
      atoms[i].GetA()->SetType(original_type);
      if( n_e != NULL && p_e != NULL )  {
        if( (n_ed == 0 || olx_sign(n_ed) == olx_sign(p_ed))&& p_ed > 0 )  {
          found_cnt++;
          TBasicApp::NewLogEntry() << "Atom type changed from " << original_type.symbol << 
            " to " << n_e->symbol << " for " << atoms[i].GetA()->GetLabel();
          atoms[i].GetA()->SetType(*n_e);
        }
        else if( n_ed < 0 && (p_ed == 0 || olx_sign(p_ed) == olx_sign(n_ed)) )  {
          found_cnt++;
          TBasicApp::NewLogEntry() << "Atom type changed from " << original_type.symbol << 
            " to " << p_e->symbol << " for " << atoms[i].GetA()->GetLabel();
          atoms[i].GetA()->SetType(*p_e);
        }
        else if( n_ed != 0 && p_ed != 0 )  {
          if( olx_sign(n_ed) != olx_sign(p_ed) )  {
            const double r = ed/(olx_abs(n_ed)+olx_abs(p_ed));
            if( olx_abs(r) > 0.5 )  {
              found_cnt++;
              if( r > 0 )  {
                TBasicApp::NewLogEntry() << "Atom type changed from " << original_type.symbol << 
                  " to " << n_e->symbol << " for " << atoms[i].GetA()->GetLabel();
                atoms[i].GetA()->SetType(*n_e);
              }
              else  {
                TBasicApp::NewLogEntry() << (olxstr("Atom type changed from ") << original_type.symbol << 
                  " to " << p_e->symbol << " for " << atoms[i].GetA()->GetLabel() << '\n');
                atoms[i].GetA()->SetType(*p_e);
              }
            }
          }
          else  { // same sign?
            found_cnt++;
            if( n_ed > 0 )  {
              TBasicApp::NewLogEntry() << "Atom type changed from " << original_type.symbol << 
                " to " << n_e->symbol << " for " << atoms[i].GetA()->GetLabel();
              atoms[i].GetA()->SetType(*n_e);
            }
            else  {
              TBasicApp::NewLogEntry() << "Atom type changed from " << original_type.symbol << 
                " to " << p_e->symbol << " for " << atoms[i].GetA()->GetLabel();
              atoms[i].GetA()->SetType(*p_e);
            }
          }
        }
      }
    }
  }
  sw.print(xapp.NewLogEntry(logInfo));
  if( found_cnt == 0 )
    TBasicApp::NewLogEntry() << "No problems were found";
  else  {
    au.InitData();
    xapp.XFile().EndUpdate();
  }
  E.SetRetVal(found_cnt != 0);
}

