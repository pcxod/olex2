//---------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//---------------------------------------------------------------------------//
#include "network.h"
#include "satom.h"
#include "sbond.h"

#include "actions.h"

#include "lattice.h"
#include "unitcell.h"
#include "asymmunit.h"

#include "bapp.h"
#include "log.h"

#include "egraph.h"

#include "olxmps.h"
#include "estopwatch.h"
#include "edict.h"
#include "emath.h"
#include "refmodel.h"

#undef GetObject

double TNetwork_FindAlignmentMatrix(const TTypeList< AnAssociation2<TSAtom*,TSAtom*> >& atoms, smatdd& res,
                                    vec3d& _centB, bool TryInversion,
                                    double (*weight_calculator)(const TSAtom&))
{
  TTypeList<AnAssociation2<vec3d,vec3d> > crds;
  crds.SetCapacity( atoms.Count() );
  const TAsymmUnit& au1 = atoms[0].GetA()->GetNetwork().GetLattice().GetAsymmUnit();
  const TAsymmUnit& au2 = atoms[0].GetB()->GetNetwork().GetLattice().GetAsymmUnit();
  for( size_t i=0; i < atoms.Count(); i++ )  {
    vec3d v = atoms[i].GetB()->ccrd();
    if( TryInversion )
      v *= -1;
    crds.AddNew(atoms[i].GetA()->crd(), au2.CellToCartesian(v) );
  }
  double sumA = 0, sumB = 0;
  vec3d centA, centB;
  for( size_t i=0; i < atoms.Count(); i++ )  {
    centA += crds[i].GetA()*weight_calculator(*atoms[i].GetA());
    sumA += weight_calculator(*atoms[i].GetA());
    centB += crds[i].GetB()*weight_calculator(*atoms[i].GetB());
    sumB += weight_calculator(*atoms[i].GetB());
  }
  res.t = centA/sumA;
  _centB = centB/sumB;
  return TNetwork::FindAlignmentMatrix(crds, res.t, _centB, res);
}

