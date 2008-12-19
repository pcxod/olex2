//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#ifdef __BORLANDC__
#pragma hdrstop
#endif

#include "network.h"
#include "satom.h"
#include "sbond.h"

#include "actions.h"

#include "lattice.h"
#include "asymmunit.h"

#include "bapp.h"
#include "log.h"

#include "egraph.h"

#include "olxmps.h"
#include "estopwatch.h"

#undef GetObject

//---------------------------------------------------------------------------
// TNetwork function bodies
//---------------------------------------------------------------------------
TNetwork::TNetwork(TLattice* P, TNetwork *N):TBasicNode<TSAtom, TSBond>(N)  {
  Lattice = P;
}
//..............................................................................
TNetwork::~TNetwork()  {
  return;
}
//..............................................................................
// sorts atoms according to the distcance from {0,0,0}
int AtomsSortByDistance(const TSAtom* A, const TSAtom* A1)  {
  double d = A->crd().QLength() - A1->crd().QLength();
  if( d < 0 )  return -1;
  if( d > 0 )  return 1;
  return 0;
}
//..............................................................................
void TNetwork::SortNodes()  {
  Nodes.QuickSorter.SortSF(Nodes, AtomsSortByDistance);
}
//..............................................................................
void TNetwork::TDisassembleTaskRemoveSymmEq::Run(long index)  {
  if( Atoms[index]->CAtom().GetExyzGroup() != NULL )  return;
  if( (Atoms[index]->GetTag() & 0x0002) != 0 )  return;
  const int ac = Atoms.Count();
  for( int i=index+1; i < ac; i++ )  {
    if( fabs(Distances[0][index] - Distances[0][i]) > 0.01 )  return;
    if( Atoms[index]->crd().QDistanceTo( Atoms[i]->crd() ) < 0.0001 )  {
      Atoms[index]->AddMatrices(Atoms[i]);
      Atoms[i]->SetTag(2);            // specify that the node has to be deleted
    }
  }
}
void TNetwork::TDisassembleTaskCheckConnectivity::Run(long index)  {
  if( Atoms[index]->IsStandalone() )  return;
  const int this_p = Atoms[index]->CAtom().GetPart();
  const int ac = Atoms.Count();
  for( int i=index+1; i < ac; i++ )  {
    if( (Distances[0][i] - Distances[0][index]) > dcMaxCBLength ||
        (Distances[0][i] - Distances[0][index]) < -dcMaxCBLength )  return;
    if( (Distances[1][i] - Distances[1][index]) > dcMaxCBLength ||
        (Distances[1][i] - Distances[1][index]) < -dcMaxCBLength )  continue;
    if( (Distances[2][i] - Distances[2][index]) > dcMaxCBLength ||
        (Distances[2][i] - Distances[2][index]) < -dcMaxCBLength )  continue;
    if( (Distances[3][i] - Distances[3][index]) > dcMaxCBLength ||
        (Distances[3][i] - Distances[3][index]) < -dcMaxCBLength )  continue;

    const double D = Atoms[index]->crd().QDistanceTo( Atoms[i]->crd());
    double D1 = (double)(Atoms[index]->GetAtomInfo().GetRad1() + Atoms[i]->GetAtomInfo().GetRad1() + Delta);
    D1 *= D1;
    const int that_p = Atoms[i]->CAtom().GetPart();
    if(  D < D1 )  {
      if( this_p == 0 || that_p == 0 )  {
        Atoms[index]->AddNode(*Atoms[i]);
        Atoms[i]->AddNode(*Atoms[index]);  // crosslinking
      }
      else if( (this_p == that_p) && (this_p > 0)  )  {
        Atoms[index]->AddNode(*Atoms[i]);
        Atoms[i]->AddNode(*Atoms[index]);  // crosslinking
      }
    }
  }
}
//..............................................................................
void TNetwork::Disassemble(TSAtomPList& Atoms, TNetPList& Frags, TSBondPList* InterBonds)  {
  if( Atoms.Count() < 2 )  return;
  TStopWatch sw(__FUNC__);
  sw.start("Initialising");
  TNetwork *Net;
  //TSAtom *A;
  int ac;
  double** Distances = new double* [4];
  double  Delta = GetLattice().GetDelta();
  Atoms.QuickSorter.SortSF(Atoms, AtomsSortByDistance);
  Distances[0] = new double[ Atoms.Count() ];  // distsnaces from {0,0,0} to an atom
  ac = Atoms.Count();
  for( int i = 0; i < ac; i++ )  {
    Distances[0][i] = Atoms[i]->crd().Length();
    Atoms[i]->SetTag(1);
    Atoms[i]->SetNetId(-1);
  }
  // find & remove symmetrical equivalenrs from AllAtoms
  TDisassembleTaskRemoveSymmEq searchEqTask( Atoms, Distances);
  // profiling has shown it gives no benifit and makes the process slow
  //TListIteratorManager<TDisassembleTaskRemoveSymmEq> searchEq(searchEqTask, Atoms.Count(), tQuadraticTask, 100);
  ac = Atoms.Count();
  for( int i=0; i < ac; i++ )
    searchEqTask.Run(i);
  // removing symmetrical equivalents from the Atoms list (passes as param)
  //............................................
  for( int i = 0; i < ac; i++ )  {
    if( (Atoms[i]->GetTag() & 0x0002) != 0 )  {
      delete Atoms[i];
      Atoms[i] = NULL;
      continue;
    }
    Atoms[i]->Clear();
    // preallocate memory to improve mulithreading
    Atoms[i]->SetCapacity( Atoms[i]->NodeCount() + Atoms[i]->CAtom().AttachedAtomCount() );
  }
  Atoms.Pack();
  if( Atoms.IsEmpty() )  return;
  //............................................

  ac = Atoms.Count();
  Distances[1] = new double[ ac ];
  Distances[2] = new double[ ac ];
  Distances[3] = new double[ ac ];
  for( int i = 0; i < ac; i++ )  {  // recalculate distances and remove some function calls
    TSAtom* A = Atoms[i];
    Distances[0][i] = A->crd().Length();
    Distances[1][i] = A->crd()[0];
    Distances[2][i] = A->crd()[1];
    Distances[3][i] = A->crd()[2];
  }
  sw.start("Connectivity analysis");
  TDisassembleTaskCheckConnectivity searchConTask( Atoms, Distances, Delta);
  TListIteratorManager<TDisassembleTaskCheckConnectivity> searchCon(searchConTask, Atoms.Count(), tQuadraticTask, 100);
  sw.start("Creating bonds");
  // creating bonds
  TNetwork& defNet = *Frags.Add( new TNetwork(&GetLattice(), this) );
  for( int i=0; i < ac; i++ )  {
    TSAtom* A1 = Atoms[i];
    if( A1->IsStandalone() )  {
      defNet.AddNode(*A1);
      A1->SetNetwork(defNet);
      continue;
    }
    if( A1->GetTag() != 0 )  {
      Net = new TNetwork(&GetLattice(), this);
      Net->AddNode(*A1);
      Frags.Add( Net );
      A1->SetNetwork(*Net);
      for( int j=0; j < Net->NodeCount(); j++ )  {
        TSAtom& A2 = Net->Node(j);
        A2.SetTag(0);
        for( int k=0; k < A2.NodeCount(); k++ )  {
          TSAtom& A3 = A2.Node(k);
          if( A3.GetTag() != 0 )  {
            Net->AddNode(A3);
            A3.SetNetwork(*Net);
            TSBond* B = new TSBond(Net);
            B->SetType(sotBond);
            B->SetA(A2); B->SetB(A3);
            A2.AddBond(*B);  A3.AddBond(*B);
            Net->AddBond(*B);
            A3.SetTag(0);
            A2.SetTag(0);
            continue;
          }
          if( A3.GetNetId() > j && A3.GetTag() == 0 )  {  // the atom is in the list, but has not been processes
            TSBond* B = new TSBond(Net);                  // in this case we need to create a bond
            B->SetType(sotBond);
            B->SetA(A2); B->SetB(A3);
            A2.AddBond(*B);  A3.AddBond(*B);
            Net->AddBond(*B);
          }
        }
      }
    }
  }
  sw.start("Searching H-bonds");
  // preallocate 50 Hbonds per fragment
  if( InterBonds )  InterBonds->SetCapacity( InterBonds->Count() + Frags.Count()*50); 
  THBondSearchTask searchHBTask( Atoms, InterBonds, Distances, GetLattice().GetDeltaI());
  TListIteratorManager<THBondSearchTask> searchHB(searchHBTask, Atoms.Count(), tQuadraticTask, 100);
  sw.start("Finalising");
  delete [] Distances[0];
  delete [] Distances[1];
  delete [] Distances[2];
  delete [] Distances[3];
  delete [] Distances;
  sw.stop();
  sw.print( TBasicApp::GetLog(), &TLog::Info );
}
//..............................................................................
void TNetwork::THBondSearchTask::Run(long ind)  {
  TSAtom *AA = NULL,
         *DA = NULL;
  TSAtom* A1 = Atoms[ind];
  int aiIndex = A1->GetAtomInfo().GetIndex();
  if( aiIndex == iHydrogenIndex )
    AA = A1;
  else if( aiIndex == iNitrogenIndex || aiIndex == iOxygenIndex || aiIndex == iFluorineIndex ||
      aiIndex == iChlorineIndex || aiIndex == iSulphurIndex )
    DA = A1;

  if( AA == NULL && DA == NULL )  return;

  const int this_p = A1->CAtom().GetPart();
  const int ac = Atoms.Count();
  for( int i=ind+1; i < ac; i++ )  {
    if( (Distances[0][ind] - Distances[0][i]) > dcMaxCBLength ||
        (Distances[0][ind] - Distances[0][i]) < -dcMaxCBLength )  return;
    if( (Distances[1][ind] - Distances[1][i]) > dcMaxCBLength ||
        (Distances[1][ind] - Distances[1][i]) < -dcMaxCBLength )  continue;
    if( (Distances[2][ind] - Distances[2][i]) > dcMaxCBLength ||
        (Distances[2][ind] - Distances[2][i]) < -dcMaxCBLength )  continue;
    if( (Distances[3][ind] - Distances[3][i]) > dcMaxCBLength ||
        (Distances[3][ind] - Distances[3][i]) < -dcMaxCBLength )  continue;

    int aiIndex1 = Atoms[i]->GetAtomInfo().GetIndex();
    if( !((AA != NULL && (aiIndex1 == iNitrogenIndex || aiIndex1 == iOxygenIndex||
                        aiIndex1 == iFluorineIndex || aiIndex1 == iChlorineIndex ||
                        aiIndex1 == iSulphurIndex))  ||
          (DA != NULL && aiIndex1 == iHydrogenIndex) ) )  continue;

    if( A1->GetNetwork() == Atoms[i]->GetNetwork() )  {
      if( A1->IsConnectedTo( *Atoms[i]) )  continue;
      // check for N-C-H (N-C) bonds are not leagal
      TSAtom *CSA = ((AA!=NULL)?AA:Atoms[i]),
             *NA  = ((DA!=NULL)?DA:Atoms[i]);
      bool connected = false;
      for( int j=0; j < CSA->NodeCount(); j++ )  {
        if( CSA->Node(j).GetAtomInfo() == iQPeakIndex )  // 17/05/2007 - skip the Q peaks!
          continue;
        if( CSA->Node(j).IsConnectedTo(*NA) )  {
          connected = true;
          break;
        }
      }
      if( connected )  continue;

      const int that_p = Atoms[i]->CAtom().GetPart();
      const double D = A1->crd().QDistanceTo( Atoms[i]->crd() );
      double D1 = A1->GetAtomInfo().GetRad1() + Atoms[i]->GetAtomInfo().GetRad1() + Delta;
      D1 *= D1;
      if(  D < D1 )  {
         if( (this_p == that_p && this_p >= 0) || this_p == 0 || that_p == 0 )  {
          TSBond* B = new TSBond(&A1->GetNetwork());
          B->SetType(sotHBond);
          B->SetA(*A1);
          B->SetB(*Atoms[i]);
          A1->AddBond(*B);
          Atoms[i]->AddBond(*B);
          A1->GetNetwork().AddBond(*B);
        }
      }
    }
    else  {
      const int that_p = Atoms[i]->CAtom().GetPart();
      const double D = A1->crd().QDistanceTo( Atoms[i]->crd() );
      double D1 = A1->GetAtomInfo().GetRad1() + Atoms[i]->GetAtomInfo().GetRad1() + Delta;
      D1 *= D1;
      if(  D < D1 )  {
        if( (this_p == that_p && this_p >= 0) || this_p == 0 || that_p == 0 )  {
          TSBond* B = new TSBond( &A1->GetNetwork() );
          B->SetType(sotHBond);
          B->SetA(*A1);
          B->SetB(*Atoms[i]);
          if( Bonds != NULL )
            Bonds->Add( B );
        }
      }
    }
  }
}
//..............................................................................
bool TNetwork::CBondExists(const TCAtom& CA1, const TCAtom& CA2, double D) const  {
  if(  D < (CA1.GetAtomInfo().GetRad1() + CA2.GetAtomInfo().GetRad1() + GetLattice().GetDelta() ) )  {
    if( (CA1.GetPart() == CA2.GetPart() && CA2.GetPart() >= 0 ) || 
         CA1.GetPart() == 0 ||CA2.GetPart() == 0 )
      return true;
  }
  return false;
}
//..............................................................................
bool TNetwork::HBondExists(const TCAtom& CA1, const TCAtom& CA2, double D) const  {
  if(  D < (CA1.GetAtomInfo().GetRad1() + CA2.GetAtomInfo().GetRad1() + GetLattice().GetDeltaI() ) )  {
    if( (CA1.GetPart() == CA2.GetPart() && CA2.GetPart() >=0 ) || 
      CA1.GetPart() == 0 || CA2.GetPart() == 0 )
      return true;
  }
  return false;
}
//..............................................................................
// HELPER function
class TNetTraverser  {
  olxstr Data;
public:
  TNetTraverser() {  Data.SetIncrement(1024);  }
  bool OnItem(const TEGraphNode<int, TSAtom*>& v) {
    olxstr tmp(v.GetData(), v.Count()*6);
    tmp << '{';
    for( int i=0; i < v.Count(); i++ )  {
      tmp << v.Item(i).GetData();
      if( (i+1) < v.Count() )  tmp << ',';
    }
    tmp << '}';
    Data << tmp;
    return true;
  }
  const olxstr& GetData()  const  {  return Data;  }
  void ClearData()  {  Data = EmptyString;  }
};

