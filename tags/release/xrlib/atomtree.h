#ifndef atom_tree_h
#define atom_tree_h
#include "catom.h"

// concrete class ids
const short antAfix = 0,
            antPart = 1,
            antSame = 2,
            antResi = 3,
            antAtom = 4;
BeginXlibNamespace()

class TTreeData  {
  TCAtomPList Atoms;
public:
  inline void SetAtomsCapacity(int c)     {  Atoms.SetCapacity(c);  }
  // atoms count
  int Count()                       const {  return Atoms.Count();  }
  TCAtom& operator [] (int i)             {  return Atoms[i];  } 
  const TCAtom& operator [] (int i) const {  return Atoms[i];  } 
};
// abstract atomt tree node, same group, RESI, Afix and Part group
class ATNode  {  
  TCAtomPList Atoms;
  TTypeList<ATNode> Nodes;
  ATNode* Parent, 
    &Root;  // root contains all atoms
protected:
  // called when an atom is added to the list
  virtual void OnAtomAdd(TCAtom& ca) {  return;  }
  // called when an atom is removed from the list
  virtual void OnAtomRemove(TCAtom& ca) {  return;  }
  // should create a shallow copy of the object
  virtual ATNode* SelfReplicate(ATNode& root) const = 0;
  ATNode& SetParent(ATNode* p)  {
    Parent = p;
    return *this;
  }
  //virtual void Assign(ATNode*
public:
  ATNode(ATNode& root, ATNode* parent) : Root(root), Parent(parent) {  }
  virtual ~ATNode()  {  Clear();  }
  // groups specific output
  virtual olxstr ToString() const = 0;
  void Clear()  {
    for( int i=0; i < Atoms.Count(); i++ ) 
      OnAtomRemove(*atoms[i]);
    Atoms.Clear();
  }
  bool IsEmpty() const {
    for( int i=0; i < Atoms.Count(); i++ )
      if( !Atoms[i]->IsDeleted() )  return false;
    return true;
  }
  
  void Assign(const ATNode& nd)  {
    Clear();
    for( int i=0; i < nd.Count(); i++ )
      Add( root[nd[i].GetId()] );
    for( int i=0; i < nd.NodeCount(); i++ )  {
      AddNode( *nd.GetNode(i).SelfReplicate(root) );
      Nodes.Last().Assign( nd.GetNode(i) );
    }
  }
  void Expand(TCAtomPList& out)  {
    for( int i=0; i < Atoms.Count(); i++ )  {
      if( !Atoms[i]->IsDeleted() )
        out.Add( Atoms[i] );
    }
    for( int i=0; i < Nodes.Count(); i++ )
      Nodes[i].Expand(out);
  }
  ATNode* GetParent()          const {  return Parent;  }
  
  // sets the capacity of nodes
  inline void SetNodesCapacity(int c) {  Atoms.SetCapacity(c);  }
  int NodeCount()               const {  return Nodes.Count();  }
  ATNode& GetNode(int i)              {  return *Nodes[i];  }
  const ATNode& GetNode(int i)  const {  return *Nodes[i];  }
  ATNode& AddNode(ATNode& nd)         {  return Nodes.Add(nd).SetParent(this);  } 
  ATNode& RemoveNode(ATNode& nd)      {
    int i = Nodes.IndexOf(&nd);
    if( i == 0 )
      throw TInvalidArgumentException(__OlxSourceInfo, "node is not in the group");
    Nodes.Delete(i);
    return nd.SetParent(NULL);
  }
  
  TCAtom& Add(TCAtom& a)  {
    Atoms.Add(&a);
    OnAtomAdd(a);
    return a;
  }
  TCAtom& Remove(TCAtom& a)  {
    int i = Atoms.IndexOf(&a);
    if( i == 0 )
      throw TInvalidArgumentException(__OlxSourceInfo, "atom is not in the group");
    OnAtomRemove(a);
    Atoms.Delete(i);
    return a;
  }
  // sets the capacity of atoms
  inline void SetAtomsCapacity(int c)     {  Atoms.SetCapacity(c);  }
  // atoms count
  int Count()                       const {  return Atoms.Count();  }
  TCAtom& operator [] (int i)             {  return Atoms[i];  } 
  const TCAtom& operator [] (int i) const {  return Atoms[i];  } 
};

