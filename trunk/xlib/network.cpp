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
  if( Atoms[index]->CAtom().IsSiteShared() )  return;
  if( (Atoms[index]->GetTag() & 0x0002) != 0 )  return;
  for( int i=index+1; i < Atoms.Count(); i++ )  {
    if( fabs(Distances[0][index] - Distances[0][i]) > 0.01 )  return;
    if( Atoms[index]->crd().QDistanceTo( Atoms[i]->crd() ) < 0.0001 )  {
      Atoms[index]->AddMatrices(Atoms[i]);
      Atoms[i]->SetTag(2);            // specify that the node has to be deleted
    }
  }
}
void TNetwork::TDisassembleTaskCheckConnectivity::Run(long index)  {
  for( int i=index+1; i < Atoms.Count(); i++ )  {
    if( (Distances[0][i] - Distances[0][index]) > dcMaxCBLength ||
        (Distances[0][i] - Distances[0][index]) < -dcMaxCBLength )  return;
    if( (Distances[1][i] - Distances[1][index]) > dcMaxCBLength ||
        (Distances[1][i] - Distances[1][index]) < -dcMaxCBLength )  continue;
    if( (Distances[2][i] - Distances[2][index]) > dcMaxCBLength ||
        (Distances[2][i] - Distances[2][index]) < -dcMaxCBLength )  continue;
    if( (Distances[3][i] - Distances[3][index]) > dcMaxCBLength ||
        (Distances[3][i] - Distances[3][index]) < -dcMaxCBLength )  continue;

    double D = Atoms[index]->crd().QDistanceTo( Atoms[i]->crd());
    double D1 = (double)(Atoms[index]->GetAtomInfo().GetRad1() + Atoms[i]->GetAtomInfo().GetRad1() + Delta);
    D1 *= D1;
    if(  D < D1 )  {
      if( (Atoms[index]->CAtom().GetPart() == 0) ||
          (Atoms[i]->CAtom().GetPart() == 0) )  {
        Atoms[index]->AddNode(*Atoms[i]);
        Atoms[i]->AddNode(*Atoms[index]);  // crosslinking
        continue;
      }
      if( (Atoms[index]->CAtom().GetPart() == Atoms[i]->CAtom().GetPart()) &&
           (Atoms[index]->CAtom().GetPart() != 0)  )  {
        Atoms[index]->AddNode(*Atoms[i]);
        Atoms[i]->AddNode(*Atoms[index]);  // crosslinking
        continue;
      }
    }
  }
}
//..............................................................................
void TNetwork::Disassemble(TSAtomPList& Atoms, TNetPList& Frags, TSBondPList* InterBonds)  {
  if( Atoms.Count() < 2 )  return;
  TNetwork *Net;
  //TSAtom *A;
  double** Distances = new double* [4];
  double  Delta = GetLattice().GetDelta();
  Atoms.QuickSorter.SortSF(Atoms, AtomsSortByDistance);
  Distances[0] = new double[ Atoms.Count() ];  // distsnaces from {0,0,0} to an atom
  for( int i = 0; i < Atoms.Count(); i++ )  {
    Distances[0][i] = Atoms[i]->crd().Length();
    Atoms[i]->SetTag(1);
    Atoms[i]->SetNetId(-1);
  }
  // find & remove symmetrical equivalenrs from AllAtoms
  TDisassembleTaskRemoveSymmEq searchEqTask( Atoms, Distances);
  // profiling has shown it gives no benifit and makes the process slow
  //TListIteratorManager<TDisassembleTaskRemoveSymmEq> searchEq(searchEqTask, Atoms.Count(), tQuadraticTask, 100);
  for( int i=0; i < Atoms.Count(); i++ )
    searchEqTask.Run(i);
  // removing symmetrical equivalents from the Atoms list (passes as param)
  //............................................
  for( int i = 0; i < Atoms.Count(); i++ )  {
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

  Distances[1] = new double[ Atoms.Count() ];
  Distances[2] = new double[ Atoms.Count() ];
  Distances[3] = new double[ Atoms.Count() ];
  for( int i = 0; i < Atoms.Count() ; i++ )  {  // recalculate distances and remove some function calls
    TSAtom* A = Atoms[i];
    Distances[0][i] = A->crd().Length();
    Distances[1][i] = A->crd()[0];
    Distances[2][i] = A->crd()[1];
    Distances[3][i] = A->crd()[2];
  }
  int64_t st = TETime::msNow();
  TBasicApp::GetLog().Info( olxstr("Starting search of connectivity") );
  TDisassembleTaskCheckConnectivity searchConTask( Atoms, Distances, Delta);
  TListIteratorManager<TDisassembleTaskCheckConnectivity> searchCon(searchConTask, Atoms.Count(), tQuadraticTask, 100);
  TBasicApp::GetLog().Info( olxstr("Completed in ") << (TETime::msNow()-st) << "ms" );
  // creating bonds
  TBasicApp::GetLog().Info( olxstr("Starting bond creation") );
  st = TETime::msNow();
  for( int i=0; i < Atoms.Count(); i++ )  {
    TSAtom* A1 = Atoms[i];
    if( A1->GetTag() )  {
      Net = new TNetwork(&GetLattice(), this);
      Net->AddNode(*A1);
      Frags.Add( Net );
      A1->SetNetwork(*Net);
      for( int j=0; j < Net->NodeCount(); j++ )  {
        TSAtom& A2 = Net->Node(j);
        A2.SetTag(0);
        for( int k=0; k < A2.NodeCount(); k++ )  {
          TSAtom& A3 = A2.Node(k);
          if( A3.GetTag() )  {
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
  TBasicApp::GetLog().Info( olxstr("Completed in ") << (TETime::msNow()-st) << "ms" );
  st = TETime::msNow();
  TBasicApp::GetLog().Info( "Start search of hydrogen bonds" );
  // preallocate 50 Hbonds per fragment
  if( InterBonds )  InterBonds->SetCapacity( InterBonds->Count() + Frags.Count()*50); 
  THBondSearchTask searchHBTask( Atoms, InterBonds, Distances, GetLattice().GetDeltaI());
  TListIteratorManager<THBondSearchTask> searchHB(searchHBTask, Atoms.Count(), tQuadraticTask, 100);
  TBasicApp::GetLog().Info( olxstr("Completed in ") << (TETime::msNow()-st) << "ms" );
  delete [] Distances[0];
  delete [] Distances[1];
  delete [] Distances[2];
  delete [] Distances[3];
  delete [] Distances;
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

  for( int i=ind+1; i < Atoms.Count(); i++ )  {
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

      double D = A1->crd().DistanceTo( Atoms[i]->crd() );
      if(  D < (A1->GetAtomInfo().GetRad1() + Atoms[i]->GetAtomInfo().GetRad1() + Delta) )  {
         if( (A1->CAtom().GetPart() == Atoms[i]->CAtom().GetPart()) ||
              A1->CAtom().GetPart() == 0 || Atoms[i]->CAtom().GetPart() == 0 )  {
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
      double D = A1->crd().DistanceTo( Atoms[i]->crd() );
      if(  D < (A1->GetAtomInfo().GetRad1() + Atoms[i]->GetAtomInfo().GetRad1() + Delta) )  {
        if( (A1->CAtom().GetPart() == Atoms[i]->CAtom().GetPart()) ||
             A1->CAtom().GetPart() == 0 || Atoms[i]->CAtom().GetPart() == 0 )  {
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
    if( (CA1.GetPart() == CA2.GetPart()) || !CA1.GetPart() ||!CA2.GetPart() )
      return true;
  }
  return false;
}
//..............................................................................
bool TNetwork::HBondExists(const TCAtom& CA1, const TCAtom& CA2, double D) const  {
  if(  D < (CA1.GetAtomInfo().GetRad1() + CA2.GetAtomInfo().GetRad1() + GetLattice().GetDeltaI() ) )  {
    if( (CA1.GetPart() == CA2.GetPart()) || !CA1.GetPart() ||!CA2.GetPart() )
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
TNetwork::RingInfo TNetwork::AnalyseRing( const TSAtomPList& ring )  {
  RingInfo ri;
  int pivot = -1, pivot_count = 0;
  double maxmw = 0;
  for( int i=0; i < ring.Count(); i++ )  {
    if( !ri.HasAfix && ring[i]->CAtom().GetAfix() != 0 )
      ri.HasAfix = true;
   
    int nhc = 0;
    double local_maxmw = 0;
    for( int j=0; j < ring[i]->NodeCount(); j++ )  {
      TSAtom& ra = ring[i]->Node(j);
      if( ra.IsDeleted() || ra.GetAtomInfo() == iQPeakIndex )  continue;
      double mw = ra.GetAtomInfo().GetMr();
      if( mw < 3 )  continue; // H, D
      if( mw > local_maxmw )  local_maxmw = mw;
      nhc++;
    }
    if( nhc > 2 )  {
      if( local_maxmw > maxmw )  {
        ri.HeaviestSubsIndex = i;
        ri.HeaviestSubsType = &ring[i]->GetAtomInfo();
        maxmw = local_maxmw;
      }
      ri.SubsNumber++;
      if( nhc-2 > ri.MaxSubsANode )
        ri.MaxSubsANode = nhc-2;
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
double TNetwork::FindAlignmentMatrix(const TTypeList< AnAssociation2<TSAtom*,TSAtom*> >& atoms,
                         smatd& res, bool TryInversion)  {
  ematd evm(4,4), ev(4,4);
  vec3d centA, centB, v;
  TAsymmUnit& au = atoms[0].GetA()->GetNetwork().GetLattice().GetAsymmUnit();
  for( int i=0; i < atoms.Count(); i++ )  {
    centB += atoms[i].GetB()->crd();
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
  centB /= atoms.Count();

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
  res.t[0] = (centB[0]);// - centA[0]);

  res.r[1][0] = 2*(qt[1]*qt[2] - qt[0]*qt[3]);
  res.r[1][1] = QRT(qt[0]) + QRT(qt[2]) - QRT(qt[1]) - QRT(qt[3]);
  res.r[1][2] = 2*(qt[2]*qt[3] + qt[0]*qt[1]);
  res.t[1] = (centB[1]);// + centA[1]);

  res.r[2][0] = 2*(qt[1]*qt[3] + qt[0]*qt[2]);
  res.r[2][1] = 2*(qt[2]*qt[3] - qt[0]*qt[1]);
  res.r[2][2] = QRT(qt[0]) + QRT(qt[3]) - QRT(qt[1]) - QRT(qt[2]);
  res.t[2] = (centB[2]);// - centA[2]);
  double rs = 0;
  for( int i=0; i < atoms.Count(); i++ )  {
    if( TryInversion )  {
      v = atoms[i].GetA()->ccrd();
      v *= -1;
      au.CellToCartesian(v);
    }
    else
      v = atoms[i].GetA()->crd();
    v -= centA;
    v *= res.r;
    v += centB;
    rs += v.DistanceTo( atoms[i].GetB()->crd() );
  }
  rs /= atoms.Count();
  return rs;
//  if( minVal <= 0 )  return 0;
//  return sqrt(minVal/atoms.Count());
}
//..............................................................................
double _dlda00(const vec3d& at, const vec3d& to, const smatd& m, double l[6])  {
  return m.r[0][0]*(m.r[0].DotProd(at)-m.t[0]-to[0]) + 2*m.r[0][0]*l[0] + l[3]*m.r[1][0] + l[4]*m.r[2][0];
}
double _dlda01(const vec3d& at, const vec3d& to, const smatd& m, double l[6])  {
  return m.r[0][1]*(m.r[0].DotProd(at)-m.t[1]-to[1]) + 2*m.r[0][1]*l[0] + l[3]*m.r[1][1] + l[4]*m.r[2][1];
}
double _dlda02(const vec3d& at, const vec3d& to, const smatd& m, double l[6])  {
  return m.r[0][2]*(m.r[0].DotProd(at)-m.t[2]-to[2]) + 2*m.r[0][2]*l[0] + l[3]*m.r[1][2] + l[4]*m.r[2][2];
}
double _dlda10(const vec3d& at, const vec3d& to, const smatd& m, double l[6])  {
  return m.r[1][0]*(m.r[1].DotProd(at)-m.t[0]-to[0]) + 2*m.r[1][0]*l[1] + l[3]*m.r[0][0] + l[6]*m.r[2][0];
}
double _dlda11(const vec3d& at, const vec3d& to, const smatd& m, double l[6])  {
  return m.r[1][1]*(m.r[1].DotProd(at)-m.t[1]-to[1]) + 2*m.r[1][1]*l[1] + l[3]*m.r[0][1] + l[6]*m.r[2][1];
}
double _dlda12(const vec3d& at, const vec3d& to, const smatd& m, double l[6])  {
  return m.r[1][2]*(m.r[1].DotProd(at)-m.t[2]-to[2]) + 2*m.r[1][2]*l[1] + l[3]*m.r[0][2] + l[6]*m.r[2][2];
}
double _dlda20(const vec3d& at, const vec3d& to, const smatd& m, double l[6])  {
  return m.r[2][0]*(m.r[2].DotProd(at)-m.t[0]-to[0]) + 2*m.r[2][0]*l[2] + l[4]*m.r[0][0] + l[6]*m.r[1][0];
}
double _dlda21(const vec3d& at, const vec3d& to, const smatd& m, double l[6])  {
  return m.r[2][1]*(m.r[2].DotProd(at)-m.t[1]-to[1]) + 2*m.r[2][1]*l[2] + l[4]*m.r[0][1] + l[6]*m.r[1][1];
}
double _dlda22(const vec3d& at, const vec3d& to, const smatd& m, double l[6])  {
  return m.r[2][2]*(m.r[2].DotProd(at)-m.t[2]-to[2]) + 2*m.r[2][2]*l[2] + l[4]*m.r[0][2] + l[6]*m.r[1][2];
}
double _dldl0(const vec3d& at, const vec3d& to, const smatd& m, double l[6])  {
  return m.r[0].DotProd(m.r[0])-1;
}
double _dldl1(const vec3d& at, const vec3d& to, const smatd& m, double l[6])  {
  return m.r[1].DotProd(m.r[1])-1;
}
double _dldl2(const vec3d& at, const vec3d& to, const smatd& m, double l[6])  {
  return m.r[2].DotProd(m.r[2])-1;
}
double _dldl3(const vec3d& at, const vec3d& to, const smatd& m, double l[6])  {
  return m.r[0].DotProd(m.r[1]);
}
double _dldl4(const vec3d& at, const vec3d& to, const smatd& m, double l[6])  {
  return m.r[0].DotProd(m.r[2]);
}
double _dldl5(const vec3d& at, const vec3d& to, const smatd& m, double l[6])  {
  return m.r[1].DotProd(m.r[2]);
}
double _dldt0(const vec3d& at, const vec3d& to, const smatd& m, double l[6])  {
  return -2*m.t[0]*(m.r[0].DotProd(at)-m.t[0]-to[0]);
}
double _dldt1(const vec3d& at, const vec3d& to, const smatd& m, double l[6])  {
  return -2*m.t[1]*(m.r[1].DotProd(at)-m.t[1]-to[1]);
}
double _dldt2(const vec3d& at, const vec3d& to, const smatd& m, double l[6])  {
  return -2*m.t[2]*(m.r[2].DotProd(at)-m.t[2]-to[2]);
}
double FindAlignmentMatrixX(const TTypeList< AnAssociation2<TSAtom*,TSAtom*> >& atoms,
                         smatd& res, bool TryInversion)  {
  ematd J(18,atoms.Count());
  smatd matr;
  double lambdas[6] = {0, 0, 0, 0, 0, 0};
  TNetwork::FindAlignmentMatrix(atoms, matr, true); 
  for( int i=0; i < atoms.Count(); i++ )  {
    J[0][i] = _dlda00(atoms[i].GetA()->crd(), atoms[i].GetA()->crd(), matr, lambdas);
    J[1][i] = _dlda01(atoms[i].GetA()->crd(), atoms[i].GetA()->crd(), matr, lambdas);
    J[2][i] = _dlda02(atoms[i].GetA()->crd(), atoms[i].GetA()->crd(), matr, lambdas);
    J[3][i] = _dlda10(atoms[i].GetA()->crd(), atoms[i].GetA()->crd(), matr, lambdas);
    J[4][i] = _dlda11(atoms[i].GetA()->crd(), atoms[i].GetA()->crd(), matr, lambdas);
    J[5][i] = _dlda12(atoms[i].GetA()->crd(), atoms[i].GetA()->crd(), matr, lambdas);
    J[6][i] = _dlda20(atoms[i].GetA()->crd(), atoms[i].GetA()->crd(), matr, lambdas);
    J[7][i] = _dlda21(atoms[i].GetA()->crd(), atoms[i].GetA()->crd(), matr, lambdas);
    J[8][i] = _dlda22(atoms[i].GetA()->crd(), atoms[i].GetA()->crd(), matr, lambdas);
    J[9][i] = _dldl0(atoms[i].GetA()->crd(), atoms[i].GetA()->crd(), matr, lambdas);
    J[10][i] = _dldl0(atoms[i].GetA()->crd(), atoms[i].GetA()->crd(), matr, lambdas);
    J[11][i] = _dldl1(atoms[i].GetA()->crd(), atoms[i].GetA()->crd(), matr, lambdas);
    J[12][i] = _dldl2(atoms[i].GetA()->crd(), atoms[i].GetA()->crd(), matr, lambdas);
    J[13][i] = _dldl3(atoms[i].GetA()->crd(), atoms[i].GetA()->crd(), matr, lambdas);
    J[14][i] = _dldl4(atoms[i].GetA()->crd(), atoms[i].GetA()->crd(), matr, lambdas);
    J[15][i] = _dldl5(atoms[i].GetA()->crd(), atoms[i].GetA()->crd(), matr, lambdas);
    J[16][i] = _dldt0(atoms[i].GetA()->crd(), atoms[i].GetA()->crd(), matr, lambdas);
    J[17][i] = _dldt1(atoms[i].GetA()->crd(), atoms[i].GetA()->crd(), matr, lambdas);
    J[18][i] = _dldt2(atoms[i].GetA()->crd(), atoms[i].GetA()->crd(), matr, lambdas);
  }
  return 0;
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
