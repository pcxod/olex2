/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_catom_list_H
#define __olx_catom_list_H
#include "residue.h"
#include "eset.h"
/*
Atom list to handle list of explicit (by label), implicit (last, first) and
expandable atom labels (C1_*, $C etc)
*/

BeginXlibNamespace()

class RefinementModel;
class ExplicitCAtomRef;

typedef TTypeListExt<ExplicitCAtomRef, IEObject> TAtomRefList_;

class IAtomRef : public IEObject {
public:
  virtual ~IAtomRef()  {}
  // returns either an atom label or a string of C1_tol kind or $C
  virtual olxstr GetExpression(TResidue *r=NULL) const = 0;
  virtual bool IsExplicit() const = 0;
  virtual size_t Expand(const RefinementModel& rm, TAtomRefList_& res,
    TResidue& resi) const = 0;
  virtual IAtomRef* Clone(RefinementModel& rm) const = 0;
  virtual void ToDataItem(TDataItem &di) const = 0;
  virtual bool IsValid() const { return true; }
  static IAtomRef &FromDataItem(const TDataItem &di, RefinementModel& rm);
};

// C1, C1_$2, C1_2 expressions handling
class ExplicitCAtomRef : public IAtomRef  {
  TCAtom *atom;
  const smatd* matrix;
  void DealWithSymm(const smatd* m);
public:
  ExplicitCAtomRef(const ExplicitCAtomRef& ar) :
    atom(ar.atom)
  {
    DealWithSymm(ar.matrix);
  }
  ExplicitCAtomRef(TCAtom& _atom, const smatd* _matrix=NULL) :
    atom(&_atom)
  {
    DealWithSymm(_matrix);
  }
  ExplicitCAtomRef(const TDataItem & di, RefinementModel& rm);
  ~ExplicitCAtomRef();
  virtual olxstr GetExpression(TResidue *) const;
  virtual bool IsExplicit() const {  return true;  }
  virtual bool IsValid() const;
  virtual size_t Expand(const RefinementModel &, TAtomRefList_& res,
    TResidue &) const
  {
    res.Add(new ExplicitCAtomRef(*this));
    return 1;
  }
  TCAtom& GetAtom() const {  return *atom;  }
  const smatd* GetMatrix() const {  return matrix;  }
  void UpdateMatrix(const smatd *m);
  vec3d GetCCrd() const {
    return matrix == 0 ? atom->ccrd() : *matrix *atom->ccrd();
  }
  // works correctly only if the atom Id is smaller than 32 bit
  uint64_t GetHash() const {
    return ((uint64_t)atom->GetId() << 32) |
      (uint64_t)(matrix == 0 ? 0 : matrix->GetId());
  }
  /* builds instance from C1 or C1_$1 expression for given residue, may return
  NULL
  */
  static ExplicitCAtomRef* NewInstance(const RefinementModel& rm,
    const olxstr& exp, TResidue* resi);
  virtual IAtomRef* Clone(RefinementModel& rm) const;
  virtual void ToDataItem(TDataItem &di) const;
  int Compare(const ExplicitCAtomRef &r) const;
  static const olxstr &GetTypeId() {
    static olxstr t = "explicit";
    return t;
  }
};
typedef TTypeListExt<ExplicitCAtomRef, IEObject> TAtomRefList;

/*
Last - last atom of a residue,
First - first atom of a residue,
* - all non H atoms of a residue
$Type - all Type atoms
$Type_tol - all Type atoms of a residue
C1_tol - all explicit atoms of a residue
C1_+ - an explicit atom of next residue
C1_- - an explicit atom of previous residue
*/
class ImplicitCAtomRef : public IAtomRef  {
  olxstr Name;
public:
  ImplicitCAtomRef(const ImplicitCAtomRef& ar) : Name(ar.Name) {}
  ImplicitCAtomRef(const olxstr& _Name) : Name(_Name) {}
  ImplicitCAtomRef(const TDataItem &di);
  // * is special char
  virtual olxstr GetExpression(TResidue *) const {
    return Name == '*' ? EmptyString() : Name;
  }
  virtual bool IsExplicit() const {  return false;  }
  virtual size_t Expand(const RefinementModel& rm, TAtomRefList_& res,
    TResidue& resi) const;
  // may return NULL
  static IAtomRef* NewInstance(const RefinementModel& rm, const olxstr& exp,
    const olxstr& resi, TResidue* _resi);
  virtual IAtomRef* Clone(RefinementModel& rm) const {
    return new ImplicitCAtomRef(Name);
  }
  virtual void ToDataItem(TDataItem &di) const;
  static const olxstr &GetTypeId() {
    static olxstr t = "implicit";
    return t;
  }
};

