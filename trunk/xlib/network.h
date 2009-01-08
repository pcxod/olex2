#ifndef networkH
#define networkH

#include "xbase.h"
#include "satom.h"
#include "sbond.h"
#include "tptrlist.h"

BeginXlibNamespace()

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

  inline TLattice& GetLattice()  const  {  return *Lattice;  }
  // empties the content of the network
  void Clear();
  // adds a node to the network and assigns its NetId
  void AddNode(TSAtom& N)  {  N.SetNetId(NodeCount());  Nodes.Add(&N);  }
  // adds a bond to the network and assigns its NetId
  void AddBond(TSBond& N)  {  N.SetNetId(BondCount());  Bonds.Add(&N);  }

  // copies the content of S to the net
  void Assign(TNetwork& S);

  // disassembles the list of Atoms into the fragments; 
  // does not affect current net
  void Disassemble(TSAtomPList& Atoms, TNetPList& Frags, TSBondPList* InterBonds);

  bool CBondExists(const class TCAtom& CA1, const TCAtom& CA2, double D) const;
  bool HBondExists(const TCAtom& CA1, const TCAtom& CA2, double D) const;

  // only pointers are compared!!
  inline bool operator == (const TNetwork& n) const  {  return this == &n;  }
  inline bool operator == (const TNetwork* n) const  {  return this == n;  }
  inline bool operator != (const TNetwork& n) const  {  return this != &n;  }
  inline bool operator != (const TNetwork* n) const  {  return this != n;  }

  // returns true if the ring is regular (distances from centroid and angles) 
  static bool IsRingRegular(const TSAtomPList& ring);

  bool DoMatch( TNetwork& net, TTypeList< AnAssociation2<int, int> >& res );
  bool IsSubgraphOf( TNetwork& net, TTypeList< AnAssociation2<int, int> >& res, const TIntList& rootsToSkip);

  void FindRings(const TPtrList<TBasicAtomInfo>& ringContent,
        TTypeList<TSAtomPList>& res);

  void FindAtomRings(TSAtom& ringAtom, const TPtrList<TBasicAtomInfo>& ringContent,
        TTypeList<TSAtomPList>& res);
  struct RingInfo  {
    int MaxSubsANode, HeaviestSubsIndex;
    TBasicAtomInfo* HeaviestSubsType;
    TIntList Ternary, // three bond inside the ring
      Substituted,    // more than two connections, two belong to the ring
      Alpha;          // susbtituted next to a ternary atom 
    TTypeList<TSAtomPList> Substituents;
    bool HasAfix;
    RingInfo() : HeaviestSubsType(NULL), MaxSubsANode(0), HeaviestSubsIndex(-1), HasAfix(false)  {  }
    RingInfo& Clear()  {
      MaxSubsANode = 0;
      HeaviestSubsIndex = -1;
      HasAfix = false;
      Ternary.Clear();
      Substituted.Clear();
      Alpha.Clear();
      Substituents.Clear();
      return *this;
    }
    bool IsSingleCSubstituted() const;  // returns true if all substituents are single CHn groups
  };
  static RingInfo& AnalyseRing( const TSAtomPList& ring, RingInfo& ri );

  /* quaternion method, Acta A45 (1989), 208
    This function finds the best match between atom pairs and returns the summ of
    distance deltas between corresponding atoms. If try inversion is specified,
    the function does the inversion of one of the atomic coordinates prior to
    the calculation
  */
  static double FindAlignmentMatrix(const TTypeList< AnAssociation2<TSAtom*,TSAtom*> >& atoms,
                  smatd& res, bool TryInversion);
  /* this fuction is used alonside the above one to allign the atoms using provided
   matrix. Also the Inverted has to be specified if the matric was calculated using
   the function above with the inverted flag on. The atomsToTransform are the atoms
   being transformed (typically, the atoms[n].GetA() is the subset of these atoms
  */
  static void DoAlignAtoms(const TTypeList< AnAssociation2<TSAtom*,TSAtom*> >& satomp,
                   const TSAtomPList& atomsToTransform, const smatd& S, bool Inverted);

  void ToDataItem(TDataItem& item) const;
  void FromDataItem(const TDataItem& item);

protected:
  class TDisassembleTaskRemoveSymmEq  {
    TSAtomPList& Atoms;
    double** Distances;
  public:
    TDisassembleTaskRemoveSymmEq(TSAtomPList& atoms, double** distances) :
      Atoms(atoms)  {  Distances = distances;  }

    void Run(long ind);
    TDisassembleTaskRemoveSymmEq* Replicate() const  {
      return new TDisassembleTaskRemoveSymmEq(Atoms, Distances);
    }
  };
  class TDisassembleTaskCheckConnectivity  {
    TSAtomPList& Atoms;
    double** Distances;
    double Delta;
  public:
    TDisassembleTaskCheckConnectivity(TSAtomPList& atoms, double** distances,
        double delta) :  Atoms(atoms)  {
      Distances = distances;
      Delta = delta;
    }
    void Run(long ind);
    TDisassembleTaskCheckConnectivity* Replicate() const  {
      return new TDisassembleTaskCheckConnectivity(Atoms, Distances, Delta);
    }
  };
  class THBondSearchTask  {
    TSAtomPList& Atoms;
    TSBondPList* Bonds;
    double** Distances;
    double Delta;
  public:
    THBondSearchTask(TSAtomPList& atoms, TSBondPList* bonds, double** distances,
        double delta) :  Atoms(atoms)  {
      Distances = distances;
      Delta = delta;
      Bonds = bonds;
    }
    void Run(long ind);
    THBondSearchTask* Replicate() const  {
      return new THBondSearchTask(Atoms, Bonds, Distances, Delta);
    }
  };

};


EndXlibNamespace()
#endif