void ResultCollector( TEGraphNode<int,TSAtom*>& subRoot,
                        TEGraphNode<int,TSAtom*>& Root,
                        TTypeList< AnAssociation2<int, int> >& res )  {
  res.AddNew( subRoot.GetObject()->GetNetId(), Root.GetObject()->GetNetId() );
  for( int i=0; i < subRoot.Count(); i++ )
    ResultCollector( subRoot.Item(i), Root.Item(i), res );
}

/*
void ExpandGraphNode( TTypeList< TEGraphNode<int,TSAtom*>* >& allNodes, TEGraphNode<int,TSAtom*>& graphNode, TSAtom* node)  {
  if( node->GetTag() == 1 )  return;
  node->Tag( 1 );
  for( int i=0; i < node->NodeCount(); i++ )  {
    TSAtom* sa = (TSAtom*)node->Node(i);
    if( sa->GetTag() != 0 )  continue;
    allNodes.AddACopy( &graphNode.NewNode( sa->GetAtomInfo()->GetIndex(), sa ) );
  }
}
void BuildGraph( TEGraphNode<int,TSAtom*>& graphNode, TSAtom* node)  {
  TTypeList< TEGraphNode<int,TSAtom*>* > allNodes;

  ExpandGraphNode(allNodes, graphNode, node);

  for( int i=0; i < allNodes.Count(); i++ )
    ExpandGraphNode( allNodes, *allNodes[i], allNodes[i]->GetObject() );
}
*/

