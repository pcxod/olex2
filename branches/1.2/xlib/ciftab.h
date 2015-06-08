/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_ciftab_H
#define __olx_xl_ciftab_H
#include "xbase.h"
#include "etable.h"
#include "catom.h"
BeginXlibNamespace()

// linked loop sort type
const short
  cCifTabSortLength = 0x0001,  // sorts by bond length
  cCifTabSortName   = 0x0002,  // sorts by atom name
  cCifTabSortMw     = 0x0004;  // sorts by atomic weight

struct CifTabAtom;
struct CifTabBond;
struct CifTabAngle;
class  TLinkedLoop;
class TCif;

struct CifTabAtom  {
  olxstr Label;
  TPtrList<CifTabBond> Bonds;
  TPtrList<CifTabAngle> Angles;
  TCAtom &CA;
  CifTabAtom(TCAtom& ca) : CA(ca)  {}
  bool operator == (const CifTabAtom& a) const {  return this == &a;  }
};
struct CifTabBond  {
  olxstr Value, S2;
  CifTabAtom &A1, &A2;
  CifTabBond(CifTabAtom& a1, CifTabAtom& a2) : A1(a1), A2(a2)  {}
  const CifTabAtom& Another(CifTabAtom& A) const;
  bool operator == (const CifTabBond &B) const;
};
struct CifTabAngle  {
  olxstr Value, S1, S3;
  CifTabAtom &A1, &A2, &A3;
  CifTabAngle(CifTabAtom& a1, CifTabAtom& a2, CifTabAtom& a3)
    : A1(a1), A2(a2), A3(a3) {}
  bool Contains(const CifTabAtom& A) const;
  bool FormedBy(const CifTabBond& B, const CifTabBond& B1) const;
};
//---------------------------------------------------------------------------
class TLLTBondSort  {
  int Compare_(const CifTabBond &I, const CifTabBond &I1) const;
public:
  CifTabAtom &Atom; // must be initilaised before the call
  const TStrList &Symmetry;
  short SortType;
  TLLTBondSort(CifTabAtom& atom, const TStrList& symm, short sort_type)
    : Atom(atom), Symmetry(symm), SortType(sort_type)  {}
  template <class item_a_t, class item_b_t>
  int Compare(const item_a_t &I, const item_b_t &I1) const {
    return Compare_(olx_ref::get(I), olx_ref::get(I1));
  }
};
//---------------------------------------------------------------------------
class TLinkedLoopTable: public IOlxObject  {
  TPtrList<CifTabAtom> FAtoms;
  TPtrList<CifTabBond> FBonds;
  TPtrList<CifTabAngle> FAngles;
  const TCif& FCif;
protected:
  CifTabAtom& AtomByName(const olxstr& Name);
  TTTable<TStrList> Table;
public:
  TLinkedLoopTable(const TCif& C);
  virtual ~TLinkedLoopTable();

  size_t AtomCount() const {  return FAtoms.Count(); }
  CifTabAtom *Atom(size_t index) {  return FAtoms[index];  }
  /* Returns a table constructed for Atom. The Atom should represent a valid
 atom label in Cif->AsymmUnit.
 */
  TTTable<TStrList>* MakeTable(const olxstr& Atom);
};

EndXlibNamespace()
#endif