//---------------------------------------------------------------------------
// TNetwork function bodies
//---------------------------------------------------------------------------
TNetwork::TNetwork(TLattice* P, TNetwork *N) : TBasicNode<TNetwork, TSAtom, TSBond>(N)  {
  Lattice = P;
}
//..............................................................................
TNetwork::~TNetwork()  {
  return;
}
//..............................................................................
// sorts atoms according to the distcance from {0,0,0}
int AtomsSortByDistance(const TSAtom* A, const TSAtom* A1)  {
  const double d = A->crd().QLength() - A1->crd().QLength();
  if( d < 0 )  return -1;
  if( d > 0 )  return 1;
  return 0;
}
//..............................................................................
void TNetwork::SortNodes()  {
  Nodes.QuickSorter.SortSF(Nodes, AtomsSortByDistance);
}
//..............................................................................
void TNetwork::TDisassembleTaskRemoveSymmEq::Run(size_t index)  {
  if( (Atoms[index]->GetTag() & 0x0002) != 0 )  return;
  const size_t ac = Atoms.Count();
  for( size_t i=index+1; i < ac; i++ )  {
    if( olx_abs(Distances[0][index] - Distances[0][i]) > 0.01 )  return;
    if( Atoms[index]->crd().QDistanceTo(Atoms[i]->crd()) < 0.0001 )  {
     // treat EXYZ atoms - only combine if both atoms refer to the same one in the AU
      if( Atoms[index]->CAtom().GetExyzGroup() != NULL && 
        Atoms[index]->CAtom().GetId() != Atoms[i]->CAtom().GetId() )  
      {
        continue;
      }
      if( Atoms[i]->CAtom().GetPart() != Atoms[index]->CAtom().GetPart() )
        continue;
      if( Atoms[i]->CAtom().GetParentAfixGroup() != NULL )  continue;
      Atoms[index]->AddMatrices(*Atoms[i]);
      Atoms[i]->SetTag(2);            // specify that the node has to be deleted
    }
  }
}
void TNetwork::TDisassembleTaskCheckConnectivity::Run(size_t index)  {
  const size_t ac = Atoms.Count();
  for( size_t i=index+1; i < ac; i++ )  {
    if( olx_abs(Distances[0][i] - Distances[0][index]) > dcMaxCBLength )  return;
    if( olx_abs(Distances[1][i] - Distances[1][index]) > dcMaxCBLength )  continue;
    if( olx_abs(Distances[2][i] - Distances[2][index]) > dcMaxCBLength )  continue;
    if( olx_abs(Distances[3][i] - Distances[3][index]) > dcMaxCBLength  )  continue;

    const double D = Atoms[index]->crd().QDistanceTo(Atoms[i]->crd());
    const double D1 = olx_sqr(Atoms[index]->CAtom().GetConnInfo().r + Atoms[i]->CAtom().GetConnInfo().r + Delta);
    if(  D < D1 && IsBondAllowed(*Atoms[index], *Atoms[i]) )  {
      if( D < 1e-5 )  // EXYZ?
        continue;
      Atoms[index]->AddNode(*Atoms[i]);
      // multithreading, Atoms[index] is unique, but not this one
      volatile olx_scope_cs _cs(TBasicApp::GetCriticalSection());
      Atoms[i]->AddNode(*Atoms[index]);  // crosslinking
    }
  }
}
//..............................................................................
void TNetwork::Disassemble(TSAtomPList& Atoms, TNetPList& Frags, TSBondPList& InterBonds)  {
  if( Atoms.Count() < 2 )  {
    if( Atoms.Count() == 1 )  {
      if( !Atoms[0]->IsDeleted() )  {
        TNetwork* net = Frags.Add(new TNetwork(&GetLattice(), this));
        Atoms[0]->SetTag(0);
        Atoms[0]->SetNetwork(*net);
        net->AddNode(*Atoms[0]);
      }
      else
        Atoms[0]->SetTag(2);
    }
    return;
  }
  TStopWatch sw(__FUNC__);
  sw.start("Initialising");
  //TSAtom *A;
  double** Distances = new double* [4];
  double  Delta = GetLattice().GetDelta();
  Atoms.QuickSorter.SortSF(Atoms, AtomsSortByDistance);
  Distances[0] = new double[Atoms.Count()];  // distances from {0,0,0} to an atom
  size_t ac = Atoms.Count();
  for( size_t i = 0; i < ac; i++ )  {
    Distances[0][i] = Atoms[i]->crd().Length();
    Atoms[i]->SetTag(Atoms[i]->IsDeleted() ? 2 : 1);
    Atoms[i]->SetNetId(~0);
  }
  // find & remove symmetrical equivalenrs from AllAtoms
  sw.start("Removing symmetrical equivalents");
  TDisassembleTaskRemoveSymmEq searchEqTask(Atoms, Distances);
  // profiling has shown it gives no benifit and makes the process slow
  TListIteratorManager<TDisassembleTaskRemoveSymmEq> searchEq(searchEqTask,
    Atoms.Count(), tQuadraticTask, 10000);  // never does the threading
  ac = Atoms.Count();
  // removing symmetrical equivalents from the Atoms list (passes as param)
  //............................................
  for( size_t i = 0; i < ac; i++ )  {
    if( (Atoms[i]->GetTag() & 0x0002) != 0 )  {
      Atoms[i]->SetDeleted(true);
      Atoms[i] = NULL;
      continue;
    }
    Atoms[i]->Clear();
    // preallocate memory to improve mulithreading
    Atoms[i]->SetCapacity(Atoms[i]->NodeCount() + Atoms[i]->CAtom().AttachedSiteCount());
  }
  Atoms.Pack();
  if( Atoms.IsEmpty() )  return;
  //............................................
  ac = Atoms.Count();
  Distances[1] = new double[ac];
  Distances[2] = new double[ac];
  Distances[3] = new double[ac];
  for( size_t i = 0; i < ac; i++ )  {  // precalculate distances and remove some function calls
    TSAtom* A = Atoms[i];
    Distances[0][i] = A->crd().Length();
    Distances[1][i] = A->crd()[0];
    Distances[2][i] = A->crd()[1];
    Distances[3][i] = A->crd()[2];
  }
  // get extrac connectivity information
  sw.start("Connectivity analysis");
  TDisassembleTaskCheckConnectivity searchConTask(Atoms, Distances, Delta);
  TListIteratorManager<TDisassembleTaskCheckConnectivity> searchCon(searchConTask,
    Atoms.Count(), tQuadraticTask, 10000);
  sw.start("Creating bonds");
  CreateBondsAndFragments(Atoms, Frags, InterBonds);
  sw.start("Searching H-bonds");
  // preallocate 50 Hbonds per fragment
  InterBonds.SetCapacity(InterBonds.Count() + Frags.Count()*50); 
  THBondSearchTask searchHBTask(Atoms, &InterBonds, Distances, GetLattice().GetDeltaI());
  TListIteratorManager<THBondSearchTask> searchHB(searchHBTask,
    Atoms.Count(), tQuadraticTask, 10000);
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
void TNetwork::CreateBondsAndFragments(TSAtomPList& Atoms, TNetPList& Frags, TSBondPList& bond_sink)  {
  // creating bonds
  const size_t ac = Atoms.Count();
  for( size_t i=0; i < ac; i++ )  {
    TSAtom* A1 = Atoms[i];
    A1->SetLattId(i);
    A1->SetStandalone(A1->NodeCount() == 0);
    if( A1->GetTag() != 0 )  {
      TNetwork* Net = Frags.Add(new TNetwork(&GetLattice(), this));
      Net->AddNode(*A1);
      A1->SetNetwork(*Net);
      A1->SetTag(0);
      for( size_t j=0; j < Net->NodeCount(); j++ )  {
        TSAtom& A2 = Net->Node(j);
        const size_t a2_cnt = A2.NodeCount(); 
        for( size_t k=0; k < a2_cnt; k++ )  {
          TSAtom& A3 = A2.Node(k);
          if( A3.GetTag() != 0 )  {
            Net->AddNode(A3);
            A3.SetNetwork(*Net);
            TSBond* B = new TSBond(Net);
            B->SetType(sotBond);
            B->SetA(A2); 
            B->SetB(A3);
            A2.AddBond(*B);  
            A3.AddBond(*B);
            Net->AddBond(*B);
            A3.SetTag(0);
          }
          else if( A3.GetNetId() > j )  {  // the atom is in the list, but has not been processes
            TSBond* B = new TSBond(Net);  // in this case we need to create a bond
            B->SetType(sotBond);
            B->SetA(A2);  B->SetB(A3);
            A2.AddBond(*B);  A3.AddBond(*B);
            Net->AddBond(*B);
          }
        }
      }
    }
  }
}
//..............................................................................
void TNetwork::THBondSearchTask::Run(size_t ind)  {
  TSAtom *AA = NULL,
         *DA = NULL;
  TSAtom* A1 = Atoms[ind];
  const cm_Element& aT = A1->GetType();
  if( aT == iHydrogenZ )
    AA = A1;
  else if( aT == iNitrogenZ || aT == iOxygenZ || aT == iFluorineZ ||
      aT == iChlorineZ || aT == iSulphurZ || aT == iBromineZ || aT == iSeleniumZ )
    DA = A1;

  if( AA == NULL && DA == NULL )  return;

  const int this_p = A1->CAtom().GetPart();
  const size_t ac = Atoms.Count();
  for( size_t i=ind+1; i < ac; i++ )  {
    if( (Distances[0][ind] - Distances[0][i]) > dcMaxCBLength ||
        (Distances[0][ind] - Distances[0][i]) < -dcMaxCBLength )  return;
    if( (Distances[1][ind] - Distances[1][i]) > dcMaxCBLength ||
        (Distances[1][ind] - Distances[1][i]) < -dcMaxCBLength )  continue;
    if( (Distances[2][ind] - Distances[2][i]) > dcMaxCBLength ||
        (Distances[2][ind] - Distances[2][i]) < -dcMaxCBLength )  continue;
    if( (Distances[3][ind] - Distances[3][i]) > dcMaxCBLength ||
        (Distances[3][ind] - Distances[3][i]) < -dcMaxCBLength )  continue;

    const cm_Element& aT1 = Atoms[i]->GetType();
    if( !((AA != NULL && (aT1 == iNitrogenZ || aT1 == iOxygenZ||
                        aT1 == iFluorineZ || aT1 == iChlorineZ ||
                        aT1 == iSulphurZ) || aT1 == iBromineZ) ||
                        aT1 == iSeleniumZ ||
          (DA != NULL && aT1 == iHydrogenZ) ) )  continue;

    if( A1->GetNetwork() == Atoms[i]->GetNetwork() )  {
      if( A1->IsConnectedTo( *Atoms[i]) )  continue;
      // check for N-C-H (N-C) bonds are not valid
      TSAtom *CSA = ((AA!=NULL)?AA:Atoms[i]),
             *NA  = ((DA!=NULL)?DA:Atoms[i]);
      bool connected = false;
      for( size_t j=0; j < CSA->NodeCount(); j++ )  {
        if( CSA->Node(j).GetType() == iQPeakZ )  // 17/05/2007 - skip the Q peaks!
          continue;
        if( CSA->Node(j).IsConnectedTo(*NA) )  {
          connected = true;
          break;
        }
      }
      if( connected )  continue;

      const double D = A1->crd().QDistanceTo(Atoms[i]->crd());
      const double D1 = olx_sqr(A1->CAtom().GetConnInfo().r + Atoms[i]->CAtom().GetConnInfo().r + Delta);
      if(  D < D1 && IsBondAllowed(*A1, *Atoms[i]) )  {
        volatile olx_scope_cs _cs(TBasicApp::GetCriticalSection());
        TSBond* B = new TSBond(&A1->GetNetwork());
        B->SetType(sotHBond);
        B->SetA(*A1);
        B->SetB(*Atoms[i]);
        A1->AddBond(*B);
        Atoms[i]->AddBond(*B);
        A1->GetNetwork().AddBond(*B);
      }
    }
    else if( Bonds != NULL )  {
      const double D = A1->crd().QDistanceTo( Atoms[i]->crd() );
      const double D1 = olx_sqr(A1->CAtom().GetConnInfo().r + Atoms[i]->CAtom().GetConnInfo().r + Delta);
      if(  D < D1 && IsBondAllowed(*A1, *Atoms[i]) )  {
        volatile olx_scope_cs _cs(TBasicApp::GetCriticalSection());
        TSBond* B = new TSBond(&A1->GetNetwork());
        B->SetType(sotHBond);
        B->SetA(*A1);
        B->SetB(*Atoms[i]);
        A1->AddBond(*B);
        Atoms[i]->AddBond(*B);
        Bonds->Add(B);
      }
    }
  }
}
//..............................................................................
void TNetwork::Disassemble(const AtomRegistry& ar, TSAtomPList& atoms, TNetPList& Frags,
  TSBondPList& InterBonds)
{
  const TUnitCell& uc = Lattice->GetUnitCell();
  for( size_t i=0; i < atoms.Count(); i++ )  {
    atoms[i]->ClearBonds();
    atoms[i]->ClearNodes();
    atoms[i]->SetTag(1);
    for( size_t j=0; j < atoms[i]->CAtom().AttachedSiteCount(); j++ )  {
      TCAtom::Site& site = atoms[i]->CAtom().GetAttachedSite(j);
      const smatd m = atoms[i]->GetMatrix(0).IsFirst() ? site.matrix :
        uc.MulMatrix(site.matrix, atoms[i]->GetMatrix(0));
      TSAtom* a = ar.Find(TSAtom::Ref(site.atom->GetId(), m.GetId()));
      if( a != NULL && !a->IsDeleted() )
        atoms[i]->AddNode(*a);
    }
    const cm_Element& thisT = atoms[i]->GetType();
    if( thisT != iHydrogenZ )  continue;
    for( size_t j=0; j < atoms[i]->CAtom().AttachedSiteICount(); j++ )  {
      TCAtom::Site& site = atoms[i]->CAtom().GetAttachedSiteI(j);
      const cm_Element& thatT = site.atom->GetType();
      if( !(thatT == iNitrogenZ || thatT == iOxygenZ || thatT == iFluorineZ ||
        thatT == iChlorineZ || thatT == iSulphurZ || thatT == iBromineZ || thatT == iSeleniumZ) )
      {
        continue;
      }
      const smatd m = atoms[i]->GetMatrix(0).IsFirst() ? site.matrix :
        uc.MulMatrix(site.matrix, atoms[i]->GetMatrix(0));
      TSAtom* a = ar.Find(TSAtom::Ref(site.atom->GetId(), m.GetId()));
      if( a != NULL && !a->IsDeleted() )  {
        bool process = true;
        for( size_t k=0; k < a->NodeCount(); k++ )  {
          if( a->Node(k).GetType() != iQPeakZ && atoms[i]->IsConnectedTo(a->Node(k)) )  {
            process = false;
            break;
          }
        }
        if( !process )  continue;
        TSBond* B = new TSBond(&atoms[i]->GetNetwork());
        B->SetType(sotHBond);
        B->SetA(*atoms[i]);
        B->SetB(*a);
        a->AddBond(*B);
        atoms[i]->AddBond(*B);
        InterBonds.Add(B);
      }
    }

  }
  CreateBondsAndFragments(atoms, Frags, InterBonds);
}
//..............................................................................
bool TNetwork::CBondExists(const TCAtom& CA1, const TCAtom& CA2, const smatd& sm, const double& D) const  {
  if(  D < (CA1.GetConnInfo().r + CA2.GetConnInfo().r + GetLattice().GetDelta() ) )  {
    return IsBondAllowed(CA1, CA2, sm);
  }
  return false;
}
//..............................................................................
bool TNetwork::CBondExistsQ(const TCAtom& CA1, const TCAtom& CA2, const smatd& sm, const double& qD) const  {
  if(  qD < olx_sqr(CA1.GetConnInfo().r + CA2.GetConnInfo().r + GetLattice().GetDelta() ) )  {
    return IsBondAllowed(CA1, CA2, sm);
  }
  return false;
}
//..............................................................................
bool TNetwork::HBondExists(const TCAtom& CA1, const TCAtom& CA2, const smatd& sm, const double& D) const  {
  if(  D < (CA1.GetConnInfo().r + CA2.GetConnInfo().r + GetLattice().GetDeltaI() ) )  {
    return IsBondAllowed(CA1, CA2, sm);
  }
  return false;
}
//..............................................................................
bool TNetwork::HBondExistsQ(const TCAtom& CA1, const TCAtom& CA2, const smatd& sm, const double& qD) const  {
  if(  qD < olx_sqr(CA1.GetConnInfo().r + CA2.GetConnInfo().r + GetLattice().GetDeltaI() ) )  {
    return IsBondAllowed(CA1, CA2, sm);
  }
  return false;
}
//..............................................................................
bool TNetwork::CBondExists(const TSAtom& A1, const TSAtom& A2, const double& D) const  {
  if(  D < (A1.CAtom().GetConnInfo().r + A2.CAtom().GetConnInfo().r + GetLattice().GetDelta() ) )  {
    return IsBondAllowed(A1, A2);
  }
  return false;
}
//..............................................................................
bool TNetwork::CBondExistsQ(const TSAtom& A1, const TSAtom& A2, const double& qD) const  {
  if(  qD < olx_sqr(A1.CAtom().GetConnInfo().r + A2.CAtom().GetConnInfo().r + GetLattice().GetDelta() ) )  {
    return IsBondAllowed(A1, A2);
  }
  return false;
}
//..............................................................................
//..............................................................................
// HELPER function
class TNetTraverser  {
  olxstr Data;
public:
  TNetTraverser() {  Data.SetIncrement(1024);  }
  bool OnItem(const TEGraphNode<size_t, TSAtom*>& v) {
    olxstr tmp(v.GetData(), v.Count()*6);
    tmp << '{';
    for( size_t i=0; i < v.Count(); i++ )  {
      tmp << v.Item(i).GetData();
      if( (i+1) < v.Count() )  tmp << ',';
    }
    tmp << '}';
    Data << tmp;
    return true;
  }
  const olxstr& GetData() const {  return Data;  }
  void ClearData()  {  Data = EmptyString;  }
};

void ResultCollector( TEGraphNode<size_t,TSAtom*>& subRoot,
                        TEGraphNode<size_t,TSAtom*>& Root,
                        TTypeList< AnAssociation2<size_t, size_t> >& res )  {
  res.AddNew( subRoot.GetObject()->GetNetId(), Root.GetObject()->GetNetId() );
  for( size_t i=0; i < olx_min(subRoot.Count(),Root.Count()); i++ )
    ResultCollector( subRoot.Item(i), Root.Item(i), res );
}
void ResultCollector( TEGraphNode<size_t,TSAtom*>& subRoot,
                        TEGraphNode<size_t,TSAtom*>& Root,
                        TTypeList< AnAssociation2<TSAtom*, TSAtom*> >& res )  {
  if( !subRoot.IsShallowEqual(Root) )
    return;
  res.AddNew( subRoot.GetObject(), Root.GetObject());
  for( size_t i=0; i < subRoot.Count(); i++ )
    ResultCollector( subRoot[i], Root[i], res );
}
/*
void ExpandGraphNode( TTypeList< TEGraphNode<int,TSAtom*>* >& allNodes, TEGraphNode<int,TSAtom*>& graphNode, TSAtom* node)  {
  if( node->GetTag() == 1 )  return;
  node->Tag( 1 );
  for( size_t i=0; i < node->NodeCount(); i++ )  {
    TSAtom* sa = (TSAtom*)node->Node(i);
    if( sa->GetTag() != 0 )  continue;
    allNodes.AddACopy( &graphNode.NewNode( sa->GetAtomInfo()->GetIndex(), sa ) );
  }
}
void BuildGraph( TEGraphNode<int,TSAtom*>& graphNode, TSAtom* node)  {
  TTypeList< TEGraphNode<int,TSAtom*>* > allNodes;

  ExpandGraphNode(allNodes, graphNode, node);

  for( size_t i=0; i < allNodes.Count(); i++ )
    ExpandGraphNode( allNodes, *allNodes[i], allNodes[i]->GetObject() );
}
*/

void BreadthFirstTag(TSAtomPList& all, TSAtom* node)  {
  for( size_t i=0; i < node->NodeCount(); i++ )  {
    TSAtom& sa = node->Node(i);
    if( sa.GetTag() != 0 )  continue;
    all.Add(sa)->SetTag(node->GetTag() + 1);
  }
}
void BreadthFirstTags(TSAtom* sa)  {
  TSAtomPList all;
  all.SetCapacity(sa->GetNetwork().NodeCount());
  all.Add(sa);
  sa->SetTag(1);
  BreadthFirstTag(all, sa);
  for( size_t i=0; i < all.Count(); i++ )  {
    BreadthFirstTag(all, all[i]);
    //all[i]->CAtom()->Label() = all[i]->GetTag();
  }
}

void ExpandGraphNode(TEGraphNode<size_t,TSAtom*>& graphNode)  {
  for( size_t i=0; i < graphNode.GetObject()->NodeCount(); i++ )  {
    TSAtom& sa = graphNode.GetObject()->Node(i);
    if( sa.GetTag() <= graphNode.GetObject()->GetTag() )  continue;
    ExpandGraphNode(graphNode.NewNode(sa.GetType().z, &sa) );
  }
}

void BuildGraph( TEGraphNode<size_t,TSAtom*>& graphNode, TSAtom* node)  {
  BreadthFirstTags(node);
  ExpandGraphNode(graphNode);
}

struct GraphAnalyser  {
  TEGraphNode<size_t,TSAtom*> &RootA, &RootB; 
  int CallsCount;
  bool Invert, CalcRMSForH;
  vec3d bCent;
  smatdd alignmentMatrix, bestMatrix;
  double minRms;
  size_t atomsToMatch;
  GraphAnalyser(TEGraphNode<size_t,TSAtom*>& rootA, TEGraphNode<size_t,TSAtom*>& rootB) :
    RootA(rootA), RootB(rootB), CallsCount(0), Invert(false) {
      minRms = -1;
      CalcRMSForH = true;
    }

  double CalcRMS()  {
    TTypeList< AnAssociation2<TSAtom*,TSAtom*> > matchedAtoms;
    matchedAtoms.SetCapacity(1024);
    ResultCollector(RootA, RootB, matchedAtoms);
    for( size_t i=0; i < matchedAtoms.Count(); i++ )
      matchedAtoms[i].A()->SetTag(i);
    if( CalcRMSForH )  {
      for( size_t i=0; i < matchedAtoms.Count(); i++ )
        if( (size_t)matchedAtoms[i].A()->GetTag() != i )
          matchedAtoms.NullItem(i);
    }
    else  {
      for( size_t i=0; i < matchedAtoms.Count(); i++ )
        if( matchedAtoms[i].A()->GetTag() != i || matchedAtoms[i].A()->GetType() == iHydrogenZ )
          matchedAtoms.NullItem(i);
    }
    matchedAtoms.Pack();
    CallsCount++;
//    if( matchedAtoms.Count() != atomsToMatch )
//      return -1;
    vec3d centB;
    const double rms = TNetwork_FindAlignmentMatrix(matchedAtoms, alignmentMatrix, centB,
      Invert, &TSAtom::weight_occu);
    if( minRms < 0 || rms < minRms )  {
      minRms = rms;
      bestMatrix = alignmentMatrix;
      bCent = centB;
    }
    return rms;
  }
  double CalcRMS(const TEGraphNode<size_t,TSAtom*>& src, const TEGraphNode<size_t,TSAtom*>& dest)  {
    if( CalcRMSForH )
      return CalcRMS();
    size_t h_cnt = 0;
    for( size_t i=0; i < src.Count(); i++ )
      if( src[i].GetData() == iHydrogenZ )
        h_cnt++;
    if( h_cnt < 2 )
      return CalcRMS();
    if( alignmentMatrix.r.Trace() == 0 )
      CalcRMS();
    //alignmentMatrix.t = aCent;
    CallsCount++;
    const TAsymmUnit& au_a = *src[0].GetObject()->CAtom().GetParent();
    const TAsymmUnit& au_b = *dest[0].GetObject()->CAtom().GetParent();
    double rsum = 0;
    for( size_t i=0; i < src.Count(); i++ )  {
      vec3d v = dest[i].GetObject()->ccrd();
      if( Invert )  v *= -1;
      v = bestMatrix*(au_b.CellToCartesian(v) - bCent);
      vec3d v1 = src[i].GetObject()->ccrd();
      rsum += v.QDistanceTo(au_a.CellToCartesian(v1));
    }
    return rsum + minRms;
  }
  void OnFinish()  {
    CalcRMS();
    HValidator(RootA, RootB);
  }
  // since the H-atoms are given a smaller weights...
  void HValidator(const TEGraphNode<size_t,TSAtom*>& n1, TEGraphNode<size_t,TSAtom*>& n2)  {
    if( !n1.IsShallowEqual(n2) )  return;
    TSizeList hpos;
    for( size_t i=0; i < n1.Count(); i++ )  {
      if( n1[i].GetData() == iHydrogenZ )
        hpos.Add(i);
      HValidator(n1[i], n2[i]);
    }
    if( hpos.Count() < 2 )  return;
    const TAsymmUnit& au = *n2[0].GetObject()->CAtom().GetParent();
    TSizeList permutation, null_permutation(hpos.Count());
    const size_t perm_cnt = olx_factorial_t<size_t, size_t>(hpos.Count());
    TPSTypeList<double, size_t> hits;
    TArrayList<vec3d> crds(hpos.Count());
    for( size_t i=0; i < hpos.Count(); i++ )  {
      vec3d v = n2[hpos[i]].GetObject()->ccrd(); 
      if( Invert )  v*= -1;
      crds[i] = bestMatrix*(au.CellToCartesian(v) - bCent);
      null_permutation[i] = i;
    }
    for( size_t i=0; i < perm_cnt; i++ )  {
      permutation = null_permutation;
      if( i != 0 )  GeneratePermutation(permutation, i);
      double sqd = 0;
      for( size_t j=0; j < hpos.Count(); j++ )
        sqd += crds[permutation[j]].QDistanceTo(n1[hpos[j]].GetObject()->crd());
      hits.Add(sqd, i);
    }
    permutation = null_permutation;
    if( hits.GetObject(0) != 0 )
      GeneratePermutation(permutation, hits.GetObject(0));
    TPtrList<TEGraphNode<size_t,TSAtom*> > nodes(n2.GetNodes());
    for( size_t i=0; i < hpos.Count(); i++ )
      n2.GetNodes()[hpos[permutation[i]]] = nodes[hpos[i]];
  } 
};

//..............................................................................
bool TNetwork::DoMatch(TNetwork& net, TTypeList<AnAssociation2<size_t,size_t> >& res,
                       bool Invert, double (*weight_calculator)(const TSAtom&))
{
  if( NodeCount() != net.NodeCount() )  return false;
  TSAtom* thisSa = NULL;
  size_t maxbc = 0;
  double maxMw = 0;
  vec3d centb, centa;
  double centa_wght = 0, centb_wght = 0;
  const TAsymmUnit& au_b = net.GetLattice().GetAsymmUnit();
  size_t HCount = 0;
  for( size_t i=0; i < net.NodeCount(); i++ )  {
    vec3d v = net.Node(i).ccrd();
    if( Invert )  v *= -1;
    au_b.CellToCartesian(v);
    centb += v*weight_calculator(net.Node(i));
    centb_wght += weight_calculator(net.Node(i));
  }
  centb /= centb_wght;

  const TAsymmUnit& au_a = this->GetLattice().GetAsymmUnit();
  for( size_t i=0; i < NodeCount(); i++ )  {
    Node(i).SetTag(0);
    if( Node(i).NodeCount() > maxbc )  {
      thisSa = &Node(i);
      maxbc = Node(i).NodeCount();
      maxMw = Node(i).GetType().GetMr();
    }
    else if( Node(i).NodeCount() == maxbc )  {
      if( Node(i).GetType().GetMr() > maxMw )  {
        thisSa = &Node(i);
        maxMw = thisSa->GetType().GetMr();
      }
    }
    if( Node(i).GetType() == iHydrogenZ )
      HCount++;
    vec3d v = Node(i).ccrd();
    au_a.CellToCartesian(v);
    centa += v*weight_calculator(Node(i));
    centa_wght += weight_calculator(Node(i));
  }
  centa /= centa_wght;

  if( thisSa == NULL )  return false;
  TEGraph<size_t, TSAtom*> thisGraph(thisSa->GetType().z, thisSa);
//  TEGraph<int, TSAtom*> thisGraph( 0, thisSa );
  BuildGraph(thisGraph.GetRoot(), thisSa);
  TNetTraverser trav;
  trav.OnItem(thisGraph.GetRoot());
  thisGraph.GetRoot().Traverser.LevelTraverse(thisGraph.GetRoot(), trav);
  TBasicApp::GetLog().Info(trav.GetData());
  for( size_t i=0; i < net.NodeCount(); i++ )  {
    TSAtom& thatSa = net.Node(i);
    if( thisSa->NodeCount() != thatSa.NodeCount() )  continue;
    if( thisSa->GetType() != thatSa.GetType() )  continue;
    for( size_t j=0; j < net.NodeCount(); j++ )
      net.Node(j).SetTag(0);
    TEGraph<size_t, TSAtom*> thatGraph(thatSa.GetType().z, &thatSa);
    BuildGraph(thatGraph.GetRoot(), &thatSa);

    trav.ClearData();
    trav.OnItem(thatGraph.GetRoot());
    thatGraph.GetRoot().Traverser.LevelTraverse(thatGraph.GetRoot(), trav);
    TBasicApp::GetLog().Info(trav.GetData());
    if( thisGraph.GetRoot().DoMatch(thatGraph.GetRoot()) )  {  // match 
      GraphAnalyser ga(thisGraph.GetRoot(), thatGraph.GetRoot());
      ga.Invert = Invert;
      ga.atomsToMatch = NodeCount();
      ga.CalcRMSForH = ((NodeCount() - HCount) < 4); 
      try  {
        if( !thisGraph.GetRoot().FullMatchEx(thatGraph.GetRoot(), ga) ) // weird, roll back
          thisGraph.GetRoot().DoMatch(thatGraph.GetRoot());
      }
      catch(const TExceptionBase& e)  {
        TBasicApp::GetLog() << e.GetException()->GetError() << '\n';
        return false;
      }
      trav.ClearData();
      trav.OnItem( thatGraph.GetRoot() );
      thatGraph.GetRoot().Traverser.LevelTraverse(thatGraph.GetRoot(), trav);
      TBasicApp::GetLog().Info( trav.GetData() );
      TBasicApp::GetLog().Info( olxstr("Number of permutations: ") << ga.CallsCount );
      ResultCollector(thisGraph.GetRoot(), thatGraph.GetRoot(), res);
      return true;
    }
  }
  return false;
}
//..............................................................................
bool TNetwork::IsSubgraphOf( TNetwork& net, TTypeList< AnAssociation2<size_t, size_t> >& res,
                             const TSizeList& rootsToSkip )
{
  if( NodeCount() > net.NodeCount() )  return false;
  TSAtom* thisSa = NULL;
  size_t maxbc = 0;
  for( size_t i=0; i < NodeCount(); i++ )  {
    Node(i).SetTag(0);
    if( Node(i).NodeCount() > maxbc )  {
      thisSa = &Node(i);
      maxbc = Node(i).NodeCount();
    }
  }
  TEGraph<size_t, TSAtom*> thisGraph( thisSa->GetType().z, thisSa );
  BuildGraph( thisGraph.GetRoot(), thisSa );
  TIntList GraphId;
  for( size_t i=0; i < net.NodeCount(); i++ )  {
    TSAtom* thatSa = &net.Node(i);
    if( thisSa->NodeCount() > thatSa->NodeCount() )  continue;
    if( thisSa->GetType() != thatSa->GetType() )  continue;
    if( rootsToSkip.IndexOf(i) != InvalidIndex )
      continue;
    for( size_t j=0; j < net.NodeCount(); j++ )
      net.Node(j).SetTag(0);
    TEGraph<size_t, TSAtom*> thatGraph(thatSa->GetType().z, thatSa);
    BuildGraph(thatGraph.GetRoot(), thatSa);
    //continue;
    if( thisGraph.GetRoot().IsSubgraphOf(thatGraph.GetRoot()) )  {
      ResultCollector(thisGraph.GetRoot(), thatGraph.GetRoot(), res);
      return true;
    }
  }
  return false;
}
//..............................................................................
bool TNetwork::TryRing(TSAtom& sa, size_t node, TSAtomPList& ring, const ElementPList& ringContent)  {
  sa.SetTag(1);
  return TryRing(sa.Node(node), ring, ringContent, 2);
}
//..............................................................................
bool TNetwork::TryRing(TSAtom& sa, TSAtomPList& ring, const ElementPList& ringContent, size_t level)  {
  if( ringContent[level-1] != NULL && (sa.GetType() != *ringContent[level-1]) )
    return false;
  sa.SetTag(level);
  for( size_t i=0; i < sa.NodeCount(); i++ )  {
    TSAtom& a = sa.Node(i);
    if( a.IsDeleted() )  continue;
    if( level >= ringContent.Count() && a.GetTag() == 1 )
      return true;
    if( a.GetTag() != 0 && a.GetTag() < (index_t)level )  continue;
    if( level < ringContent.Count() && TryRing(a, ring, ringContent, level+1) ) {
      ring.Add(a);
      return true;
    }
  }
  sa.SetTag(0); // unroll the tags
  return false;
}
//..............................................................................
bool TNetwork::TryRing(TSAtom& sa, size_t node, TSAtomPList& ring)  {
  sa.SetTag(1);
  return TryRing(sa.Node(node), ring, 2);
}
//..............................................................................
bool TNetwork::TryRing(TSAtom& sa, TSAtomPList& ring, size_t level)  {
  if( level > 12 )  // safety precaution
    return false;
  sa.SetTag(level);
  for( size_t i=0; i < sa.NodeCount(); i++ )  {
    TSAtom& a = sa.Node(i);
    if( a.IsDeleted() || a.GetType() == iQPeakZ )  continue;
    if( a.GetTag() == 1 && level != 2 )
      return true;
    if( a.GetTag() == 0 )  {
      if( TryRing(a, ring, level+1) ) {
        ring.Add(a);
        return true;
      }
      else
        continue;
    }
    if( (a.GetTag()+1) == (index_t)level )  // previous?
      continue;
    else
      break;
  }
  sa.SetTag(0); // unroll the tags
  return false;
}
//..............................................................................
void TNetwork::UnifyRings(TTypeList<TSAtomPList>& rings)  {
  for( size_t i=0; i < rings.Count(); i++ )
    rings[i].QuickSorter.SortSF( rings[i], TNetwork_SortRingAtoms);
  // leave unique rings only
  for( size_t i=0; i < rings.Count(); i++ )  {
    if( rings.IsNull(i) )  continue;
    for( size_t j=i+1; j < rings.Count(); j++ )  {
      if( rings.IsNull(j) )  continue;
      bool found = true;
      for( size_t k=0; k < rings[i].Count(); k++ )  {
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
//..............................................................................
void TNetwork::FindRings(const ElementPList& ringContent, TTypeList<TSAtomPList>& res)  {
  if( ringContent.IsEmpty() )  return;
  TSAtomPList all;
  all.SetCapacity( NodeCount() );
  for( size_t i=0; i < NodeCount(); i++ )  {
    TSAtom& sa = Node(i);
    sa.SetTag(0);
    if( sa.IsDeleted() ) continue;
    if( sa.NodeCount() > 1 )  // a ring node must have at least two bonds!
      all.Add( &sa );
  }
  // we have to keep the order of the ring atoms, so need an extra 'rings' array
  TSAtomPList ring;
  TTypeList<TSAtomPList> rings;
  size_t resCount = res.Count();
  for( size_t i=0; i < all.Count(); i++ )  {
    if( all[i]->GetType() != *ringContent[0] )  continue;
    ring.Clear();
    for( size_t j=0; j < NodeCount(); j++ )
      Node(j).SetTag(0);
    ring.Add(all[i]);
    if( TryRing(*all[i], ring, ringContent) )  {
      res.AddCCopy(ring);
      rings.AddCCopy(ring);
    }
  }
  for( size_t i=0; i < NodeCount(); i++ )
    Node(i).SetTag(i);
  UnifyRings(rings);
  for( size_t i=0; i < rings.Count(); i++ )  {
    if( rings.IsNull(i) )
      res.NullItem(resCount + i);
  }
  res.Pack();
}
//..............................................................................
void TNetwork::FindAtomRings(TSAtom& ringAtom, TTypeList<TSAtomPList>& res)  {
  if( ringAtom.NodeCount() < 2 || &ringAtom.GetNetwork() != this )  return;
  TSAtomPList all;
  all.SetCapacity(NodeCount());
  for( size_t i=0; i < NodeCount(); i++ )  {
    TSAtom& a = Node(i);
    a.SetTag(0);
    if( a.IsDeleted() || a.NodeCount() < 2 || a.GetType() == iQPeakZ )  continue;
    all.Add(a);
  }
  // we have to keep the order of the ring atoms, so need an extra 'rings' array
  TSAtomPList ring;
  TTypeList<TSAtomPList> rings;
  size_t resCount = res.Count();
  for( size_t i=0; i < ringAtom.NodeCount(); i++ )  {
    TSAtom& a = ringAtom.Node(i);
    if( a.IsDeleted() || a.NodeCount() < 2 || a.GetType() == iQPeakZ )  continue;
    ring.Clear();
    for( size_t j=0; j < NodeCount(); j++ )
      Node(j).SetTag(0);
    if( TryRing(ringAtom, i, ring) )  {
      ring.Add(a);  // the ring is in reverse order
      ring.Add(ringAtom);
      res.AddCCopy(ring);
      rings.AddCCopy(ring);
    }
  }
  for( size_t i=0; i < NodeCount(); i++ )
    Node(i).SetTag(i);
  UnifyRings(rings);
  for( size_t i=0; i < rings.Count(); i++ )  {
    if( rings.IsNull(i) )
      res.NullItem(resCount + i);
  }
  res.Pack();
}
//..............................................................................
void TNetwork::FindAtomRings(TSAtom& ringAtom, const ElementPList& ringContent,
                             TTypeList<TSAtomPList>& res)  {
  if( ringAtom.NodeCount() < 2 || &ringAtom.GetNetwork() != this )  return;
  if( ringContent[0] != NULL && ringAtom.GetType() != *ringContent[0] )  return;
  TSAtomPList all;
  all.SetCapacity( NodeCount() );
  for( size_t i=0; i < NodeCount(); i++ )  {
    TSAtom& sa = Node(i);
    sa.SetTag(0);
    if( sa.IsDeleted() ) continue;
    if( sa.NodeCount() > 1 )  // a ring node must have at least two bonds!
      all.Add(sa);
  }
  // we have to keep the order of the ring atoms, so need an extra 'rings' array
  TSAtomPList ring;
  TTypeList<TSAtomPList> rings;
  size_t resCount = res.Count();
  for( size_t i=0; i < ringAtom.NodeCount(); i++ )  {
    TSAtom& a = ringAtom.Node(i);
    if( a.IsDeleted() || a.NodeCount() < 2 || 
       (ringContent[1] != NULL && a.GetType() != *ringContent[1]) )  continue;
    ring.Clear();
    for( size_t j=0; j < NodeCount(); j++ )
      Node(j).SetTag(0);
    if( TryRing(ringAtom, i, ring, ringContent) )  {
      ring.Add(a);  // the ring is in reverse order
      ring.Add(ringAtom);
      res.AddCCopy(ring);
      rings.AddCCopy(ring);
    }
  }
  for( size_t i=0; i < NodeCount(); i++ )
    Node(i).SetTag(i);
  UnifyRings(rings);
  for( size_t i=0; i < rings.Count(); i++ )  {
    if( rings.IsNull(i) )
      res.NullItem(resCount + i);
  }
  res.Pack();
}
//..............................................................................
TNetwork::RingInfo& TNetwork::AnalyseRing( const TSAtomPList& ring, TNetwork::RingInfo& ri )  {
  int pivot = -1, pivot_count = 0;
  double maxmw = 0;
  for( size_t i=0; i < ring.Count(); i++ )  {
    if( !ri.HasAfix && ring[i]->CAtom().GetAfix() != 0 )
      ri.HasAfix = true;
    TSAtomPList& al = ri.Substituents.AddNew();
   
    size_t nhc = 0, // not hydrogen atom count
      rnc = 0;   // of which belong to the ring
    double local_maxmw = 0;
    for( size_t j=0; j < ring[i]->NodeCount(); j++ )  {
      TSAtom& ra = ring[i]->Node(j);
      if( ra.IsDeleted() )  continue;
      const double mw = ra.GetType().GetMr();
      if( mw < 3 )  continue; // Q, H, D
      if( ra.crd().DistanceTo( ring[i]->crd() ) > 2 )  continue;  // skip M-E bonds
      if( mw > local_maxmw )  local_maxmw = mw;
      if( ring.IndexOf(&ra) != InvalidIndex )
        rnc++;
      else
        al.Add(&ra);
      nhc++;
    }
    if( nhc != rnc )  {
      if( local_maxmw > maxmw )  {
        ri.HeaviestSubsIndex = i;
        ri.HeaviestSubsType = &ring[i]->GetType();
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
  for( size_t i=0; i < ri.Substituted.Count(); i++ )  {
    for( size_t j=0;  j < ri.Ternary.Count(); j++ )  {
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
  for( size_t i=0; i < ring.Count(); i++ )
    cent += ring[i]->crd();
  cent /= ring.Count();
  double avAng = 2*M_PI/ring.Count(), avDis = 0;
  for( size_t i=0; i < ring.Count(); i++ )
    avDis += cent.DistanceTo( ring[i]->crd() );
  avDis /= ring.Count();
  for( size_t i=0; i < ring.Count(); i++ )  {
    double d = cent.DistanceTo( ring[i]->crd() );
    if( olx_abs(d-avDis) > 0.2 )  return false;
  }
  vec3d a, b;
  for( size_t i=0; i < ring.Count(); i++ )  {
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
    if( olx_abs(ca-avAng) > 5./M_PI )  return false;
  }
  return true;
}
//..............................................................................
// finds quaternions to map B to A
void TNetwork::FindAlignmentQuaternions(const TTypeList< AnAssociation2<vec3d,vec3d> >& crds, 
  const vec3d& centA, const vec3d& centB, ematd& quaternions, evecd& rms)  
{
  ematd evm(4,4);
  for( size_t i=0; i < crds.Count(); i++ )  {
    vec3d v = crds[i].GetA() - centA;
    const double 
      xm = v[0] - (crds[i].GetB()[0]-centB[0]),
      xp = v[0] + (crds[i].GetB()[0]-centB[0]),
      yp = v[1] + (crds[i].GetB()[1]-centB[1]),
      ym = v[1] - (crds[i].GetB()[1]-centB[1]),
      zm = v[2] - (crds[i].GetB()[2]-centB[2]),
      zp = v[2] + (crds[i].GetB()[2]-centB[2]);
    evm[0][0] += (xm*xm + ym*ym + zm*zm);
      evm[0][1] += (yp*zm - ym*zp);
      evm[0][2] += (xm*zp - xp*zm);
      evm[0][3] += (xp*ym - xm*yp);
    evm[1][0] = evm[0][1];
      evm[1][1] += (yp*yp + zp*zp + xm*xm);
      evm[1][2] += (xm*ym - xp*yp);
      evm[1][3] += (xm*zm - xp*zp);
    evm[2][0] = evm[0][2];
      evm[2][1] = evm[1][2];
      evm[2][2] += (xp*xp + zp*zp + ym*ym);
      evm[2][3] += (ym*zm - yp*zp);
    evm[3][0] = evm[0][3];
      evm[3][1] = evm[1][3];
      evm[3][2] = evm[2][3];
      evm[3][3] += (xp*xp + yp*yp + zm*zm);
  }
  ematd::EigenValues(evm, quaternions.Resize(4,4).I());
  rms.Resize(4);
  for( size_t i=0; i < 4; i++ )  {
    if( evm[i][i] < 0 )
      rms[i] = 0;
    else
      rms[i] = sqrt(evm[i][i]/crds.Count());
  }
  bool changes = true;
  while( changes )  {
    changes = false;
    for( size_t i=0; i < 3; i++ )  {
      if( rms[i+1] < rms[i] )  {
        quaternions.SwapRows(i, i+1);
        olx_swap(rms[i], rms[i+1]);
        changes = true;
      }
    }
  }
}
//..............................................................................
double TNetwork::FindAlignmentMatrix(const TTypeList<AnAssociation2<vec3d,vec3d> >& crds, 
  const vec3d& centA, const vec3d& centB, smatdd& res)  
{
  ematd quaternions(4,4);
  evecd rms(4);
  FindAlignmentQuaternions(crds, centA, centB, quaternions, rms);
  QuaternionToMatrix(quaternions[0], res.r);
  res.r.Transpose();
  return rms[0];
}
/* gradient descent shows that the procedure does converge and needs no refinement */
double TNetwork::FindAlignmentMatrix(const TTypeList< AnAssociation2<TSAtom*,TSAtom*> >& atoms,
                         smatdd& res, bool TryInversion, double (*weight_calculator)(const TSAtom&))
{
  vec3d centB;
  return TNetwork_FindAlignmentMatrix(atoms, res, centB, TryInversion, weight_calculator);
}
//..............................................................................
void TNetwork::PrepareESDCalc(const TTypeList< AnAssociation2<TSAtom*,TSAtom*> >& atoms, 
    bool Inverted,
    TSAtomPList& atoms_out,
    vec3d_alist& crd_out, 
    TDoubleList& wght_out,
    double (*weight_calculator)(const TSAtom&))
{
  atoms_out.SetCount(atoms.Count()*2);
  crd_out.SetCount(atoms.Count()*2);
  wght_out.SetCount(atoms.Count()*2);
  if( Inverted )  {
    const TAsymmUnit& au2 = atoms[0].GetB()->GetNetwork().GetLattice().GetAsymmUnit();
    for(size_t i=0; i < atoms.Count(); i++ )  {
      atoms_out[i] = atoms[i].A();
      atoms_out[atoms.Count()+i] = atoms[i].B();
      wght_out[i] = weight_calculator(*atoms[i].GetA());
      wght_out[atoms.Count()+i] = weight_calculator(*atoms[i].GetB());
      vec3d v = atoms[i].GetB()->ccrd() * -1;
      au2.CellToCartesian(v);
      crd_out[i] = atoms[i].GetA()->crd(); 
      crd_out[atoms.Count()+i] = v;
    }
  }
  else  {
    for(size_t i=0; i < atoms.Count(); i++ )  {
      atoms_out[i] = atoms[i].A();
      atoms_out[atoms.Count()+i] = atoms[i].B();
      wght_out[i] = weight_calculator(*atoms[i].GetA());
      wght_out[atoms.Count()+i] = weight_calculator(*atoms[i].GetB());
      crd_out[i] = atoms[i].GetA()->crd(); 
      crd_out[atoms.Count()+i] = atoms[i].GetB()->crd();
    }
  }
}
//..............................................................................
void TNetwork::DoAlignAtoms(const TTypeList< AnAssociation2<TSAtom*,TSAtom*> >& satomp,
                            const TSAtomPList& atomsToTransform, const smatdd& S, bool Inverted,
                            double (*weight_calculator)(const TSAtom&))  
{
  if( atomsToTransform.IsEmpty() )  return;
  vec3d mcent;
  const TAsymmUnit& au1 = atomsToTransform[0]->GetNetwork().GetLattice().GetAsymmUnit();
  const TAsymmUnit& au2 = satomp[0].GetB()->GetNetwork().GetLattice().GetAsymmUnit();
  double sum  = 0;
  if( Inverted )  {
    for( size_t i=0; i < atomsToTransform.Count(); i++ )
      au1.CellToCartesian(-atomsToTransform[i]->ccrd(), atomsToTransform[i]->crd());
    for( size_t i=0; i < satomp.Count(); i++ )  {
      vec3d v = -satomp[i].GetB()->ccrd();
      au2.CellToCartesian(v);
      mcent += v*weight_calculator(*satomp[i].GetB());
      sum += weight_calculator(*satomp[i].GetB());
    }
  }
  else  {
    for( size_t i=0; i < atomsToTransform.Count(); i++ )
      au1.CellToCartesian(atomsToTransform[i]->ccrd(), atomsToTransform[i]->crd());
    for( size_t i=0; i < satomp.Count(); i++ )  {
      mcent += satomp[i].GetB()->crd()*weight_calculator(*satomp[i].GetB());
      sum += weight_calculator(*satomp[i].GetB());
    }
  }
  mcent /= sum;

  TUnitCell& uc = atomsToTransform[0]->GetNetwork().GetLattice().GetUnitCell();
  for( size_t i=0; i < atomsToTransform.Count(); i++ )  {
    atomsToTransform[i]->crd() = S*(atomsToTransform[i]->crd() - mcent);
    if( atomsToTransform[i]->GetEllipsoid() != NULL )  {
      if( atomsToTransform[i]->GetEllipsoid()->GetTag() != 0 )  {
        TBasicApp::GetLog().Error(olxstr("Ellipsoid has already been rotated for: ") << atomsToTransform[i]->GetLabel());
        continue;
      }
      uc.GetEllp(atomsToTransform[i]->GetEllipsoid()->GetId());
      atomsToTransform[i]->GetEllipsoid()->SetTag(1);
      atomsToTransform[i]->GetEllipsoid()->MultMatrix(S.r);
    }
  }
}
//..............................................................................
bool TNetwork::RingInfo::IsSingleCSubstituted() const  {
  for( size_t i=0; i < Substituents.Count(); i++ )  {
    if( Substituents[i].Count() != 1 )  return false;
    TSAtom& sa = *Substituents[i][0];
    if( sa.GetType() != iCarbonZ )  return false;
    size_t nhc = 0;
    for( size_t j=0; j < sa.NodeCount(); j++ )  {
      TSAtom& ra = sa.Node(j);
      if( ra.GetType().GetMr() < 3 )  continue;  // H,D,Q
      nhc++;
    }
    if( nhc > 1 )  return false;  // only one to ring bond 
  }
  return true;
}
//..............................................................................
void TNetwork::ToDataItem(TDataItem& item) const {
  item.AddField("net_id", Network == NULL ? -1 : Network->GetTag());
  TDataItem& nodes = item.AddItem("Nodes");
  for( size_t i=0; i < Nodes.Count(); i++ )  {
    if( Nodes[i]->IsDeleted() )  continue;
    nodes.AddField("node_id", Nodes[i]->GetTag());
  }
  TDataItem& bonds = item.AddItem("Bonds");
  for( size_t i=0; i < Bonds.Count(); i++ )  {
    if( Bonds[i]->IsDeleted() )  continue;
    bonds.AddField("bond_id", Bonds[i]->GetTag());
  }
}
//..............................................................................
void TNetwork::FromDataItem(const TDataItem& item) {
  const int net_id = item.GetRequiredField("net_id").ToInt();
  Network = (net_id == -1 ? NULL : &Lattice->GetFragment(net_id));
  const TDataItem& nodes = item.FindRequiredItem("Nodes");
  Nodes.SetCapacity( nodes.FieldCount() );
  for( size_t i=0; i < nodes.FieldCount(); i++ )
    Nodes.Add(&Lattice->GetAtom(nodes.GetField(i).ToInt()));
  const TDataItem& bonds = item.FindRequiredItem("Bonds");
  Bonds.SetCapacity( bonds.FieldCount() );
  for( size_t i=0; i < bonds.FieldCount(); i++ )
    Bonds.Add(&Lattice->GetBond(bonds.GetField(i).ToInt()));
}
//..............................................................................
ContentList TNetwork::GetContentList() const {
  ElementDict elms;
  for( size_t i=0; i < NodeCount(); i++ )  {
    const TSAtom& a = Node(i);
    if( a.IsDeleted() || a.GetType() == iQPeakZ )  continue;
    size_t ind = elms.IndexOf(&a.GetType());
    if( ind == InvalidIndex )
      elms.Add(&a.GetType(), a.CAtom().GetOccu()*a.CAtom().GetDegeneracy());
    else
      elms.GetValue(ind) += (a.CAtom().GetOccu()*a.CAtom().GetDegeneracy());
  }
  ContentList rv;
  for( size_t i=0; i < elms.Count(); i++ )
    rv.AddNew(*elms.GetKey(i), elms.GetValue(i));
  return rv;
}
//..............................................................................
