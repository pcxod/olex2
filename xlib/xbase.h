//----------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#ifndef xbaseH
#define xbaseH
//---------------------------------------------------------------------------

#define BeginXlibNamespace()  namespace xlib {
#define EndXlibNamespace()  };\
  using namespace xlib;
#define UseXlibNamespace()  using namespace xlib;
#define GlobalXlibFunction( fun )     xlib::fun
#define XlibObject( obj )     xlib::obj

#include "ebase.h"
#include "tptrlist.h"
#include "dataitem.h"

BeginXlibNamespace()

const  short sotNone  = 0x0000,
             sotAtom = 0x0001,
             sotBond = 0x0002,
             sotHBond = 0x0003,
             sotSCBond = 0x0004,
             sotTBond = 0x0005,
             sotBBond = 0x0006;
// LoaderId values for CAtom             
const short  liCentroid = -1,
             liNewAtom  = -2;

extern const float dcMaxCBLength, // 3.5 maximum length of a covalent bond
                   dcMaxHBLength; // 4.5 maximu length of a short interaction
extern const float caDefIso;      // 0.05 default atom isotropic parameter;

template <class Net> class TSObject: public ACollectionItem  {
protected:
  Net* Network;  // a pointer to parrent Network
  short   Type;    // object type: eg bond, atom, centroid, etc
  int     NetId;    // reference in network container
  int     LatId;    // reference in lattice container
  short   NodId;    // reference in node container
public:
  TSObject(Net* Parent) : Network(Parent), Type(sotNone) {  }
  virtual ~TSObject() {  }
  void Assign(TSObject* S) {
    SetType( S->GetType() );
    Network = &S->GetNetwork();
    SetTag( S->GetTag() );
    SetNetId( S->GetNetId() );
    SetLatId( S->GetLatId() );
  }

  inline Net& GetNetwork()  const  {  return *Network;  }
  inline void SetNetwork(Net& n)   {  Network = &n;  }

  DefPropP(int, NetId)
  DefPropP(int, LatId)
  DefPropP(int, NodId)
  DefPropP(int, Type)
};
//---------------------------------------------------------------------------
// TBasicNode - encapsulate basic bond
//---------------------------------------------------------------------------
template <class Net, class Node>
class TBasicBond : public TSObject<Net>  {
protected:
  Node *FA,      // first bond atom
       *FB;    // second bond atom
  virtual void OnAtomSet() = 0;
public:
  TBasicBond(Net* P) : TSObject<Net>(P), FA(NULL), FB(NULL) {
    TSObject<Net>::Type = sotBBond;
  }
  virtual ~TBasicBond() {}
  const Node& GetA() const {  return *FA;  }
  Node& A()                {  return *FA;  }
  void SetA(Node& a)       {  FA = &a;  OnAtomSet();  }

  const Node& GetB() const {  return *FB;  }
  Node& B()                {  return *FB;  }
  void SetB(Node& a)       {  FB = &a;  OnAtomSet();  }

  Node& Another(Node& A) {  return (&A == FA) ? *FB : *FA; }
};
//---------------------------------------------------------------------------
// TBasicNode -  basic node
//---------------------------------------------------------------------------
template <class Net, class NodeType, class BondType>
class TBasicNode : public TSObject<Net>  {
protected:
  TPtrList<NodeType> Nodes;  // list of attached nodes
  TPtrList<BondType> Bonds;  // list of bonds. for quick referencing etc
public:
  TBasicNode(Net* N) : TSObject<Net>(N) {  }
  virtual ~TBasicNode()  {  }

  void Assign(const TBasicNode& S) {  }
  
  void Clear()  {
    Nodes.Clear();
    Bonds.Clear();
  }

  inline int NodeCount()            const {  return Nodes.Count(); }
  inline NodeType& Node(int i)            {  return *Nodes[i]; }
  inline void AddNode(NodeType& N)        {  N.SetNodId(Nodes.Count());  Nodes.Add(&N);  }
  inline bool IsConnectedTo(NodeType &N)  {  return Nodes.IndexOf(&N)!=-1;  }
  inline void NullNode(int i)             {  Nodes[i] = NULL; }
  inline void PackNodes()                 {  Nodes.Pack(); }

  inline int BondCount()            const {  return Bonds.Count(); }
  inline BondType& Bond(int i)            {  return *Bonds[i]; }
  inline void AddBond(BondType& N)        {  N.SetNodId(Bonds.Count());  Bonds.Add(&N);  }
  inline void PackBonds()                 {  Bonds.Pack(); };

  inline void SetCapacity(int v)          {  Nodes.SetCapacity(v);  Bonds.SetCapacity(v);  }
};


EndXlibNamespace()


#endif

 