void BreadthFirstTag(TSAtomPList& all, TSAtom* node)  {
  for( int i=0; i < node->NodeCount(); i++ )  {
    TSAtom& sa = node->Node(i);
    if( sa.GetTag() != 0 )  continue;
    all.Add( &sa )->SetTag( node->GetTag() + 1);
  }
}
void BreadthFirstTags(TSAtom* sa)  {
  TSAtomPList all;
  all.SetCapacity( sa->GetNetwork().NodeCount() );
  all.Add( sa );
  sa->SetTag(1);
  BreadthFirstTag(all, sa);
  for( int i=0; i < all.Count(); i++ )  {
    BreadthFirstTag( all, all[i] );
    //all[i]->CAtom()->Label() = all[i]->GetTag();
  }
}

void ExpandGraphNode(TEGraphNode<int,TSAtom*>& graphNode)  {

  for( int i=0; i < graphNode.GetObject()->NodeCount(); i++ )  {
    TSAtom& sa = graphNode.GetObject()->Node(i);
    if( sa.GetTag() <= graphNode.GetObject()->GetTag() )  continue;
    ExpandGraphNode( graphNode.NewNode( sa.GetAtomInfo().GetIndex(), &sa ) );
  }
}

void BuildGraph( TEGraphNode<int,TSAtom*>& graphNode, TSAtom* node)  {
  BreadthFirstTags(node);
  ExpandGraphNode(graphNode);
}

