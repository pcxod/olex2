/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

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
#include "threex3.h"
BeginXlibNamespace()

const short
  sotNone   = 0x0000,
  sotAtom   = 0x0001,
  sotBond   = 0x0002,
  sotHBond  = 0x0003,
  sotSCBond = 0x0004,
  sotTBond  = 0x0005,
  sotBBond  = 0x0006;

const float caDefIso = 0.05f;  // default atom isotropic parameter;

template <class Net> class TSObject: public virtual ACollectionItem {
protected:
  Net* Network;  // a pointer to parent Network
  short Type;  // object type: eg bond, atom, etc
  size_t OwnerId, FragmentId;
public:
  TSObject(Net* Parent) : Network(Parent), Type(sotNone), OwnerId(~0)  {}
  virtual ~TSObject()  {}
  void Assign(const TSObject& S)  {
    SetType(S.GetType());
    Network = &S.GetNetwork();
    SetTag(S.GetTag());
    SetOwnerId(S.GetOwnerId());
    SetFragmentId(S.GetFragmentId());
  }

  inline Net& GetNetwork() const {  return *Network;  }
  inline void SetNetwork(Net& n)  {  Network = &n;  }

  DefPropP(short, Type)
  // this must be updated by the owner container
  inline size_t GetOwnerId() const {  return OwnerId;  }
  inline size_t GetFragmentId() const {  return FragmentId;  }
  // for the owner usage
  inline void SetOwnerId(size_t v)  {  OwnerId = v;  }
  inline void SetFragmentId(size_t v)  {  FragmentId = v;  }
};
//---------------------------------------------------------------------------
// TBasicNode - encapsulate basic bond
//---------------------------------------------------------------------------
template <class Net, class Node>
class TBasicBond : public TSObject<Net>  {
protected:
  Node *FA, // first bond atom
       *FB; // second bond atom
  virtual void OnAtomSet() = 0;
public:
  TBasicBond(Net* P) : TSObject<Net>(P), FA(NULL), FB(NULL) {
    TSObject<Net>::Type = sotBBond;
  }
  virtual ~TBasicBond() {}
  bool IsValid() const {  return FA != NULL && FB != NULL;  }
  Node& A() const {  return *FA;  }
  void SetA(Node& a) {  FA = &a;  OnAtomSet();  }

  Node& B() const {  return *FB;  }
  void SetB(Node& a) {  FB = &a;  OnAtomSet();  }

  Node& Another(const Node& A) const {  return (&A == FA) ? *FB : *FA; }

  Node* GetShared(const TBasicBond &b) const {
    if (FA == b.FA || FA == b.FB)
      return FA;
    if (FB == b.FA || FB == b.FB)
      return FB;
    return NULL;
  }
  ConstPtrList<Node> GetDihedral(const TBasicBond &b) const {
    TPtrList<Node> rv;
    if (FA->IsConnectedTo(*b.FA))
      rv << FB << FA << b.FA << b.FB;
    else if (FA->IsConnectedTo(*b.FB))
      rv << FB << FA << b.FB << b.FA;
    else if (FB->IsConnectedTo(*b.FA))
      rv << FA << FB << b.FA << b.FB;
    else if (FB->IsConnectedTo(*b.FB))
      rv << FA << FB << b.FB << b.FA;
    return rv;
  }

  ConstPtrList<Node> GetAngle(const TBasicBond &b) const {
    TPtrList<Node> rv;
    if (FA == b.FA)
      rv << FB << FA << b.FB;
    else if (FA == b.FB)
      rv << FB << FA << b.FA;
    else if (FB == b.FA)
      rv << FA << FB << b.FB;
    else if (FB == b.FB)
      rv << FA << FB << b.FA;
    return rv;
  }
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
  TBasicNode(Net* N) : TSObject<Net>(N)  {}
  virtual ~TBasicNode()  {}

  void Assign(const TBasicNode& S)  {}

  void Clear()  {
    Nodes.Clear();
    Bonds.Clear();
  }

  inline size_t NodeCount() const {  return Nodes.Count(); }
  inline NodeType& Node(size_t i) const {  return *Nodes[i]; }
  const TPtrList<NodeType>& GetNodes() const {  return Nodes;  }
  inline NodeType& AddNode(NodeType& N)  {  return *Nodes.Add(N);  }
  inline bool IsConnectedTo(NodeType &N)  {
    return Nodes.IndexOf(N) != InvalidIndex;
  }
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
  const TPtrList<BondType>& GetBonds() const {  return Bonds;  }
  inline void NullBond(size_t i)  {  Bonds[i] = NULL;  }
  inline bool NullBond(const BondType& N) {
    const size_t ind = Bonds.IndexOf(N);
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


struct WBoxInfo  {
  vec3d r_from, r_to, // dimensions for given radii
        s_from, s_to, //dimensions using the r_sfil
        center,       // center of the box
        d;            // three values for the planes: dist = p.DotProd(normals[i]) - d[i]
  mat3d normals;      // face normals
};

EndXlibNamespace()

#endif
