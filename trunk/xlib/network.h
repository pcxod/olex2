#ifndef __olx_xl_network_H
#define __olx_xl_network_H
#include "math/align.h"
#include "satom.h"
#include "sbond.h"
#include "tptrlist.h"
#include "conninfo.h"

BeginXlibNamespace()

const int DefNoPart = -100;

typedef TTypeListExt<class TNetwork, IEObject> TNetList;
typedef TPtrList<class TNetwork> TNetPList;

class TNetwork: public TBasicNode<TNetwork, TSAtom, TSBond>  {
protected:
  class TLattice  *Lattice;
  // sorts atoms by diatnce from {0,0,0}, must be called before search for Hbonds
  void SortNodes();
public:
  TNetwork(TLattice* P, TNetwork *N);
  virtual ~TNetwork();

  inline TLattice& GetLattice() const {  return *Lattice;  }
  // empties the content of the network
  void Clear();
  // adds a node to the network and assigns its NetId
  void AddNode(TSAtom& N)  { Nodes.Add(N)->SetNetId(NodeCount());  }
  // adds a bond to the network and assigns its NetId
  void AddBond(TSBond& B)  {  Bonds.Add(B)->SetNetId(BondCount());  }

  // copies the content of S to the net
  void Assign(TNetwork& S);

  /* disassembles the list of Atoms into the fragments; does not affect current net */
  void Disassemble(TSAtomPList& Atoms, TNetPList& Frags, TSBondPList& InterBonds);
  /* creates bonds and fragments for atoms initialised by Disassemble, all Q-bonds end up 
  in the bond_sink*/
  void CreateBondsAndFragments(TSAtomPList& Atoms, TNetPList& Frags, TSBondPList& bond_sink);
  // returns true if the two atoms share a matrix
  static bool HaveSharedMatrix(const TSAtom& sa, const TSAtom& sb)  {
    for( size_t i=0; i < sa.MatrixCount(); i++ )  {
      for( size_t j=0; j < sb.MatrixCount(); j++ )  {
        if( sa.GetMatrix(i).GetId() == sb.GetMatrix(j).GetId() )  
          return true;
      }
    }
    return false;
  }
  static inline bool IsBondAllowed(const TSAtom& sa, const TSAtom& sb)  {
    if( (sa.CAtom().GetPart() < 0 || sb.CAtom().GetPart() < 0) && sa.CAtom().GetPart() == sb.CAtom().GetPart() )
      return HaveSharedMatrix(sa, sb);
    else if( sa.CAtom().GetPart() == 0 || sb.CAtom().GetPart() == 0 || 
             (sa.CAtom().GetPart() == sb.CAtom().GetPart()) )
      return true;
    return false;
  }
  static inline bool IsBondAllowed(const TSAtom& sa, const TCAtom& cb, const smatd& sm)  {
    if( (sa.CAtom().GetPart() < 0 || cb.GetPart() < 0) && sa.CAtom().GetPart() == cb.GetPart() )
      return sa.ContainsMatrix(sm.GetId());
    else if( sa.CAtom().GetPart() == 0 || cb.GetPart() == 0 || 
             (sa.CAtom().GetPart() == cb.GetPart()) )
      return true;
    return false;
  }

  static inline bool IsBondAllowed(const TCAtom& ca, const TCAtom& cb, const smatd& sm)  {
    if( (ca.GetPart() | cb.GetPart()) < 0 )
      return sm.IsFirst();  // is identity and no translation
    else if( ca.GetPart() == 0 || cb.GetPart() == 0 || 
             (ca.GetPart() == cb.GetPart()) )
      return true;
    return false;
  }

  bool CBondExists(const TSAtom& A1, const TSAtom& CA2, const double& D) const;
  // considers quadratic distance
  bool CBondExistsQ(const TSAtom& A1, const TSAtom& CA2, const double& qD) const;

  bool CBondExists(const TCAtom& CA1, const TCAtom& CA2, const smatd& sm, const double& D) const;
  // compares the quadratic distances
  bool CBondExistsQ(const TCAtom& CA1, const TCAtom& CA2, const smatd& sm, const double& qD) const;
  