//..............................................................................
bool TNetwork::DoMatch( TNetwork& net, TTypeList< AnAssociation2<int, int> >& res )  {
  if( NodeCount() != net.NodeCount() )  return false;
  TSAtom* thisSa = NULL;
  int maxbc = 0;
  double maxMw = 0;
  for( int i=0; i < NodeCount(); i++ )  {
    Node(i).SetTag(0);
    if( Node(i).NodeCount() > maxbc )  {
      thisSa = &Node(i);
      maxbc = Node(i).NodeCount();
      maxMw = Node(i).GetAtomInfo().GetMr();
    }
    else if( Node(i).NodeCount() == maxbc )  {
      if( Node(i).GetAtomInfo().GetMr() > maxMw )  {
        thisSa = &Node(i);
        maxMw = thisSa->GetAtomInfo().GetMr();
      }
    }
  }
  if( thisSa == NULL )  return false;
  TEGraph<int, TSAtom*> thisGraph( thisSa->GetAtomInfo().GetIndex(), thisSa );
//  TEGraph<int, TSAtom*> thisGraph( 0, thisSa );
  BuildGraph( thisGraph.GetRoot(), thisSa );
  TNetTraverser trav;
  trav.OnItem( thisGraph.GetRoot() );
  thisGraph.GetRoot().Traverser.LevelTraverse(thisGraph.GetRoot(), trav);
  TBasicApp::GetLog().Info( trav.GetData() );
  for( int i=0; i < net.NodeCount(); i++ )  {
    TSAtom* thatSa = &net.Node(i);
    if( thisSa->NodeCount() != thatSa->NodeCount() )  continue;
    if( thisSa->GetAtomInfo().GetIndex() != thatSa->GetAtomInfo().GetIndex() )  continue;
    for( int j=0; j < net.NodeCount(); j++ )
      net.Node(j).SetTag(0);
    TEGraph<int, TSAtom*> thatGraph( thatSa->GetAtomInfo().GetIndex(), thatSa );
    BuildGraph(thatGraph.GetRoot(), thatSa);

    trav.ClearData();
    trav.OnItem( thatGraph.GetRoot() );
    thatGraph.GetRoot().Traverser.LevelTraverse(thatGraph.GetRoot(), trav);
    TBasicApp::GetLog().Info( trav.GetData() );
    if( thisGraph.GetRoot().DoMatch( thatGraph.GetRoot() ) )  {
      trav.ClearData();
      trav.OnItem( thatGraph.GetRoot() );
      thatGraph.GetRoot().Traverser.LevelTraverse(thatGraph.GetRoot(), trav);
      TBasicApp::GetLog().Info( trav.GetData() );
      ResultCollector( thisGraph.GetRoot(), thatGraph.GetRoot(), res);
      return true;
    }
  }
  return false;
}
//..............................................................................
bool TNetwork::IsSubgraphOf( TNetwork& net, TTypeList< AnAssociation2<int, int> >& res,
                             const TIntList& rootsToSkip )  {

  if( NodeCount() > net.NodeCount() )  return false;
  TSAtom* thisSa = NULL;
  int maxbc = 0;
  for( int i=0; i < NodeCount(); i++ )  {
    Node(i).SetTag(0);
    if( Node(i).NodeCount() > maxbc )  {
      thisSa = &Node(i);
      maxbc = Node(i).NodeCount();
    }
  }
  TEGraph<int, TSAtom*> thisGraph( thisSa->GetAtomInfo().GetIndex(), thisSa );
  BuildGraph( thisGraph.GetRoot(), thisSa );
  TIntList GraphId;
  for( int i=0; i < net.NodeCount(); i++ )  {
    TSAtom* thatSa = &net.Node(i);
    if( thisSa->NodeCount() > thatSa->NodeCount() )  continue;
    if( thisSa->GetAtomInfo().GetIndex() != thatSa->GetAtomInfo().GetIndex() )  continue;
    if( rootsToSkip.IndexOf(i) != -1 )
      continue;
    for( int j=0; j < net.NodeCount(); j++ )
      net.Node(j).SetTag(0);
    TEGraph<int, TSAtom*> thatGraph( thatSa->GetAtomInfo().GetIndex(), thatSa );
    BuildGraph(thatGraph.GetRoot(), thatSa);
    //continue;
    if( thisGraph.GetRoot().IsSubgraphOf( thatGraph.GetRoot() ) )  {
      ResultCollector( thisGraph.GetRoot(), thatGraph.GetRoot(), res);
      return true;
    }
  }
  return false;
}
//..............................................................................
void RS_BreadthFirstTag(TSAtomPList& all, TSAtom* node)  {
  for( int i=0; i < node->NodeCount(); i++ )  {
    TSAtom& sa = node->Node(i);
    if( sa.GetTag() != 0 )  continue;
    all.Add( &sa )->SetTag( node->GetTag() + 1);
  }
}
void RS_BreadthFirstTags(TSAtom* sa, int stopAfter)  {
  TSAtomPList all;
  all.SetCapacity( sa->GetNetwork().NodeCount() );
  all.Add( sa );
  sa->SetTag(1);
  BreadthFirstTag(all, sa);
  for( int i=0; i < all.Count(); i++ )  {
    BreadthFirstTag( all, all[i] );
    //all[i]->CAtom()->Label() = all[i]->GetTag();
  }
}

