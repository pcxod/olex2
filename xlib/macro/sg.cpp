#include "egc.h"
#include "xmacro.h"

#include "hkl.h"
#include "sgtest.h"

#include "log.h"
#include "etable.h"

#include "integration.h"

using namespace olex;

//..............................................................................
void XLibMacros::macSG(TStrObjList &Cmds, const TParamList &Options, TMacroError &E)  {
  TXApp &XApp = TXApp::GetInstance();
  TPtrList<TSpaceGroup>* rv = NULL;
  if( E.RetObj() != NULL )
    rv = E.GetRetObj< TPtrList<TSpaceGroup> >();

  IOlexProcessor* olx_inst = IOlexProcessor::GetInstance();

  if( olx_inst != NULL )
    olx_inst->setVar( IOlexProcessor::SGListVarName, EmptyString );

  olxstr HklFN( XApp.LocateHklFile() );

  if( HklFN.IsEmpty() )  {
    E.ProcessingError(__OlxSrcInfo, "could not locate HKL file" );
    return;
  }
  TTypeList<TBravaisLatticeRef> BravaisLattices;
  if( XApp.XFile().HasLastLoader() )  {
    TSymmLib::GetInstance()->FindBravaisLattices( XApp.XFile().GetAsymmUnit(), BravaisLattices );
    int MatchCount = 0;
    if( BravaisLattices.Count() )  {
      XApp.GetLog() << "\nPossible crystal systems from cell parameters\n";
      for( int i=0; i < BravaisLattices.Count(); i++ )  {
        if( BravaisLattices[i].GetB() == 0 )  {
          XApp.GetLog() << BravaisLattices[i].GetA()->GetName() << '\n';
          MatchCount++;
        }
      }
      if( MatchCount < BravaisLattices.Count() )  {
        XApp.GetLog() << ("Possible crystal systems of lower symmetry\n");
        for( int i=0; i < BravaisLattices.Count(); i++ )  {
          if( BravaisLattices[i].GetB() == -1 )
            XApp.GetLog() << BravaisLattices[i].GetA()->GetName() << '\n';
        }
      }
    }
  }
  TSGTest  SGTest( HklFN, XApp.XFile().GetRM().GetHKLF_mat() );
  XApp.GetLog() << '\n';
  XApp.GetLog() << ( olxstr("HKL reflections count/ (unique in P1): ") << 
    SGTest.GetHklRefCount() ) << '/' << SGTest.GetP1RefCount() << '\n';
  XApp.GetLog() << ( olxstr("Maximum/minimum intensity: ") << SGTest.GetMaxI() <<
                            '('    << SGTest.GetMaxIS() << ')'
                            << '/' << SGTest.GetMinI() << '('
                            << SGTest.GetMinIS() << ')') << '\n';
  XApp.GetLog() << ( olxstr("Average intensity/error: ")
                          << olxstr::FormatFloat(2, SGTest.GetAverageI()) << '/'
                          << olxstr::FormatFloat(2, SGTest.GetAverageIS()) ) << '\n';
  TStrList Output;

  TPtrList<TSpaceGroup> LaueClasses;
  TTypeList<TSGStats> LaueClassStats;
  for( int i=0; i < TSymmLib::GetInstance()->BravaisLatticeCount(); i++ )  {
    for( int j=0; j < TSymmLib::GetInstance()->GetBravaisLattice(i).SymmetryCount(); j++ )
    if( LaueClasses.IndexOf( &(TSymmLib::GetInstance()->GetBravaisLattice(i).GetSymmetry(j)) ) == -1 )
      LaueClasses.Add( &(TSymmLib::GetInstance()->GetBravaisLattice(i).GetSymmetry(j)) );
  }
//  for( int i=0; i < TSymmLib::GetInstance()->PointGroupCount(); i++ )  {
//    LaueClasses.AddACopy( &(TSymmLib::GetInstance()->GetPointGroup(i)) );
//  }

  //LaueClasses.Add( TSymmLib::GetInstance()->FindGroup("P112/m") );
  //LaueClasses.Add( TSymmLib::GetInstance()->FindGroup("P2/m11") );

  SGTest.MergeTest(LaueClasses, LaueClassStats);
  TPtrList<TSpaceGroup> CalculatedLaueClasses;
  // calculate average Sum(I-Ieq)/Count
  double averageLaueHit = 0;
  int laueHitCount = 0;
  for( int i=0; i < LaueClassStats.Count(); i++ )  {
    if( LaueClassStats[i].GetCount() != 0 )  {
      averageLaueHit += ( LaueClassStats[i].GetSummI()/LaueClassStats[i].GetCount() );
      laueHitCount ++;
    }
  }
  if( laueHitCount )  averageLaueHit /= laueHitCount;
  // print the table of the results and evaluate possible Laue classes
  TTTable<TStrList> laueTab(LaueClassStats.Count(), 4);
  laueTab.ColName(0) = "Class";
  laueTab.ColName(1) = "(I-Ieq)/Count";
  laueTab.ColName(2) = "Count";
  laueTab.ColName(3) = "Flag";
  for( int i=0; i < LaueClassStats.Count(); i++ )  {
    laueTab[i][0] = LaueClassStats[i].GetSpaceGroup().GetBareName();
    if( LaueClassStats[i].GetCount() != 0 )  {
      double dv = LaueClassStats[i].GetSummI()/LaueClassStats[i].GetCount();
      laueTab[i][1] = olxstr::FormatFloat(2, dv );
      laueTab[i][1] << '(' << olxstr::FormatFloat(2, LaueClassStats[i].GetSummSI()/LaueClassStats[i].GetCount() ) << ')';
      if( dv < averageLaueHit/2 )  {
        laueTab[i][3] = '+';
        CalculatedLaueClasses.Add( &LaueClassStats[i].GetSpaceGroup() );
      }
      else
        laueTab[i][3] = '-';
    }
    else  {
      laueTab[i][1] = '-';
      CalculatedLaueClasses.Add( &LaueClassStats[i].GetSpaceGroup() );
    }
    laueTab[i][2] = LaueClassStats[i].GetCount();
  }
  XApp.GetLog() << ( EmptyString );
  laueTab.CreateTXTList(Output, "1. Merge test", true, true, ' ');
  XApp.GetLog() << ( Output );
  // analyse the crystal systems from the cell parameters and the diffraction matches
  // and give warnings
  for( int i=0; i < BravaisLattices.Count(); i++ )  {
    if( BravaisLattices[i].GetB() == 0 )  {  // exact match
      bool found = false;
      for( int j=0; j < CalculatedLaueClasses.Count(); j++ )  {
        if( BravaisLattices[i].GetA() == &CalculatedLaueClasses[j]->GetBravaisLattice() )  {
          found = true;
          break;
        }
      }
      if( !found )  {
        XApp.GetLog().Warning( olxstr("Could not find match for ") << BravaisLattices[i].GetA()->GetName()
                               << " crystal system");
      }
    }
  }
  // try if a higher symmetry is found and give a warnig in the case
  for( int i=0; i < CalculatedLaueClasses.Count(); i++ )  {
    bool found = false;
    for( int j=0; j < BravaisLattices.Count(); j++ )  {
      if( BravaisLattices[j].GetA() == &CalculatedLaueClasses[i]->GetBravaisLattice() )  {
        found = true;
        break;
      }
    }
    if( !found )  {
      XApp.GetLog() << ( olxstr("An alternative symmetry found: ") <<
        CalculatedLaueClasses[i]->GetBravaisLattice().GetName()) << '\n';
    }
  }
  // evaluete and print systematic absences; also fill the Present elements list
  TPSTypeList<double, TCLattice*> SortedLatticeHits;
  TPtrList<TSpaceGroup>  SGToConsider;
  TPtrList<TSymmElement> PresentElements, AllElements, UniqueElements;

  TTypeList<TElementStats<TCLattice*> > LatticeHits;
  TTypeList<TSAStats> SAHits;
  SGTest.LatticeSATest( LatticeHits, SAHits );
  TTTable<TStrList> saStat(SAHits.Count(), 4);
  saStat.ColName(0) = "Symmetry element";
  saStat.ColName(1) = "Summ(I)/Count";
  saStat.ColName(2) = "Count";
  saStat.ColName(3) = "Flag";
  for( int i=0; i < SAHits.Count(); i++ )  {
    saStat[i][0] = SAHits[i].GetSymmElement().GetName();
    AllElements.Add( &SAHits[i].GetSymmElement() );
    if( SAHits[i].GetCount() != 0 )  {
      double v = SAHits[i].GetSummI()/SAHits[i].GetCount();
      saStat[i][1] << olxstr::FormatFloat(2, v) << '('
                               << olxstr::FormatFloat(2, SAHits[i].GetSummSI()/SAHits[i].GetCount()) << ')';
      if( SAHits[i].IsPresent() )  {
        saStat[i][3] = '+';
        if( SAHits[i].IsExcluded() )
          saStat[i][3] << '-';
        else 
          UniqueElements.Add(&SAHits[i].GetSymmElement());
        PresentElements.Add( &SAHits[i].GetSymmElement() );
      }
      else 
        saStat[i][3] = '-';
    }
    else  {
      saStat[i][3] << '-' << '+';
      PresentElements.Add( &SAHits[i].GetSymmElement() );
    }
    saStat[i][2] = SAHits[i].GetCount();
  }
  Output.Clear();
  XApp.GetLog() << ( EmptyString );
  saStat.CreateTXTList(Output, "2. Systematic absences test", true, true, ' ');
  XApp.GetLog() << ( Output );
  // print the cell centering statistics
  TTTable<TStrList> latTab(LatticeHits.Count(), 5);
  latTab.ColName(0) = "Centering";
  latTab.ColName(1) = "Strong I/Count";
  latTab.ColName(2) = "Count";
  latTab.ColName(3) = "Weak I/Count";
  latTab.ColName(4) = "Count";
  TPtrList<TCLattice>  ChosenLats;
  const double threshold = SGTest.GetAverageI()/20;
  for( int i=0; i < LatticeHits.Count(); i++ )  {
    if( LatticeHits[i].GetStrongCount() != 0 )  {
      if( LatticeHits[i].GetWeakCount() != 0 )
        SortedLatticeHits.Add(LatticeHits[i].GetSummWeakI()/LatticeHits[i].GetWeakCount(), LatticeHits[i].GetObject() );
      else
        SortedLatticeHits.Add(-1, LatticeHits[i].GetObject() );
    }
  }
  if( SortedLatticeHits.GetComparable(0) > threshold || SortedLatticeHits.GetComparable(0) == -1 )
    ChosenLats.Add( TSymmLib::GetInstance()->FindLattice("P") );
  for( int i=0; i < SortedLatticeHits.Count(); i++ )  {
    if( SortedLatticeHits.GetComparable(i) == -1 || SortedLatticeHits.GetComparable(i) < threshold )
      ChosenLats.Add( SortedLatticeHits.GetObject(i) );
  }
  if( ChosenLats.IsEmpty() )
    ChosenLats.Add( SortedLatticeHits.GetObject(0) );

  for( int i=0; i < LatticeHits.Count(); i++ )  {
    latTab[i][0] = LatticeHits[i].GetObject()->GetSymbol();
    if( LatticeHits[i].GetStrongCount() != 0 )
      latTab[i][1] << olxstr::FormatFloat(2, LatticeHits[i].GetSummStrongI()/LatticeHits[i].GetStrongCount() )
                               << '(' << olxstr::FormatFloat(2, LatticeHits[i].GetSummStrongSI()/LatticeHits[i].GetStrongCount() )  << ')';
    else
      latTab[i][1] = '-';
    latTab[i][2] = LatticeHits[i].GetStrongCount();
    if( LatticeHits[i].GetWeakCount() != 0 )
      latTab[i][3] << olxstr::FormatFloat(2, LatticeHits[i].GetSummWeakI()/LatticeHits[i].GetWeakCount() )
                               << '(' << olxstr::FormatFloat(2, LatticeHits[i].GetSummWeakSI()/LatticeHits[i].GetWeakCount() )  << ')';

    else
      latTab[i][3] = '-';
    latTab[i][4] = LatticeHits[i].GetWeakCount();
  }
  Output.Clear();
  XApp.GetLog() << ( EmptyString );
  latTab.CreateTXTList(Output, "3. Cell centering test", true, true, ' ');
  XApp.GetLog() << ( Output );

  olxstr Tmp;
  for( int i=0; i < ChosenLats.Count(); i++ )  {
    Tmp << ChosenLats[i]->GetSymbol();
    if( (i+1) < ChosenLats.Count() )  Tmp << ',';
  }
  XApp.GetLog() << ( olxstr("Chosen lattice(s): ") << Tmp << '\n');
  // print current spacegroup
  TSpaceGroup* sg = NULL;
  try  { sg = &XApp.XFile().GetLastLoaderSG();  }
  catch(...)  {}
  if( sg != NULL )  {
    olxstr Tmp("Current space group: ");
    Tmp << sg->GetName();
    if( sg->GetFullName().Length() )
      Tmp << " (" << sg->GetFullName() << ')';

    Tmp << " #" << sg->GetNumber();
    XApp.GetLog() << '\n';
    XApp.GetLog() << (Tmp << '\n');
  }
  // nor systematic absences in P-1!
  if( CalculatedLaueClasses.Count() == 1 &&
      CalculatedLaueClasses[0] == TSymmLib::GetInstance()->FindGroup("P-1") )  {
    PresentElements.Clear();
  }
  if( PresentElements.Count() >= 0 )  {
    for( int i=0; i < TSymmLib::GetInstance()->SGCount(); i++ )  {
      TSpaceGroup& sg = TSymmLib::GetInstance()->GetGroup(i);
      if( sg.GetNumber() > 230 )  continue;
      bool matchLaueClass = false;
      for( int j=0; j < CalculatedLaueClasses.Count(); j++ )  {
        if( &sg.GetLaueClass() == CalculatedLaueClasses[j] )  {
          matchLaueClass = true;
          break;
        }
      }
      if( !matchLaueClass )  continue;
      bool matchLattice = false;
      for( int j=0; j < ChosenLats.Count(); j++ ) {
        if( &sg.GetLattice() == ChosenLats[j] )  {
          matchLattice = true;
          break;
        }
      }
      if( !matchLattice )  continue;
      SGToConsider.Add( &sg);
    }
    if( SGToConsider.Count() != 0 )  {
      XApp.GetLog() << ( olxstr("Testing ") << SGToConsider.Count() << " selected space groups\n");
    }
  }
  TTypeList<TElementStats<TSpaceGroup*> > SATestResults;
  SGTest.WeakRefTest(SGToConsider, SATestResults);
  TPSTypeList<double, AnAssociation3<TElementStats<TSpaceGroup*>*, int, int>* > sortedSATestResults;

  for( int i=0; i < SATestResults.Count(); i++ )  {
    if( SATestResults[i].GetWeakCount() == 0  ) continue;
    double v = SATestResults[i].GetSummWeakI()/SATestResults[i].GetWeakCount();
    if( v < 0 )  v = 0;
    double mult = pow(10, olx_min(fabs(v)/threshold, 100.0) );
    while( v < 1 )  v ++;
    v *= mult;

    sortedSATestResults.Add( SATestResults[i].GetWeakCount()/v, new AnAssociation3<TElementStats<TSpaceGroup*>*, int, int>(&SATestResults[i], 0, 0));
  }

  //TPtrList< AnAssociation3<TElementStats<TSpaceGroup*>*, int, int> >  
  TTTable<TStrList> sgTab(sortedSATestResults.Count(), 8);
  sgTab.ColName(0) = "SG";
  sgTab.ColName(1) = "Strong I/Count";
  sgTab.ColName(2) = "Count";
  sgTab.ColName(3) = "Weak I/Count";
  sgTab.ColName(4) = "Count";
  sgTab.ColName(5) = "Laue class";
  sgTab.ColName(6) = "SA match";
  sgTab.ColName(7) = "SA/SG SA";
  int maxElementFound = 0, maxUniqueElementFound = 0;
  bool FilterByElementCount = false;
  for( int i=0; i < sortedSATestResults.Count(); i++ )  {
    sgTab[i][0] = sortedSATestResults.GetObject(i)->GetA()->GetObject()->GetName();
    if( sortedSATestResults.GetObject(i)->GetA()->GetStrongCount() != 0 )  {
      sgTab[i][1] = olxstr::FormatFloat(2,
              sortedSATestResults.GetObject(i)->GetA()->GetSummStrongI()/sortedSATestResults.GetObject(i)->GetA()->GetStrongCount() );
    }
    else  {
      sgTab[i][1] = '-';
    }
    sgTab[i][2] = sortedSATestResults.GetObject(i)->GetA()->GetStrongCount();
    if( sortedSATestResults.GetObject(i)->GetA()->GetWeakCount() != 0 )  {
      sgTab[i][3] = olxstr::FormatFloat(2,
              sortedSATestResults.GetObject(i)->GetA()->GetSummWeakI()/sortedSATestResults.GetObject(i)->GetA()->GetWeakCount() );
    }
    else  {
      sgTab[i][3] = '-';
    }
    sgTab[i][4] = sortedSATestResults.GetObject(i)->GetA()->GetWeakCount();
    sgTab[i][5] = sortedSATestResults.GetObject(i)->GetA()->GetObject()->GetLaueClass().GetBareName();
    if( !PresentElements.IsEmpty() )  {
      smatd_list sgMl;
      sortedSATestResults.GetObject(i)->GetA()->GetObject()->GetMatrices( sgMl, mattAll);
      TPtrList<TSymmElement> sgElmAll, sgElmFound;
      TSpaceGroup::SplitIntoElements(sgMl, AllElements, sgElmAll);
      TSpaceGroup::SplitIntoElements(sgMl, PresentElements, sgElmFound);
      // validate all sg elements are in the list of present ones
      bool all_present = true;
      for( int j=0; j < sgElmAll.Count(); j++ )  {
        if( PresentElements.IndexOf(sgElmAll[j]) == -1 )  {
          all_present = false;
          break;
        }
      }
      sgTab[i][6] << olxstr::FormatFloat(0, (double)sgElmFound.Count()*100/PresentElements.Count()) << '%';
      if( all_present )  {
        int unique_elm = 0;
        for( int j=0; j < sgElmFound.Count(); j++ )
          if( UniqueElements.IndexOf(sgElmFound[j]) >= 0 )
            unique_elm++;
        if( unique_elm > maxUniqueElementFound )
          maxUniqueElementFound = unique_elm;
        if( sgElmFound.Count() > maxElementFound )
          maxElementFound = sgElmFound.Count();
        if( sgElmFound.Count() <= maxElementFound )
          FilterByElementCount = true;
        sgTab[i][7] << '+';
        sortedSATestResults.GetObject(i)->B() = sgElmFound.Count();
        sortedSATestResults.GetObject(i)->C() = unique_elm;
      }
      else  {
        sgTab[i][7] << '-';
        sortedSATestResults.GetObject(i)->B() = 0;
        sortedSATestResults.GetObject(i)->C() = 0;
      }
    }
  }
  // print the result of analysis
  if( sortedSATestResults.Count() != 0 )  {
    Output.Clear();
    sgTab.CreateTXTList(Output, "4. Space group test", true, true, ' ');
    XApp.GetLog() << ( Output );
  }
  TPtrList<TSpaceGroup> FoundSpaceGroups;
  if( !PresentElements.IsEmpty() )  {
    TPtrList<TSpaceGroup> ToAppend;  // alternative groups, but lower probability
    for( int i=sortedSATestResults.Count()-1; i >= 0; i-- )  {
      if( sortedSATestResults.GetObject(i)->GetA()->GetWeakCount() != 0 )  {
        double v = sortedSATestResults.GetObject(i)->GetA()->GetSummWeakI()/sortedSATestResults.GetObject(i)->GetA()->GetWeakCount();
        if( v > SGTest.GetAverageI()/5 )
          break;
      }
      if( FilterByElementCount )  {
        if( maxElementFound > UniqueElements.Count() )  {
          if( sortedSATestResults.GetObject(i)->GetC() == maxUniqueElementFound )  {
            FoundSpaceGroups.Add( sortedSATestResults.GetObject(i)->GetA()->GetObject() );
          }  // this is still a good match!
          else if( sortedSATestResults.GetObject(i)->GetB() == maxElementFound )
            ToAppend.Add( sortedSATestResults.GetObject(i)->GetA()->GetObject() );
        }
        else  {
          if( sortedSATestResults.GetObject(i)->GetB() == maxElementFound )
            FoundSpaceGroups.Add( sortedSATestResults.GetObject(i)->GetA()->GetObject() );
        }
      }
      else
        FoundSpaceGroups.Add( sortedSATestResults.GetObject(i)->GetA()->GetObject() );
    }
    FoundSpaceGroups.AddList( ToAppend );
    // try to recover...
    if( FilterByElementCount && FoundSpaceGroups.IsEmpty() )  {
      for( int i=sortedSATestResults.Count()-1; i >= olx_max(0, sortedSATestResults.Count()-6) ; i-- )  {
        if( sortedSATestResults.GetObject(i)->GetB() == maxElementFound )
          FoundSpaceGroups.Add( sortedSATestResults.GetObject(i)->GetA()->GetObject() );
      }
    }
  }
  else  {
    // three hits from here
    for( int i=sortedSATestResults.Count()-1; i >= 0; i-- )  {
      if( sortedSATestResults.GetObject(i)->GetA()->GetWeakCount() != 0 )  {
        double v = sortedSATestResults.GetObject(i)->GetA()->GetSummWeakI()/sortedSATestResults.GetObject(i)->GetA()->GetWeakCount();
        if( v > SGTest.GetAverageI()/5 )
          break;
      }
      FoundSpaceGroups.Add( sortedSATestResults.GetObject(i)->GetA()->GetObject() );
    }
    // check all spacegroups without translations as well
    TPtrList<TSpaceGroup> laueClassGroups, possibleGroups;
    for( int i=CalculatedLaueClasses.Count()-1; i >= 0 ; i-- )  {
      laueClassGroups.Clear();
      TSymmLib::GetInstance()->FindLaueClassGroups( *CalculatedLaueClasses[i], laueClassGroups);
      for( int j=0; j < laueClassGroups.Count(); j++ )  {
        if( ChosenLats.IndexOf( &laueClassGroups[j]->GetLattice() ) == -1 )  continue;
        if( !laueClassGroups[j]->HasTranslations() )
          FoundSpaceGroups.Add( laueClassGroups[j] );
      }
    }
  }
  if( !FoundSpaceGroups.IsEmpty() )  {
    XApp.GetLog() << "Possible space groups:";
    olxstr tmp, sglist;
    TStrList Output1;
    Output.Clear();
    int cscount = 0, ncscount = 0;
    for( int i=0; i < FoundSpaceGroups.Count(); i++ )  {

      sglist << FoundSpaceGroups[i]->GetName() << ';';
      tmp = "  ";
      tmp << FoundSpaceGroups[i]->GetName();
      tmp.Format(10, true, ' ');
      tmp << "(#" << FoundSpaceGroups[i]->GetNumber() << ", Laue class " <<
      FoundSpaceGroups[i]->GetLaueClass().GetBareName() << ", Point group " <<
      FoundSpaceGroups[i]->GetPointGroup().GetBareName() << ')';

      if( FoundSpaceGroups[i]->IsCentrosymmetric() )  {
        Output.Add( tmp );  cscount++;
      }
      else  {
        Output1.Add( tmp );  ncscount++;
      }
    }
    if( olx_inst != NULL )
      olx_inst->setVar( IOlexProcessor::SGListVarName, sglist );

    XApp.GetLog() << '\n';
    XApp.GetLog() << "Noncentrosymmetric:\n";
    if( Output1.Count() )
      XApp.GetLog() << Output1;
    else
      XApp.GetLog() << "  None";
    XApp.GetLog() << '\n';
    XApp.GetLog() << "Centrosymmetric:\n";
    if( Output.Count() )
      XApp.GetLog() << Output;
    else
      XApp.GetLog() << "  None";

    TTTable<TStrList> sgOutput( olx_max(cscount,ncscount), 2 );
    cscount = 0;  ncscount=0;
    for( int i=0; i < FoundSpaceGroups.Count(); i++ )  {
      tmp = "<a href=\"reset -s=";
      tmp << FoundSpaceGroups[i]->GetName() << "\">" << FoundSpaceGroups[i]->GetName() << "</a>";
      if( FoundSpaceGroups[i]->IsCentrosymmetric() )  {
        sgOutput[cscount][0] = tmp;
        cscount++;
      }
      else  {
        sgOutput[ncscount][1] = tmp;
        ncscount++;
      }
      if( rv != NULL )
        rv->Add( FoundSpaceGroups[i] );
    }
    Output.Clear();
    sgOutput.CreateHTMLList(Output, EmptyString, false, false, false);
  }
  else  {
    XApp.GetLog().Error( "Could not find any suitable space group");
    TTTable<TStrList> sgOutput( 1, 2 );
    sgOutput[0][0] = "n/a";
    sgOutput[0][1] = "n/a";
    Output.Clear();
    sgOutput.CreateHTMLList(Output, EmptyString, false, false, false);
  }
  if( olx_inst != NULL )
    TCStrList(Output).SaveToFile(olx_inst->getDataDir()+"spacegroups.htm");

  for( int i=0; i < sortedSATestResults.Count(); i++ )
    delete sortedSATestResults.GetObject(i);
/* // does not work as necessary yet

  XApp.GetLog() << ("  --  CENTER OF SYMMETRY SEARCH  --  " );
  XApp.GetLog() << ( olxstr("Average test (0.637 for cs and 0.785 for non-cs): ") << olxstr::FormatFloat(3, SGTest.GetAverageTestValue()) );
  XApp.GetLog() << ( olxstr("Variance test (2 for cs and 1 for non-cs): ") << olxstr::FormatFloat(2, SGTest.GetVarianceTestValue()) );

  if( CheckFileType<TCRSFile>() )  {
    TSpaceGroup* sg = ((TCRSFile*)XApp.XFile().GetLastLoader())->GetSG();
    if( sg != NULL )
      XApp.GetLog() << ( olxstr("The procedure might have failed as the hkl file was preprocessed for ") << sg->GetName(), &ErrorFontColor);
  }
*/
}