//manages C1 > C5 and C5 < C1 expressions
class ListIAtomRef : public IAtomRef {
  IAtomRef &start, &end;
  olxstr op;
public:
  ListIAtomRef(IAtomRef& _start, IAtomRef& _end, const olxstr& _op) :
    start(_start), end(_end), op(_op)
  {}
  ListIAtomRef(const TDataItem &di, RefinementModel& rm);
  virtual ~ListIAtomRef()  {
    delete &start;
    delete &end;
  }
  IAtomRef &GetStart() { return start;  }
  IAtomRef &GetEnd() { return end; }
  virtual bool IsExpandable() const { return true; }
  virtual bool IsExplicit() const {
    return start.IsExplicit() && end.IsExplicit();
  }
  // * is special char
  virtual olxstr GetExpression(TResidue *r) const {
    return olxstr(start.GetExpression(r) << ' ' << op << ' ' <<
      end.GetExpression(r));
  }
  virtual size_t Expand(const RefinementModel& rm, TAtomRefList_& res,
    TResidue& resi) const;
  virtual IAtomRef* Clone(RefinementModel& rm) const {
    return new ListIAtomRef(*start.Clone(rm), *end.Clone(rm), op);
  }
  virtual void ToDataItem(TDataItem &di) const;
  static const olxstr &GetTypeId() {
    static olxstr t = "list";
    return t;
  }
};

class AtomRefList  {
  TTypeList<IAtomRef> refs;
  RefinementModel& rm;
  olxstr residue;
  olxstr expression;
  bool Valid, ContainsImplicitAtoms;
  olxstr BuildExpression(TResidue *r) const {
    olxstr rv;
    for( size_t i=0; i < refs.Count(); i++ )  {
      rv << refs[i].GetExpression(r);
      if( (i+1) < refs.Count() )
        rv << ' ';
    }
    return rv;
  }
  void EnsureAtomGroups(const RefinementModel& rm, TAtomRefList& al,
    size_t groups_size) const;
  void EnsureAtomGroups(size_t group_size);
public:
  /* creates an instance of the object from given expression for given residue
  class, number or alias. Empty residue specifies the main residue.
  */
  AtomRefList(RefinementModel& rm, const olxstr& exp,
    const olxstr& resi=EmptyString());
  AtomRefList(RefinementModel& rm)
    : rm(rm), Valid(true), ContainsImplicitAtoms(false)
  {}
  /* expands the underlying expressions into a list. If the residue name is a
  class name (and there are several residues of the kind), there will be more
  than one entry in the res with each entry corresponding to any particular
  residue. One of the list type constants can be provided to validate the lists
  content to have pairs or triplets of atoms
  */
  TTypeList<TAtomRefList> &Expand(const RefinementModel& rm,
    TTypeList<TAtomRefList>& res, size_t group_size=InvalidSize) const;
  TTypeList<TAtomRefList>::const_list_type Expand(const RefinementModel& rm,
    size_t group_size=InvalidSize) const
  {
    TTypeList<TAtomRefList> res;
    return Expand(rm, res, group_size);
  }
  TAtomRefList::const_list_type ExpandList(const RefinementModel& rm,
    size_t group_size=InvalidSize) const;
  /* parses the expression into a list */
  void Build(const olxstr& exp,
    const olxstr& resi=EmptyString());
  /* recreates the expression for the object. If there are any explicit atom
  names - the new names will come from the updated model. Implicit atoms will
  stay as provided in the constructor
  */
  olxstr GetExpression() const;
  const olxstr &GetResi() const { return residue; }
  /* expands the list and returns if resulting explicit list is not empty */
  bool IsExpandable(const RefinementModel& rm,
    size_t group_size=InvalidSize) const;
  /* this can be used to decide if the atom list is valid */
  virtual bool IsExplicit() const {
    return (!ContainsImplicitAtoms && residue.IsEmpty());
  }
  void AddExplicit(TCAtom &a, const smatd *m=NULL) {
    refs.Add(new ExplicitCAtomRef(a, m));
  }
  void AddExplicit(class TSAtom &a);
  // checks if all atoms are in the same RESI
  void UpdateResi();
  void Clear() { refs.Clear(); }
  bool IsEmpty() const { return refs.IsEmpty(); }
  AtomRefList &Validate(size_t group_size = InvalidSize);
  void Assign(const AtomRefList &arl);
  void ToDataItem(TDataItem &di) const;
  void FromDataItem(const TDataItem &di);
  // returns all explicit refereces
  TPtrList<ExplicitCAtomRef>::const_list_type
    GetExplicit() const;
  void OnAUUpdate();
};

EndXlibNamespace()

#endif