  bool HBondExists(const TCAtom& CA1, const TCAtom& CA2, const smatd& sm, const double& D) const;
  // compares the quadratic distances
  bool HBondExistsQ(const TCAtom& CA1, const TCAtom& CA2, const smatd& sm, const double& qD) const;

  static bool BondExists(const TSAtom& a1, const TSAtom& a2, double D, double delta)  {
    if(  D < (a1.CAtom().GetConnInfo().r + a2.CAtom().GetConnInfo().r + delta) )
      return IsBondAllowed(a1, a2);
    return false;
  }
  static bool BondExistsQ(const TSAtom& a1, const TSAtom& a2, double qD, double delta)  {
    if(  qD < olx_sqr(a1.CAtom().GetConnInfo().r + a2.CAtom().GetConnInfo().r + delta) )
      return IsBondAllowed(a1, a2);
    return false;
  }
  static bool BondExists(const TSAtom& a1, const TCAtom& a2, const smatd& m, double D, double delta)  {
    if(  D < (a1.CAtom().GetConnInfo().r + a2.GetConnInfo().r + delta) )
      return IsBondAllowed(a1, a2, m);
    return false;
  }
  static bool BondExistsQ(const TSAtom& a1, const TCAtom& a2, const smatd& m, double qD, double delta)  {
    if(  qD < olx_sqr(a1.CAtom().GetConnInfo().r + a2.GetConnInfo().r + delta) )
      return IsBondAllowed(a1, a2, m);
    return false;
  }
  static bool BondExists(const TCAtom& a1, const TCAtom& a2, const smatd& m, double D, double delta)  {
    if( D < (a1.GetConnInfo().r + a2.GetConnInfo().r + delta) )
      return IsBondAllowed(a1, a2, m);
    return false;
  }
  static bool BondExistsQ(const TCAtom& a1, const TCAtom& a2, const smatd& m, double qD, double delta)  {
    if( qD < olx_sqr(a1.GetConnInfo().r + a2.GetConnInfo().r + delta) )
      return IsBondAllowed(a1, a2, m);
    return false;
  }


  // only pointers are compared!!
  inline bool operator == (const TNetwork& n) const {  return this == &n;  }
  inline bool operator == (const TNetwork* n) const {  return this == n;  }
  inline bool operator != (const TNetwork& n) const {  return this != &n;  }
  inline bool operator != (const TNetwork* n) const {  return this != n;  }