bool TNetwork_TryRing( TSAtom* sa, TSAtomPList& ring, const TPtrList<TBasicAtomInfo>& ringContent, int level=1 )  {
  if( ringContent[level-1] != NULL && (sa->GetAtomInfo().GetIndex() != ringContent[level-1]->GetIndex()) )
    return false;
  sa->SetTag(level);
  for( int i=0; i < sa->NodeCount(); i++ )  {
    TSAtom& a = sa->Node(i);
    if( a.IsDeleted() )  continue;
    if( level >= ringContent.Count() && a.GetTag() == 1 )
      return true;
    if( a.GetTag() != 0 && a.GetTag() < level )  continue;
    if( level < ringContent.Count() && TNetwork_TryRing(&a, ring, ringContent, level+1) ) {
      ring.Add( &a );
      return true;
    }
  }
  sa->SetTag(0); // unroll the tags
  return false;
}
// tries to find the ring in given direction
bool TNetwork_TryRing( TSAtom& sa, int node, TSAtomPList& ring, const TPtrList<TBasicAtomInfo>& ringContent )  {
  sa.SetTag(1);
  if( TNetwork_TryRing(&sa.Node(node), ring, ringContent, 2) ) {
    return true;
  }
  return false;
}

int TNetwork_SortRingAtoms( const TSAtom* a, const TSAtom* b )  {
  return a->GetTag() - b->GetTag();
}

void TNetwork_UnifyRings(TTypeList<TSAtomPList>& rings)  {
  for( int i=0; i < rings.Count(); i++ )
    rings[i].QuickSorter.SortSF( rings[i], TNetwork_SortRingAtoms);
  // leave unique rings only
  for( int i=0; i < rings.Count(); i++ )  {
    if( rings.IsNull(i) )  continue;
    for( int j=i+1; j < rings.Count(); j++ )  {
      if( rings.IsNull(j) )  continue;
      bool found = true;
      for( int k=0; k < rings[i].Count(); k++ )  {
        TSAtom* a = rings[i][k],
          *b = rings[j][k];
        if( rings[i][k]->GetTag() != rings[j][k]->GetTag() )  {
          found = false;
          break;
        }
      }
      if( found )  rings.NullItem(j);
    }
  }
}