class TAResidue : public ATNode  {
  olxstr ClassName, Alias;
  int Number;
protected:
  virtual void OnAtomRemove(TCAtom& ca)  {}
  virtual void OnAtomAdd(TCAtom& ca)  {}
  virtual ATNode* SelfReplicate(ATNode& root) const {
    return new TResidue(root, NULL, ClassName, Number, Alias);
  }
public:
  TResidue(ATNode& root, ATNode* parent, const olxstr& cl=EmptyString, int number = 0, const olxstr& alias=EmptyString) : 
      ATNode(root, parent), ClassName(cl), Number(number), Alias(alias) {  }
  DefPropC(olxstr, ClassName)
  DefPropC(olxstr, Alias)
  DefPropP(int, Number)
  virtual olxstr ToString() const {
    if( IsEmpty() || (Number == 0 && ClassName.IsEmpty() && Alias.IsEmpty()) )  return EmptyString;
    olxstr rv("RESI ");
    rv << ClassName;
    if( Number != 0 )  rv << ' ' << Number;
    return (rv << (Alias.IsEmpty() ? EmptyString : (olxstr(' ') << Alias)));
  }
};
class TAfix : public ATNode {
  double D, Sof, U;
  int Afix;
protected:
  virtual void OnAtomRemove(TCAtom& ca)  {}
  virtual void OnAtomAdd(TCAtom& ca)  {}
  virtual ATNode* SelfReplicate(ATNode& root) const {
    return new TAfix(root, NULL, D, Sof, U);
  }
public:
  TAfix(ATNode& root, ATNode* parent, int afix, double d = 0, double sof = 0, double u = 0 ) :
      ATNode(root, parent), Afix(afix), D(d), Sof(sof), U(u)  {  }
  DefPropP(double, D)
  DefPropP(double, Sof)
  DefPropP(double, U)
  DefPropP(int, Afix)
  virtual olxstr ToString() const {
    if( Afix == 0 || IsEmpty() )  return EmptyString;
    olxstr rv("AFIX ");
    rv << Afix;
    if( D != 0 )  {
      rv << ' ' << D;
      if( Sof != 0 )  {
        rv << ' ' << Sof;
        if( U != 0 )
          rv << ' ' << U;
      }
    }
    return rv;
  }
};
class TPart : public ATNode  {
  int Part;
  double Sof;
protected:
  virtual void OnAtomRemove(TCAtom& ca)  {}
  virtual void OnAtomAdd(TCAtom& ca)  {}
  virtual ATNode* SelfReplicate(ATNode& root) const {
    return new TPart(root, NULL, Part, Sof);
  }
public:
  TPart(ATNode& root, ATNode* parent, int part, double sof = 0 ) :
      ATNode(root, parent), Part(part), Sof(sof)  {  }
  DefPropP(int, Part)
  DefpropP(double, Sof)
  virtual olxstr ToString() const {
    if( Part == 0 || IsEmpty() )  return EmptyString;
    olxstr rv("PART ");
    rv << Part;
    if( Sof != 0 )
      rv << ' ' << Sof;
    return rv;
  }

};
class TAtom : public ATNode {
  TCAtom& Atom;
protected:
  virtual void OnAtomRemove(TCAtom& ca)  {}
  virtual void OnAtomAdd(TCAtom& ca)  {}
  virtual ATNode* SelfReplicate(ATNode& root) const {
    return new TAtom(root, NULL, &root[ Atom.GetId() ]);
  }
public:
  TAtom(ATNode& root, ATNode* parent, TCAtom& ca) :
      ATNode(root, parent), Atom(ca)  {  }
};

class TAtomTree {
  TResidue* Root;
  TTypeList<TAResidu> Residues;
  TTypeList<TParts> Parts;
  TTypeList<TAfix> Afixes;
  TTypeList<TSame> Sames;

public:
  TAtomTree(){}
};

EndXlibNamespace()

#endif
