/******************************************************************************
* Copyright (c) 2004-2011 O. Dolomanov, OlexSys                               *
*                                                                             *
* This file is part of the OlexSys Development Framework.                     *
*                                                                             *
* This source file is distributed under the terms of the licence located in   *
* the root folder.                                                            *
******************************************************************************/

#ifndef __olx_xl_atomref_H
#define __olx_xl_atomref_H
#include "refmodel.h"
#include "residue.h"

BeginXlibNamespace()
/* Possible Shelx syntax
  C_*  - 'C' atom for all residues
  C_Res - 'C' of particular residue
  $C_Res - ? all carbons of particular residue
  $C_Res$Eqiv - all carbons of Res in Eqiv symmetry
  C_$1
  C_+, C_- - cannot be expanded, just references for previous and next residues
    when something is being calculated
  O21 > O25 -> [021..O25]
  O25 < O21 -> [025..O21]
*/
//TODO: add C? C?1 kind of expressions
class ASelectionOwner {
  bool DoClearSelection;
public:
  ASelectionOwner() : DoClearSelection(true) {}
  virtual void ExpandSelection(TCAtomGroup& catoms) = 0;
  virtual void ExpandSelectionEx(TPtrList<class TSAtom>& satoms) = 0;
  virtual ConstPtrList<TSObject<class TNetwork> > GetSelected() = 0;
  DefPropP(bool, DoClearSelection)
};

struct IAtomSelectionCriterium {
  virtual bool Match(const TCAtom& a) const = 0;
  virtual ~IAtomSelectionCriterium() {}
};

struct AtomSelectionNotCriterium : public IAtomSelectionCriterium {
  IAtomSelectionCriterium *c;
  AtomSelectionNotCriterium(IAtomSelectionCriterium* c) : c(c) {}
  ~AtomSelectionNotCriterium() {
    delete c;
  }
  virtual bool Match(const TCAtom& a) const {
    return !c->Match(a);
  }
};

struct AtomTypeSelectionCriterium : public IAtomSelectionCriterium {
  const cm_Element& e;
  AtomTypeSelectionCriterium(const cm_Element& e) : e(e) {}
  virtual bool Match(const TCAtom& a) const {
    return e.GetIndex() == a.GetType().GetIndex();
  }
};

struct AtomLabelSelectionCriterium : public IAtomSelectionCriterium {
  olxstr label;
  AtomLabelSelectionCriterium(const olxstr& l) : label(l) {}
  virtual bool Match(const TCAtom& a) const {
    return label.Equalsi(a.GetLabel());
  }
};

struct AtomZSelectionCriterium : public IAtomSelectionCriterium {
  const cm_Element& e;
  AtomZSelectionCriterium(const cm_Element& e) : e(e) {}
  virtual bool Match(const TCAtom& a) const {
    return e.z == a.GetType().z;
  }
};

struct AtomFlagSelectionCriterium : public IAtomSelectionCriterium {
  short flag;
  AtomFlagSelectionCriterium(short flag) : flag(flag) {}
  virtual bool Match(const TCAtom& a) const {
    return a.CheckFlags(flag);
  }
};

struct AtomPartSelectionCriterium : public IAtomSelectionCriterium {
  int part;
  AtomPartSelectionCriterium(int part) : part(part) {}
  virtual bool Match(const TCAtom& a) const {
    return a.GetPart() == part;
  }
};

struct AtomAFIXSelectionCriterium : public IAtomSelectionCriterium {
  int afix;
  AtomAFIXSelectionCriterium(int afix) : afix(afix) {}
  virtual bool Match(const TCAtom& a) const {
    return a.GetAfix() == afix;
  }
};

struct AtomSelectionLogicalCriterium : public IAtomSelectionCriterium {
  TPtrList<IAtomSelectionCriterium> criteria;
  ~AtomSelectionLogicalCriterium() {
    for (size_t i = 0; i < criteria.Count(); i++) {
      delete criteria[i];
    }
  }
};

struct AtomSelectionOrCriterium : public AtomSelectionLogicalCriterium {
  virtual bool Match(const TCAtom& a) const {
    for (size_t i = 0; i < criteria.Count(); i++) {
      if (criteria[i]->Match(a)) {
        return true;
      }
    }
    return false;
  }
};

struct AtomSelectionAndCriterium : public AtomSelectionLogicalCriterium {
  virtual bool Match(const TCAtom& a) const {
    for (size_t i = 0; i < criteria.Count(); i++) {
      if (!criteria[i]->Match(a)) {
        return false;
      }
    }
    return true;
  }
};

class TAtomReference : public IOlxObject {
  olxstr Expression;
protected:
  inline bool IsValidAtom(TCAtom* ca) {
    return !ca->IsDeleted() && ca->GetType().z > 1;
  }
  ASelectionOwner* SelectionOwner;
public:
  // if a NULL selectionOwner is given - the one of the TXApp is used
  TAtomReference(const olxstr& expression,
    ASelectionOwner* selectionOwner = 0);

  const olxstr& GetExpression() const { return Expression; }

  size_t _Expand(RefinementModel& rm, TCAtomGroup& atoms,
    TResidue* CurrResi);
  // prcesses shelx expressions and returns unprocessed expressions (like sel)
  olxstr Expand(RefinementModel& rm, TCAtomGroup& atoms, const olxstr& DefResi,
    size_t& atomAGroup);

  /* decodes M to all metals, X to halogens, extensions are possible... 
  */
  static ConstSortedElementPList ExpandAcronym(const olxstr& type,
    const TAsymmUnit& au);
  /*Returns a list of elements reffered like $C,O - all O and C or $*,C
  (alternatively: $*-C - all but C
  */
  static ConstSortedElementPList DecodeTypes(const olxstr& types,
    const TAsymmUnit& au);
  /* returns root criteria */
  static olx_object_ptr <IAtomSelectionCriterium>
    BuildCriteria(const olxstr& expr, const TAsymmUnit& au);
};

EndXlibNamespace()
#endif