void TNetwork::FindRings( const TPtrList<TBasicAtomInfo>& ringContent,
                          TTypeList<TSAtomPList>& res)  {
  TSAtomPList all;
  all.SetCapacity( NodeCount() );
  for( int i=0; i < NodeCount(); i++ )  {
    TSAtom& sa = Node(i);
    sa.SetTag(0);
    if( sa.IsDeleted() ) continue;
    if( sa.NodeCount() > 1 )  // a ring node must have at least two bonds!
      all.Add( &sa );
  }
  // we have to keep the order of the ring atoms, so need an extra 'rings' array
  TSAtomPList ring;
  TTypeList<TSAtomPList> rings;
  int resCount = res.Count();
  for( int i=0; i < all.Count(); i++ )  {
    if( all[i]->GetAtomInfo() != *ringContent[0] )  continue;
    ring.Clear();
    for( int j=0; j < NodeCount(); j++ )
      Node(j).SetTag(0);
    ring.Add( all[i] );
    if( TNetwork_TryRing( all[i], ring, ringContent) )  {
      res.AddCCopy( ring );
      rings.AddCCopy( ring );
    }
  }
  for( int i=0; i < NodeCount(); i++ )
    Node(i).SetTag(i);
  TNetwork_UnifyRings( rings );
  for( int i=0; i < rings.Count(); i++ )  {
    if( rings.IsNull(i) )
      res.NullItem( resCount+i );
  }
  res.Pack();
}
//..............................................................................
void TNetwork::FindAtomRings(TSAtom& ringAtom, const TPtrList<TBasicAtomInfo>& ringContent,
                             TTypeList<TSAtomPList>& res)  {
  if( ringAtom.NodeCount() < 2 || &ringAtom.GetNetwork() != this )  return;
  if( ringContent[0] != NULL && ringAtom.GetAtomInfo() != *ringContent[0] )  return;
  TSAtomPList all;
  all.SetCapacity( NodeCount() );
  for( int i=0; i < NodeCount(); i++ )  {
    TSAtom& sa = Node(i);
    sa.SetTag(0);
    if( sa.IsDeleted() ) continue;
    if( sa.NodeCount() > 1 )  // a ring node must have at least two bonds!
      all.Add( &sa );
  }
  // we have to keep the order of the ring atoms, so need an extra 'rings' array
  TSAtomPList ring;
  TTypeList<TSAtomPList> rings;
  int resCount = res.Count();
  for( int i=0; i < ringAtom.NodeCount(); i++ )  {
    TSAtom& a = ringAtom.Node(i);
    if( a.IsDeleted() || a.NodeCount() < 2 || 
       (ringContent[1] != NULL && a.GetAtomInfo() != *ringContent[1]) )  continue;
    ring.Clear();
    for( int j=0; j < NodeCount(); j++ )
      Node(j).SetTag(0);
    if( TNetwork_TryRing( ringAtom, i, ring, ringContent) )  {
      ring.Add( &a );  // the ring is in reverse order
      ring.Add(&ringAtom);
      res.AddCCopy( ring );
      rings.AddCCopy( ring );
    }
  }
  for( int i=0; i < NodeCount(); i++ )
    Node(i).SetTag(i);
  TNetwork_UnifyRings(rings);
  for( int i=0; i < rings.Count(); i++ )  {
    if( rings.IsNull(i) )
      res.NullItem( resCount+i );
  }
  res.Pack();
}
//..............................................................................
TNetwork::RingInfo& TNetwork::AnalyseRing( const TSAtomPList& ring, TNetwork::RingInfo& ri )  {
  int pivot = -1, pivot_count = 0;
  double maxmw = 0;
  for( int i=0; i < ring.Count(); i++ )  {
    if( !ri.HasAfix && ring[i]->CAtom().GetAfix() != 0 )
      ri.HasAfix = true;
    TSAtomPList& al = ri.Substituents.AddNew();
   
    int nhc = 0, // not hydrogen atom count
      rnc = 0;   // of which belong to the ring
    double local_maxmw = 0;
    for( int j=0; j < ring[i]->NodeCount(); j++ )  {
      TSAtom& ra = ring[i]->Node(j);
      if( ra.IsDeleted() || ra.GetAtomInfo() == iQPeakIndex )  continue;
      double mw = ra.GetAtomInfo().GetMr();
      if( mw < 3 )  continue; // H, D
      if( ra.crd().DistanceTo( ring[i]->crd() ) > 2 )  continue;  // skip M-E bonds
      if( mw > local_maxmw )  local_maxmw = mw;
      if( ring.IndexOf(&ra) != -1 )
        rnc++;
      else
        al.Add(&ra);
      nhc++;
    }
    if( nhc != rnc )  {
      if( local_maxmw > maxmw )  {
        ri.HeaviestSubsIndex = i;
        ri.HeaviestSubsType = &ring[i]->GetAtomInfo();
        maxmw = local_maxmw;
      }
      ri.Substituted.Add(i);
      if( nhc-rnc > ri.MaxSubsANode )
        ri.MaxSubsANode = nhc-rnc;
    }
    else if( rnc > 2 ) 
      ri.Ternary.Add(i);
  }
  // analyse alpha atoms (substituted next to ternary atoms)
  for( int i=0; i < ri.Substituted.Count(); i++ )  {
    for( int j=0;  j < ri.Ternary.Count(); j++ )  {
      if( ring[ri.Substituted[i]]->IsConnectedTo( *ring[ri.Ternary[j]] ) )  {
        ri.Alpha.Add( ri.Substituted[i] );
        break;
      }
    }
  }
  return ri;
}
//..............................................................................
bool TNetwork::IsRingRegular(const TSAtomPList& ring)  {
  vec3d cent;
  for( int i=0; i < ring.Count(); i++ )
    cent += ring[i]->crd();
  cent /= ring.Count();
  double avAng = 2*M_PI/ring.Count(), avDis = 0;
  for( int i=0; i < ring.Count(); i++ )
    avDis += cent.DistanceTo( ring[i]->crd() );
  avDis /= ring.Count();
  for( int i=0; i < ring.Count(); i++ )  {
    double d = cent.DistanceTo( ring[i]->crd() );
    if( fabs(d-avDis) > 0.2 )  return false;
  }
  vec3d a, b;
  for( int i=0; i < ring.Count(); i++ )  {
    a = ring[i]->crd();
    if( (i+1) == ring.Count() )
      b = ring[0]->crd();
    else
      b = ring[i+1]->crd();
    a -= cent;
    b -= cent;
    double ca = a.CAngle(b);
    if( ca < -1 )  ca = -1;
    if( ca > 1 ) ca = 1;
    ca = acos(ca);
    if( fabs(ca-avAng) > 5./M_PI )  return false;
  }
  return true;
}
//..............................................................................
// quaternion method, Acta A45 (1989), 208
double TNetwork_FindAlignmentMatrix(const TTypeList< AnAssociation2<TSAtom*,TSAtom*> >& atoms,
                         smatd& res, bool TryInversion)  {
  ematd evm(4,4), ev(4,4);
  vec3d centA, centB(res.t), v;
  TAsymmUnit& au = atoms[0].GetA()->GetNetwork().GetLattice().GetAsymmUnit();
  for( int i=0; i < atoms.Count(); i++ )  {
    if( TryInversion )  {
      v = atoms[i].GetA()->ccrd();
      v *= -1;
      au.CellToCartesian(v);
      centA += v;
    }
    else
      centA += atoms[i].GetA()->crd();
  }
  centA /= atoms.Count();
  for( int i=0; i < atoms.Count(); i++ )  {
    if( TryInversion )  {
      v = atoms[i].GetA()->ccrd();
      v *= -1;
      au.CellToCartesian(v);
    }
    else
      v = atoms[i].GetA()->crd();
    v -= centA;

    double xm = v[0] - (atoms[i].GetB()->crd()[0]-centB[0]),
           xp = v[0] + (atoms[i].GetB()->crd()[0]-centB[0]),
           yp = v[1] + (atoms[i].GetB()->crd()[1]-centB[1]),
           ym = v[1] - (atoms[i].GetB()->crd()[1]-centB[1]),
           zm = v[2] - (atoms[i].GetB()->crd()[2]-centB[2]),
           zp = v[2] + (atoms[i].GetB()->crd()[2]-centB[2]);
    evm[0][0] += (QRT(xm) + QRT(ym) + QRT(zm));
      evm[0][1] += (yp*zm - ym*zp);
      evm[0][2] += (xm*zp - xp*zm);
      evm[0][3] += (xp*ym - xm*yp);
    evm[1][0] = evm[0][1];
      evm[1][1] += (QRT(yp) + QRT(zp) + QRT(xm));
      evm[1][2] += (xm*ym - xp*yp);
      evm[1][3] += (xm*zm - xp*zp);
    evm[2][0] = evm[0][2];
      evm[2][1] = evm[1][2];
      evm[2][2] += (QRT(xp) + QRT(zp) + QRT(ym));
      evm[2][3] += (ym*zm - yp*zp);
    evm[3][0] = evm[0][3];
      evm[3][1] = evm[1][3];
      evm[3][2] = evm[2][3];
      evm[3][3] += (QRT(xp) + QRT(yp) + QRT(zm));
  }

  ev.E();
  ematd::EigenValues(evm, ev);
  double minVal = 1000;
  int minInd = -1;
  for(int i=0; i < 4; i++ )  {
    if( evm[i][i] < minVal )  {
      minInd = i;
      minVal = evm[i][i];
    }
  }
  if( minInd < 0 )  return -1;
  const evecd& qt = ev[minInd];
  res.r[0][0] = QRT(qt[0]) + QRT(qt[1]) - QRT(qt[2]) - QRT(qt[3]);
  res.r[0][1] = 2*(qt[1]*qt[2] + qt[0]*qt[3]);
  res.r[0][2] = 2*(qt[1]*qt[3] - qt[0]*qt[2]);

  res.r[1][0] = 2*(qt[1]*qt[2] - qt[0]*qt[3]);
  res.r[1][1] = QRT(qt[0]) + QRT(qt[2]) - QRT(qt[1]) - QRT(qt[3]);
  res.r[1][2] = 2*(qt[2]*qt[3] + qt[0]*qt[1]);

  res.r[2][0] = 2*(qt[1]*qt[3] + qt[0]*qt[2]);
  res.r[2][1] = 2*(qt[2]*qt[3] - qt[0]*qt[1]);
  res.r[2][2] = QRT(qt[0]) + QRT(qt[3]) - QRT(qt[1]) - QRT(qt[2]);
  res.t = centB;
  return (minVal <= 0) ? 0 : sqrt(minVal/atoms.Count());
}
void TNetwork_CalcAMDiff(const TTypeList< AnAssociation2<TSAtom*,TSAtom*> >& atoms,
                         smatd& res, mat3d& df, bool TryInversion)  {
  static const double delta = 1e-10;
  vec3d v(res.t), d;
  for( int i=0; i < 3; i++ )  {
    v[i] += delta;
    res.t = v;
    double df1 = TNetwork_FindAlignmentMatrix(atoms, res, TryInversion);
    v[i] -= 2*delta;
    res.t = v;
    double df2 = TNetwork_FindAlignmentMatrix(atoms, res, TryInversion);
    v[i] += delta;
    d[i] = (df1 - df2)/(2*delta);
  }
  for( int i=0; i < 3; i++ )  {
    for( int j=i; j < 3; j++ )  {
      df[i][j] = d[i]*d[j];
      df[j][i] = df[i][j];
    }
  }
  res.t = v;  // restore
}
double TNetwork_CalcAM(const TTypeList< AnAssociation2<TSAtom*,TSAtom*> >& atoms,
                         smatd& res, bool TryInversion) {
  mat3d df;
  vec3d v(0.01, 0.01, 0.01), sn(res.t), so(-100, -100, -100);
  while( sn.DistanceTo(so) > 1e-10 )  {
    so = sn;
    res.t = sn;
    TNetwork_CalcAMDiff(atoms, res, df, TryInversion);
    sn = so - df*v;
  }
  return TNetwork_FindAlignmentMatrix(atoms, res, TryInversion);
}
/* gradient descent shows that the procedure does converge and needs no refinement */
double TNetwork::FindAlignmentMatrix(const TTypeList< AnAssociation2<TSAtom*,TSAtom*> >& atoms,
                         smatd& res, bool TryInversion)  {
  vec3d centB;
  TAsymmUnit& au = atoms[0].GetA()->GetNetwork().GetLattice().GetAsymmUnit();
  for( int i=0; i < atoms.Count(); i++ )
    centB += atoms[i].GetB()->crd();
  centB /= atoms.Count();
  res.t = centB;
//  double rms = TNetwork_CalcAM(atoms, res, TryInversion);
  return TNetwork_FindAlignmentMatrix(atoms, res, TryInversion);
}
//..............................................................................
void TNetwork::DoAlignAtoms(const TTypeList< AnAssociation2<TSAtom*,TSAtom*> >& satomp,
                            const TSAtomPList& atomsToTransform, const smatd& S, bool Inverted)  {
  vec3d acent, mcent, v;
  TAsymmUnit& au = satomp[0].GetA()->GetNetwork().GetLattice().GetAsymmUnit();
  if( Inverted )  {
    for( int i=0; i < atomsToTransform.Count(); i++ )  {
      //atomsToTransform[i]->ccrd() *= -1;
      v = atomsToTransform[i]->ccrd();
      v *= -1;
      au.CellToCartesian(v);
      atomsToTransform[i]->crd() = v;
    }
  }

  for(int i=0; i < satomp.Count(); i++ )  {
    mcent += satomp[i].GetA()->crd();
  }
  mcent /= satomp.Count();
  for( int i=0; i < atomsToTransform.Count(); i++ )  {
    acent = atomsToTransform[i]->crd();
    acent -= mcent;
    acent *= S.r;
    acent += S.t;
    atomsToTransform[i]->crd() = acent;
  }
}
//..............................................................................
//..............................................................................
//..............................................................................
bool TNetwork::RingInfo::IsSingleCSubstituted() const  {
  for( int i=0; i < Substituents.Count(); i++ )  {
    if( Substituents[i].Count() != 1 )  return false;
    TSAtom& sa = *Substituents[i][0];
    if( sa.GetAtomInfo() != iCarbonIndex )  return false;
    int nhc = 0;
    for( int j=0; j < sa.NodeCount(); j++ )  {
      TSAtom& ra = sa.Node(j);
      if( ra.GetAtomInfo().GetMr() < 3 || ra.GetAtomInfo() == iQPeakIndex )  continue;
      nhc++;
    }
    if( nhc > 1 )  return false;  // only one to ring bond 
  }
  return true;
}
