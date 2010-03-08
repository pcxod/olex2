//----------------------------------------------------------------------------//
// (c) Oleg V. Dolomanov, 2004
//----------------------------------------------------------------------------//
#ifndef __olx_xl_base_H
#define __olx_xl_base_H

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

const short
  sotNone   = 0x0000,
  sotAtom   = 0x0001,
  sotBond   = 0x0002,
  sotHBond  = 0x0003,
  sotSCBond = 0x0004,
  sotTBond  = 0x0005,
  sotBBond  = 0x0006;

const float
  dcMaxCBLength = 5.6, // maximum length of a covalent bond
  dcMaxHBLength = 6; // maximu length of a short interaction
const float caDefIso = 0.05;  // default atom isotropic parameter;

template <class Net> class TSObject: public ACollectionItem  {
protected:
  Net* Network;  // a pointer to parrent Network
  short   Type;    // object type: eg bond, atom, etc
  size_t     NetId;    // reference in network container
  size_t     LattId;    // reference in lattice container
public:
  TSObject(Net* Parent) : Network(Parent), Type(sotNone) {  }
  virtual ~TSObject() {  }
  void Assign(const TSObject& S) {
    SetType( S.GetType() );
    Network = &S.GetNetwork();
    SetTag( S.GetTag() );
    SetNetId( S.GetNetId() );
    SetLattId( S.GetLattId() );
  }

  inline Net& GetNetwork()  const  {  return *Network;  }
  inline void SetNetwork(Net& n)   {  Network = &n;  }

  DefPropP(size_t, NetId)
  DefPropP(size_t, LattId)
  DefPropP(short, Type)
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
  Node& A()    const {  return *FA;  }
  void SetA(Node& a) {  FA = &a;  OnAtomSet();  }

  Node& B()    const {  return *FB;  }
  void SetB(Node& a) {  FB = &a;  OnAtomSet();  }

  Node& Another(const Node& A) const {  return (&A == FA) ? *FB : *FA; }
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

  inline size_t NodeCount() const {  return Nodes.Count(); }
  inline NodeType& Node(size_t i) const {  return *Nodes[i]; }
  inline NodeType& AddNode(NodeType& N)  {  return *Nodes.Add(N);  }
  inline bool IsConnectedTo(NodeType &N)  {  return Nodes.IndexOf(N) != InvalidIndex;  }
  inline void NullNode(size_t i)  {  Nodes[i] = NULL; }
  inline bool NullNode(const NodeType& N)  {  
    size_t ind = Nodes.IndexOf(N);
    if( ind != InvalidIndex )  {
      Nodes[ind] = NULL; 
      return true;
    }
    return false;
  }
  inline void PackNodes()  {  Nodes.Pack();  }
  inline void ClearNodes()  {  Nodes.Clear();  }

  inline size_t BondCount() const {  return Bonds.Count(); }
  inline BondType& Bond(size_t i) const {  return *Bonds[i]; }
  inline BondType& AddBond(BondType& N)  {  return *Bonds.Add(N);  }
  inline void NullBond(size_t i)  {  Bonds[i] = NULL;  }
  inline bool NullBond(const BondType& N) {  
    size_t ind = Bonds.IndexOf(N);
    if( ind != InvalidIndex )  {
      Bonds[ind] = NULL; 
      return true;
    }
    return false;
  }
  inline void PackBonds()  {  Bonds.Pack();  } 
  inline void ClearBonds()  {  Bonds.Clear();  }

  inline void SetCapacity(size_t v)  {  
    Nodes.SetCapacity(v);  
    Bonds.SetCapacity(v);  
  }
};

EndXlibNamespace()

#endif

 