  // returns true if the ring is regular (distances from centroid and angles) 
  static bool IsRingRegular(const TSAtomPList& ring);
  // inverttion must be specified for the permutational graph match
  bool DoMatch(TNetwork& net, TTypeList< AnAssociation2<size_t, size_t> >& res, bool Invert,
    double (*weight_calculator)(const TSAtom&));
  bool IsSubgraphOf( TNetwork& net, TTypeList< AnAssociation2<size_t, size_t> >& res, const TSizeList& rootsToSkip);

protected:
  static int TNetwork_SortRingAtoms(const TSAtom* a, const TSAtom* b)  {
    return (int)(a->GetTag()-b->GetTag());
  }
  static bool TryRing(TSAtom& sa, TSAtomPList& ring, const ElementPList& ringContent, size_t level=1);
  static bool TryRing(TSAtom& sa, TSAtomPList& ring, size_t level=1);
// tries to find the ring in given direction
  static bool TryRing(TSAtom& sa, size_t node, TSAtomPList& ring, const ElementPList& ringContent);
  static bool TryRing(TSAtom& sa, size_t node, TSAtomPList& ring);
  void UnifyRings(TTypeList<TSAtomPList>& rings);

public:
  // finds only primitive rings
  void FindRings(const ElementPList& ringContent, TTypeList<TSAtomPList>& res);
  // returns a content of this fragment
  ContentList GetContentList() const;
  olxstr GetFormula() const;
  void FindAtomRings(TSAtom& ringAtom, const ElementPList& ringContent,
        TTypeList<TSAtomPList>& res);
  // finds all rings
  void FindAtomRings(TSAtom& ringAtom, TTypeList<TSAtomPList>& res);
  struct RingInfo  {
    size_t MaxSubsANode, HeaviestSubsIndex;
    const cm_Element* HeaviestSubsType;
    TSizeList Ternary, // three bond inside the ring
      Substituted,    // more than two connections, two belong to the ring
      Alpha;          // susbtituted next to a ternary atom 
    TTypeList<TSAtomPList> Substituents;
    bool HasAfix;
    RingInfo() : HeaviestSubsType(NULL), MaxSubsANode(0), HeaviestSubsIndex(InvalidIndex), HasAfix(false)  {  }
    RingInfo& Clear()  {
      MaxSubsANode = 0;
      HeaviestSubsIndex = InvalidIndex;
      HasAfix = false;
      Ternary.Clear();
      Substituted.Clear();
      Alpha.Clear();
      Substituents.Clear();
      return *this;
    }
    bool IsSingleCSubstituted() const;  // returns true if all substituents are single CHn groups
  };
  static RingInfo& AnalyseRing(const TSAtomPList& ring, RingInfo& ri);
  // returns true of the fragment has enough nodes to be matched/aligned to others
  bool IsSuitableForMatching() const {  return NodeCount() >= 3 && !Node(0).CAtom().IsDetached();  }
  struct AlignInfo  {
    align::out align_out;
    TEValueD rmsd;  // unweighted rmsd
    bool inverted;
  };
  static AlignInfo GetAlignmentRMSD(const TTypeList< AnAssociation2<TSAtom*,TSAtom*> >& atoms,
    bool invert,
    double (*weight_calculator)(const TSAtom&));
  // prepares a list of atoms, coordinates and weights for VcoV calculations
  static void PrepareESDCalc(const TTypeList<AnAssociation2<TSAtom*,TSAtom*> >& atoms, 
    bool TryInversion,
    TSAtomPList& atoms_out,
    vec3d_alist& crd_out, 
    TDoubleList& wght_out,
    double (*weight_calculator)(const TSAtom&));

  static void DoAlignAtoms(const TSAtomPList& atomsToTransform, const AlignInfo& align_info);
  
  static TArrayList<align::pair>& AtomsToPairs(
    const TTypeList<AnAssociation2<TSAtom*,TSAtom*> >& atoms,
    bool invert,
    double (*weight_calculator)(const TSAtom&),
    TArrayList<align::pair>& pairs);
  static align::out GetAlignmentInfo(
    const TTypeList<AnAssociation2<TSAtom*,TSAtom*> >& atoms,
    bool invert,
    double (*weight_calculator)(const TSAtom&));
  void ToDataItem(TDataItem& item) const;
  void FromDataItem(const TDataItem& item);

protected:
  class TDisassembleTaskRemoveSymmEq  {
    TSAtomPList& Atoms;
    double** Distances;
  public:
    TDisassembleTaskRemoveSymmEq(TSAtomPList& atoms, double** distances) :
      Atoms(atoms)  {  Distances = distances;  }

    void Run(size_t ind);
    TDisassembleTaskRemoveSymmEq* Replicate() const  {
      return new TDisassembleTaskRemoveSymmEq(Atoms, Distances);
    }
  };
  class TDisassembleTaskCheckConnectivity  {
    TSAtomPList& Atoms;
    double** Distances;
    double Delta;
  public:
    TDisassembleTaskCheckConnectivity(TSAtomPList& atoms, 
      double** distances, double delta) :  Atoms(atoms) 
    {
      Distances = distances;
      Delta = delta;
    }
    void Run(size_t ind);
    TDisassembleTaskCheckConnectivity* Replicate() const {
      return new TDisassembleTaskCheckConnectivity(Atoms, Distances, Delta);
    }
  };
  class THBondSearchTask  {
    TSAtomPList& Atoms;
    TSBondPList* Bonds;
    double** Distances;
    double Delta;
  public:
    THBondSearchTask(TSAtomPList& atoms, 
      TSBondPList* bonds, 
      double** distances, double delta) :  Atoms(atoms)  
    {
      Distances = distances;
      Delta = delta;
      Bonds = bonds;
    }
    void Run(size_t ind);
    THBondSearchTask* Replicate() const {
      return new THBondSearchTask(Atoms, Bonds, Distances, Delta);
    }
  };

};


EndXlibNamespace()
#endif

