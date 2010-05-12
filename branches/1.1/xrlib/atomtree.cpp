#include "atomtree.h"

TCAtom& ATNode::Add(TCAtom& a)  {
  Nodes.Add( new TAtom(Root, this, a);
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
